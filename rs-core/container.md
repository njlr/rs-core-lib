# Core Containers #

By Ross Smith

* `#include "rs-core/container.hpp"`

## Contents ##

[TOC]

## ArrayMap - sequence based associative array ##

* `template <typename K, typename T> class` **`ArrayMap`**
    * `using ArrayMap::`**`const_iterator`** `= [random access iterator]`
    * `using ArrayMap::`**`iterator`** `= [random access iterator]`
    * `using ArrayMap::`**`key_type`** `= K`
    * `using ArrayMap::`**`mapped_type`** `= T`
    * `using ArrayMap::`**`value_type`** `= [K if T is void, otherwise std::pair<K, T>]`
    * `ArrayMap::`**`ArrayMap`**`() noexcept`
    * `ArrayMap::`**`ArrayMap`**`(std::initializer_list<value_type> list)`
    * `ArrayMap::`**`~ArrayMap`**`() noexcept`
    * `ArrayMap::`**`ArrayMap`**`(const ArrayMap& a)`
    * `ArrayMap::`**`ArrayMap`**`(ArrayMap&& a) noexcept`
    * `ArrayMap& ArrayMap::`**`operator=`**`(const ArrayMap& a)`
    * `ArrayMap& ArrayMap::`**`operator=`**`(ArrayMap&& a) noexcept`
    * `T& ArrayMap::`**`operator[]`**`(const K& k) [only defined if T is not void]`
    * `iterator ArrayMap::`**`begin`**`() noexcept`
    * `const_iterator ArrayMap::`**`begin`**`() const noexcept`
    * `iterator ArrayMap::`**`end`**`() noexcept`
    * `const_iterator ArrayMap::`**`end`**`() const noexcept`
    * `size_t ArrayMap::`**`capacity`**`() const noexcept`
    * `void ArrayMap::`**`clear`**`() noexcept`
    * `bool ArrayMap::`**`empty`**`() const noexcept`
    * `bool ArrayMap::`**`erase`**`(const K& k) noexcept`
    * `void ArrayMap::`**`erase`**`(const_iterator i) noexcept`
    * `iterator ArrayMap::`**`find`**`(const K& k) noexcept`
    * `const_iterator ArrayMap::`**`find`**`(const K& k) const noexcept`
    * `bool ArrayMap::`**`has`**`(const K& k) const noexcept`
    * `std::pair<iterator, bool> ArrayMap::`**`insert`**`(const value_type& v)`
    * `void ArrayMap::`**`reserve`**`(size_t n)`
    * `size_t ArrayMap::`**`size`**`() const noexcept`
* `template <typename K> using` **`ArraySet`** `= ArrayMap<K, void>`

An associative array that uses a simple sequential container internally,
finding keys by linear search (using equality comparison). This can be more
efficient than a tree or hash based container for small element counts; it can
also be useful when the element type has no natural ordering or hash function.
Member functions have their usual meaning for associative containers.

## Blob - binary large object ##

