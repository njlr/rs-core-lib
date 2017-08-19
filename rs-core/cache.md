# Function Memoization #

By Ross Smith

* `#include "rs-core/cache.hpp"`

## Contents ##

[TOC]

## Function cache ##

* `template <typename> class` **`Cache`**
* `template <typename Result, typename... Args> class` **`Cache`**`<Result(Args...)>`
    * `using Cache::`**`argument_tuple`** `= tuple<Args...>`
    * `using Cache::`**`function_type`** `= function<Result(Args...)>`
    * `using Cache::`**`result_type`** `= Result`
    * `Cache::`**`Cache`**`()`
    * `template <typename F> explicit Cache::`**`Cache`**`(F f, size_t n = npos)`
    * `Cache::`**`Cache`**`(const Cache& c)`
    * `Cache::`**`Cache`**`(Cache&& c) noexcept`
    * `Cache::`**`~Cache`**`() noexcept`
    * `Cache& Cache::`**`operator=`**`(const Cache& c)`
    * `Cache& Cache::`**`operator=`**`(Cache&& c) noexcept`
    * `Result Cache::`**`operator()`**`(Args... args)`
    * `void Cache::`**`clear`**`() noexcept`
* `template <typename F> Cache<...>` **`memoize`**`(F f, size_t n = npos)`

A `Cache` function object wraps the original function, calling it
transparently when necessary, but saving the `n` most recently requested
results, returning cached results instead of calling the function again.
