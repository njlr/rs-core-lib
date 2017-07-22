# Core Threads #

By Ross Smith

* `#include "rs-core/thread.hpp"`

This section of the library exists because some of the compilers I need to
support don't implement C++11 threads yet.

## Contents ##

[TOC]

## Thread class ##

* `class Thread`
    * `using Thread::`**`callback`** `= function<void()>`
    * `using Thread::`**`id_type`** `= ...`
    * `Thread::`**`Thread`**`() noexcept`
    * `explicit Thread::`**`Thread`**`(callback f)`
    * `Thread::`**`Thread`**`(Thread&& t)`
    * `Thread::`**`~Thread`**`() noexcept`
    * `Thread& Thread::`**`operator=`**`(Thread&& t)`
    * `Thread::id_type Thread::`**`get_id`**`() const noexcept`
    * `bool Thread::`**`poll`**`() noexcept`
    * `void Thread::`**`wait`**`()`
    * `size_t Thread::`**`cpu_threads`**`() noexcept`
    * `static Thread::id_type Thread::`**`current`**`() noexcept`
    * `static void Thread::`**`yield`**`() noexcept`

This is a wrapper for an operating system thread. The constructor accepts a
callback function, which will be called as the thread's payload. If a null
function is supplied, or the default constructor is used, no actual thread
will be created, and `wait()` will return immediately.

The `get_id()` function returns a thread identifier. This is guaranteed to be
unique only between threads running simultaneously; threads whose lifetimes do
not overlap may re-use the same ID (a thread's lifetime starts during the
constructor, and ends during the first call to `wait()` or the destructor). If
the thread was default constructed, the value of `get_id()` is unspecified.
The `current()` function returns the calling thread's ID.

The `poll()` function returns true if the thread's payload has completed
(meaning that the thread is either awaiting joining or already joined), false
if it is still running. A default constructed thread is considered to be
already completed.

The `wait()` function and the destructor wait for the thread to finish. Any
exceptions thrown by the callback function will be rethrown by `wait()`, or
silently discarded by the destructor. If the thread has already been
successfully waited on (or was default constructed), subsequent waits will
return immediately (and will not rethrow exceptions). Behaviour is undefined
if more than one thread is waiting on the same thread, or if a thread attempts
to wait on itself.

Apart from `wait()`, the other methods can be safely called simultaneously on
the same thread object from multiple other threads without synchronisation
precautions.

The constructor and the `wait()` function may throw `std::system_error` if
anything goes wrong in the underlying native thread API, and `wait()` may
rethrow as described above.

The `cpu_threads()` function returns the number of hardware threads available.

The `yield()` function causes the current thread to yield its scheduler time
slice.

## Synchronisation objects ##

* `class` **`Mutex`**
    * `Mutex::`**`Mutex`**`() noexcept`
    * `Mutex::`**`~Mutex`**`() noexcept`
    * `void Mutex::`**`lock`**`() noexcept`
    * `bool Mutex::`**`try_lock`**`() noexcept`
    * `void Mutex::`**`unlock`**`() noexcept`
* `class` **`MutexLock`**
    * `MutexLock::`**`MutexLock`**`() noexcept`
    * `explicit MutexLock::`**`MutexLock`**`(Mutex& m) noexcept`
    * `MutexLock::`**`~MutexLock`**`() noexcept`
    * `MutexLock::`**`MutexLock`**`(MutexLock&& lock) noexcept`
    * `MutexLock& MutexLock::`**`operator=`**`(MutexLock&& lock) noexcept`
* `MutexLock` **`make_lock`**`(Mutex& m) noexcept`

Mutex and exclusive lock. (A version of `make_lock()` that returns a
`std::unique_lock<std::mutex>` is also defined on systems that support it.)

* `class` **`ConditionVariable`**
    * `ConditionVariable::`**`ConditionVariable`**`()`
    * `ConditionVariable::`**`~ConditionVariable`**`() noexcept`
    * `void ConditionVariable::`**`notify_all`**`() noexcept`
    * `void ConditionVariable::`**`notify_one`**`() noexcept`
    * `void ConditionVariable::`**`wait`**`(MutexLock& lock)`
    * `template <typename Pred> void ConditionVariable::`**`wait`**`(MutexLock& lock, Pred p)`
    * `template <typename R, typename P, typename Pred> bool ConditionVariable::`**`wait_for`**`(MutexLock& lock, std::chrono::duration<R, P> t, Pred p)`
    * `template <typename C, typename D, typename Pred> bool ConditionVariable::`**`wait_until`**`(MutexLock& lock, std::chrono::time_point<C, D> t, Pred p)`

Condition variable.