* `class` **`Blob`**
    * `Blob::`**`Blob`**`()`
    * `explicit Blob::`**`Blob`**`(size_t n)`
    * `Blob::`**`Blob`**`(size_t n, uint8_t x)`
    * `Blob::`**`Blob`**`(void* p, size_t n)`
    * `template <typename F> Blob::`**`Blob`**`(void* p, size_t n, F f)`
    * `Blob::`**`~Blob`**`() noexcept`
    * `Blob::`**`Blob`**`(const Blob& b)`
    * `Blob::`**`Blob`**`(Blob&& b) noexcept: ptr(b.ptr), len(b.len)`
    * `Blob& Blob::`**`operator=`**`(const Blob& b)`
    * `Blob& Blob::`**`operator=`**`(Blob&& b) noexcept`
    * `void* Blob::`**`data`**`() noexcept`
    * `const void* Blob::`**`data`**`() const noexcept`
    * `uint8_t* Blob::`**`bdata`**`() noexcept`
    * `const uint8_t* Blob::`**`bdata`**`() const noexcept`
    * `char* Blob::`**`cdata`**`() noexcept`
    * `const char* Blob::`**`cdata`**`() const noexcept`
    * `Irange<uint8_t*> Blob::`**`bytes`**`() noexcept`
    * `Irange<const uint8_t*> Blob::`**`bytes`**`() const noexcept`
    * `Irange<char*> Blob::`**`chars`**`() noexcept`
    * `Irange<const char*> Blob::`**`chars`**`() const noexcept`
    * `void Blob::`**`clear`**`() noexcept`
    * `void Blob::`**`copy`**`(const void* p, size_t n)`
    * `bool Blob::`**`empty`**`() const noexcept`
    * `void Blob::`**`fill`**`(uint8_t x) noexcept`
    * `size_t Blob::`**`hash`**`() const noexcept`
    * `U8string Blob::`**`hex`**`(size_t block = 0) const`
    * `void Blob::`**`reset`**`(size_t n)`
    * `void Blob::`**`reset`**`(size_t n, uint8_t x)`
    * `void Blob::`**`reset`**`(void* p, size_t n)`
    * `template <typename F> void Blob::`**`reset`**`(void* p, size_t n, F f)`
    * `size_t Blob::`**`size`**`() const noexcept`
    * `string Blob::`**`str`**`() const`
    * `void Blob::`**`swap`**`(Blob& b) noexcept`
* `bool` **`operator==`**`(const Blob& lhs, const Blob& rhs) noexcept`
* `bool` **`operator!=`**`(const Blob& lhs, const Blob& rhs) noexcept`
* `bool` **`operator<`**`(const Blob& lhs, const Blob& rhs) noexcept`
* `bool` **`operator>`**`(const Blob& lhs, const Blob& rhs) noexcept`
* `bool` **`operator<=`**`(const Blob& lhs, const Blob& rhs) noexcept`
* `bool` **`operator>=`**`(const Blob& lhs, const Blob& rhs) noexcept`
* `void` **`swap`**`(Blob& sb1, Blob& sb2) noexcept`
* `class std::`**`hash`**`<Blob>`

A simple wrapper for a block of memory.

The constructor and `reset()` functions that take a length, and optionally a
byte value, allocate memory using `malloc()`. The versions that take only a
length do not initialize the newly allocated memory; the ones that take a
length and value will fill the entire blob with the value. The versions that
take a pointer, size, and optional deallocation function take ownership of the
referenced data, and will deallocate it when it is discarded. If no
deallocation function is supplied, the memory is assumed to have been acquired
with `malloc()`, and will be released with `free()`. When a blob is copied,
the memory for the new copy is always allocated using `malloc()`, regardless
of how the source blob was allocated.

Unlike a `std::vector`, a `Blob` always allocates exactly the required amount
of memory. Any operation that changes the blob's size will reallocate it and
invalidate all pointers into the old blob.

For all functions that take a pointer and length, if a null pointer is passed,
the length is ignored and the effect is the same as passing a valid pointer
and zero length.

The `[bc]data()` functions return pointers to the beginning of the blob. The
`bytes()` and `chars()` functions return a pair of pointers marking the
beginning and end of the blob.

The `copy()` function reallocates the blob to the required size and copies the
referenced data.

The `hex()` function returns the blob's data in hex form; if a nonzero block
size is supplied, a line feed will be inserted after each block.

The `str()` function copies the entire blob into a string.

The comparison operators perform bytewise comparison by calling `memcmp()`.

## Grid - multidimensional array ##

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

## IndexTable - associative array with multiple indexing ##

The `IndexTable` and `Index` classes provide a collection over which an
arbitrary set of associative indexes can be created.

