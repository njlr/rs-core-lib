#pragma once

#include "rs-core/common.hpp"
#include "rs-core/optional.hpp"
#include "rs-core/thread.hpp"
#include "rs-core/time.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <exception>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

namespace RS {

    // Forward declarations

    class BufferChannel;
    class Channel;
    class Dispatch;
    class EventChannel;
    class FalseChannel;
    template <typename T> class GeneratorChannel;
    class Interval;
    template <typename T> class MessageChannel;
    class Polled;
    template <typename T> class QueueChannel;
    class StreamChannel;
    class TimerChannel;
    class TrueChannel;
    template <typename T> class ValueChannel;

    // Time interval mixin class

    class Interval {
    public:
        using time = std::chrono::microseconds;
        time interval() const noexcept { return delta; }
        template <typename R, typename P> void set_interval(std::chrono::duration<R, P> t) noexcept
            { delta = std::max(std::chrono::duration_cast<time>(t), time()); }
    private:
        time delta;
    };

    // Channel base classes

    class Channel {
    public:
        enum state { closed = -1, waiting = 0, ready = 1 };
        virtual ~Channel() noexcept { drop(); }
        Channel(const Channel&) = delete;
        Channel(Channel&& c) noexcept { owner = std::exchange(c.owner, nullptr); }
        Channel& operator=(const Channel&) = delete;
        Channel& operator=(Channel&& c) noexcept;
        virtual void close() noexcept = 0;
        virtual state poll() { return do_wait({}); }
        virtual bool sync() const noexcept { return false; }
        template <typename R, typename P> state wait(std::chrono::duration<R, P> t)
            { return do_wait(std::chrono::duration_cast<Interval::time>(t)); }
    protected:
        virtual state do_wait(Interval::time t) = 0;
    private:
        friend class Dispatch;
        friend class EventChannel;
        template <typename T> friend class MessageChannel;
        friend class StreamChannel;
        Dispatch* owner = nullptr;
        Channel() = default;
        void drop() noexcept; // Defined after Dispatch
    };

        inline Channel& Channel::operator=(Channel&& c) noexcept {
            if (&c != this) {
                drop();
                owner = std::exchange(c.owner, nullptr);
            }
            return *this;
        }

    class EventChannel:
    public Channel {
    public:
        using callback = std::function<void()>;
    protected:
        EventChannel() = default;
    };

    template <typename T>
    class MessageChannel:
    public Channel {
    public:
        using callback = std::function<void(const T&)>;
        using value_type = T;
        virtual bool read(T& t) = 0;
        Optional<T> read_opt();
    protected:
        MessageChannel() = default;
    };

        template <typename T>
        Optional<T> MessageChannel<T>::read_opt() {
            T t;
            if (read(t))
                return std::move(t);
            else
                return {};
        }

    class StreamChannel:
    public Channel {
    public:
        using callback = std::function<void(std::string&)>;
        static constexpr size_t default_buffer = 16384;
        virtual size_t read(void* dst, size_t maxlen) = 0;
        size_t buffer() const noexcept { return bytes; }
        void set_buffer(size_t n) noexcept { bytes = n; }
        std::string read_all();
        std::string read_str();
        size_t read_to(std::string& dst);
    protected:
        StreamChannel() = default;
    private:
        size_t bytes = default_buffer;
    };

        inline std::string StreamChannel::read_all() {
            using namespace std::chrono;
            std::string s;
            auto cs = waiting;
            while (cs != closed) {
                cs = wait(seconds(1));
                if (cs == ready)
                    read_to(s);
            }
            return s;
        }

        inline std::string StreamChannel::read_str() {
            std::string s(bytes, '\0');
            size_t n = read(&s[0], bytes);
            s.resize(n);
            return s;
        }

        inline size_t StreamChannel::read_to(std::string& dst) {
            size_t pos = dst.size(), n = 0;
            ScopeExit guard([&] { dst.resize(pos + n); });
            dst.resize(pos + bytes);
            n = read(&dst[0] + pos, bytes);
            return n;
        }

