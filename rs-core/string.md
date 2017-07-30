# Core String Algorithms #

By Ross Smith

* `#include "rs-core/string.hpp"`

## Contents ##

[TOC]

## Character functions ##

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

## General string functions ##

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

* `U8string` **`dent`**`(size_t depth)`

Returns a string containing `4*depth` spaces, for indentation.

* `template <typename InputRange> string` **`join`**`(const InputRange& range, const string& delim = "", bool term = false)`
* `template <typename OutputIterator> void` **`split`**`(const string& src, OutputIterator dst, const string& delim = ascii_whitespace)`
* `Strings` **`splitv`**`(const string& src, const string& delim = ascii_whitespace)`

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

## String formatting and parsing functions ##

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

## HTML/XML tags ##

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

## Type names ##

* `std::string` **`demangle`**`(const std::string& name)`
* `std::string` **`type_name`**`(const std::type_info& t)`
* `std::string` **`type_name`**`(const std::type_index& t)`
* `template <typename T> std::string` **`type_name`**`()`
* `template <typename T> std::string` **`type_name`**`(const T& t)`

Demangle a type name. The original mangled name can be supplied as an explicit
string, as a `std::type_info` or `std:type_index` object, as a type argument
to a template function (e.g. `type_name<int>()`), or as an object whose type
is to be named (e.g. `type_name(42)`). The last version will report the
dynamic type of the referenced object.

## Unicode functions ##

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
