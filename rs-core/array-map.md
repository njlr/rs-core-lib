# Sequence Based Associative Array #

By Ross Smith

* `#include "rs-core/array-map.hpp"`

## Contents ##

[TOC]

## Class ArrayMap ##

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
