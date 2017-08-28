# Deterministic Stack #

By Ross Smith

* `#include "rs-core/stack.hpp"`

## Contents ##

[TOC]

## Class Stack ##

* `template <typename T> class` **`Stack`**
    * `using Stack::`**`const_iterator`** `= [random access iterator]`
    * `using Stack::`**`iterator`** `= [random access iterator]`
    * `using Stack::`**`value_type`** `= T`
    * `Stack::`**`Stack`**`()`
    * `Stack::`**`~Stack`**`() noexcept`
    * `Stack::`**`Stack`**`(Stack&& s)`
    * `Stack& Stack::`**`operator=`**`(Stack&& s)`
    * `iterator Stack::`**`begin`**`() noexcept`
    * `const_iterator Stack::`**`begin`**`() const noexcept`
    * `iterator Stack::`**`end`**`() noexcept`
    * `const_iterator Stack::`**`end`**`() const noexcept`
    * `void Stack::`**`clear`**`() noexcept`
    * `template <typename... Args> void Stack::`**`emplace`**`(Args&&... args)`
    * `bool Stack::`**`empty`**`() const noexcept`
    * `void Stack::`**`pop`**`() noexcept`
    * `void Stack::`**`push`**`(const T& t)`
    * `void Stack::`**`push`**`(T&& t)`
    * `size_t Stack::`**`size`**`() const noexcept`
    * `T& Stack::`**`top`**`() noexcept`
    * `const T& Stack::`**`top`**`() const noexcept`

A simple LIFO container, whose main function is to ensure that its elements
are destroyed in reverse order of insertion (this is not guaranteed by any
standard container, but is often useful for RAII). Behaviour is undefined if
`pop()` or `top()` is called on an empty stack.
