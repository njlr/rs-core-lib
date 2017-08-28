# Multidimensional Array #

By Ross Smith

* `#include "rs-core/grid.hpp"`

## Contents ##

[TOC]

## Class Grid ##

* `template <typename T, size_t N> class` **`Grid`**

The `Grid` class represents an `N`-dimensional array. The grid is indexed by a
fixed-size vector of integers (`Vector<ptrdiff_t, N>`). The element type (`T`)
can be any type that can be default constructed, copied, and moved.

* `class Grid::`**`iterator`**
    * `using iterator::`**`iterator_category`** `= std::forward_iterator_tag`
    * `using iterator::`**`value_type`** `= T`
    * `void iterator::`**`move`**`(size_t axis, ptrdiff_t delta) noexcept`
    * `Grid::index_type iterator::`**`pos`**`() const`
    * _[standard forward iterator features]_
* `class Grid::`**`const_iterator`**
    * `using const_iterator::`**`iterator_category`** `= std::forward_iterator_tag`
    * `using const_iterator::`**`value_type`** `= T`
    * `void const_iterator::`**`move`**`(size_t axis, ptrdiff_t delta) noexcept`
    * `Grid::index_type const_iterator::`**`pos`**`() const`
    * _[standard forward iterator features]_

Iterators over the grid's elements. If an iterator is simply incremented from
`begin()` to `end()` in the normal way, it will visit every element of the
grid; the order is unspecified except that it will start with the element at
the origin (all indices zero).

In addition to the normal iterator functions, grid iterators have two extra
functions, `move()` and `pos()`. The `move()` function accepts an index
indicating which axis to move on, and a delta to indicate how far to move;
behaviour is undefined if the axis index is out of range (`0<=axis<N`). Moving
an iterator outside the bounds of the grid is legal, and safe provided it is
not dereferenced or compared to any other iterator while outside the grid (and
the coordinate arithmetic involved does not overflow). An iterator outside the
grid can be moved back onto a valid element and then used normally.

The `pos()` function returns the iterator's current location in the grid, as a
coordinate vector. Behaviour is undefined if `pos()` is called on an out of
bounds iterator (including `end()`).

* `using Grid::`**`const_reference`** `= const T&`
* `using Grid::`**`difference_type`** `= ptrdiff_t`
* `using Grid::`**`index_type`** `= Vector<ptrdiff_t, N>`
* `using Grid::`**`reference`** `= T&`
* `using Grid::`**`size_type`** `= size_t`
* `using Grid::`**`value_type`** `= T`

Other member types.

* `static constexpr size_t Grid::`**`dim`** `= N`

Member constant.

* `Grid::`**`Grid`**`()`
* `explicit Grid::`**`Grid`**`(const index_type& shape)`
* `Grid::`**`Grid`**`(const index_type& shape, const T& t)`
* `template <typename... Args> explicit Grid::`**`Grid`**`(Args... shape)`
* `Grid::`**`Grid`**`(const Grid& g)`
* `Grid::`**`Grid`**`(Grid&& g) noexcept`
* `Grid::`**`~Grid`**`() noexcept`
* `Grid& Grid::`**`operator=`**`(const Grid& g)`
* `Grid& Grid::`**`operator=`**`(Grid&& g) noexcept`

Life cycle functions. The default constructor creates an empty grid (all
dimensions are zero). The next three constructors create a grid with a
specified shape and size; the dimensions can be supplied either as an
`index_type` vector or as an explicit list of integers. The third constructor
will set all elements to the given value; the other two will set all elements
to `T`'s default constructor. Dimensions of zero are allowed, but behaviour is
undefined if any dimension is negative, or if the total number of elements
would be out of range for a `ptrdiff_t`.

* `T& Grid::`**`operator[]`**`(const index_type& pos) noexcept`
* `const T& Grid::`**`operator[]`**`(const index_type& pos) const noexcept`
* `template <typename... Args> T& Grid::`**`operator`**`()(Args... pos) noexcept`
* `template <typename... Args> const T& Grid::`**`operator`**`()(Args... pos) const noexcept`
* `T& Grid::`**`at`**`(const index_type& pos)`
* `const T& Grid::`**`at`**`(const index_type& pos) const`
* `template <typename... Args> T& Grid::`**`at`**`(Args... pos)`
* `template <typename... Args> const T& Grid::`**`at`**`(Args... pos) const`

Element access functions. An element can be addressed either with an
`index_type` vector (in `operator[]` and the first two versions of `at()`), or
an explicit list of coordinates (in `operator()` and the last two versions of
`at()`). If any coordinate is out of bounds, behaviour is undefined for
`operator[]` and `operator()`; `at()` will throw `std::out_of_range`.

* `Grid::iterator Grid::`**`begin`**`() noexcept`
* `Grid::const_iterator Grid::`**`begin`**`() const noexcept`
* `Grid::iterator Grid::`**`end`**`() noexcept`
* `Grid::const_iterator Grid::`**`end`**`() const noexcept`

Standard container iterator functions.

* `void Grid::`**`clear`**`() noexcept`

Discards the grid's contents and sets all dimensions to zero. All iterators
over the grid are invalidated.

* `bool Grid::`**`contains`**`(const index_type& pos) const noexcept`
* `template <typename... Args> bool Grid::`**`contains`**`(Args... pos) const noexcept`

True if the specified position is inside the grid (i.e. if `pos[i]>=0` and
`pos[i]<shape()[i]` for all `i<N`).

* `bool Grid::`**`empty`**`() const noexcept`

True if the grid contains no elements (i.e. if any dimension is zero).

* `void Grid::`**`fill`**`(const T& t)`

Sets all elements to the given value.

* `Grid::iterator Grid::`**`locate`**`(const index_type& pos) noexcept`
* `Grid::const_iterator Grid::`**`locate`**`(const index_type& pos) const noexcept`
* `template <typename... Args> Grid::iterator Grid::`**`locate`**`(Args... pos) noexcept`
* `template <typename... Args> Grid::const_iterator Grid::`**`locate`**`(Args... pos) const noexcept`

These return an iterator pointing to the element at the given coordinates.
Behaviour is undefined if any coordinate is out of range.

* `void Grid::`**`reset`**`(const index_type& shape, const T& t = T())`
* `template <typename... Args> void Grid::`**`reset`**`(Args... shape)`

These replace the grid with a new one, constructed with the given dimensions
and element value. Behaviour is undefined if any dimension is negative, or if
the total number of elements would be out of range for a `ptrdiff_t`. These
are equivalent to assignment from a newly constructed grid. After calling
`reset()`, all iterators over the grid are invalidated.

* `void Grid::`**`reshape`**`(const index_type& shape, const T& t = T())`
* `template <typename... Args> void Grid::`**`reshape`**`(Args... shape)`

These change the grid's shape, as in `reset()`, but differ from `reset()` in
that any elements that the old and new grids have in common will retain their
value. The supplied element value (defaulting to `T`'s default constructor)
will only be used to fill in any newly created cells. After calling
`reshape()`, all iterators over the grid are invalidated.

* `Grid::index_type Grid::`**`shape`**`() const noexcept`

Returns the grid's dimensions, as supplied to the constructor or the last call
to `reset()` or `reshape()`.

* `size_t Grid::`**`size`**`() const noexcept`

Returns the total number of elements in the grid (the product of its
dimensions).

* `bool` **`operator==`**`(const Grid& lhs, const Grid& rhs) noexcept`
* `bool` **`operator!=`**`(const Grid& lhs, const Grid& rhs) noexcept`

Element-wise comparison operators.