`IndexTable` is the actual data collection. This does not in itself provide
any associative semantics; it simply acts as a collection over which an index
can be built. The second class, `Index`, maintains an index over a table. The
index is generated when the `Index` object is constructed, and automatically
kept up to date as elements are inserted or erased from the underlying table.

An index may be unique keyed (no two keys may be equivalent, according to the
comparison function) or duplicate keyed (equivalent keys are allowed). This
choice is controlled by a template argument to the `Index` class. An attempt
to create a duplicate key in a unique index will throw an exception. A single
`IndexTable` can have separate unique and non-unique indexes at the same time.

The only requirement on the data type (`T`) and the key type (`K`) is that
they must be copyable. Functions must be provided to extract a key from a data
value, and to compare keys; these default to `static_cast<K>(T)` and
`std::less<K>` respectively.

All `IndexTable` and `Index` iterators are bidirectional iterators, with `T`
as their value type. Inserting an element does not invalidate any iterators;
erasing an element only invalidates any iterators that were pointing to that
element. Although the non-`const` iterators over an `IndexTable` or `Index`
provide a reference through which data elements may be updated, behaviour is
undefined if any operation on an element (other than insertion or deletion)
would change the key associated with it in any currently existing index.

In the complexity specifications, `n` is the total number of elements in the
table, and `k` is the number involved in a single operation (e.g. the number
inserted or erased).

### Supporting definitions ###

* `class` **`IndexCollision`**`: public std::runtime_error`

An `IndexCollision` exception is thrown if a duplicate key is inserted into a
unique index.

* `enum class` **`IndexMode`**
    * `IndexMode::`**`unique`**
    * `IndexMode::`**`duplicate`**

Flags used in the `Index` class template to indicate whether duplicates are
allowed.

### Index table class ###

* `template <typename T> class` **`IndexTable`**

The basic data table class.

* `using IndexTable::`**`const_iterator`** `= [bidirectional iterator]`
* `using IndexTable::`**`iterator`** `= [bidirectional iterator]`
* `using IndexTable::`**`const_reference`** `= const T&`
* `using IndexTable::`**`difference_type`** `= ptrdiff_t`
* `using IndexTable::`**`reference`** `= T&`
* `using IndexTable::`**`size_type`** `= size_t`
* `using IndexTable::`**`value_type`** `= T`

Member types.

* `IndexTable::`**`IndexTable`**`()`
* `template <typename Range> explicit IndexTable::`**`IndexTable`**`(const Range& src)`
* `template <typename Iterator> IndexTable::`**`IndexTable`**`(Iterator i1, Iterator i2)`
* `IndexTable::`**`~IndexTable`**`() noexcept`

Life cycle functions. The default constructor creates an empty table; the
second and third constructors copy elements from the range into the table.
_(Complexity: Amortised constant for the default constructor, otherwise
`O(n)`.)_

* `iterator IndexTable::`**`begin`**`()`
* `const_iterator IndexTable::`**`begin`**`() const`
* `iterator IndexTable::`**`end`**`()`
* `const_iterator IndexTable::`**`end`**`() const`

Iterators over the elements of the table, in the order in which they were
inserted. _(Complexity: Constant.)_

* `void IndexTable::`**`clear`**`()`

Deletes all elements. _(Complexity: `O(n)`.)_

* `bool IndexTable::`**`empty`**`() const noexcept`

True if the table is empty. _(Complexity: Constant.)_

* `void IndexTable::`**`erase`**`(iterator i)`
* `void IndexTable::`**`erase`**`(iterator i1, iterator i2)`

Remove elements from the table. _(Complexity: Amortised constant for the first
version, `O(k)` for the second.)_

* `void IndexTable::`**`insert`**`(const T& t)`
* `template <typename Range> void IndexTable::`**`insert`**`(const Range& src)`
* `template <typename Iterator> void IndexTable::`**`insert`**`(Iterator i1, Iterator i2)`
* `void IndexTable::`**`push_back`**`(const T& t)`

