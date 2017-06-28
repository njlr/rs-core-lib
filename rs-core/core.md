# Core Utilities #

By Ross Smith

* `#include "rs-core/core.hpp"`

## Contents ##

[TOC]

## Preprocessor macros ##

* `#define` **`RS_ASSERT`**`(expr)`

A simple assertion macro for quick and dirty debugging. If the expression
(explicitly cast to `bool`) is false, this throws a `std::logic_error`
exception, with a message containing the text of the expression and the source
code location.

* `#define` **`RS_ASSUME`**`(expr)` _[compiler may assume `bool(expr)` is always true]_
* `#define` **`RS_LIKELY`**`(expr)` _[returns `bool(expr)`; compiler should optimize for the true case]_
* `#define` **`RS_UNLIKELY`**`(expr)` _[returns `bool(expr)`; compiler should optimize for the false case]_
* `#define` **`RS_NOTREACHED`** _[compiler may assume this line will never be reached]_

Optimization hints. These have no semantic effect on the program (if the
absolute conditions in `RS_ASSUME` and `RS_NOTREACHED` are not violated), but
give the compiler hints concerning its expected behaviour.

* `#define` **`RS_BITMASK_OPERATORS`**`(EC)`

Defines bit manipulation and related operators for an `enum class` (unary `!`,
`~`; binary `&`, `&=`, `|`, `|=`, `^`, `^=`).

* `#define` **`RS_CHAR`**`(C, T)`
* `#define` **`RS_CSTR`**`(S, T)`

These resolve to the character `C` as a `constexpr T`, or the C-style string
literal `S` as a `constexpr const T*`. For example, `RS_CHAR('A',wchar_t)`
resolves to `L'A'`; `RS_CSTR("Hello",char32_t)` resolves to `U"Hello"`. `C`
must be a simple character literal; `T` must be one of the four character
types that can be used in strings (`char`, `char16_t`, `char32_t`, or
`wchar_t`). Behaviour is undefined if `T` is not one of those four types, or
(for `RS_CHAR`) if `C` is not representable by a single code unit.

* `#define` **`RS_ENUM`**`(EnumType, IntType, first_value, first_name, ...)`
* `#define` **`RS_ENUM_CLASS`**`(EnumType, IntType, first_value, first_name, ...)`

These define an enumeration, given the name of the enumeration type, the
underlying integer type, the integer value of the first entry, and a list of
value names. They will also define the following functions:

* `std::ostream&` **`operator<<`**`(std::ostream& out, EnumType t)`
* `constexpr bool` **`enum_is_valid`**`(EnumType t) noexcept`
* `std::vector<EnumType>` **`enum_values<EnumType>`**`()`

The output operator prints the name of an enumeration constant (qualified with
the class name if this is an `enum class`), or the integer value if the
argument is not a named value. The `enum_is_valid()` function reports whether
or not the argument is a named value of the enumeration. The
`enum_values<T>()` function returns a `vector` containing a list of the
enumeration's values.

Example:

    RS_ENUM(Foo, 1, alpha, bravo, charlie)
    RS_ENUM_CLASS(Bar, 1, delta, echo, foxtrot)

Equivalent code:

    enum Foo { alpha = 1, bravo, charlie };
    std::ostream& operator<<(std::ostream& out, Foo f) { ... }
    enum class Bar { delta = 1, echo, foxtrot };
    std::ostream& operator<<(std::ostream& out, Bar b) { ... }

* `#define` **`RS_LDLIB`**`([tag:] lib ...)`

This instructs the makefile to link with one or more static libraries. Specify
library names without the `-l` prefix (e.g. `RS_LDLIB(foo)` will link with
`-lfoo`). If link order is important for a particular set of libraries, supply
them in a space delimited list in a single `RS_LDLIB()` line.

Libraries that are needed only on specific targets can be prefixed with one of
the target identifiers listed below (e.g. `RS_LDLIB(apple:foo)` will link
with `-lfoo` for Apple targets only). Only one target can be specified per
invocation; if the same libraries are needed on multiple targets, but not on
all targets, you will need a separate `RS_LDLIB()` line for each target.

<!-- TEXT -->
* `apple:`
* `linux:`
* `mingw:`
* `cygwin:`

`RS_LDLIB()` lines are picked up at the `"make dep"` stage; if you change a
link library, the change will not be detected until dependencies are rebuilt.

* `#define` **`RS_MOVE_ONLY`**`(T)`
    * `T(const T&) = delete;`
    * `T(T&&) = default;`
    * `T& operator=(const T&) = delete;`
    * `T& operator=(T&&) = default;`
* `#define` **`RS_NO_COPY_MOVE`**`(T)`
    * `T(const T&) = delete;`
    * `T(T&&) = delete;`
    * `T& operator=(const T&) = delete;`
    * `T& operator=(T&&) = delete;`

Convenience macros for defaulted or deleted copy and move operations.

* `#define` **`RS_OVERLOAD`**`(f) [] (auto&&... args) { return f(std::forward<decltype(args)>(args)...); }`