    // Concrete channel classes

    class TrueChannel:
    public EventChannel {
    public:
        RS_NO_COPY_MOVE(TrueChannel);
        TrueChannel() = default;
        virtual void close() noexcept { open = false; }
    protected:
        virtual state do_wait(Interval::time /*t*/) { return open ? ready : closed; }
    private:
        std::atomic<bool> open{true};
    };

    class FalseChannel:
    public EventChannel {
    public:
        RS_NO_COPY_MOVE(FalseChannel);
        FalseChannel() = default;
        virtual void close() noexcept;
    protected:
        virtual state do_wait(Interval::time t);
    private:
        mutable Mutex mutex;
        ConditionVariable cv;
        bool open = true;
    };

        inline void FalseChannel::close() noexcept {
            auto lock = make_lock(mutex);
            open = false;
            cv.notify_all();
        }

        inline Channel::state FalseChannel::do_wait(Interval::time t) {
            auto lock = make_lock(mutex);
            if (! open)
                return closed;
            else if (t <= Interval::time())
                return waiting;
            cv.wait_for(lock, t, [&] { return ! open; });
            return open ? waiting : closed;
        }

    class TimerChannel:
    public EventChannel,
    public Interval {
    public:
        RS_NO_COPY_MOVE(TimerChannel);
        TimerChannel() { set_interval(Interval::time::max()); }
        template <typename R, typename P> explicit TimerChannel(std::chrono::duration<R, P> t) noexcept;
        virtual void close() noexcept;
        void flush() noexcept;
        auto next() const noexcept { return next_tick; }
    protected:
        virtual state do_wait(Interval::time t);
    private:
        using clock_type = ReliableClock;
        using time_point = clock_type::time_point;
        mutable Mutex mutex;
        ConditionVariable cv;
        time_point next_tick = time_point::max();
        bool open = true;
    };

        template <typename R, typename P>
        TimerChannel::TimerChannel(std::chrono::duration<R, P> t) noexcept {
            set_interval(t);
            next_tick = clock_type::now() + interval();
        }

        inline void TimerChannel::close() noexcept {
            auto lock = make_lock(mutex);
            open = false;
            cv.notify_all();
        }

        inline void TimerChannel::flush() noexcept {
            auto lock = make_lock(mutex);
            if (! open)
                return;
            auto now = clock_type::now();
            if (now < next_tick)
                return;
            auto skip = (now - next_tick).count() / interval().count();
            next_tick += interval() * (skip + 1);
        }

        inline Channel::state TimerChannel::do_wait(Interval::time t) {
            using namespace std::chrono;
            auto lock = make_lock(mutex);
            if (! open)
                return closed;
            auto now = clock_type::now();
            if (next_tick <= now) {
                next_tick += interval();
                return ready;
            }
            if (t <= Interval::time())
                return waiting;
            auto remaining = duration_cast<Interval::time>(next_tick - now);
            if (t < remaining) {
                cv.wait_for(lock, t, [&] { return ! open; });
                return open ? waiting : closed;
            } else {
                cv.wait_for(lock, remaining, [&] { return ! open; });
                if (! open)
                    return closed;
                next_tick += interval();
                return ready;
            }
        }

    template <typename T>
    class GeneratorChannel:
    public MessageChannel<T> {
    public:
        RS_NO_COPY_MOVE(GeneratorChannel);
        using generator = std::function<T()>;
        GeneratorChannel() = default;
        template <typename F> explicit GeneratorChannel(F f): mutex(), gen(f) {}
        virtual void close() noexcept;
        virtual bool read(T& t);
    protected:
        virtual Channel::state do_wait(Interval::time t);
    private:
        Mutex mutex;
        generator gen;
    };

        template <typename T>
        void GeneratorChannel<T>::close() noexcept {
            auto lock = make_lock(mutex);
            gen = nullptr;
        }