These insert items into the table. The first version of `insert()` is a
synonym for `push_back()`. All of these will throw `IndexCollision` if it
would create a duplicate key in an index that does not allow duplicates.
_(Complexity: `O(k)`.)_

* `size_t IndexTable::`**`size`**`() const noexcept`

Returns the number of items in the table. _(Complexity: Amortised constant.)_

### Index class ###

* `template <typename K, typename T, IndexMode M = IndexMode::unique> class` **`Index`**

The index type. The template arguments are the key type, the data type, and a
flag indicating whether duplicate keys will be allowed.

* `using Index::`**`iterator`** `= [bidirectional iterator]`
    * `const K& iterator::`**`key`**`() const noexcept`
* `using Index::`**`const_iterator`** `= [bidirectional iterator]`
    * `const K& const_iterator::`**`key`**`() const noexcept`
* `using Index::`**`compare_function`** `= function<bool(const K&, const K&)>`
* `using Index::`**`const_reference`** `= const T&`
* `using Index::`**`difference_type`** `= ptrdiff_t`
* `using Index::`**`extract_function`** `= function<K(const T&)>`
* `using Index::`**`key_type`** `= K`
* `using Index::`**`reference`** `= T&`
* `using Index::`**`size_type`** `= size_t`
* `using Index::`**`table_type`** `= IndexTable<T>`
* `using Index::`**`value_type`** `= T`

Member types. In addition to the normal bidirectional iterator operations, the
iterators have a `key()` member function that returns the element's key
(behaviour is undefined if this is called on a non-dereferenceable iterator).

* `static constexpr IndexMode Index::`**`mode`** `= M`

Member constants.

* `Index::`**`Index`**`()`
* `explicit Index::`**`Index`**`(table_type& table)`
* `Index::`**`Index`**`(table_type& table, extract_function extract)`
* `Index::`**`Index`**`(table_type& table, extract_function extract, compare_function compare)`
* `Index::`**`~Index`**`() noexcept`

Life cycle functions. The constructors take a reference to the table to be
indexed, and optionally a key extraction function (used to obtain the key from
a data value) and a comparison function (used to compare keys). The default
extraction function simply performs a `static_cast<K>(T)`; the default
comparison function is `std::less<K>`. A default constructed index is not
associated with any table and is always empty.

The constructors will build an index over the existing elements in the table;
they will throw `IndexCollision` if the table contains values with duplicate
keys but the index does not allow duplicates. An index may be destroyed before
or after its associated table. If the table is destroyed first, the index will
become empty. _(Complexity: Constant for the default constructor, `O(n)` for
the other constructors, amortised constant for the destructor.)_

* `iterator Index::`**`begin`**`()`
* `const_iterator Index::`**`begin`**`() const`
* `iterator Index::`**`end`**`()`
* `const_iterator Index::`**`end`**`() const`

Iterators over the table, in the order defined by the keys. If duplicates are
allowed, elements with duplicate keys are visited in order of insertion.
_(Complexity: Constant.)_

* `compare_function Index::`**`compare`**`() const`
* `extract_function Index::`**`extract`**`() const`

These return copies of the key comparison and extraction functions supplied to
the constructor. _(Complexity: Constant.)_

* `size_t Index::`**`count`**`(const K& k) const noexcept`

Returns the number of items in the table with the specified key. _(Complexity:
`O(log n)+O(k)`.)_

* `bool Index::`**`empty`**`() const noexcept`

True if the table is empty. This is equivalent to `table().empty()`, except
that it will return true if the table has been destroyed. _(Complexity:
Constant.)_

* `Irange<iterator> Index::`**`equal_range`**`(const K& k)`
* `Irange<const_iterator> Index::`**`equal_range`**`(const K& k) const`
* `iterator Index::`**`find`**`(const K& k)` _(unique keys only)_
* `const_iterator Index::`**`find`**`(const K& k) const` _(unique keys only)_
* `iterator Index::`**`lower_bound`**`(const K& k)`
* `const_iterator Index::`**`lower_bound`**`(const K& k) const`
* `iterator Index::`**`upper_bound`**`(const K& k)`
* `const_iterator Index::`**`upper_bound`**`(const K& k) const`

