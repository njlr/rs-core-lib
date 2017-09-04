#include "rs-core/channel.hpp"
#include "rs-core/optional.hpp"
#include "rs-core/string.hpp"
#include "rs-core/unit-test.hpp"
#include <chrono>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

using namespace RS;
using namespace std::chrono;
using namespace std::literals;

namespace {

    void check_true_channel() {

        TrueChannel chan;
        Channel::state cs = Channel::closed;

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);

    }

    void check_false_channel() {

        FalseChannel chan;
        Channel::state cs = Channel::closed;

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);
        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);

    }

    void check_generator_channel() {

        int i = 0, j = 0;
        GeneratorChannel<int> chan([&i] { return ++i; });
        Channel::state cs = Channel::closed;
        Optional<int> oi;

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(j));
        TEST_EQUAL(j, 1);

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(j));
        TEST_EQUAL(j, 2);

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(oi = chan.read_opt());
        TEST(oi);
        TEST_EQUAL(*oi, 3);

        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);
        TEST(! chan.read(j));
        TRY(oi = chan.read_opt());
        TEST(! oi);

    }

    void check_buffer_channel() {

        BufferChannel chan;
        Channel::state cs = Channel::closed;
        U8string s;
        size_t n = 0;

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TEST(chan.write_str("Hello"));
        TEST(chan.write_str("World"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(n = chan.read_to(s));
        TEST_EQUAL(n, 10);
        TEST_EQUAL(s, "HelloWorld");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        s.clear();
        TEST(chan.write_str("Hello"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(n = chan.read_to(s));
        TEST_EQUAL(n, 5);
        TEST_EQUAL(s, "Hello");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        s.clear();
        TEST(chan.write_str("Hello"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(n = chan.read_to(s));
        TEST_EQUAL(n, 5);
        TEST_EQUAL(s, "Hello");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);
        TEST(chan.write_str("World"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(n = chan.read_to(s));
        TEST_EQUAL(n, 5);
        TEST_EQUAL(s, "HelloWorld");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        s.clear();
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);
        TRY(n = chan.read_to(s));
        TEST_EQUAL(n, 0);
        TEST_EQUAL(s, "");

        s.clear();
        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);
        TRY(n = chan.read_to(s));
        TEST_EQUAL(n, 0);
        TEST_EQUAL(s, "");

    }

    void check_queue_channel() {

        QueueChannel<U8string> chan;
        Channel::state cs = Channel::closed;
        U8string s;

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.write("Hello"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(s));
        TEST_EQUAL(s, "Hello");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.write("Hello"));
        TRY(chan.write("World"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(s));
        TEST_EQUAL(s, "Hello");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(s));
        TEST_EQUAL(s, "World");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);

    }

    void check_value_channel() {

        ValueChannel<U8string> chan;
        Channel::state cs = Channel::closed;
        U8string s;

        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.write("Hello"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(s));
        TEST_EQUAL(s, "Hello");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.write("Hello"));
        TRY(chan.write("World"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(s));
        TEST_EQUAL(s, "World");
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.write("World"));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);

    }

    void check_timer_channel() {

        TimerChannel chan(25ms);
        Channel::state cs = Channel::closed;

        TRY(cs = chan.wait(1ms));
        TEST_EQUAL(cs, Channel::waiting);
        TRY(cs = chan.wait(100ms));
        TEST_EQUAL(cs, Channel::ready);

        TRY(sleep_for(150ms));
        TRY(cs = chan.wait(1ms));
        TEST_EQUAL(cs, Channel::ready);
        TRY(cs = chan.wait(1ms));
        TEST_EQUAL(cs, Channel::ready);

        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);

    }

    class TestPoll:
    public MessageChannel<int>,
    public Polled {
    public:
        virtual void close() noexcept {
            auto lock = make_lock(mutex);
            open = false;
        }
        virtual state poll() {
            auto lock = make_lock(mutex);
            if (! open)
                return closed;
            else if (queue.empty())
                return waiting;
            else
                return ready;
        }
        virtual bool read(int& t) {
            auto lock = make_lock(mutex);
            if (! open || queue.empty())
                return false;
            t = queue.front();
            queue.pop_front();
            return true;
        }
        void write(int n) {
            auto lock = make_lock(mutex);
            queue.push_back(n);
        }
    protected:
        virtual state do_wait(Interval::time t) {
            return polled_wait(t);
        }
    private:
        Mutex mutex;
        std::deque<int> queue;
        bool open = true;
    };

    void check_polled_channel() {

        TestPoll chan;
        Channel::state cs = Channel::closed;
        int i = 0;

        TRY(cs = chan.poll());
        TEST_EQUAL(cs, Channel::waiting);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.write(1));
        TRY(chan.write(2));
        TRY(chan.write(3));
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(i));
        TEST_EQUAL(i, 1);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(i));
        TEST_EQUAL(i, 2);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::ready);
        TEST(chan.read(i));
        TEST_EQUAL(i, 3);
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::waiting);

        TRY(chan.close());
        TRY(cs = chan.wait(10ms));
        TEST_EQUAL(cs, Channel::closed);

    }

    class TestException:
    public std::runtime_error {
    public:
        TestException(): std::runtime_error("Test exception") {}
    };

    void check_dispatcher() {

        static constexpr int cycles = 100;
        static constexpr auto time_interval = 1ms;

        auto cs = Channel::closed;

        {
            // Stopwatch w("Run with no channels");
            Dispatch disp;
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), nullptr);
            TEST(! disp.last_error());
        }

        {
            // Stopwatch w("Terminate by closing from callback (sync)");
            Dispatch disp;
            TimerChannel chan(time_interval);
            int n = 0;
            TRY(disp.add(chan, Dispatch::sync, [&] { ++n; if (n == cycles) chan.close(); }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST(! disp.last_error());
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
            TEST_EQUAL(n, cycles);
        }

        {
            // Stopwatch w("Terminate by closing from callback (async)");
            Dispatch disp;
            TimerChannel chan(time_interval);
            int n = 0;
            TRY(disp.add(chan, Dispatch::async, [&] { ++n; if (n == cycles) chan.close(); }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST(! disp.last_error());
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
            TEST_EQUAL(n, cycles);
        }

        {
            // Stopwatch w("Terminate by exception (sync)");
            Dispatch disp;
            TimerChannel chan(time_interval);
            int n = 0;
            TRY(disp.add(chan, Dispatch::sync, [&] { ++n; if (n == cycles) throw TestException(); }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST_THROW(rethrow(disp.last_error()), TestException);
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(n, cycles);
        }

        {
            // Stopwatch w("Terminate by exception (async)");
            Dispatch disp;
            TimerChannel chan(time_interval);
            int n = 0;
            TRY(disp.add(chan, Dispatch::async, [&] { ++n; if (n == cycles) throw TestException(); }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST_THROW(rethrow(disp.last_error()), TestException);
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(n, cycles);
        }

        {
            // Stopwatch w("Two sync channels, terminate one by exception");
            TimerChannel chan1(time_interval);
            TimerChannel chan2(time_interval);
            int n1 = 0, n2 = 0;
            {
                Dispatch disp;
                TRY(disp.add(chan1, Dispatch::sync, [&] { ++n1; if (n1 == cycles) throw TestException(); }));
                TRY(disp.add(chan2, Dispatch::sync, [&] { ++n2; }));
                TRY(disp.run());
                TEST_EQUAL(disp.last_channel(), &chan1);
                TEST_THROW(rethrow(disp.last_error()), TestException);
                TRY(cs = chan1.wait(1ms));
                TEST_EQUAL(n1, cycles);
            }
            TRY(cs = chan2.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
        }

        {
            // Stopwatch w("Two async channels, terminate one by exception");
            TimerChannel chan1(time_interval);
            TimerChannel chan2(time_interval);
            int n1 = 0, n2 = 0;
            {
                Dispatch disp;
                TRY(disp.add(chan1, Dispatch::async, [&] { ++n1; if (n1 == cycles) throw TestException(); }));
                TRY(disp.add(chan2, Dispatch::async, [&] { ++n2; }));
                TRY(disp.run());
                TEST_EQUAL(disp.last_channel(), &chan1);
                TEST_THROW(rethrow(disp.last_error()), TestException);
                TRY(cs = chan1.wait(1ms));
                TEST_EQUAL(n1, cycles);
            }
            TRY(cs = chan2.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
        }

        {
            // Stopwatch w("Two different channels, terminate sync by exception");
            TimerChannel chan1(time_interval);
            TimerChannel chan2(time_interval);
            int n1 = 0, n2 = 0;
            {
                Dispatch disp;
                TRY(disp.add(chan1, Dispatch::sync, [&] { ++n1; if (n1 == cycles) throw TestException(); }));
                TRY(disp.add(chan2, Dispatch::async, [&] { ++n2; }));
                TRY(disp.run());
                TEST_EQUAL(disp.last_channel(), &chan1);
                TEST_THROW(rethrow(disp.last_error()), TestException);
                TRY(cs = chan1.wait(1ms));
                TEST_EQUAL(n1, cycles);
            }
            TRY(cs = chan2.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
        }

        {
            // Stopwatch w("Two different channels, terminate async by exception");
            TimerChannel chan1(time_interval);
            TimerChannel chan2(time_interval);
            int n1 = 0, n2 = 0;
            {
                Dispatch disp;
                TRY(disp.add(chan1, Dispatch::sync, [&] { ++n1; }));
                TRY(disp.add(chan2, Dispatch::async, [&] { ++n2; if (n2 == cycles) throw TestException(); }));
                TRY(disp.run());
                TEST_EQUAL(disp.last_channel(), &chan2);
                TEST_THROW(rethrow(disp.last_error()), TestException);
                TRY(cs = chan2.wait(1ms));
                TEST_EQUAL(n2, cycles);
            }
            TRY(cs = chan1.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
        }

        {
            // Stopwatch w("Test with message channel (sync)");
            Dispatch disp;
            QueueChannel<int> chan;
            U8string s;
            std::vector<int> v;
            for (int i = 1; i <= 10; ++i)
                TRY(chan.write(i));
            TRY(disp.add(chan, Dispatch::sync, [&] (int i) {
                v.push_back(i);
                if (i >= 5)
                    chan.close();
            }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST(! disp.last_error());
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
            s = to_str(v);
            TEST_EQUAL(s, "[1,2,3,4,5]");
        }

        {
            // Stopwatch w("Test with message channel (async)");
            Dispatch disp;
            QueueChannel<int> chan;
            U8string s;
            std::vector<int> v;
            for (int i = 1; i <= 10; ++i)
                TRY(chan.write(i));
            TRY(disp.add(chan, Dispatch::async, [&] (int i) {
                v.push_back(i);
                if (i >= 5)
                    chan.close();
            }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST(! disp.last_error());
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
            s = to_str(v);
            TEST_EQUAL(s, "[1,2,3,4,5]");
        }

        {
            // Stopwatch w("Test with stream channel (sync)");
            Dispatch disp;
            BufferChannel chan;
            U8string s;
            TRY(chan.write_str("Hello world\n"));
            TRY(chan.set_buffer(5));
            TRY(disp.add(chan, Dispatch::sync, [&] (std::string& t) {
                s += t;
                t.clear();
                if (s.find('\n') != npos)
                    chan.close();
            }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST(! disp.last_error());
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
            TEST_EQUAL(s, "Hello world\n");
        }

        {
            // Stopwatch w("Test with stream channel (async)");
            Dispatch disp;
            BufferChannel chan;
            U8string s;
            TRY(chan.write_str("Hello world\n"));
            TRY(chan.set_buffer(5));
            TRY(disp.add(chan, Dispatch::async, [&] (std::string& t) {
                s += t;
                t.clear();
                if (s.find('\n') != npos)
                    chan.close();
            }));
            TRY(disp.run());
            TEST_EQUAL(disp.last_channel(), &chan);
            TEST(! disp.last_error());
            TRY(cs = chan.wait(1ms));
            TEST_EQUAL(cs, Channel::closed);
            TEST_EQUAL(s, "Hello world\n");
        }

    }

}

TEST_MODULE(core, channel) {

    check_true_channel();
    check_false_channel();
    check_generator_channel();
    check_queue_channel();
    check_value_channel();
    check_timer_channel();
    check_buffer_channel();
    check_polled_channel();
    check_dispatcher();

}