        template <typename T>
        bool GeneratorChannel<T>::read(T& t) {
            auto lock = make_lock(mutex);
            if (gen)
                t = gen();
            return bool(gen);
        }

        template <typename T>
        Channel::state GeneratorChannel<T>::do_wait(Interval::time /*t*/) {
            auto lock = make_lock(mutex);
            return gen ? Channel::ready : Channel::closed;
        }

    template <typename T>
    class QueueChannel:
    public MessageChannel<T> {
    public:
        RS_NO_COPY_MOVE(QueueChannel);
        QueueChannel() = default;
        virtual void close() noexcept;
        virtual bool read(T& t);
        void clear() noexcept;
        bool write(const T& t);
        bool write(T&& t);
    protected:
        virtual Channel::state do_wait(Interval::time t);
    private:
        Mutex mutex;
        ConditionVariable cv;
        bool open = true;
        std::deque<T> queue;
        Channel::state get_status() noexcept;
    };

        template <typename T>
        void QueueChannel<T>::close() noexcept {
            auto lock = make_lock(mutex);
            open = false;
            cv.notify_all();
        }

        template <typename T>
        bool QueueChannel<T>::read(T& t) {
            auto lock = make_lock(mutex);
            if (get_status() != Channel::ready)
                return false;
            t = queue.front();
            queue.pop_front();
            if (! queue.empty())
                cv.notify_all();
            return true;
        }

        template <typename T>
        void QueueChannel<T>::clear() noexcept {
            auto lock = make_lock(mutex);
            queue.clear();
        }

        template <typename T>
        bool QueueChannel<T>::write(const T& t) {
            auto lock = make_lock(mutex);
            if (! open)
                return false;
            queue.push_back(t);
            cv.notify_all();
            return true;
        }

        template <typename T>
        bool QueueChannel<T>::write(T&& t) {
            auto lock = make_lock(mutex);
            if (! open)
                return false;
            queue.push_back(std::move(t));
            cv.notify_all();
            return true;
        }

        template <typename T>
        Channel::state QueueChannel<T>::do_wait(Interval::time t) {
            auto lock = make_lock(mutex);
            if (get_status() == Channel::waiting && t > Interval::time())
                cv.wait_for(lock, t, [&] { return get_status() != Channel::waiting; });
            return get_status();
        }

        template <typename T>
        Channel::state QueueChannel<T>::get_status() noexcept {
            if (! open)
                return Channel::closed;
            else if (queue.empty())
                return Channel::waiting;
            else
                return Channel::ready;
        }

    template <typename T>
    class ValueChannel:
    public MessageChannel<T> {
    public:
        RS_NO_COPY_MOVE(ValueChannel);
        ValueChannel() = default;
        explicit ValueChannel(const T& t): value(t) {}
        virtual void close() noexcept;
        virtual bool read(T& t);
        void clear() noexcept;
        bool write(const T& t);
        bool write(T&& t);
    protected:
        virtual Channel::state do_wait(Interval::time t);
    private:
        Mutex mutex;
        ConditionVariable cv;
        T value;
        Channel::state st = Channel::waiting;
    };

        template <typename T>
        void ValueChannel<T>::close() noexcept {
            auto lock = make_lock(mutex);
            st = Channel::closed;
            cv.notify_all();
        }

        template <typename T>
        bool ValueChannel<T>::read(T& t) {
            auto lock = make_lock(mutex);
            if (st != Channel::ready)
                return false;
            t = value;
            st = Channel::waiting;
            return true;
        }

        template <typename T>
        bool ValueChannel<T>::write(const T& t) {
            auto lock = make_lock(mutex);
            if (st == Channel::closed)
                return false;
            if (t == value)
                return true;
            value = t;
            st = Channel::ready;
            cv.notify_all();
            return true;
        }

        template <typename T>
        bool ValueChannel<T>::write(T&& t) {
            auto lock = make_lock(mutex);
            if (st == Channel::closed)
                return false;
            if (t == value)
                return true;
            value = std::move(t);
            st = Channel::ready;
            cv.notify_all();
            return true;
        }

