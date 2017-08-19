# Core Alorithms #

By Ross Smith

* `#include "rs-core/algorithm.hpp"`

## Contents ##

[TOC]

## General algorithms ##

### Difference algorithm ###

* `template <typename RandomAccessRange> struct` **`DiffEntry`**
    * `using DiffEntry::`**`iterator`** `= RangeIterator<const RandomAccessRange>`
    * `using DiffEntry::`**`subrange`** `= Irange<iterator>`
    * `DiffEntry::subrange DiffEntry::`**`del`**
    * `DiffEntry::subrange DiffEntry::`**`ins`**
* `template <typename RandomAccessRange> using` **`DiffList`** `= std::vector<DiffEntry<RandomAccessRange>>`
* `template <typename RandomAccessRange> DiffList<RandomAccessRange>` **`diff`**`(const RandomAccessRange& lhs, const RandomAccessRange& rhs)`
* `template <typename RandomAccessRange, typename EqualityPredicate> DiffList<RandomAccessRange>` **`diff`**`(const RandomAccessRange& lhs, const RandomAccessRange& rhs, EqualityPredicate eq)`

This is an implementation of the algorithm described in [Eugene Myers' 1986
paper](http://xmailserver.org/diff2.pdf). The return value is a list of diffs,
each consisting of two pairs of iterators: the `del` member is a subrange of
`lhs` indicating which elements have been removed, and the `ins` member is a
subrange of `rhs` indicating which elements have been inserted in the same
location; at least one subrange in each diff entry will be non-empty.

Complexity: `O((n1+n2)d)`, where `n1` and `n2` are the lengths of the input
ranges and `d` is the number of differences.

### Edit distance ###

* `template <typename ForwardRange1, typename ForwardRange2> size_t` **`edit_distance`**`(const ForwardRange1& range1, const ForwardRange2& range2)`
* `template <typename ForwardRange1, typename ForwardRange2, typename T> T` **`edit_distance`**`(const ForwardRange1& range1, const ForwardRange2& range2, T ins, T del, T sub)`

These return the edit distance (Levenshtein distance) between two ranges,
based on the number of insertions, deletions, and substitutions required to
transform one range into the other. By default, each operation is given a
weight of 1; optionally, explicit weights can be given to each operation. The
weight type (`T`) must be an arithmetic type. Behaviour is undefined if any of
the weights are negative.

Complexity: Quadratic.

### Find optimum ###

* `template <typename ForwardRange, typename UnaryFunction> [range iterator]` **`find_optimum`**`(ForwardRange& range, UnaryFunction f)`
* `template <typename ForwardRange, typename UnaryFunction, typename Compare> [range iterator]` **`find_optimum`**`(ForwardRange& range, UnaryFunction f, Compare c)`

These return an iterator identifying the range element for which `f(x)` has
the minimum value, according to the given comparison function. The comparison
function is expected to have less-than semantics, and defaults to
`std::less<>()`; use `std::greater<>()` to get the maximum value of `f(x)`
instead of the minimum. If there is more than one optimum element, the first
one will be returned. These will return an end iterator only if the range is
empty.

Complexity: Linear.

## Paired range operations ##

For all of these algorithms, if the two ranges supplied have different
lengths, the length of the shorter range is used; the excess elements in the
longer range are ignored.

### Paired for each ###

* `template <typename InputRange1, typename InputRange2, typename BinaryFunction> void` **`paired_for_each`**`(InputRange1& range1, InputRange2& range2, BinaryFunction f)`

Calls `f(x,y)` for each pair of corresponding elements in the two ranges.

### Paired sort ###

* `template <typename RandomAccessRange1, typename RandomAccessRange2> void` **`paired_sort`**`(RandomAccessRange1& range1, RandomAccessRange2& range2)`
* `template <typename RandomAccessRange1, typename RandomAccessRange2, typename Compare> void` **`paired_sort`**`(RandomAccessRange1& range1, RandomAccessRange2& range2, Compare comp)`
* `template <typename RandomAccessRange1, typename RandomAccessRange2> void` **`paired_stable_sort`**`(RandomAccessRange1& range1, RandomAccessRange2& range2)`
* `template <typename RandomAccessRange1, typename RandomAccessRange2, typename Compare> void` **`paired_stable_sort`**`(RandomAccessRange1& range1, RandomAccessRange2& range2, Compare comp)`

Sort `range1` according to the comparison function (with the usual `std::less`
default), also reordering `range2` in the same way (that is, each element in
`range2` goes into the same position as the corresponding element of
`range1`).

### Paired transform ###

* `template <typename InputRange1, typename InputRange2, typename OutputIterator, typename BinaryFunction> void` **`paired_transform`**`(const InputRange1& range1, const InputRange2& range2, OutputIterator out, BinaryFunction f)`

For each pair of corresponding elements in the two ranges, this calls `f(x,y)`
and appends the result to the output range.

## Topological order ##

### Exceptions ###

* `class` **`TopologicalOrderError`**`: public std::runtime_error`
    * `class` **`TopologicalOrderCycle`**`: public TopologicalOrderError`
    * `class` **`TopologicalOrderEmpty`**`: public TopologicalOrderError`

Exceptions reporting errors in topological order operations.

### Class TopologicalOrder ###

* `template <typename T, typename Compare = std::less<T>> class` **`TopologicalOrder`**

A container that sorts its elements according to a defined topological order,
given a set of _(before,after)_ relations. The comparison object is used only
to sort elements within a subset; it does not participate in the topological
relation.

* `using TopologicalOrder::`**`compare_type`** `= Compare`
* `using TopologicalOrder::`**`value_type`** `= T`

Member types.

* `TopologicalOrder::`**`TopologicalOrder`**`()`
* `explicit TopologicalOrder::`**`TopologicalOrder`**`(Compare c)`
* `TopologicalOrder::`**`~TopologicalOrder`**`() noexcept`
* `TopologicalOrder::`**`TopologicalOrder`**`(const TopologicalOrder& t)`
* `TopologicalOrder::`**`TopologicalOrder`**`(TopologicalOrder&& t) noexcept`
* `TopologicalOrder& TopologicalOrder::`**`operator=`**`(const TopologicalOrder& t)`
* `TopologicalOrder& TopologicalOrder::`**`operator=`**`(TopologicalOrder&& t) noexcept`

Life cycle functions.

* `void TopologicalOrder::`**`clear`**`() noexcept`

Remove all elements from the container.

* `Compare TopologicalOrder::`**`comp`**`() const`

Returns a copy of the comparison predicate.

* `bool TopologicalOrder::`**`empty`**`() const noexcept`

True if the container is empty.

* `bool TopologicalOrder::`**`erase`**`(const T& t)`

Erase a specific element from the container. This will do nothing if the
element was not present.

* `T TopologicalOrder::`**`front`**`() const`
* `std::vector<T> TopologicalOrder::`**`front_set`**`() const`
* `T TopologicalOrder::`**`back`**`() const`
* `std::vector<T> TopologicalOrder::`**`back_set`**`() const`

Query the current front set (elements that never appear on the right of an
ordering relation) or back set (elements that never appear on the left), or
the first element in each set. The elements in each returned list will be
ordered according to the container's comparison predicate. An element that has
no ordering relations with any other element will appear in both sets. The
`front()` and `back()` functions will throw `TopologicalOrderEmpty` if the
container is empty, or `TopologicalOrderCycle` if a cycle is detected; the set
functions will throw if a cycle is detected but will simply return an empty
set if the container is empty. Not all cycles will be detected by these
functions (a cycle that involves only interior elements, while still leaving
well defined front and back sets, will not be detected here).

* `bool TopologicalOrder::`**`has`**`(const T& t) const`

True if the element is in the container.

* `template <typename... Args> void TopologicalOrder::`**`insert`**`(const T& t, const Args&... args)`

Insert one or more elements. If only one element is inserted, it has no
implied ordering relations with any other elements. If multiple elements are
listed, an ordering relation is implied between each element and all elements
after it in the list. In all insert functions, implied relations between an
element and itself are ignored.

* `template <typename Range> void TopologicalOrder::`**`insert_1n`**`(const T& t1, const Range& r2)`
* `void TopologicalOrder::`**`insert_1n`**`(const T& t1, std::initializer_list<T> r2)`
* `template <typename Range> void TopologicalOrder::`**`insert_n1`**`(const Range& r1, const T& t2)`
* `void TopologicalOrder::`**`insert_n1`**`(std::initializer_list<T> r1, const T& t2)`
* `template <typename Range1, typename Range2> void TopologicalOrder::`**`insert_mn`**`(const Range1& r1, const Range2& r2)`
* `void TopologicalOrder::`**`insert_mn`**`(std::initializer_list<T> r1, std::initializer_list<T> r2)`

Insert one or more pairs of elements, with ordering relations between the left
and right arguments. The `insert_1n()` function creates an ordering relation
between the first argument and every element of the second range;
`insert_n1()` creates an ordering relation between every element of the first
range and the second element; `insert_mn()` creates an ordering relation
between every pair in the cross product of the two ranges.

* `bool TopologicalOrder::`**`is_front`**`(const T& t) const`
* `bool TopologicalOrder::`**`is_back`**`(const T& t) const`

True if the give element is part of the current front or back set.

* `T TopologicalOrder::`**`pop_front`**`()`
* `std::vector<T> TopologicalOrder::`**`pop_front_set`**`()`
* `T TopologicalOrder::`**`pop_back`**`()`
* `std::vector<T> TopologicalOrder::`**`pop_back_set`**`()`

Remove one or all elements of the current front or back set. These will throw
`TopologicalOrderCycle` or `TopologicalOrderEmpty` under the same
circumstances as the corresponding front or back element or set functions.

* `size_t TopologicalOrder::`**`size`**`() const noexcept`

Returns the number of elements in the container.
