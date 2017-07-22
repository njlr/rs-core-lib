# Keyword Arguments #

By Ross Smith

* `#include "rs-core/kwargs.hpp"`

## Contents ##

[TOC]

## Keyword arguments ##

* `template <typename K> struct` **`Kwarg`**
    * `template <typename A> ... Kwarg::`**`operator=`**`(const A& arg) const`
* `template <typename K, typename V, typename... Args> bool` **`kwget`**`(const Kwarg<K>& key, V& var, const Args&... args)`

This provides a simple implementation of variadic keyword arguments for C++
functions.

Define a `Kwarg<K>` object for each keyword argument, where `K` is the
argument type. Functions that will take keyword arguments should be declared
with a variadic argument pack, possibly preceded by ordinary positional
arguments. The variadic arguments must be passed by reference, since the
global `Kwarg` objects are identified by address.

When calling the function, the keyword arguments should be supplied in the
form `key=value`, where `key` is a `Kwarg<K>` object, and `value` is the
argument value. The value type must be convertible to `K`. If `K` is `bool`,
the keyword alone can be passed as an argument, with the value defaulting to
`true`.

In the function body, call `kwget()` for each possible keyword argument, with
the corresponding `Kwarg<K>` object as the key, a reference to the variable
that will receive the value, and the variadic arguments from the enclosing
function. The variable passed by reference will be updated with the value
supplied with the key, or left unchanged if no corresponding keyword argument
was supplied. The return value indicates whether the variable was updated. If
the same key appears more than once in the actual argument list, it is
unspecified which value will be copied.

Example:

    constexpr Kwarg<int> win_width, win_height;
    constexpr Kwarg<string> win_title;
    constexpr Kwarg<bool> win_visible;

    class Window {
    public:
        template <typename... Args> explicit Window(const Args&... args) {
            int width = 640, height = 480;
            string title = "New Window";
            bool visible = false;
            kwget(win_width, width, args...);
            kwget(win_height, height, args...);
            kwget(win_title, title, args...);
            kwget(win_visible, visible, args...);
            // ...
        }
    };

    Window app_window(win_title = "Hello World", win_width = 1024, win_height = 768, win_visible);