        template <typename T>
        Channel::state ValueChannel<T>::do_wait(Interval::time t) {
            auto lock = make_lock(mutex);
            if (st == Channel::waiting && t > Interval::time())
                cv.wait_for(lock, t, [&] { return st != Channel::waiting; });
            return st;
        }

    class BufferChannel:
    public StreamChannel {
    public:
        RS_NO_COPY_MOVE(BufferChannel);
        BufferChannel() = default;
        virtual void close() noexcept;
        virtual size_t read(void* dst, size_t maxlen);
        void clear() noexcept;
        bool write(const void* src, size_t len);
        bool write_str(const std::string& src) { return write(src.data(), src.size()); }
    protected:
        virtual state do_wait(Interval::time t);
    private:
        Mutex mutex;
        ConditionVariable cv;
        std::string buf;
        size_t ofs = 0;
        bool open = true;
    };

        inline void BufferChannel::close() noexcept {
            auto lock = make_lock(mutex);
            buf.clear();
            ofs = 0;
            open = false;
            cv.notify_all();
        }

        inline size_t BufferChannel::read(void* dst, size_t maxlen) {
            if (! dst || ! maxlen)
                return 0;
            auto lock = make_lock(mutex);
            if (! open)
                return 0;
            size_t n = std::min(buf.size() - ofs, maxlen);
            memcpy(dst, buf.data() + ofs, n);
            ofs += n;
            if (ofs == buf.size()) {
                buf.clear();
                ofs = 0;
            } else if (2 * ofs >= buf.size()) {
                buf.erase(0, ofs);
                ofs = 0;
            }
            return n;
        }

        inline bool BufferChannel::write(const void* src, size_t len) {
            auto lock = make_lock(mutex);
            if (! open)
                return false;
            buf.append(static_cast<const char*>(src), len);
            if (ofs < buf.size())
                cv.notify_all();
            return true;
        }

        inline void BufferChannel::clear() noexcept {
            auto lock = make_lock(mutex);
            buf.clear();
            ofs = 0;
        }

        inline Channel::state BufferChannel::do_wait(Interval::time t) {
            auto lock = make_lock(mutex);
            if (open && ofs == buf.size() && t > Interval::time())
                cv.wait_for(lock, t, [&] { return ! open || ofs < buf.size(); });
            if (! open)
                return closed;
            else if (ofs == buf.size())
                return waiting;
            else
                return ready;
        }

    // Polling mixin class

    class Polled:
    public Interval {
    public:
        static constexpr auto default_interval = std::chrono::milliseconds(10);
        virtual ~Polled() = default;
        virtual Channel::state poll() = 0;
    protected:
        Polled() { set_interval(default_interval); }
        Channel::state polled_wait(Interval::time t);
    };

        inline Channel::state Polled::polled_wait(Interval::time t) {
            using namespace std::chrono;
            if (t <= Interval::time())
                return poll();
            auto deadline = ReliableClock::now() + t;
            for (;;) {
                Channel::state s = poll();
                if (s != Channel::waiting)
                    return s;
                auto now = ReliableClock::now();
                if (now >= deadline)
                    return Channel::waiting;
                auto remaining = std::min(duration_cast<Interval::time>(deadline - now), interval());
                sleep_for(remaining);
            }
        }

    // Dispatch control class

