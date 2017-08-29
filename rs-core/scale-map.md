# Interpolating Associative Array #

By Ross Smith

* `#include "rs-core/scale-map.hpp"`

## Contents ##

[TOC]

## Class ScaleMap ##

* `template <typename X, typename Y = X> class` **`ScaleMap`**
    * `using ScaleMap::`**`key_type`** `= X`
    * `using ScaleMap::`**`mapped_type`** `= Y`
    * `ScaleMap::`**`ScaleMap`**`()`
    * `ScaleMap::`**`ScaleMap`**`(std::initializer_list<...> list)`
    * `ScaleMap::`**`ScaleMap`**`(const ScaleMap& m)`
    * `ScaleMap::`**`ScaleMap`**`(ScaleMap&& m)`
    * `ScaleMap::`**`~ScaleMap`**`()`
    * `ScaleMap& ScaleMap::`**`operator=`**`(const ScaleMap& m)`
    * `ScaleMap& ScaleMap::`**`operator=`**`(ScaleMap&& m)`
    * `Y ScaleMap::`**`operator()`**`(X x) const`
    * `void ScaleMap::`**`clear`**`() noexcept`
    * `bool ScaleMap::`**`empty`**`() const noexcept`
    * `void ScaleMap::`**`erase`**`(X x) noexcept`
    * `void ScaleMap::`**`erase`**`(X x1, X x2) noexcept`
    * `void ScaleMap::`**`insert`**`(X x, Y y)`
    * `void ScaleMap::`**`insert`**`(X x, Y yl, Y yr)`
    * `void ScaleMap::`**`insert`**`(X x, Y yl, Y y, Y yr)`
    * `X ScaleMap::`**`min`**`() const noexcept`
    * `X ScaleMap::`**`max`**`() const noexcept`
    * `template <typename T> void ScaleMap::`**`scale_x`**`(T a, X b = 0)`
    * `template <typename T> void ScaleMap::`**`scale_y`**`(T a, Y b = Y())`

A `ScaleMap` holds a list of `(x,y)` pairs, returning an interpolated `y`
value for arbitrary `x`. The input type, `X`, must be a floating point
arithmetic type; the output type, `Y`, is the same type as `X` by default, but
will work with any other floating point type, or any other type for which the
`interpolate()` function from the core library will work, or a similar
function that can be found by argument dependent lookup:

* `Y interpolate(X x1, Y y1, X x2, Y y2, X x)`

Legal output types include [`Vector` and `Matrix`](vector.html), if the scalar
type is floating point.

The constructor that takes an initializer list expects a list of braced
tuples, each containing one `X` value and one to three `Y` values, interpreted
in the same way as the `insert()` functions described below. If an `X` value
is repeated, later entries overwrite earlier ones. Example:

    ScaleMap<float> map = {
        {10, 100},            // equivalent to insert(10, 100)
        {20, 200, 300, 400},  // equivalent to insert(20, 200, 300, 400)
        {30, 500, 600},       // equivalent to insert(30, 500, 600)
    };

The function call operator returns the interpolated `y` value corresponding to
the given `x`. If the map is empty (no `(x,y)` pairs have been supplied since
default construction or the last call to `clear()`), a default constructed `Y`
will be returned. If the `x` value is out of range (less than the smallest
value supplied so far, or greater than the largest), the `y` value from the
nearest end of the range will be returned.

The first version of `insert()` adds a simple `(x,y)` pair (overwriting an
existing pair if there was one with the exact same value of `x`). The second
and third versions allow a stepwise change in the output; `yl` will be used as
the `y` for interpolation on the left of the given value, `yr` on the right.
Optionally a third value can be supplied to be used for this exact value of
`x`; otherwise, the average of the two `y` values will be used.

The first version of `erase()` deletes only an entry with this exact `x`
value, otherwise doing nothing; the second version erases all entries with `x`
values in the closed interval `[x1,x2]` (the arguments can be supplied in
either order).

The `min()` and `max()` functions return the range of `x` values for which a
mapping has been defined; they will return zero if the map is empty.

The `scale_x()` and `scale_y()` functions transform all of the `x` or `y`
values in the map by `ax+b` or `ay+b`; `T` must be an arithmetic type with a
suitable multiplication operator with `X` or `Y`.