These find elements or subranges matching the specified key (following the
usual associative container semantics). The `find()` functions are not defined
for indices that allow duplicate keys. _(Complexity: `O(log n)`.)_

* `void Index::`**`erase`**`(iterator i)`
* `void Index::`**`erase`**`(iterator i1, iterator i2)`
* `void Index::`**`erase`**`(const K& k)`

These erase elements from the table, identified either by iterators or a key.
Erasing a nonexistent key is harmless. If the index allows duplicates, the
third version of `erase()` will erase all matching elements. _(Complexity:
Amortised constant for the first version; `O(log n)+O(k)` for the others.)_

* `size_t Index::`**`size`**`() const noexcept`

Returns the number of items in the table. This is equivalent to
`table().size()`, except that it will return zero if the table has been
destroyed. _(Complexity: Amortised constant.)_

* `table_type& Index::`**`table`**`() noexcept`
* `const table_type& Index::`**`table`**`() const noexcept`

These return references to the associated table. Behaviour is undefined if
the table has been destroyed or the index was default constructed.
_(Complexity: Constant.)_

## InterpolatedMap - associative array with interpolation ##

* `template <typename X, typename Y = X> class` **`InterpolatedMap`**
    * `using InterpolatedMap::`**`key_type`** `= X`
    * `using InterpolatedMap::`**`mapped_type`** `= Y`
    * `InterpolatedMap::`**`InterpolatedMap`**`()`
    * `InterpolatedMap::`**`InterpolatedMap`**`(std::initializer_list<...> list)`
    * `InterpolatedMap::`**`InterpolatedMap`**`(const InterpolatedMap& m)`
    * `InterpolatedMap::`**`InterpolatedMap`**`(InterpolatedMap&& m)`
    * `InterpolatedMap::`**`~InterpolatedMap`**`()`
    * `InterpolatedMap& InterpolatedMap::`**`operator=`**`(const InterpolatedMap& m)`
    * `InterpolatedMap& InterpolatedMap::`**`operator=`**`(InterpolatedMap&& m)`
    * `Y InterpolatedMap::`**`operator()`**`(X x) const`
    * `void InterpolatedMap::`**`clear`**`() noexcept`
    * `bool InterpolatedMap::`**`empty`**`() const noexcept`
    * `void InterpolatedMap::`**`erase`**`(X x) noexcept`
    * `void InterpolatedMap::`**`erase`**`(X x1, X x2) noexcept`
    * `void InterpolatedMap::`**`insert`**`(X x, Y y)`
    * `void InterpolatedMap::`**`insert`**`(X x, Y yl, Y yr)`
    * `void InterpolatedMap::`**`insert`**`(X x, Y yl, Y y, Y yr)`

An `InterpolatedMap` holds a list of `(x,y)` pairs, returning an interpolated
`y` value for arbitrary `x`. The input type, `X`, must be a floating point
arithmetic type; the output type, `Y`, is the same type as `X` by default, but
will work with any other floating point type, or any other type for which the
`interpolate()` function from the core library will work, or a similar
function that can be found by argument dependent lookup:

* `Y interpolate(X x1, Y y1, X x2, Y y2, X x)`

Legal output types include [`Vector`](vector.html) (if the scalar type is
floating point) and `Graphics::Colour`.

The constructor that takes an initializer list expects a list of braced
tuples, each containing one `X` value and one to three `Y` values, interpreted
in the same way as the `insert()` functions described below. If an `X` value
is repeated, later entries overwrite earlier ones. Example:

    InterpolatedMap<float> imap = {
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

## Stack - stack with reverse destruction order ##

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