    class Dispatch:
    public Interval {
    public:
        RS_NO_COPY_MOVE(Dispatch);
        enum mode_type { sync = 1, async };
        static constexpr auto default_interval = std::chrono::milliseconds(1);
        Dispatch() noexcept { set_interval(default_interval); }
        ~Dispatch() noexcept { stop(); }
        template <typename F> void add(EventChannel& chan, mode_type mode, F func);
        template <typename T, typename F> void add(MessageChannel<T>& chan, mode_type mode, F func);
        template <typename F> void add(StreamChannel& chan, mode_type mode, F func);
        void drop(Channel& chan) noexcept;
        bool empty() const noexcept { return tasks.empty(); }
        void run() noexcept;
        void stop() noexcept;
        Channel* last_channel() const noexcept { return channel_ptr; }
        std::exception_ptr last_error() const noexcept { return error_ptr; }
    private:
        using callback = std::function<void()>;
        struct task_info {
            mode_type mode;
            callback call;
            Thread thread;
            std::atomic<bool> done;
            std::exception_ptr error;
        };
        std::map<Channel*, task_info> tasks;
        Channel* channel_ptr = nullptr;
        std::exception_ptr error_ptr;
        void add_task(Channel& chan, mode_type mode, callback call);
        template <typename C, typename F> static typename C::callback make_callback(C& /*chan*/, const F& func) { return func; }
    };

        template <typename F>
        void Dispatch::add(EventChannel& chan, mode_type mode, F func) {
            auto handler = make_callback(chan, func);
            if (handler)
                add_task(chan, mode, handler);
        }

        template <typename T, typename F>
        void Dispatch::add(MessageChannel<T>& chan, mode_type mode, F func) {
            auto handler = make_callback(chan, func);
            if (! handler)
                return;
            auto call = [&chan,handler,t=T()] () mutable {
                if (chan.read(t))
                    handler(t);
            };
            add_task(chan, mode, call);
        }

        template <typename F>
        void Dispatch::add(StreamChannel& chan, mode_type mode, F func) {
            auto handler = make_callback(chan, func);
            if (! handler)
                return;
            auto call = [&chan,handler,s=std::string()] () mutable {
                if (chan.read_to(s))
                    handler(s);
            };
            add_task(chan, mode, call);
        }

        inline void Dispatch::drop(Channel& chan) noexcept {
            tasks.erase(&chan);
            chan.owner = nullptr;
        }

        inline void Dispatch::run() noexcept {
            channel_ptr = nullptr;
            error_ptr = nullptr;
            if (tasks.empty())
                return;
            ScopeExit guard([&] { drop(*channel_ptr); });
            for (;;) {
                int calls = 0;
                for (auto& t: tasks) {
                    channel_ptr = t.first;
                    if (t.second.mode == sync) {
                        try {
                            auto cs = t.first->poll();
                            if (cs == Channel::closed)
                                return;
                            if (cs == Channel::ready) {
                                t.second.call();
                                ++calls;
                            }
                        }
                        catch (...) {
                            error_ptr = std::current_exception();
                            return;
                        }
                    } else if (t.second.done) {
                        error_ptr = t.second.error;
                        return;
                    }
                }
                if (calls == 0)
                    sleep_for(interval());
                else
                    Thread::yield();
            }
        }

        inline void Dispatch::stop() noexcept {
            for (auto& t: tasks)
                t.first->close();
            while (! tasks.empty())
                run();
        }

        inline void Dispatch::add_task(Channel& chan, mode_type mode, callback call) {
            using namespace std::chrono;
            if (tasks.count(&chan))
                throw std::invalid_argument("Duplicate dispatch channel");
            if (mode != sync && mode != async)
                throw std::invalid_argument("Invalid dispatch mode");
            if (! call)
                throw std::invalid_argument("Dispatch callback is null");
            chan.owner = this;
            auto& task = tasks[&chan];
            task.mode = chan.sync() ? sync : mode;
            task.call = call;
            if (task.mode == async) {
                auto payload = [&] () noexcept {
                    try {
                        auto cs = Channel::waiting;
                        while (cs != Channel::closed) {
                            cs = chan.wait(seconds(1));
                            if (cs == Channel::ready)
                                task.call();
                        }
                    }
                    catch (...) {
                        task.error = std::current_exception();
                    }
                    task.done = true;
                };
                task.thread = Thread(payload);
            }
        }

        inline void Channel::drop() noexcept {
            if (owner) {
                owner->drop(*this);
                owner = nullptr;
            }
        }

}