Creates a function object wrapping a set of overloaded functions, that can be
passed to a context expecting a function (such as an STL algorithm) without
having to explicitly resolve the overload at the call site. (From an idea by
Arthur O'Dwyer on the C++ standard proposals mailing list, 14 Sep 2015.)

* `#define` **`RS_STATIC_ASSERT`**`(expr) static_assert((expr), # expr)`

Shorthand for `static_assert`, using the assertion expression as the error
message. (A planned change in C++17 will make this unnecessary.)

## Types ##

### Basic types ###

* `using` **`U8string`** `= std::string`

Use `U8string` for strings that are expected to be in UTF-8 (or ASCII, since
any ASCII string is also valid UTF-8), while plain `std::string` is used where
the string is expected to be in some non-Unicode encoding, or where the string
is being used simply as an array of bytes rather than encoded text.

### Arithmetic types ###

#### Endian integers ####

* `enum` **`ByteOrder`**
    * **`big_endian`**
    * **`little_endian`**
* `template <typename T, ByteOrder B> class` **`Endian`**
    * `using Endian::`**`value_type`** `= T`
    * `static constexpr ByteOrder Endian::`**`byte_order`** `= B`
    * `constexpr Endian::`**`Endian`**`() noexcept`
    * `constexpr Endian::`**`~Endian`**`() noexcept`
    * `constexpr Endian::`**`Endian`**`(const Endian& e) noexcept`
    * `constexpr Endian::`**`Endian`**`(Endian&& e) noexcept`
    * `constexpr Endian& Endian::`**`operator=`**`(const Endian& e) noexcept`
    * `constexpr Endian& Endian::`**`operator=`**`(Endian&& e) noexcept`
    * `constexpr Endian::`**`Endian`**`(T t) noexcept`
    * `explicit Endian::`**`Endian`**`(const void* p) noexcept`
    * `constexpr Endian::`**`operator T`**`() const noexcept`
    * `constexpr T Endian::`**`get`**`() const noexcept`
    * `constexpr const T* Endian::`**`ptr`**`() const noexcept`
    * `T* Endian::`**`ptr`**`() noexcept`
    * `constexpr T Endian::`**`rep`**`() const noexcept`
    * `T& Endian::`**`rep`**`() noexcept`
* `template <typename T> using` **`BigEndian`** `= Endian<T, big_endian>`
* `template <typename T> using` **`LittleEndian`** `= Endian<T, little_endian>`
* `template <typename T, ByteOrder B> std::ostream&` **`operator<<`**`(std::ostream& out, Endian<T, B> t)`
* `template <typename T, ByteOrder B> class std::`**`hash`**`<Endian<T, B>>`
* _comparison operators between all combinations of `Endian` and `T`_

An `Endian` object holds an integer in a defined byte order. This is a literal
type and can be used in `constexpr` expressions.

Assignment to or from an endian integer performs any necessary reordering
transparently. The default constructor sets the value to zero; the constructor
from a pointer copies `sizeof(T)` bytes into the object. The `ptr()` and
`rep()` functions give access to the internal, byte ordered form. The hash
function gives the same result as the underlying integer type's hash.

### Containers ###

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

* `template <typename T> class` **`Stack`**
    * `Stack<T>::`**`Stack`**`()`
    * `Stack<T>::`**`~Stack`**`()`
    * `Stack<T>::`**`Stack`**`(Stack&& s)`
    * `Stack& Stack<T>::`**`operator=`**`(Stack&& s)`
    * `void Stack<T>::`**`clear`**`() noexcept`
    * `bool Stack<T>::`**`empty`**`() const noexcept`
    * `void Stack<T>::`**`pop`**`() noexcept`
    * `void Stack<T>::`**`push`**`(const T& t)`
    * `void Stack<T>::`**`push`**`(T&& t)`
    * `size_t Stack<T>::`**`size`**`() const noexcept`
    * `const T& Stack<T>::`**`top`**`() const noexcept`

A simple LIFO container, whose main function is to ensure that its elements
are destroyed in reverse order of insertion (this is not guaranteed by any
standard container, but is often useful for RAII). Behaviour is undefined if
`pop()` or `top()` is called on an empty stack.

### Exceptions ###

* `void` **`rethrow`**`(std::exception_ptr p)`

Rethrows the exception, if one is present. This is the same as
`std::rethrow_exception()`, except that it does nothing if the exception
pointer is null.

* `class` **`WindowsCategory`**`: public std::error_category`
    * `virtual U8string WindowsCategory::`**`message`**`(int ev) const`
    * `virtual const char* WindowsCategory::`**`name`**`() const noexcept`
* `const std::error_category&` **`windows_category`**`() noexcept`

An error category instance for translating Win32 API error codes; naturally
this is only defined on builds that use the Win32 API. (MSVC provides this
through `std::system_category()`, but GCC does not supply any equivalent on
Cygwin or Mingw.)

### Metaprogramming and type traits ###

* `template <typename T> using` **`BinaryType`** `= [unsigned integer type]`

Yields an unsigned integer type the same size as `T`. This will fail to
compile if no such type exists.

* `template <typename T1, typename T2> using` **`CopyConst`** `= ...`

Yields a type created by transferring the `const` qualification (or lack of
it) from `T1` to the unqualified type of `T2`. For example, `CopyConst<int,
const string>` yields `string`, while `CopyConst<const int, string>` yields
`const string`.

* `template <size_t Bits> using` **`SignedInteger`** `= [signed integer type]`
* `template <size_t Bits> using` **`UnsignedInteger`** `= [unsigned integer type]`

Signed and unsigned integer types with the specified number of bits (the same
types as `int8_t`, `int16_t`, etc). These will fail to compile if `Bits` is
not a power of 2 in the supported range (8-64).

### Mixins ###

<!-- DEFN --> These are convenience base classes that define members and
operators that would normally just be repetitive boilerplate (similar to the
ones in Boost). They all use the CRTP idiom; a class `T` should derive from
`Mixin<T>` to automatically generate the desired boilerplate code. The table
below shows which operations the user is required to define, and which ones
the mixin will automatically define. (Here, `t` and `u` are objects of type
`T`, `v` is an object of `T`'s value type, and `n` is an integer.)

Mixin                                       | Requires                         | Defines
-----                                       | --------                         | -------
**`EqualityComparable`**`<T>`               | `t==u`                           | `t!=u`
**`LessThanComparable`**`<T>`               | `t==u, t<u`                      | `t!=u, t>u, t<=u, t>=u`
**`InputIterator`**`<T,CV>`                 | `*t, ++t, t==u`                  | `t->, t++, t!=u`
**`OutputIterator`**`<T>`                   | `t=v`                            | `*t, ++t, t++`
**`ForwardIterator`**`<T,CV>`               | `*t, ++t, t==u`                  | `t->, t++, t!=u`
**`BidirectionalIterator`**`<T,CV>`         | `*t, ++t, --t, t==u`             | `t->, t++, t--, t!=u`
**`RandomAccessIterator`**`<T,CV>`          | `*t, t+=n, t-u`                  | `t->, t[n], ++t, t++, --t, t--, t-=n, t+n, n+t, t-n,`<br>`t==u, t!=u, t<u, t>u, t<=u, t>=u`
**`FlexibleRandomAccessIterator`**`<T,CV>`  | `*t, ++t, --t, t+=n, t-u, t==u`  | `t->, t[n], t++, t--, t-=n, t+n, n+t, t-n,`<br>`t!=u, t<u, t>u, t<=u, t>=u`

In the iterator mixins, `CV` is either `V` or `const V`, where `V` is the
iterator's value type, depending on whether a mutable or const iterator is
required.

The first version of `RandomAccessIterator` uses the minimal set of user
supplied operations to generate all of those required;
`FlexibleRandomAccessIterator` requires more user supplied operations, but
will decay safely to one of the simpler iterator types if an underlying type
does not supply all of the corresponding operations.

In addition to the operators listed in the table above, all iterator mixins
supply the standard member types:

* `using` **`difference_type`** `= ptrdiff_t`
* `using` **`iterator_category`** `= [standard iterator tag type]`
* `using` **`pointer`** `= CV*`
* `using` **`reference`** `= CV&`
* `using` **`value_type`** `= std::remove_const_t<CV>`

### Smart pointers ###

* `template <typename T> class` **`Nnptr`**
    * `using Nnptr::`**`element_type`** `= T`
    * `Nnptr::`**`Nnptr`**`(const Nnptr& p) noexcept`
    * `Nnptr::`**`Nnptr`**`(Nnptr&& p) noexcept`
    * `template <typename T2> Nnptr::`**`Nnptr`**`(const Nnptr<T2>& p) noexcept`
    * `template <typename T2> explicit Nnptr::`**`Nnptr`**`(T2* p)`
    * `template <typename T2, typename D> Nnptr::`**`Nnptr`**`(T2* p, D d)`
    * `template <typename T2, typename D> Nnptr::`**`Nnptr`**`(unique_ptr<T2, D>&& p)`
    * `Nnptr::`**`~Nnptr`**`() noexcept`
    * `Nnptr& Nnptr::`**`operator=`**`(const Nnptr& p) noexcept`
    * `Nnptr& Nnptr::`**`operator=`**`(Nnptr&& p) noexcept`
    * `template <typename T2> Nnptr& Nnptr::`**`operator=`**`(const Nnptr<T2>& p) noexcept`
    * `template <typename T2, typename D> Nnptr& Nnptr::`**`operator=`**`(unique_ptr<T2, D>&& p)`
    * `explicit Nnptr::`**`operator bool`**`() const noexcept`
    * `bool Nnptr::`**`operator!`**`() const noexcept`
    * `T& Nnptr::`**`operator*`**`() const noexcept`
    * `T* Nnptr::`**`operator->`**`() const noexcept`
    * `T* Nnptr::`**`get`**`() const noexcept`
    * `template <typename T2> void Nnptr::`**`reset`**`(T2* p)`
    * `template <typename T2, typename D> void Nnptr::`**`reset`**`(T2* p, D d)`
    * `bool Nnptr::`**`unique`**`() const noexcept`
* `class` **`NullPointer`**`: public std::runtime_error`
* `template <typename T, typename... Args> Nnptr<T>` **`make_nnptr`**`(Args&&... args)`
* `template <typename T1, typename T2> Nnptr<T1>` **`const_pointer_cast`**`(const Nnptr<T2>& r) noexcept`
* `template <typename T1, typename T2> Nnptr<T1>` **`dynamic_pointer_cast`**`(const Nnptr<T2>& r) noexcept`
* `template <typename T1, typename T2> Nnptr<T1>` **`static_pointer_cast`**`(const Nnptr<T2>& r) noexcept`
* _Comparison operators (see below)_

This is basically the same as a `shared_ptr`, except that the pointer can
never be null. Any method that implicitly takes ownership of a pointer (i.e.
any that is not marked `noexcept`) will throw `NullPointer` if the pointer is
null.

Note that there is no default constructor. Move construction and assignment
are implemented as reference incrementing copies on the underlying pointer
(i.e. the same as copy construction and assignment). Behaviour is otherwise
the same as the corresponding operations on a `shared_ptr`.

Comparison operators are defined between two heterogeneous `Nnptr` objects,
between an `Nnptr` and a raw pointer, and between an `Nnptr` and a `nullptr`.
These call the corresponding operations on the underlying raw pointers.

* `template <typename T> Nnptr<T>` **`nnptr`**`(const T& t)`
* `template <typename T> shared_ptr<T>` **`shptr`**`(const T& t)`
* `template <typename T> unique_ptr<T>` **`unptr`**`(const T& t)`

Shorthand for `make_nnptr/shared/unique<T>(t)`.

### Type adapters ###

* `template <typename T, int Def = 0> struct` **`Movable`**
    * `using Movable::`**`value_type`** `= T`
    * `T Movable::`**`value`**
    * `Movable::`**`Movable`**`()`
    * `Movable::`**`Movable`**`(const T& t)`
    * `Movable::`**`~Movable`**`() noexcept`
    * `Movable::`**`Movable`**`(const Movable& m)`
    * `Movable::`**`Movable`**`(Movable&& m) noexcept`
    * `Movable& Movable::`**`operator=`**`(const Movable& m)`
    * `Movable& Movable::`**`operator=`**`(Movable&& m) noexcept`

This wraps a `T` object and provides reset-on-move behaviour: `Movable<T>`'s
move constructor and move assignment operator set the value in the moved-from
object to `T`'s default value. If `T` is explicitly convertible from an `int`,
a default value other than zero can be provided. The move functions call the
corresponding functions on `T`, which are assumed to not throw. If `T` is also
copyable, `Movable<T>` will be copyable.

### Type related functions ###

* `template <typename T2, typename T1> bool` **`is`**`(const T1& ref) noexcept`
* `template <typename T2, typename T1> bool` **`is`**`(const T1* ptr) noexcept`
* `template <typename T2, typename T1> bool` **`is`**`(const unique_ptr<T1>& ptr) noexcept`
* `template <typename T2, typename T1> bool` **`is`**`(const shared_ptr<T1>& ptr) noexcept`
* `template <typename T2, typename T1> T2&` **`as`**`(T1& ref)`
* `template <typename T2, typename T1> const T2&` **`as`**`(const T1& ref)`
* `template <typename T2, typename T1> T2&` **`as`**`(T1* ptr)`
* `template <typename T2, typename T1> const T2&` **`as`**`(const T1* ptr)`
* `template <typename T2, typename T1> T2&` **`as`**`(unique_ptr<T1>& ptr)`
* `template <typename T2, typename T1> T2&` **`as`**`(const unique_ptr<T1>& ptr)`
* `template <typename T2, typename T1> T2&` **`as`**`(shared_ptr<T1>& ptr)`
* `template <typename T2, typename T1> T2&` **`as`**`(const shared_ptr<T1>& ptr)`

These are simple wrappers around `dynamic_cast`. The `is()` function returns
true if the pointer or reference's target has the requested dynamic type;
`as()` returns a reference to the object converted to the requested dynamic
type, or throws `std::bad_cast` if the `dynamic_cast` fails.

* `template <typename T2, typename T1> T2` **`binary_cast`**`(const T1& t) noexcept`
* `template <typename T2, typename T1> T2` **`implicit_cast`**`(const T1& t)`

Type conversions. The `binary_cast()` operation does a simple bitwise copy
from one type to another; it will fail to compile if the two types have
different sizes, but does no other safety checks. The `implicit_cast()`
operation performs the conversion only if it would be allowed as an implicit
conversion.

* `string` **`demangle`**`(const string& name)`
* `string` **`type_name`**`(const std::type_info& t)`
* `string` **`type_name`**`(const std::type_index& t)`
* `template <typename T> string` **`type_name`**`()`
* `template <typename T> string` **`type_name`**`(const T& t)`

Demangle a type name. The original mangled name can be supplied as an explicit
string, as a `std::type_info` or `std:type_index` object, as a type argument
to a template function (e.g. `type_name<int>()`), or as an object whose type
is to be named (e.g. `type_name(42)`). The last version will report the
dynamic type of the referenced object.

### UUID ###

* `class` **`Uuid`**
    * `Uuid::`**`Uuid`**`() noexcept`
    * `Uuid::`**`Uuid`**`(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h, uint8_t i, uint8_t j, uint8_t k, uint8_t l, uint8_t m, uint8_t n, uint8_t o, uint8_t p) noexcept`
    * `Uuid::`**`Uuid`**`(uint32_t abcd, uint16_t ef, uint16_t gh, uint8_t i, uint8_t j, uint8_t k, uint8_t l, uint8_t m, uint8_t n, uint8_t o, uint8_t p) noexcept`
    * `explicit Uuid::`**`Uuid`**`(const uint8_t* ptr) noexcept`
    * `explicit Uuid::`**`Uuid`**`(const string& s)`
    * `Uuid::`**`Uuid`**`(const Uuid& u) noexcept`
    * `Uuid::`**`Uuid`**`(Uuid&& u) noexcept`
    * `Uuid::`**`~Uuid`**`() noexcept`
    * `Uuid& Uuid::`**`operator=`**`(const Uuid& u) noexcept`
    * `Uuid& Uuid::`**`operator=`**`(Uuid&& u) noexcept`
    * `uint8_t& Uuid::`**`operator[](`**`size_t i) noexcept`
    * `const uint8_t& Uuid::`**`operator[]`**`(size_t i) const noexcept`
    * `uint8_t* Uuid::`**`begin`**`() noexcept`
    * `const uint8_t* Uuid::`**`begin`**`() const noexcept`
    * `uint8_t* Uuid::`**`end`**`() noexcept`
    * `const uint8_t* Uuid::`**`end`**`() const noexcept`
    * `size_t Uuid::`**`hash`**`() const noexcept`
    * `U8string Uuid::`**`str`**`() const`
* `bool` **`operator==`**`(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool` **`operator!=`**`(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool` **`operator<`**`(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool` **`operator>`**`(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool` **`operator<=`**`(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool` **`operator>=`**`(const Uuid& lhs, const Uuid& rhs) noexcept`
* `std::ostream&` **`operator<<`**`(std::ostream& o, const Uuid& u)`
* `class std::`**`hash`**`<Uuid>`

This class holds a standard 16 byte universally unique identifier (UUID).

The default constructor sets all bytes to zero. The second and third
constructors accept explicit byte values, either as a list of 16 bytes, or in
the standard breakdown format. The fourth constructor copies the next 16 bytes
from the location pointed to; a null pointer will set all bytes to zero.

The fifth constructor parses the string representation of a UUID. It expects
the string to hold exactly 32 hex digits, in groups each containing an even
number of digits (optionally prefixed with `"0x"`), and will treat any
characters that are not ASCII alphanumerics as delimiters. It will throw
`std::invalid_argument` if the string is not a valid UUID.

The `begin()`, `end()`, and `operator[]` functions grant access to the byte
representation. Behaviour is undefined if the index to `operator[]` is greater
than 15.

A specialization of `std::hash` is provided to allow `Uuid` to be used as the
key in an unordered container.

The `str()` function, and the output operator, format the UUID in the standard
broken down hex representation, e.g. `"01234567-89ab-cdef-0123-456789abcdef"`.

The comparison operators perform the natural bytewise lexicographical
comparisons.

* `struct` **`RandomUuid`**
    * `using RandomUuid::`**`result_type`** `= RandomUuid::Uuid`
    * `template <typename Rng> Uuid RandomUuid::`**`operator()`**`(Rng& rng) const`

Generates a random version 4 UUID.

### Version number ###

* `class` **`Version`**
    * `using Version::`**`value_type`** `= unsigned`
    * `Version::`**`Version`**`() noexcept`
    * `template <typename... Args> Version::`**`Version`**`(unsigned n, Args... args)`
    * `explicit Version::`**`Version`**`(const U8string& s)`
    * `Version::`**`Version`**`(const Version& v)`
    * `Version::`**`Version`**`(Version&& v) noexcept`
    * `Version::`**`~Version`**`() noexcept`
    * `Version& Version::`**`operator=`**`(const Version& v)`
    * `Version& Version::`**`operator=`**`(Version&& v) noexcept`
    * `unsigned Version::`**`operator[]`**`(size_t i) const noexcept`
    * `const unsigned* Version::`**`begin`**`() const noexcept`
    * `const unsigned* Version::`**`end`**`() const noexcept`
    * `unsigned Version::`**`major`**`() const noexcept`
    * `unsigned Version::`**`minor`**`() const noexcept`
    * `unsigned Version::`**`patch`**`() const noexcept`
    * `string Version::`**`str`**`(size_t min_elements = 2) const`
    * `string Version::`**`suffix`**`() const`
    * `uint32_t Version::`**`to32`**`() const noexcept`
    * `static Version Version::`**`from32`**`(uint32_t n) noexcept`
* `bool` **`operator==`**`(const Version& lhs, const Version& rhs) noexcept`
* `bool` **`operator!=`**`(const Version& lhs, const Version& rhs) noexcept`
* `bool` **`operator<`**`(const Version& lhs, const Version& rhs) noexcept;`
* `bool` **`operator>`**`(const Version& lhs, const Version& rhs) noexcept`
* `bool` **`operator<=`**`(const Version& lhs, const Version& rhs) noexcept`
* `bool` **`operator>=`**`(const Version& lhs, const Version& rhs) noexcept`
* `std::ostream&` **`operator<<`**`(std::ostream& o, const Version& v)`

A version number, represented as an array of integers optionally followed by a
trailing string (e.g. `1.2.3beta`). The default constructor sets the version
number to zero; the second constructor expects one or more integers followed
by a string; the third constructor parses a string (a string that does not
start with an integer is assumed to start with an implicit zero). Results are
unspecified if a version number element is too big to fit in an `unsigned
int`.

The indexing operator returns the requested element; it will return zero if
the index is out of range for the stored array. The `major()`, `minor()`, and
`patch()` functions return elements 0, 1, and 2. The `suffix()` function
returns the trailing string element.

The `str()` function (and the output operator) formats the version number in
the conventional form; a minimum number of elements can be requested. The
`to32()` and `from32()` functions pack or unpack the version into a 32 bit
integer, with one byte per element (e.g. version `1.2.3` becomes
`0x01020300`); `to32()` truncates elements higher than 255 and ignores the
string element.

## Constants and literals ##

### Arithmetic constants ###

* `#define` **`RS_DEFINE_CONSTANT`**`(name, value)`
    * `static constexpr float` **`name##_f`** `= value##f`
    * `static constexpr double` **`name##_d`** `= value`
    * `static constexpr long double` **`name##_ld`** `= value##l`
    * `template <typename T> static constexpr T` **`name##_c`** `= T(name##_ld)`

Defines a floating point constant, in several forms. This generates a set of
three named constants (for the three standard floating point types) and a
variable template. The `value` argument must be a floating point literal with
no type suffix.

(This intentionally uses a different naming convention to
[P0631](https://wg21.link/p0631), to avoid collisions if P0631 becomes part of
the standard library.)

The values quoted for the following constants are approximate; the values
actually supplied are accurate to at least `long double` precision where an
exact value is defined, or to the best known accuracy for the empirical
constants.

* _Mathematical constants from Posix/P0631_
    * **`e`** _= 2.7183_
    * **`log2e`** _= log<sub>2</sub> e = 1.4427_
    * **`log10e`** _= log<sub>10</sub> e = 0.4343_
    * **`ln2`** _= log<sub>e</sub> 2 = 0.6931_
    * **`ln10`** _= log<sub>e</sub> 10 = 2.3026_
    * **`pi`** _= &pi; = 3.1416_
    * **`pi_2`** _= &pi;/2 = 1.5708_
    * **`pi_4`** _= &pi;/4 = 0.7854_
    * **`one_pi`** _= 1/&pi; = 0.3183_
    * **`two_pi`** _= 2/&pi; = 0.6366_
    * **`two_sqrtpi`** _= 2/&radic;&pi; = 1.1284_
    * **`sqrt2`** _= &radic;2 = 1.4142_
    * **`one_sqrt2`** _= 1/&radic;2 = 0.7071_
* _Other mathematical constants_
    * **`sqrt3`** _= &radic;3 = 1.7321_
    * **`sqrt5`** _= &radic;5 = 2.2361_
    * **`sqrtpi`** _= &radic;&pi; = 1.7725_
    * **`sqrt2pi`** _= &radic;(2&pi;) = 2.5066_
* _Conversion factors_
    * **`inch`** _= 0.0254 m_
    * **`foot`** _= 0.3048 m_
    * **`yard`** _= 0.9144 m_
    * **`mile`** _= 1609 m_
    * **`nautical_mile`** _= 1852 m_
    * **`ounce`** _= 0.02835 kg_
    * **`pound`** _= 0.4536 kg_
    * **`short_ton`** _= 907.2 kg_
    * **`long_ton`** _= 1016 kg_
    * **`pound_force`** _= 4.448 N_
    * **`erg`** _= 10<sup>-7</sup> J_
    * **`foot_pound`** _= 1.356 J_
    * **`calorie`** _= 4.184 J_
    * **`ton_tnt`** _= 4.184&times;10<sup>9</sup> J_
    * **`horsepower`** _= 745.7 W_
    * **`mmHg`** _= 133.3 Pa_
    * **`atmosphere`** _= 101300 Pa_
    * **`zero_celsius`** _= 273.15 K_
* _Physical constants_
    * **`atomic_mass_unit`** _= 1.661&times;10<sup>-27</sup> kg_
    * **`avogadro_constant`** _= 6.022&times;10<sup>23</sup> mol<sup>-1</sup>_
    * **`boltzmann_constant`** _= 1.381&times;10<sup>-23</sup> J K<sup>-1</sup>_
    * **`elementary_charge`** _= 1.602&times;10<sup>-19</sup> C_
    * **`gas_constant`** _= 8.314 J mol<sup>-1</sup> K<sup>-1</sup>_
    * **`gravitational_constant`** _= 6.674&times;10<sup>-11</sup> m<sup>3</sup> kg<sup>-1</sup> s<sup>-2</sup>_
    * **`planck_constant`** _= 6.626&times;10<sup>-34</sup> J s_
    * **`speed_of_light`** _= 2.998&times;10<sup>8</sup> m s<sup>-1</sup>_
    * **`stefan_boltzmann_constant`** _= 5.670&times;10<sup>-8</sup> W m<sup>-2</sup> K<sup>-4</sup>_
* _Astronomical constants_
    * **`earth_mass`** _= 5.972&times;10<sup>24</sup> kg_
    * **`earth_radius`** _= 6.371&times;10<sup>6</sup> m_
    * **`earth_gravity`** _= 9.807 m s<sup>-2</sup>_
    * **`jupiter_mass`** _= 1.899&times;10<sup>27</sup> kg_
    * **`jupiter_radius`** _= 6.991&times;10<sup>7</sup> m_
    * **`solar_mass`** _= 1.989&times;10<sup>30</sup> kg_
    * **`solar_radius`** _= 6.963&times;10<sup>8</sup> m_
    * **`solar_luminosity`** _= 3.846&times;10<sup>26</sup> W_
    * **`solar_temperature`** _= 5778 K_
    * **`astronomical_unit`** _= 1.496&times;10<sup>11</sup> m_
    * **`light_year`** _= 9.461&times;10<sup>15</sup> m_
    * **`parsec`** _= 3.086&times;10<sup>16</sup> m_
    * **`julian_day`** _= 86400 s_
    * **`julian_year`** _= 31557600 s_
    * **`sidereal_year`** _= 31558150 s_
    * **`tropical_year`** _= 31556925 s_

### Arithmetic literals ###

These are all in `namespace RS::Literals`.

* `constexpr int8_t` **`operator""_s8`**`(unsigned long long n) noexcept`
* `constexpr uint8_t` **`operator""_u8`**`(unsigned long long n) noexcept`
* `constexpr int16_t` **`operator""_s16`**`(unsigned long long n) noexcept`
* `constexpr uint16_t` **`operator""_u16`**`(unsigned long long n) noexcept`
* `constexpr int32_t` **`operator""_s32`**`(unsigned long long n) noexcept`
* `constexpr uint32_t` **`operator""_u32`**`(unsigned long long n) noexcept`
* `constexpr int64_t` **`operator""_s64`**`(unsigned long long n) noexcept`
* `constexpr uint64_t` **`operator""_u64`**`(unsigned long long n) noexcept`
* `constexpr wchar_t` **`operator""_wc`**`(unsigned long long n) noexcept`
* `constexpr char16_t` **`operator""_c16`**`(unsigned long long n) noexcept`
* `constexpr char32_t` **`operator""_c32`**`(unsigned long long n) noexcept`
* `constexpr ptrdiff_t` **`operator""_t`**`(unsigned long long n) noexcept`
* `constexpr size_t` **`operator""_z`**`(unsigned long long n) noexcept`

Integer literals.

* `constexpr float` **`operator""_degf`**`(long double x) noexcept`
* `constexpr float` **`operator""_degf`**`(unsigned long long x) noexcept`
* `constexpr double` **`operator""_deg(`**`long double x) noexcept`
* `constexpr double` **`operator""_deg(`**`unsigned long long x) noexcept`
* `constexpr long double` **`operator""_degl`**`(long double x) noexcept`
* `constexpr long double` **`operator""_degl`**`(unsigned long long x) noexcept`

Angle literals, converting degrees to radians.

### String related constants ###

* `constexpr const char*` **`ascii_whitespace`** `= "\t\n\v\f\r "`

ASCII whitespace characters.

* `constexpr size_t` **`npos`** `= string::npos`

Defined for convenience. Following the conventions established by the standard
library, this value is often used as a function argument to mean "as large as
possible" or "no limit", or as a return value to mean "not found".

### Other constants ###

* `constexpr bool` **`big_endian_target`**
* `constexpr bool` **`little_endian_target`**

One of these will be true and the other false, reflecting the target system's
byte order.

* `constexpr unsigned` **`KB`** `= 1024`
* `constexpr unsigned long` **`MB`** `= 1 048 576`
* `constexpr unsigned long` **`GB`** `= 1 073 741 824`
* `constexpr unsigned long long` **`TB`** `= 1 099 511 627 776`

Powers of 2<sup>10</sup>.

## Algorithms and ranges ##

### Generic algorithms ###

* `template <typename Container> [output iterator]` **`append`**`(Container& con)`
* `template <typename Container> [output iterator]` **`overwrite`**`(Container& con)`

These create output iterators that will append elements to a standard
container (see `append_to()` below). The `append()` function is similar to
`std::back_inserter()` (but supports containers without `push_back()`), while
`overwrite()` will first clear the container and then return the append
iterator.

* `template <typename Container, typename T> void` **`append_to`**`(Container& con, const T& t)`

Appends an item to a container; used by `append()` and `overwrite()`. The
generic version calls `con.insert(con.end(), t)`; overloads (found by argument
dependent lookup) can be used for container-like types that do not have a
suitable `insert()` method.

* `template <typename Range1, typename Range2> int` **`compare_3way`**`(const Range1& r1, const Range2& r2)`
* `template <typename Range1, typename Range2, typename Compare> int` **`compare_3way`**`(const Range1& r1, const Range2& r2, Compare cmp)`

Compare two ranges, returning -1 if the first range is less than the second,
zero if they are equal, and +1 if the first range is greater.

* `template <typename Range, typename Container> void` **`con_append`**`(const Range& src, Container& dst)`
* `template <typename Range, typename Container> void` **`con_overwrite`**`(const Range& src, Container& dst)`

These are just shorthand for a `std::copy()` from a range to an append or
overwrite iterator.

* `template <typename Container, typename T> void` **`con_remove`**`(Container& con, const T& t)`
* `template <typename Container, typename Predicate> void` **`con_remove_if`**`(Container& con, Predicate p)`
* `template <typename Container, typename Predicate> void` **`con_remove_if_not`**`(Container& con, Predicate p)`
* `template <typename Container> void` **`con_unique`**`(Container& con)`
* `template <typename Container, typename BinaryPredicate> void` **`con_unique`**`(Container& con, BinaryPredicate p)`
* `template <typename Container> void` **`con_sort_unique`**`(Container& con)`
* `template <typename Container, typename Compare> void` **`con_sort_unique`**`(Container& con, Compare cmp)`

These carry out the same algorithms as the similarly named STL functions,
except that unwanted elements are removed from the container rather than
shuffled to the end. The `con_sort_unique()` functions perform a sort followed
by removing equivalent elements from the container; like `std::sort()`, its
predicate has less-than semantics (whereas that of `con_unique()`, like that
of `std::unique()`, has equality semantics).

* `template <typename F> void` **`do_n`**`(size_t n, F f)`
* `template <typename F> void` **`for_n`**`(size_t n, F f)`

These simply call the function `n` times. The `do_n()` loop expects a function
that takes no arguments, while `for_n()` passes the iteration count (zero to
`n-1`) as an argument.

* `template <typename M, typename K> M::mapped_type` **`find_in_map`**`(const M& map, const K& key)`
* `template <typename M, typename K, typename T> M::mapped_type` **`find_in_map`**`(const M& map, const K& key, const T& def)`

Find a key in a map, returning the corresponding value, or the supplied
default value (or a default constructed `mapped_type`) if the key is not
found.

* `template <typename Range1, typename Range2> bool` **`sets_intersect`**`(const Range1& r1, const Range2& r2)`
* `template <typename Range1, typename Range2, typename Compare> bool` **`sets_intersect`**`(const Range1& r1, const Range2& r2, Compare c)`

Test whether two ordered ranges (sets) have any elements in common. The ranges
are assumed to be ordered in accordance with the comparison predicate.

* `template <typename Range> [value type]` **`sum_of`**`(const Range& r)`
* `template <typename Range> [value type]` **`product_of`**`(const Range& r)`

Simple range sum and product functions, to save the trouble of calling
`std::accumulate()` in trivial cases. Calling `sum_of()` on an empty range
will return a default constructed value. The value type of `product_of()` must
be constructible by a `static_cast` from `int`; an empty range will return 1.

### Integer sequences ###

* `template <typename T> Irange<[random access iterator]>` **`iseq`**`(T stop) noexcept`
* `template <typename T> Irange<[random access iterator]>` **`iseq`**`(T init, T stop) noexcept`
* `template <typename T> Irange<[random access iterator]>` **`iseq`**`(T init, T stop, T delta) noexcept`
* `template <typename T> Irange<[random access iterator]>` **`xseq`**`(T stop) noexcept`
* `template <typename T> Irange<[random access iterator]>` **`xseq`**`(T init, T stop) noexcept`
* `template <typename T> Irange<[random access iterator]>` **`xseq`**`(T init, T stop, T delta) noexcept`

Linear sequences of integers (`T` must be an integer type). The generated
range starts with `init` (defaulting to zero) and runs to `stop`, in steps of
`delta` (defaulting to 1). The `iseq()` function produces an inclusive
(closed) sequence that includes `stop`, while `xseq()` is an exclusive (half
open) sequence that excludes it (if the exact value `stop` would never be
generated, because `stop-init` is not a multiple of `delta`, there is no
difference between the two).

If `stop=init`, or if `delta=0`, the `iseq()` sequence contains a single
value, while the `xseq()` sequence is empty. Both sequences will be empty if
`stop-init` and `delta` have opposite signs.

The iterators must be able to generate a value beyond the end of the sequence
in order to check for an end iterator; for `iseq()`, this means that
`stop+delta` must be in range for `T`.

### Memory algorithms ###

* `int` **`mem_compare`**`(const void* lhs, size_t n1, const void* rhs, size_t n2) noexcept`

This returns -1 if the first block is less than the second (by a
lexicographical bytewise comparison), 0 if they are equal, 1 if the first is
greater than the second. If the two blocks are equal up to the length of the
shorter one, the shorter is considered less than the longer. This is similar
to `memcmp()`, except that the memory blocks being compared may have different
lengths. A null pointer is treated as less than any non-null pointer; the
length accompanying a null pointer is ignored.

* `size_t` **`mem_match`**`(const void* lhs, const void* rhs, size_t n) noexcept`

Returns the number of leading bytes the two blocks have in common. This will
return zero if either pointer is null.

* `void` **`mem_swap`**`(void* ptr1, void* ptr2, size_t n) noexcept`

Swap two blocks of memory. This will work if the two ranges overlap or are the
same, but behaviour is undefined if either pointer is null.

### Range traits ###

* `template <typename Range> using` **`RangeIterator`** `= ...`
* `template <typename Range> using` **`RangeValue`** `= ...`

The iterator and value types of a range.

* `template <typename T, size_t N> constexpr size_t` **`array_count`**`(T[N]) noexcept`
* `template <typename Range> size_t` **`range_count`**`(const Range& r)`
* `template <typename Range> bool` **`range_empty`**`(const Range& r)`

Return the length of a range. The `range_count()` function is just shorthand
for `std::distance(begin(r),end(r))`, and `range_empty()` has the obvious
meaning. The `array_count()` version returns the same value, but only works on
C-style arrays and is `constexpr`.

### Range types ###

* `template <typename Iterator> struct` **`Irange`**
    * `using Irange::`**`iterator`** `= Iterator`
    * `using Irange::`**`value_type`** `= [Iterator's value type]`
    * `Iterator Irange::`**`first`**
    * `Iterator Irange::`**`second`**
    * `constexpr Iterator Irange::`**`begin`**`() const { return first; }`
    * `constexpr Iterator Irange::`**`end`**`() const { return second; }`
    * `constexpr bool Irange::`**`empty`**`() const { return first == second; }`
    * `constexpr size_t Irange::`**`size`**`() const { return std::distance(first, second); }`
* `template <typename Iterator> constexpr Irange<Iterator>` **`irange`**`(const Iterator& i, const Iterator& j)`
* `template <typename Iterator> constexpr Irange<Iterator>` **`irange`**`(const pair<Iterator, Iterator>& p)`

A wrapper for a pair of iterators, usable as a range in standard algorithms.

* `template <typename T> constexpr Irange<T*>` **`array_range`**`(T* ptr, size_t len)`

Returns `irange(ptr,ptr+len)`.

## Arithmetic functions ##

### Generic arithmetic functions ###

* `template <typename T, typename... Args> constexpr T` **`static_min`**`(T t, Args... args) noexcept`
* `template <typename T, typename... Args> constexpr T` **`static_max`**`(T t, Args... args) noexcept`

Compile time maximum and minimum functions.

* `template <typename T, typename T2, typename T3> constexpr T` **`clamp`**`(const T& x, const T2& min, const T3& max) noexcept`

Clamps a value to a fixed range. This returns `static_cast<T>(min)` if
`t<min`, `static_cast<T>(max)` if `t>max`, otherwise `t`.

* `template <typename T> pair<T, T>` **`divide`**`(T x, T y) noexcept`
* `template <typename T> T` **`quo`**`(T x, T y) noexcept`
* `template <typename T> T` **`rem`**`(T x, T y) noexcept`

These perform floor division operations: if the division is not exact, the
remainder is always positive (regardless of the signs of the arguments), and
the quotient is the integer that satisfies `x=q*y+r` (apart from rounding
errors). The pair returned by `divide()` contains the quotient and remainder.
For all three functions, behaviour is undefined if `y=0` or the quotient is
out of `T`'s representable range.

* `template <typename T> std::pair<T, T>` **`symmetric_divide`**`(T x, T y) noexcept`
* `template <typename T> T` **`symmetric_quotient`**`(T x, T y) noexcept`
* `template <typename T> T` **`symmetric_remainder`**`(T x, T y) noexcept`

These perform the same operations as the division functions above, except that
the remainder is constrained to `(-|y|/2,|y|/2]` instead of `[0,|y|)`.
Behaviour is undefined if `y=0` or the quotient is out of `T`'s representable
range. These functions do not make sense (and will not compile) if `T` is
unsigned.

* `template <typename T> T` **`shift_left`**`(T t, int n) noexcept`
* `template <typename T> T` **`shift_right`**`(T t, int n) noexcept`

Multiply or divide `t` by <code>2<sup>n</sup></code>. `T` may be an integer or
floating point type. Results are unspecified if the correct result would be
out of `T`'s representable range.

### Integer arithmetic functions ###

* `template <typename T> constexpr std::make_signed_t<T>` **`as_signed`**`(T t) noexcept`
* `template <typename T> constexpr std::make_unsigned_t<T>` **`as_unsigned`**`(T t) noexcept`

These return their argument converted to a signed or unsigned value of the
same size (the argument is returned unchanged if `T` already had the desired
signedness). Behaviour is undefined if `T` is not an integer or enumeration
type.

* `template <typename T> T` **`binomial`**`(T a, T b) noexcept`
* `double` **`xbinomial`**`(int a, int b) noexcept`

These return the binomial coefficient (`a!/b!(a-b)!` if `0<=b<=a`, otherwise
zero). `T` must be an integer type. Behaviour is undefined if the correct
result would be out of range for the return type.

* `template <typename T> constexpr T` **`gcd`**`(T a, T b) noexcept`
* `template <typename T> constexpr T` **`lcm`**`(T a, T b) noexcept`

Return the greatest common divisor or lowest common multiple of two numbers.
The argument type must be an integer. If one argument is zero, `gcd()` will
return the other one, and `lcm()` will return zero. The return value is always
positive (if not zero), regardless of the signs of the arguments. If the
correct result for `lcm()` would be outside the range of `T`, behaviour is
undefined if `T` is signed, unspecified if `T` is unsigned.

* `template <typename T> T` **`int_power`**`(T x, T y) noexcept`

Calculates <code>x<sup>y</sup></code> for integer types.
<code>0<sup>0</sup></code> will return 1. Behaviour is undefined if `y` is
negative, or if `T` is signed and the true result would be out of range.

* `template <typename T> T` **`int_sqrt`**`(T t) noexcept`

Returns the integer square root of the argument (the true square root
truncated to an integer). Behaviour is undefined if the argument is negative.

### Bitwise operations ###

For all functions in this section that are templated on the argument type,
results are unspecified if the argument is negative, and the function will
fail to compile if the argument is not an integer.

* `template <typename T> constexpr int` **`ibits`**`(T t) noexcept`

Returns the umber of 1 bits in the argument.

* `template <typename T> constexpr T` **`ifloor2`**`(T t) noexcept`
* `template <typename T> constexpr T` **`iceil2`**`(T t) noexcept`

Return the argument rounded down or up to a power of 2. The result of
`iceil2()` is unspecified if the correct answer is not representable.

* `template <typename T> constexpr int` **`ilog2p1`**`(T t) noexcept`

Returns `floor(log2(t))+1`, equal to the number of significant bits in `t`,
or zero if `t` is zero.

* `template <typename T> constexpr bool` **`ispow2`**`(T t) noexcept`

True if the argument is an exact power of 2.

* `constexpr uint64_t` **`letter_to_mask`**`(char c) noexcept`

Converts a letter to a mask with bit 0-51 set (corresponding to `[A-Za-z]`.
Returns zero if the argument is not an ASCII letter.

* `template <typename T> constexpr T` **`rotl`**`(T t, int n) noexcept`
* `template <typename T> constexpr T` **`rotr`**`(T t, int n) noexcept`

Bitwise rotate left or right. The bit count is reduced modulo the number of
bits in `T`; a negative shift in one direction is treated as a positive shift
in the other.

### Floating point arithmetic functions ###

* `template <typename T> constexpr T` **`degrees`**`(T rad) noexcept`
* `template <typename T> constexpr T` **`radians`**`(T deg) noexcept`

Convert between degrees and radians.

* `template <typename T1, typename T2> constexpr T2` **`interpolate`**`(T1 x1, T2 y1, T1 x2, T2 y2, T1 x) noexcept`

Returns the value of `y` corresponding to `x`, by interpolating or
extrapolating the line between `(x1,y1)` and `(x2,y2)`. If `x1=x2` it will
return the average of `y1` and `y2`. The usual arithmetic rules apply if the
result is not representable by the return type.

* `template <typename T2, typename T1> T2` **`iceil`**`(T1 value) noexcept`
* `template <typename T2, typename T1> T2` **`ifloor`**`(T1 value) noexcept`
* `template <typename T2, typename T1> T2` **`iround`**`(T1 value) noexcept`

Rounds the argument toward positive infinity (`iceil()`), toward negative
infinity (`ifloor()`), or to the nearest integer (`iround()`; halves round
up), converting the result to the specified return type. The usual arithmetic
rules apply if the result is not representable by the return type.

## Functional utilities ##

### Function traits ###

* `template <typename Function> struct` **`Arity`**
    * `static constexpr size_t Arity::`**`value`**
* `template <typename Function> using` **`ArgumentTuple`** `= [tuple type]`
* `template <typename Function, size_t Index> using` **`ArgumentType`** `= [type of given argument]`
* `template <typename Function> using` **`ReturnType`** `= [return type]`
* `template <typename Function> using` **`FunctionSignature`** `= [result(args...)]`
* `template <typename Function> using` **`StdFunction`** `= function<FunctionSignature<Function>>`

Properties of a function type. This will work with ordinary functions (if not
overloaded), function pointers, function objects, lambda expressions, and
`std::function` instantiations.

### Function operations ###

* `template <typename Function> StdFunction<Function>` **`stdfun`**`(Function& f)`

Wraps a function in the appropriate standard function type.

* `template<typename Function, typename Tuple> decltype(auto)` **`tuple_invoke`**`(Function&& f, Tuple&& t)`

Calls a function, passing a tuple as the argument list.

### Generic function objects ###

* `struct` **`DoNothing`**
    * `void` **`operator()`**`() const noexcept {}`
    * `template <typename T> void` **`operator()`**`(T&) const noexcept {}`
    * `template <typename T> void` **`operator()`**`(const T&) const noexcept {}`
* `constexpr DoNothing` **`do_nothing`**
* `struct` **`Identity`**
    * `template <typename T> T&` **`operator()`**`(T& t) const noexcept { return t; }`
    * `template <typename T> const T&` **`operator()`**`(const T& t) const noexcept { return t; }`
* `constexpr Identity` **`identity`**

Trivial function objects.

### Hash functions ###

* `class` **`Djb2a`**
    * `Djb2a&` **`Djb2a`**`::operator()(const void* ptr, size_t n) noexcept`
    * `Djb2a&` **`Djb2a`**`::operator()(const string& s) noexcept`
    * `Djb2a::`**`operator uint32_t`**`() const noexcept`
* `uint32_t` **`djb2a`**`(const void* ptr, size_t n) noexcept`

A simple, efficient hash algorithm for a string of bytes.

* `template <typename... Args> size_t` **`hash_value`**`(const Args&... args) noexcept`
* `template <typename... Args> void` **`hash_combine`**`(size_t& hash, const Args&... args) noexcept`
* `template <typename Range> size_t` **`hash_range`**`(const Range& range) noexcept`
* `template <typename Range> void` **`hash_range`**`(size_t& hash, const Range& range) noexcept`

Functions for combining hashes incrementally, or for generating the hash of a
number of objects in one call, for use in implementing hash functions for
compound types. The first version of each pair of functions (`hash_value()`
and `hash_combine()` have different names to avoid overload resolution
problems) calculates the hash of the supplied data; the second version takes
an existing hash value and mixes it with additional data. All of these call
the element type's `std::hash`.

* `template <typename... Args> struct` **`TupleHash`**
    * `size_t` **`operator()`**`(const tuple<Args...>& t) const`

Hash function for a tuple.

### Keyword arguments ###

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

### Scope guards ###

* `template <typename T> class` **`Resource`**
    * `using Resource::`**`resource_type`** `= T`
    * `using Resource::`**`value_type`** `= [dereferenced T]` _[only if T is a pointer]_
    * `Resource::`**`Resource`**`() noexcept`
    * `explicit Resource::`**`Resource`**`(T t)`
    * `template <typename D> Resource::`**`Resource`**`(T t, D d)`
    * `Resource::`**`Resource`**`(Resource&& r) noexcept`
    * `Resource::`**`~Resource`**`() noexcept`
    * `Resource& Resource::`**`operator=`**`(Resource&& r) noexcept`
    * `Resource::`**`operator T&`**`() noexcept`
    * `Resource::`**`operator T`**`() const noexcept`
    * `explicit Resource::`**`operator bool`**`() const noexcept`
    * `bool Resource::`**`operator!`**`() const noexcept`
    * `value_type& Resource::`**`operator*`**`() noexcept` _[only if T is a non-void pointer]_
    * `const value_type& Resource::`**`operator*`**`() const noexcept` _[only if T is a non-void pointer]_
    * `T Resource::`**`operator->`**`() const noexcept` _[only if T is a non-void pointer]_
    * `T& Resource::`**`get`**`() noexcept`
    * `T Resource::`**`get`**`() const noexcept`
    * `T Resource::`**`release`**`() noexcept`
* `template <typename T, typename D> Resource<T>` **`make_resource`**`(T t, D d)`

This holds a resource of some kind, and a deleter function that will be called
on destruction, similar to a `unique_ptr` (but without the requirement that
the resource type be a pointer, or that the deleter type be specified
explicitly in the class template). The deleter function passed to the
constructor is expected to take a single argument of type `T`; it defaults to
a null function. The destructor will call `d(t)`, unless `release()` has been
called, the resource object has been moved from, or the deleter is null. The
constructor will call `d(t)` if anything goes wrong (in practise this can only
happen if copying `D` throws).

The template is specialized for pointer types; if `T` is a pointer, the
deleter will never be called if the resource pointer is null. The `bool`
conversion and `!` operator have their usual meaning for pointers; for other
types, they will fail to compile if `T` does not have an explicit conversion
to `bool`. Behaviour is otherwise the same for pointer and non-pointer types.

* `class` **`ScopeExit`**
    * `using ScopeExit::`**`callback`** `= function<void()>`
    * `explicit ScopeExit::`**`ScopeExit`**`(callback f)`
    * `ScopeExit::`**`~ScopeExit`**`() noexcept`
    * `void ScopeExit::`**`release`**`() noexcept`
* `class` **`ScopeSuccess`**
    * `using ScopeSuccess::`**`callback`** `= function<void()>`
    * `explicit ScopeSuccess::`**`ScopeSuccess`**`(callback f)`
    * `ScopeSuccess::`**`~ScopeSuccess`**`() noexcept`
    * `void ScopeSuccess::`**`release`**`() noexcept`
* `class` **`ScopeFailure`**
    * `using ScopeFailure::`**`callback`** `= function<void()>`
    * `explicit ScopeFailure::`**`ScopeFailure`**`(callback f)`
    * `ScopeFailure::`**`~ScopeFailure`**`() noexcept`
    * `void ScopeFailure::`**`release`**`() noexcept`

These store a function object, to be called when the guard is destroyed
(passing `nullptr` will just do nothing). `ScopeExit` calls the function
unconditionally, `ScopeSuccess` calls it only on normal exit (not when
unwinding due to an exception), and `ScopeFailure` calls it only when an
exception causes stack unwinding (not on normal exit). If the constructor
throws an exception (this is only possible if the function object's copy
constructor or assignment operator throws), `ScopeExit` and `ScopeFailure`
will call the function before propagating the exception, while `ScopeSuccess`
will not. Any exceptions thrown by the function call in the destructor are
silently ignored (normally the function should be written so as not to throw
anything).

The `release()` function discards the saved function; after it is called, the
scope guard object will do nothing on destruction.

* `class` **`SizeGuard`**`: public ScopeFailure`
    * `template <typename T> explicit SizeGuard::`**`SizeGuard`**`(T& t)`

Saves the current size of a container, and restores it if an exception unwinds
the stack. `T` must be a container type with `size()` and `resize()`
functions.

* `class` **`ValueGuard`**`: public ScopeFailure`
    * `template <typename T> explicit ValueGuard::`**`ValueGuard`**`(T& t)`

Saves the current value of a variable, and restores it if an exception unwinds
the stack.

* `class` **`ScopedTransaction`**
    * `using ScopedTransaction::`**`callback`** `= function<void()>`
    * `ScopedTransaction::`**`ScopedTransaction`**`() noexcept`
    * `ScopedTransaction::`**`~ScopedTransaction`**`() noexcept`
    * `void ScopedTransaction::`**`call`**`(callback func, callback undo)`
    * `void ScopedTransaction::`**`commit`**`() noexcept`
    * `void ScopedTransaction::`**`rollback`**`() noexcept`

This holds a stack of "undo" operations, to be carried out if anything goes
wrong during a sequence of operations. The `call()` function accepts two
function objects; `func()` is called immediately, while `undo()` is saved on
the stack. If `func()` throws an exception, its `undo()` is not called, but
any pre-existing undo stack is invoked. Either function can be a null pointer
if no action is required.

Calling `commit()` discards the saved undo functions; `rollback()` calls them
in reverse order of insertion, silently ignoring any exceptions (normally the
undo functions should be written so as not to throw anything). The destructor
will perform a rollback if neither `commit()` nor `rollback()` have been
called since the last `call()`.

A single `ScopedTransaction` object can be used for multiple transactions.
Once `commit()` or `rollback()` is called, the undo stack is discarded, and
any newly added function pairs become part of a new cycle, equivalent to a
newly constructed `ScopedTransaction`.

## I/O utilities ##

### File I/O operations ###

Functions that take a file name have overloads that take a UTF-16 string in
native Windows builds (not on Cygwin, where the `_wfopen()` function is not
available). In all of these functions, passing an empty string or `"-"` as the
file name will cause the function to read from standard input or write to
standard output.

* `bool` **`load_file`**`(const string& file, string& dst, size_t limit = npos)`
* `bool` **`load_file`**`(const wstring& file, string& dst, size_t limit = npos)` _(Native Windows only)_

Read a file's contents into a string. Optionally, a maximum number of bytes
can be specified The return value is true if everything went well, false if
there was an error while opening or reading the file. If the function returns
false, the destination string will be cleared.

* `bool` **`save_file`**`(const string& file, const void* ptr, size_t n, bool append = false)`
* `bool` **`save_file`**`(const string& file, const string& src, bool append = false)`
* `bool` **`save_file`**`(const wstring& file, const void* ptr, size_t n, bool append = false)` _(Native Windows only)_
* `bool` **`save_file`**`(const wstring& file, const string& src, bool append = false)` _(Native Windows only)_

Write a string's contents into a file, optionally appending it to the file
instead of overwriting it if it already exists (if the file does not exist, or
is standard output, the `append` flag has no effect). The return value is true
if everything went well, false if there was an error while opening or writing
the file.

### Logging ###

* `void` **`logx`**`(const U8string& msg) noexcept`
* `void` **`logx`**`(const char* msg) noexcept`
* `template <typename... Args> void` **`logx`**`(Args... args) noexcept`
* `void` **`logt`**`(const U8string& msg) noexcept`
* `void` **`logt`**`(const char* msg) noexcept`
* `template <typename... Args> void` **`logt`**`(Args... args) noexcept`

These write a message to standard output, followed by a line feed and an
output flush. If multiple arguments are supplied, they are delimited with a
space. The `logt()` functions prefix each message with the time (in UTC).

These functions are intended mainly for use in multithreaded code. A private
mutex is used to ensure that messages from different threads are not
interleaved. Output from different threads will be given different colours,
chosen at random based on a hash of the thread ID. Because these are intended
only for debugging the code around them, any exceptions thrown by their
internal workings are silently ignored.

* `class` **`Stopwatch`**
    * `explicit Stopwatch::`**`Stopwatch`**`(const U8string& name, int precision = 3) noexcept`
    * `explicit Stopwatch::`**`Stopwatch`**`(const char* name, int precision = 3) noexcept`
    * `Stopwatch::`**`~Stopwatch`**`() noexcept`

Simple timer for debugging. The destructor will write (using `logx()`) the
name and the elapsed time since construction. The note on exceptions for
`logx()` above applies here too.

### Process I/O operations ###

`std::string` **`run_command`**`(const U8string& cmd)`

Runs a shell command synchronously (via `popen()` or the equivalent),
capturing standard output. No error indication is returned; standard error
still goes to its usual destination, unless explicitly redirected in the
command line.

### Terminal I/O operations ###

* `bool` **`is_stdout_redirected`**`() noexcept`

Attempts to detect whether standard output has been redirected to a file or
pipe (true), or is going directly to a terminal (false). This is not always
possible to detect reliably; this function is fairly reliable on Unix, less so
on Windows.

* `constexpr const char*` **`xt_up`** `=           <esc> [A    = Cursor up`
* `constexpr const char*` **`xt_down`** `=         <esc> [B    = Cursor down`
* `constexpr const char*` **`xt_right`** `=        <esc> [C    = Cursor right`
* `constexpr const char*` **`xt_left`** `=         <esc> [D    = Cursor left`
* `constexpr const char*` **`xt_erase_left`** `=   <esc> [1K   = Erase left`
* `constexpr const char*` **`xt_erase_right`** `=  <esc> [K    = Erase right`
* `constexpr const char*` **`xt_erase_above`** `=  <esc> [1J   = Erase above`
* `constexpr const char*` **`xt_erase_below`** `=  <esc> [J    = Erase below`
* `constexpr const char*` **`xt_erase_line`** `=   <esc> [2K   = Erase line`
* `constexpr const char*` **`xt_clear`** `=        <esc> [2J   = Clear screen`
* `constexpr const char*` **`xt_reset`** `=        <esc> [0m   = Reset attributes`
* `constexpr const char*` **`xt_bold`** `=         <esc> [1m   = Bold`
* `constexpr const char*` **`xt_under`** `=        <esc> [4m   = Underline`
* `constexpr const char*` **`xt_bold_off`** `=     <esc> [22m  = Bold off`
* `constexpr const char*` **`xt_under_off`** `=    <esc> [24m  = Underline off`
* `constexpr const char*` **`xt_colour_off`** `=   <esc> [39m  = Colour off`
* `constexpr const char*` **`xt_black`** `=        <esc> [30m  = Black foreground`
* `constexpr const char*` **`xt_red`** `=          <esc> [31m  = Red foreground`
* `constexpr const char*` **`xt_green`** `=        <esc> [32m  = Green foreground`
* `constexpr const char*` **`xt_yellow`** `=       <esc> [33m  = Yellow foreground`
* `constexpr const char*` **`xt_blue`** `=         <esc> [34m  = Blue foreground`
* `constexpr const char*` **`xt_magenta`** `=      <esc> [35m  = Magenta foreground`
* `constexpr const char*` **`xt_cyan`** `=         <esc> [36m  = Cyan foreground`
* `constexpr const char*` **`xt_white`** `=        <esc> [37m  = White foreground`
* `constexpr const char*` **`xt_black_bg`** `=     <esc> [40m  = Black background`
* `constexpr const char*` **`xt_red_bg`** `=       <esc> [41m  = Red background`
* `constexpr const char*` **`xt_green_bg`** `=     <esc> [42m  = Green background`
* `constexpr const char*` **`xt_yellow_bg`** `=    <esc> [43m  = Yellow background`
* `constexpr const char*` **`xt_blue_bg`** `=      <esc> [44m  = Blue background`
* `constexpr const char*` **`xt_magenta_bg`** `=   <esc> [45m  = Magenta background`
* `constexpr const char*` **`xt_cyan_bg`** `=      <esc> [46m  = Cyan background`
* `constexpr const char*` **`xt_white_bg`** `=     <esc> [47m  = White background`

Xterm cursor movement and output formatting codes.

* `string` **`xt_move_up`**`(int n)      = <esc> [<n>A             = Cursor up n spaces`
* `string` **`xt_move_down`**`(int n)    = <esc> [<n>B             = Cursor down n spaces`
* `string` **`xt_move_right`**`(int n)   = <esc> [<n>C             = Cursor right n spaces`
* `string` **`xt_move_left`**`(int n)    = <esc> [<n>D             = Cursor left n spaces`
* `string` **`xt_colour`**`(int rgb)     = <esc> [38;5;<16-231>m   = Set foreground colour to an RGB value`
* `string` **`xt_colour_bg`**`(int rgb)  = <esc> [48;5;<16-231>m   = Set background colour to an RGB value`
* `string` **`xt_grey`**`(int grey)      = <esc> [38;5;<232-255>m  = Set foreground colour to a grey level`
* `string` **`xt_grey_bg`**`(int grey)   = <esc> [48;5;<232-255>m  = Set background colour to a grey level`

Functions for generating variable Xterm codes. The RGB levels passed to the
`xt_colour[_bg]()` functions are in the form of a 3 digit number from 111 to
666; each digit is clamped to a range of 1-6, yielding 216 different colours.
The grey level passed to the `xt_grey[_bg]()` functions is clamped to a range
of 1-24.

## Random numbers ##

### Simple random generators ###

* `template <typename RNG> bool` **`random_bool`**`(RNG& rng)` _- True with probability 1/2_
* `template <typename RNG> bool` **`random_bool`**`(RNG& rng, double p)` _- True with probability p (clamped to 0-1)_
* `template <typename RNG, typename T> bool` **`random_bool`**`(RNG& rng, T num, T den)` _- True with probability num/den (clamped to 0-1)_
* `template <typename T, typename RNG> T` **`random_integer`**`(RNG& rng, T t)` _- Random integer from 0 to t-1 (always 0 if t<2)_
* `template <typename T, typename RNG> T` **`random_integer`**`(RNG& rng, T a, T b)` _- Random integer from a to b inclusive (limits may be in either order)_
* `template <typename T, typename RNG> T` **`random_dice`**`(RNG& rng, T n = 1, T faces = 6)` _- Roll n dice numbered from 1 to faces (0 if either argument is <1)_
* `template <typename T, typename RNG> T` **`random_float`**`(RNG& rng, T a = 1, T b = 0)` _- Random number between a and b (limits may be in either order)_
* `template <typename T, typename RNG> T` **`random_normal`**`(RNG& rng)` _- Normal distribution with mean 0, sd 1_
* `template <typename T, typename RNG> T` **`random_normal`**`(RNG& rng, T m, T s)` _- Normal distribution with given mean and sd_
* `template <typename Range, typename RNG> [value type]` **`random_choice`**`(RNG& rng, const Range& range)` _- Random element from range (default constructed value if range is empty)_
* `template <typename T, typename RNG> T` **`random_choice`**`(RNG& rng, initializer_list<T> list)` _- Random element from explicit list_

Simple random number functions. These do not call the standard distribution
classes; given the same underlying random number engine (the `RNG` type),
these can be relied on to return the same values on different systems and
compilers (apart from small differences due to rounding errors in the floating
point functions).

## Strings and related functions ##

### Character functions ###

* `constexpr bool` **`ascii_isalnum`**`(char c) noexcept`
* `constexpr bool` **`ascii_isalpha`**`(char c) noexcept`
* `constexpr bool` **`ascii_iscntrl`**`(char c) noexcept`
* `constexpr bool` **`ascii_isdigit`**`(char c) noexcept`
* `constexpr bool` **`ascii_isgraph`**`(char c) noexcept`
* `constexpr bool` **`ascii_islower`**`(char c) noexcept`
* `constexpr bool` **`ascii_isprint`**`(char c) noexcept`
* `constexpr bool` **`ascii_ispunct`**`(char c) noexcept`
* `constexpr bool` **`ascii_isspace`**`(char c) noexcept`
* `constexpr bool` **`ascii_isupper`**`(char c) noexcept`
* `constexpr bool` **`ascii_isxdigit`**`(char c) noexcept`

These are simple ASCII-only versions of the standard character type functions.
All of them will always return false for bytes outside the ASCII range
(0-127).

* `constexpr bool` **`ascii_isalnum_w`**`(char c) noexcept`
* `constexpr bool` **`ascii_isalpha_w`**`(char c) noexcept`
* `constexpr bool` **`ascii_ispunct_w`**`(char c) noexcept`

These behave the same as the corresponding functions without the `"_w"`
suffix, except that the underscore character is counted as a letter instead of
a punctuation mark. (The suffix is intended to suggest the `"\w"` regex
element, which does much the same thing.)

* `constexpr char` **`ascii_tolower`**`(char c) noexcept`
* `constexpr char` **`ascii_toupper`**`(char c) noexcept`

Simple ASCII-only case conversion functions. All non-ASCII characters are left
unchanged.

* `template <typename T> constexpr T` **`char_to`**`(char c) noexcept`

A simple conversion function that casts a `char` to a larger integer type by
first passing it through `unsigned char`, to ensure that characters with the
high bit set end up as integers in the 128-255 range, and not as negative or
extremely high values.

### General string functions ###

* `bool` **`ascii_icase_equal`**`(const string& lhs, const string& rhs) noexcept`
* `bool` **`ascii_icase_less`**`(const string& lhs, const string& rhs) noexcept`

Comparison functions that treat upper and lower case ASCII letters as
equivalent (comparison between letters and non-letters uses the upper case
code points).

* `string` **`ascii_lowercase`**`(const string& s)`
* `string` **`ascii_uppercase`**`(const string& s)`
* `string` **`ascii_titlecase`**`(const string& s)`
* `string` **`ascii_sentencecase`**`(const string& s)`

Simple ASCII-only case conversion functions. All non-ASCII characters are left
unchanged. The sentence case function capitalizes the first letter of every
sentence (delimited by a full stop or two consecutive line breaks), leaving
everything else alone.

* `template <typename C> basic_string<C>` **`cstr`**`(const C* ptr)`
* `template <typename C> basic_string<C>` **`cstr`**`(const C* ptr, size_t n)`

These construct a string from a pointer to a null-terminated character
sequence, or a pointer and a length. They differ from the corresponding string
constructors in that passing a null pointer will yield an empty string, or a
string of `n` null characters, instead of undefined behaviour.

* `template <typename C> size_t` **`cstr_size`**`(const C* ptr)`

Returns the length of a null-terminated string (a generalized version of
`strlen()`). This will return zero if the pointer is null.

* `U8string` **`dent`**`(size_t depth)`

Returns a string containing `4*depth` spaces, for indentation.

* `template <typename InputRange> string` **`join`**`(const InputRange& range, const string& delim = "", bool term = false)`
* `template <typename OutputIterator> void` **`split`**`(const string& src, OutputIterator dst, const string& delim = ascii_whitespace)`
* `vector<string>` **`splitv`**`(const string& src, const string& delim = ascii_whitespace)`

Join strings into a single string, using the given delimiter, or split a
string into substrings, discarding any sequence of delimiter characters. The
`splitv()` version returns a vector of strings instead of writing to an
existing container. The value type of the `InputRange` or `OutputIterator`
must be assignment compatible with `string`. If the `term` argument to
`join()` is set, an extra delimiter will be added after the last element
(useful when joining lines to form a text that would be expected to end with a
line break).

* `std::string` **`linearize`**`(const std::string& str)`

Replaces all whitespace in a string with a single space, and trims leading and
trailing whitespace. All non-ASCII bytes are treated as non-whitespace
characters.

* `template <typename C> void` **`null_term`**`(basic_string<C>& str) noexcept`

Cuts off a string at the first null character (useful after the string has
been used as an output buffer by some C APIs).

* `U8string` **`quote`**`(const string& str)`
* `U8string` **`bquote`**`(const string& str)`

Return a quoted string; internal quotes, backslashes, and control characters
are escaped. The `bquote()` function always escapes all non-ASCII bytes;
`quote()` passes valid UTF-8 unchanged, but will switch to `bquote()` mode if
the string is not valid UTF-8.

* `bool` **`starts_with`**`(const string& str, const string& prefix) noexcept`
* `bool` **`ends_with`**`(const string& str, const string& suffix) noexcept`

True if the string starts or ends with the specified substring.

* `bool` **`string_is_ascii`**`(const string& str) noexcept`

True if the string contains no 8-bit bytes.

* `string` **`trim`**`(const string& str, const string& chars = ascii_whitespace)`
* `string` **`trim_left`**`(const string& str, const string& chars = ascii_whitespace)`
* `string` **`trim_right`**`(const string& str, const string& chars = ascii_whitespace)`

Trim unwanted bytes from the ends of a string.

* `string` **`unqualify`**`(const string& str, const string& delims = ".:")`

Strips off any prefix ending in one of the delimiter characters (e.g.
`unqualify("RS::unqualify()")` returns `"unqualify()"`). This will return the
original string unchanged if the delimiter string is empty or none of its
characters are found.

### String formatting and parsing functions ###

* `template <typename T> U8string` **`bin`**`(T x, size_t digits = 8 * sizeof(T))`
* `template <typename T> U8string` **`dec`**`(T x, size_t digits = 1)`
* `template <typename T> U8string` **`hex`**`(T x, size_t digits = 2 * sizeof(T))`
* `unsigned long long` **`binnum`**`(const string& str) noexcept`
* `long long` **`decnum`**`(const string& str) noexcept`
* `unsigned long long` **`hexnum`**`(const string& str) noexcept`
* `double` **`fpnum`**`(const string& str) noexcept`

Simple string and number conversion functions. The `bin()`, `dec()`, and
`hex()` functions convert an integer to a binary, decimal, or hexadecimal
string, generating at least the specified number of digits. The `binnum()`,
`decnum()`, and `hexnum()` functions perform the reverse conversion, while
`fpnum()` converts a string to a floating point number; these will ignore any
trailing characters that are not part of a number, and will return zero if the
string is empty or does not contain a valid number. Results that are out of
range will be clamped to the nearest end of the return type's range (for
`fpnum()` this will normally be positive or negative infinity).

* `template <typename... Args> U8string` **`fmt`**`(const U8string& pattern, const Args&... args)`

This performs string interpolation, inserting the variadic arguments
(formatted with `to_str()`, below) in place of each occurrence of `"$n"` or
`"${n}"` in the pattern string, where `n` is a decimal integer interpreted as
a 1-based index into the variadic argument list. An index out of bounds will
be replaced with an empty string. If a dollar sign is not followed by a bare
or braced number, the dollar sign is discarded and the next character is
copied unchanged (so `"$$"` will produce a literal dollar sign).

* `template <typename T> U8string` **`fp_format`**`(T t, char mode = 'g', int prec = 6)`

Simple floating point formatting, by calling `snprintf()`. `T` must be an
arithmetic type; it will be converted to `double` internally. The additional
format `'Z/z'` is the same as `'G/g'` except that trailing zeros are not
stripped. This will throw `std::invalid_argument` if the mode is not one of
`[EFGZefgz]`; it may throw `std::system_error` under implementation defined
circumstances.

* `int64_t` **`si_to_int`**`(const U8string& s)`
* `double` **`si_to_float`**`(const U8string& s)`

These parse a number from a string representation tagged with an SI multiplier
abbreviation (e.g. `"123k"`). For the integer version, only tags representing
positive powers of 1000 (starting with`"k"`) are recognised, and are case
insensitive. For the floating point version, all tags representing powers of
100 are recognised (`"u"` is used for "micro"), and are case sensitive, except
that `"K"` is equivalent to `"k"`. For both versions, a space is allowed
between the number and the tag, and any additional text after the number or
tag is ignored.

This will throw `std::invalid_argument` if the string does not start with a
valid number, or `std::range_error` if the result is too big for the return
type.

* `U8string` **`hexdump`**`(const void* ptr, size_t n, size_t block = 0)`
* `U8string` **`hexdump`**`(const string& str, size_t block = 0)`

Converts a block of raw data into hexadecimal bytes. If `block` is not zero, a
line feed is inserted after each block.

* `U8string` **`tf`**`(bool b)`
* `U8string` **`yn`**`(bool b)`

Convert a boolean to `"true/false"` or `"yes/no"`.

* `template <typename T> U8string` **`to_str`**`(const T& t)`

Formats an object as a string. This uses the following rules for formatting
various types:

* `std::string`, `char`, character pointers, and anything with an implicit
conversion to `string` - The string content is simply copied directly without
using an output stream; a null character pointer is treated as an empty
string.
* Integer types - Formatted using `dec()`.
* Floating point types - Formatted using `fp_format()`.
* `bool` - Written as `"true"` or `"false"`.
* Ranges (other than strings) - Serialized in a format similar to a JSON array
(e.g. `"[1,2,3]"`), or an object (e.g. `"{1:a,2:b,3:c}"`) if the range's value
type is a pair; `to_str()` is called recursively on each range element.
* Otherwise, the type's output operator will be called.

### HTML/XML tags ###

* `class` **`Tag`**
    * `Tag::`**`Tag`**`()`
    * `Tag::`**`Tag`**`(std::ostream& out, const std::string& element)`
    * `Tag::`**`~Tag`**`() noexcept`
    * `Tag::`**`Tag`**`(Tag&& t) noexcept`
    * `Tag& Tag::`**`operator=`**`(Tag&& t) noexcept`

This class writes an HTML/XML tag in its constructor, then writes the
corresponding closing tag in its destructor. If the supplied string ends with
one line feed, a line feed will be written after the closing tag, but not the
opening one; if it ends with two line feeds, one will be written after both
tags.

The opening tag can be supplied with or without enclosing angle brackets. The
constructor does not attempt any validation of the tag's format (except that
an empty tag will cause the class to do nothing); no promises are made about
the output if the `element` argument is not a valid HTML/XML tag.

If the opening tag is standalone, the text will simply be written as is, and
no closing tag will be written. Standalone tags are identified by a closing
slash; the class is not aware of HTML's list of automatic self closing tags.

* `template <typename... Args> void` **`tagged`**`(std::ostream& out, const std::string& element, const Args&... args)`
* `template <typename T> void` **`tagged`**`(std::ostream& out, const std::string& element, const T& t)`

This function can be used to write a piece of literal text enclosed in one or
more tags. The arguments are the output stream, a list of tags (using the same
format as the `Tag` class), and an object that will be written to the output
stream enclosed by the tags.

Example:

    tagged(std::cout, "p\n", "code", "Hello world");

Output:

    <p><code>Hello world</code></p>\n

### Unicode functions ###

* `template <typename S2, typename S1> S2` **`uconv`**`(const S1& s)`

Converts between UTF representations. The input and output types (`S1` and
`S2`) must be instantiations of `basic_string` with 8, 16, or 32 bit character
types. If the character types are the same size, the input string is copied to
the output without any validity checking; otherwise, invalid UTF in the input
is replaced with the standard Unicode replacement character (`U+FFFD`) in the
output.

* `template <typename C> bool` **`uvalid`**`(const basic_string<C>& s) noexcept`
* `template <typename C> bool` **`uvalid`**`(const basic_string<C>& s, size_t& n) noexcept`

Check a string for a valid UTF encoding; the encoding is deduced from the size
of the character type. The second version reports the number of code units
before an invalid code sequence was encountered (i.e. the size of a valid UTF
prefix; this will be equal to `s.size()` if the function returns true). These
will always succeed for an empty string.

## Threads ##

This section of the library exists because some of the compilers I need to
support don't implement C++11 threads yet.

### Thread class ###

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

### Synchronisation objects ###

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

## Time and date operations ##

In this documentation, the `std::chrono::` prefix on the standard date and
time types is left out for brevity. For all functions here that accept or
return a broken down date, results are unspecified if the date is outside the
range of a `time_t` (1970-2038 on 32-bit systems), or if the fields are set to
values that do not represent a valid date.

### Time and date types ###

* `using` **`Dseconds`** `= duration<double>`
* `using` **`Ddays`** `= duration<double, ratio<86400>>`
* `using` **`Dyears`** `= duration<double, ratio<31557600>>`

Floating point duration types.

* `using` **`ReliableClock`** `= [high_resolution_clock or steady_clock]`

This is the highest resolution clock that can be trusted to be steady. It will
be `high_resolution_clock` if `high_resolution_clock::is_steady` is true,
otherwise `steady_clock`.

### General time and date operations ###

* `enum class` **`Zone`**
    * `Zone::`**`utc`**
    * `Zone::`**`local`**

This is passed to some of the time and date functions to indicate whether a
broken down date is expressed in UTC or the local time zone. For all functions
that take a `Zone` argument, behaviour is unspecified if the argument is not
one of these two values.

* `template <typename R, typename P> void` **`from_seconds`**`(double s, duration<R, P>& d) noexcept`
* `template <typename R, typename P> double` **`to_seconds`**`(const duration<R, P>& d) noexcept`

Convenience functions to convert between a `duration` and a floating point
number of seconds.

* `system_clock::time_point` **`make_date`**`(int year, int month, int day, int hour = 0, int min = 0, double sec = 0, Zone z = Zone::utc) noexcept`

Converts a broken down date into a time point. Behaviour if any of the date
arguments are invalid follows the same rules as `mktime()`.

* `template <typename R, typename P> void` **`sleep_for`**`(duration<R, P> t) noexcept`
* `void` **`sleep_for`**`(double t) noexcept`
* `template <typename C, typename D> void` **`sleep_until`**`(time_point<C, D> t) noexcept`

Suspend the current thread for the specified interval, supplied as a duration,
a number of seconds, or an absolute time. Calling this with a duration less
than or equal to zero will cause the thread to yield its time slice.
Resolution is system dependent. Behaviour is undefined if the duration exceeds
31 days.

### Time and date formatting ###

* `U8string` **`format_date`**`(system_clock::time_point tp, int prec = 0, Zone z = Zone::utc)`
* `U8string` **`format_date`**`(system_clock::time_point tp, const U8string& format, Zone z = Zone::utc)`

These convert a time point into a broken down date and format it. The first
version writes the date in ISO 8601 format (`"yyyy-mm-dd hh:mm:ss"`). If
`prec` is greater than zero, the specified number of decimal places will be
added to the seconds field.

The second version writes the date using the conventions of `strftime()`. This
will return an empty string if anything goes wrong; there is no way to
distinguish between a conversion error and a legitimately empty result (this
is a limitation of `strftime()`).

For reference, the portable subset of the `strftime()` formatting codes are:

| Code  | Description                          | Code    | Description                          |
| ----  | -----------                          | ----    | -----------                          |
|       **Date elements**                      | |       **Weekday elements**                   | |
| `%Y`  | Year number                          | `%a`    | Local weekday abbreviation           |
| `%y`  | Last 2 digits of the year (`00-99`)  | `%w`    | Sunday-based weekday number (`0-6`)  |
| `%m`  | Month number (`00-12`)               | `%A`    | Local weekday name                   |
| `%B`  | Local month name                     | `%U`    | Sunday-based week number (`00-53`)   |
| `%b`  | Local month abbreviation             | `%W`    | Monday-based week number (`00-53`)   |
| `%d`  | Day of the month (`01-31`)           |         **Other elements**                     | |
|       **Time of day elements**               | | `%c`  | Local standard date/time format      |
| `%H`  | Hour on 24-hour clock (`00-23`)      | `%x`    | Local standard date format           |
| `%I`  | Hour on 12-hour clock (`01-12`)      | `%X`    | Local standard time format           |
| `%p`  | Local equivalent of a.m./p.m.        | `%j`    | Day of the year (`001-366`)          |
| `%M`  | Minute (`00-59`)                     | `%Z`    | Time zone name                       |
| `%S`  | Second (`00-60`)                     | `%z`    | Time zone offset                     |

* `template <typename R, typename P> U8string` **`format_time`**`(const duration<R, P>& time, int prec = 0)`

Formats a time duration in days, hours, minutes, seconds, and (if `prec>0`)
fractions of a second.

### System specific time and date conversions ###

* _Unix_
    * `template <typename R, typename P> timespec` **`duration_to_timespec`**`(const duration<R, P>& d) noexcept`
    * `template <typename R, typename P> timeval` **`duration_to_timeval`**`(const duration<R, P>& d) noexcept`
    * `timespec` **`timepoint_to_timespec`**`(const system_clock::time_point& tp) noexcept`
    * `timeval` **`timepoint_to_timeval`**`(const system_clock::time_point& tp) noexcept`
    * `template <typename R, typename P> void` **`timespec_to_duration`**`(const timespec& ts, duration<R, P>& d) noexcept`
    * `system_clock::time_point` **`timespec_to_timepoint`**`(const timespec& ts) noexcept`
    * `template <typename R, typename P> void` **`timeval_to_duration`**`(const timeval& tv, duration<R, P>& d) noexcept`
    * `system_clock::time_point` **`timeval_to_timepoint`**`(const timeval& tv) noexcept`
* _Windows_
    * `system_clock::time_point` **`filetime_to_timepoint`**`(const FILETIME& ft) noexcept`
    * `FILETIME` **`timepoint_to_filetime`**`(const system_clock::time_point& tp) noexcept`

Conversion functions between C++ chrono types and system API types. Some of
these return their result through a reference argument to avoid having to
explicitly specify the duration type. Behaviour is undefined if the value
being represented is out of range for either the source or destination type.

The Windows functions are only defined on Win32 builds; the Unix functions are
always defined (since the relevant time structures are defined in the Windows
API).

For reference, the system time types are:

* _Unix_
    * `#include <time.h>`
        * `struct` **`timespec`**
            * `time_t timespec::`**`tv_sec`** `// seconds`
            * `long timespec::`**`tv_nsec`** `// nanoseconds`
    * `#include <sys/time.h>`
        * `struct` **`timeval`**
            * `time_t timeval::`**`tv_sec`** `// seconds`
            * `suseconds_t timeval::`**`tv_usec`** `// microseconds`
* _Windows_
    * `#include <windows.h>`
        * `struct` **`FILETIME`**
            * `DWORD FILETIME::`**`dwLowDateTime`** `// low 32 bits`
            * `DWORD FILETIME::`**`dwHighDateTime`** `// high 32 bits`
