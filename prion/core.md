Title: Core Utilities
CSS: style.css

# Core Utilities #

By Ross Smith

* `#include "prion/core.hpp"`

On Microsoft Windows builds, including Prion will bring in the whole of
`<windows.h>`. This is annoying but unfortunately unavoidable for a library
that needs to make calls to system APIs while remaining header-only. Under
Cygwin, this will only happen if `_WIN32` is defined.

## Contents ##

* [Preprocessor macros][]
* [Basic types][]
* [Constants][]
* [Arithmetic literals][]
* [Arithmetic functions][]
* [Byte order][]
* [Character functions][]
* [Exceptions][]
* [Flag sets][]
* [Functional utilities][]
* [Hash functions][]
* [I/O utilities][]
* [Keyword arguments][]
* [Mixins][]
* [Range utilities][]
* [String functions][]
* [Threads][]
* [Time and date functions][]
* [Type properties][]
* [UUID][]
* [Version number][]

## Preprocessor macros ##

* `#define PRI_TARGET_UNIX 1`
    * `#define PRI_TARGET_DARWIN 1`
        * `#define PRI_TARGET_MACOSX 1`
        * `#define PRI_TARGET_IOS 1`
    * `#define PRI_TARGET_LINUX 1`
* `#define PRI_TARGET_WINDOWS 1`
    * `#define PRI_TARGET_MINGW 1`
    * `#define PRI_TARGET_NATIVE_WINDOWS 1`
* `#define PRI_TARGET_CYGWIN 1`

Some of these will be defined to provide a consistent way to identify the
target operating system for conditional compilation. At least one of
`PRI_TARGET_UNIX` or `PRI_TARGET_WINDOWS` will always be defined (both will be
defined on Cygwin if `_WIN32` has been defined).

On Apple platforms, `PRI_TARGET_DARWIN` will always be defined; one of
`PRI_TARGET_IOS` or `PRI_TARGET_MACOSX` will also be defined.

On Microsoft Windows, `PRI_TARGET_NATIVE_WINDOWS` will be defined on any build
not targeting Cygwin. Currently only Mingw builds are supported for native
Windows targets, so `PRI_TARGET_MINGW` will always be defined; I may add MSVC
support in a future version.

* `#define PRI_BOUNDS(range) std::begin(range), std::end(range)`

Convenience macro for inserting the beginning and end of a range into an
argument list.

* `#define PRI_LDLIB(libs)`

This instructs the makefile to link with one or more static libraries. Specify
library names without the `-l` prefix (e.g. `PRI_LDLIB(foo)` will link with
`-lfoo`). Libraries can be supplied as a space delimited list in a single
`PRI_LDLIB()` invocation, or in multiple invocations; link order is only
preserved within a single invocation. Libraries that are needed only on
specific targets can be prefixed with a target identifier, e.g.
`PRI_LDLIB(mac: foo bar)` will link with `-lfoo -lbar` only in Mac OS X
builds.

Tag        | Build target    | Corresponding macro
---        | ------------    | -------------------
`mac:`     | Mac OS X + iOS  | `PRI_TARGET_DARWIN`
`linux:`   | Linux           | `PRI_TARGET_LINUX`
`mingw:`   | Mingw           | `PRI_TARGET_MINGW`
`cygwin:`  | Cygwin          | `PRI_TARGET_CYGWIN`

Only one target can be specified per invocation; if the same libraries are
needed on multiple targets, but not on all targets, you will need a separate
`PRI_LDLIB()` line for each target.

* `#define PRI_STATIC_ASSERT(expr) static_assert((expr), # expr)`

Shorthand for `static_assert`, using the assertion expression as the error
message. (A planned change in C++17 will make this unnecessary.)

## Basic types ##

* `using std::basic_string`
* `using std::string`
* `using std::u16string`
* `using std::u32string`
* `using std::wstring`

Imported for convenience.

* `using u8string = std::string`

We use `u8string` for strings that are expected to be in UTF-8 (or ASCII,
since any ASCII string is also valid UTF-8), while plain `string` is used
where the string is expected to be in some non-Unicode encoding, or where the
string is being used simply as an array of bytes rather than encoded text.

* `using int128_t = __int128`
* `using uint128_t = unsigned __int128`

Named for consistency.

## Constants ##

* `constexpr const char* ascii_whitespace = "\t\n\v\f\r "`

ASCII whitespace characters.

* `constexpr size_t npos = string::npos`

Defined for convenience. Following the conventions established by the standard
library, this value is often used as a function argument to mean "as large as
possible", or as a return value to mean "not found".

* `#define PRI_DEFINE_CONSTANT(name, value)`
    * `static constexpr double name = value`
    * `static constexpr float name ## _f = value ## f`
    * `static constexpr long double name ## _ld = value ## l`
    * `template <typename T> constexpr T c_ ## name() noexcept`

Defines a floating point constant, in several forms. This generates a set of
three named constants (for the three standard floating point types), and a
function template that converts the long double value by default (with
specializations for the standard types). The `value` argument must be a
floating point literal with no type suffix.

* _Mathematical constants_
    * `e` _= 2.7183_
    * `ln2` _= log<sub>e</sub> 2 = 0.6931_
    * `ln10` _= log<sub>e</sub> 10 = 2.3026_
    * `pi` _= &pi; = 3.1416_
    * `sqrt2` _= &radic;2 = 1.4142_
    * `sqrt3` _= &radic;3 = 1.7321_
    * `sqrt5` _= &radic;5 = 2.2361_
    * `sqrt2pi` _= &radic;(2&pi;) = 2.5066_ -->
* _Physical constants_
    * `atomic_mass_unit` _= 1.661&times;10<sup>-27</sup> kg_
    * `avogadro_constant` _= 6.022&times;10<sup>23</sup> mol<sup>-1</sup>_
    * `boltzmann_constant` _= 1.381&times;10<sup>-23</sup> J K<sup>-1</sup>_
    * `elementary_charge` _= 1.602&times;10<sup>-19</sup> C_
    * `gas_constant` _= 8.314 J mol<sup>-1</sup> K<sup>-1</sup>_
    * `gravitational_constant` _= 6.674&times;10<sup>-11</sup> m<sup>3</sup> kg<sup>-1</sup> s<sup>-2</sup>_
    * `planck_constant` _= 6.626&times;10<sup>-34</sup> J s_
    * `speed_of_light` _= 2.998&times;10<sup>8</sup> m s<sup>-1</sup>_
    * `stefan_boltzmann_constant` _= 5.670&times;10<sup>-8</sup> W m<sup>-2</sup> K<sup>-4</sup>_
* _Astronomical constants_
    * `earth_mass` _= 5.972&times;10<sup>24</sup> kg_
    * `earth_radius` _= 6.371&times;10<sup>6</sup> m_
    * `jupiter_mass` _= 1.899&times;10<sup>27</sup> kg_
    * `jupiter_radius` _= 6.991&times;10<sup>7</sup> m_
    * `solar_mass` _= 1.989&times;10<sup>30</sup> kg_
    * `solar_radius` _= 6.963&times;10<sup>8</sup> m_
    * `solar_luminosity` _= 3.846&times;10<sup>26</sup> W_
    * `solar_temperature` _= 5778 K_
    * `astronomical_unit` _= 1.496&times;10<sup>11</sup> m_
    * `light_year` _= 9.461&times;10<sup>15</sup> m_
    * `parsec` _= 3.086&times;10<sup>16</sup> m_
    * `julian_day` _= 86400 s_
    * `julian_year` _= 31557600 s_
    * `sidereal_year` _= 31558150 s_
    * `tropical_year` _= 31556925 s_

Constants defined using the macro above. The values quoted here are
approximate; the values actually supplied are accurate to at least `long
double` precision for the mathematical constants, or to the best known
accuracy for the physical ones.

## Arithmetic literals ##

These are all in `namespace Prion::Literals`.

* `template <char... CS> constexpr int128_t operator"" _s128() noexcept`
* `template <char... CS> constexpr uint128_t operator"" _u128() noexcept`

Signed and unsigned 128 bit integer literals. Hexadecimal constants (with the
usual `"0x"` prefix) are supported as well as decimal ones.

* `constexpr float operator"" _degf(long double x) noexcept`
* `constexpr float operator"" _degf(unsigned long long x) noexcept`
* `constexpr double operator"" _deg(long double x) noexcept`
* `constexpr double operator"" _deg(unsigned long long x) noexcept`
* `constexpr long double operator"" _degl(long double x) noexcept`
* `constexpr long double operator"" _degl(unsigned long long x) noexcept`

Angle literals, converting degrees to radians.

## Arithmetic functions ##

Some of these duplicate functionality already in the standard library; this is
to ensure that the functions are available for 128 bit integers even if the
standard library implementation doesn't support them.

* `template <typename T, typename... Args> constexpr T static_min(T t, Args... args) noexcept`
* `template <typename T, typename... Args> constexpr T static_max(T t, Args... args) noexcept`

Compile time maximum and minimum functions.

* `template <typename T> T abs(T t) noexcept`

Absolute value function. `Prion::abs()` is the same as `std::abs()` except for
adding 128 bit integer support.

* `template <typename T> std::make_signed_t<T> as_signed(T t) noexcept`
* `template <typename T> std::make_unsigned_t<T> as_unsigned(T t) noexcept`

These return their argument converted to a signed or unsigned value of the
same size (the argument is returned unchanged if `T` already had the desired
signedness). Behaviour is undefined if `T` is not an integer or enumeration
type.

* `template <typename T, typename T2, typename T3> constexpr T clamp(const T& x, const T2& min, const T3& max) noexcept`

Clamps a value to a fixed range. This returns `static_cast<T>(min)` if
`t<min`, `static_cast<T>(max)` if `t>max`, otherwise `t`.

* `template <typename T> constexpr T degrees(T rad) noexcept`
* `template <typename T> constexpr T radians(T deg) noexcept`

Convert between degrees and radians.

* `template <typename T> std::pair<T, T> divide(T lhs, T rhs) noexcept`
* `template <typename T> T quo(T lhs, T rhs) noexcept`
* `template <typename T> T rem(T lhs, T rhs) noexcept`

These perform floor division operations: if the division is not exact, the
remainder is always positive (regardless of the signs of the arguments), and
the quotient is the integer that satisfies `lhs=quo*rhs+rem` (apart from
rounding errors). The pair returned by `divide()` contains the quotient and
remainder. For all three functions, behaviour is undefined if `rhs=0` or the
quotient is out of `T`'s representable range.

* `template <typename T1, typename T2> constexpr T2 interpolate(T1 x1, T2 y1, T1 x2, T2 y2, T1 x)`

Returns the value of `y` corresponding to `x`, by interpolating or
extrapolating the line between `(x1,y1)` and `(x2,y2)`. Behaviour is undefined
if `x1==x2` and `y1!=y2`, or if either type is not an arithmetic type; the
result is unspecified if either type is not floating point.

* `template <typename T> T int_sqrt(T t) noexcept`

Returns the integer square root of the argument (the true square root
truncated to an integer). This will return zero if the argument is negative.

* `template <typename T> constexpr T rotl(T t, int n) noexcept`
* `template <typename T> constexpr T rotr(T t, int n) noexcept`

Bitwise rotate left or right. As for the standard shift operators, behaviour
is undefined if `n` is negative or greater than or equal to the number of bits
in `T`.

* `template <typename T2, typename T1> T2 round(T1 value) noexcept`

Rounds the value to the nearest integer, and returns the resulting value
converted to the return type (which must be specified explicitly). Halves are
rounded up (toward positive infinity). The usual arithmetic rules apply if the
result is not representable by the return type.

* `template <typename T> constexpr int sign_of(T t) noexcept`

Returns the sign of its argument (-1 if `t<0`, 0 if `t=0`, 1 if `t>0`).

## Byte order ##

* `static constexpr bool big_endian_target`
* `static constexpr bool little_endian_target`

One of these will be true and the other false, reflecting the target system's
byte order.

* `template <typename T> T big_endian(T t) noexcept`
* `template <typename T> T little_endian(T t) noexcept`

Convert a number between the native byte order and big or little endian.

* `template <typename T> T read_be(const void* ptr, size_t ofs = 0) noexcept`
* `template <typename T> T read_be(const void* ptr, size_t ofs, size_t len) noexcept`
* `template <typename T> void read_be(T& t, const void* ptr, size_t ofs = 0) noexcept`
* `template <typename T> void read_be(T& t, const void* ptr, size_t ofs, size_t len) noexcept`
* `template <typename T> T read_le(const void* ptr, size_t ofs = 0) noexcept`
* `template <typename T> T read_le(const void* ptr, size_t ofs, size_t len) noexcept`
* `template <typename T> void read_le(T& t, const void* ptr, size_t ofs = 0) noexcept`
* `template <typename T> void read_le(T& t, const void* ptr, size_t ofs, size_t len) noexcept`
* `template <typename T> void write_be(T t, void* ptr, size_t ofs = 0) noexcept`
* `template <typename T> void write_be(T t, void* ptr, size_t ofs, size_t len) noexcept`
* `template <typename T> void write_le(T t, void* ptr, size_t ofs = 0) noexcept`
* `template <typename T> void write_le(T t, void* ptr, size_t ofs, size_t len) noexcept`

Read or write an integer from a block of bytes, in big endian or little endian
order. The versions that take only a pointer deduce the size from the integer
type; the other versions take an offset and length in bytes.

## Character functions ##

* `constexpr bool ascii_isalnum(char c) noexcept`
* `constexpr bool ascii_isalpha(char c) noexcept`
* `constexpr bool ascii_iscntrl(char c) noexcept`
* `constexpr bool ascii_isdigit(char c) noexcept`
* `constexpr bool ascii_isgraph(char c) noexcept`
* `constexpr bool ascii_islower(char c) noexcept`
* `constexpr bool ascii_isprint(char c) noexcept`
* `constexpr bool ascii_ispunct(char c) noexcept`
* `constexpr bool ascii_isspace(char c) noexcept`
* `constexpr bool ascii_isupper(char c) noexcept`
* `constexpr bool ascii_isxdigit(char c) noexcept`

These are simple ASCII-only versions of the standard character type functions.
All of them will always return false for bytes outside the ASCII range
(0-127).

* `constexpr char ascii_tolower(char c) noexcept`
* `constexpr char ascii_toupper(char c) noexcept`

Simple ASCII-only case conversion functions. All non-ASCII characters are left
unchanged.

* `template <typename T> constexpr T char_to(char c) noexcept`

A simple conversion function that casts a `char` to a larger integer type by
first passing it through `unsigned char`, to ensure that characters with the
high bit set end up as integers in the 128-255 range, and not as negative or
extremely high values.

## Exceptions ##

* `class SystemError: public std::runtime_error`
    * `int SystemError::error() const noexcept`
    * `const char* SystemError::function() const noexcept`
* `class CrtError: public SystemError`
    * `CrtError::CrtError(int error, const char* function)`
    * `CrtError::CrtError(int error, const u8string& function)`
    * `static u8string CrtError::translate(int error)`
* `class WindowsError: public SystemError` _[Windows only]_
    * `WindowsError::WindowsError(uint32_t error, const char* function)`
    * `WindowsError::WindowsError(uint32_t error, const u8string& function)`
    * `static u8string WindowsError::translate(uint32_t error)`

Exceptions thrown to report an error from a system call, or a function that
uses the same error codes as a system call. `SystemError` is an abstract base
class; `CrtError` is used for C and Unix standard library errors;
`WindowsError` (only defined in Windows builds) is used for Win32 API errors.
The `translate()` functions convert an error code into an error message; this
is called by the constructor and included in the complete message
(`translate()` may return an empty string if the error code is not
recognized).

_(Error message translation does not work on Windows in the current version,
because I'm still trying to find a way to do that in a header-only library.)_

## Flag sets ##

* `class Flagset`

This represents a set of boolean flags, which can supplied as either a bitmask
(produced by bitwise-OR-ing the desired values) or a string of character
flags. Functions that take multiple boolean options usually have a `Flagset`
argument, allowing the caller to supply the flags as either an integer bitmask
or an abbreviated string. This is intended to be similar to the way many Unix
commands can be given either long, multi-word options, or the same options as
a set of single letters (e.g. `grep --extended-regexp --ignore-case` vs `grep
-Ei`).

Flags can be defined by using `Flagset::value()` to specify both the full name
(as an integer constant containing a one-bit mask) and the single-character
abbreviation. For example, a regular expression constructor takes flags like
these (among others):

    constexpr uint64_t rx_caseless = Flagset::value('i'); // Case insensitive matching
    constexpr uint64_t rx_extended = Flagset::value('x'); // Free-form mode (ignore whitespace and comments)
    constexpr uint64_t rx_notempty = Flagset::value('z'); // Do not match an empty string
    constexpr uint64_t rx_optimize = Flagset::value('o'); // Take extra effort to optimize the regex

If you want a case-insensitive regex that will never match an empty string,
you can pass the flags as either `rx_caseless|rx_notempty` or `"iz"`.

The flag set is stored internally as a 64-bit unsigned integer, which means
there aren't enough bits to actually allow any character, or even any ASCII
character, as a flag abbreviation. The allowed flags are the 52 ASCII letters,
and 12 of the punctuation marks: `# $ % & * + - / < = > @` (flags are case
sensitive).

* `constexpr Flagset::Flagset() noexcept`
* `constexpr Flagset(std::nullptr_t) noexcept`
* `constexpr Flagset([integer type] flags) noexcept`
* `template <typename C> Flagset::Flagset(const C* flags) noexcept`
* `template <typename C> Flagset::Flagset(const basic_string<C>& flags) noexcept`
* `Flagset::Flagset(const Flagset& f) noexcept`
* `Flagset::Flagset(Flagset&& f) noexcept`
* `Flagset::~Flagset() noexcept`
* `Flagset& Flagset::operator=(const Flagset& f) noexcept`
* `Flagset& Flagset::operator=(Flagset&& f) noexcept`

Life cycle operations. The flag set can be implicitly constructed from an
integer bitmask or a string of character flags. (Constructor overloads are
supplied for all integer types, including `bool`; these are imposed on us by
the need to make `FlagSet(0)` do the right thing in the face of C++'s baroque
implicit conversion rules.)

* `void Flagset::allow(Flagset allowed, const char* domain) const`
* `void Flagset::exclusive(Flagset xgroup, const char* domain) const`

These functions can be used to check for valid or invalid combinations of
flags. Both take a set of flags and a string as arguments; the `domain` string
will be included in the exception's error message to supply some context. The
`allow()` function will throw `FlagError` if the source object contains any
flags that are not in the `allowed` set; `exclusive()` will throw `FlagError`
if it contains more than one flag in the `xgroup` set.

* `constexpr bool Flagset::empty() const noexcept`

True if no flags are set.

* `constexpr uint64_t Flagset::get() const noexcept`
* `constexpr bool Flagset::get(uint64_t flags) const noexcept`

The first version of `get()` returns the complete bitmask as an integer. The
second version returns a boolean value to indicate the presence or absence of
a specific flag; if it is passed a bitmask with more than one bit set (i.e. a
combination of two or more flags), it will return true if any of the flags are
set.

* `template <typename C> constexpr bool Flagset::getc(C flag) const noexcept`
* `template <typename C> bool Flagset::getc(const C* flags) const noexcept`
* `template <typename C> bool Flagset::getc(const basic_string<C>& flags) const noexcept`

These are similar to the second version of `get()`, except that the flag or
flags to be tested are passed as one or more characters instead of an integer.
They will always return false if none of the characters in the argument are
valid flag characters.

* `void Flagset::set(uint64_t flags, bool state = true) noexcept`
* `template <typename C> void Flagset::setc(C flag, bool state = true) noexcept`

Set or clear one flag bit. The `setc()` function will do nothing if the
character is not one of the allowed flag characters.

* `u8string Flagset::str() const`
* `template <typename C> basic_string<C> Flagset::ustr() const`
* `std::ostream& operator<<(std::ostream& out, Flagset f)`

These generate a string containing the characters corresponding to the flags
in the set.

* `constexpr Flagset Flagset::operator~() const noexcept`
* `Flagset& Flagset::operator&=(Flagset rhs) noexcept`
* `Flagset& Flagset::operator|=(Flagset rhs) noexcept`
* `Flagset& Flagset::operator^=(Flagset rhs) noexcept`
* `constexpr Flagset operator&(Flagset lhs, Flagset rhs) noexcept`
* `constexpr Flagset operator|(Flagset lhs, Flagset rhs) noexcept`
* `constexpr Flagset operator^(Flagset lhs, Flagset rhs) noexcept`

Bitwise arithmetic operators.

* `constexpr bool operator==(Flagset lhs, Flagset rhs) noexcept`
* `constexpr bool operator!=(Flagset lhs, Flagset rhs) noexcept`

Equality comparison operators.

* `template <typename C> static constexpr uint64_t Flagset::value(C flag) noexcept`

Returns the bitmask value corresponding to a character flag, or zero if the
character is not a valid flag (see above).

* `class FlagError: public std::runtime_error`
    * `FlagError::FlagError(Flagset flags, const char* domain)`
    * `Flagset FlagError::flags() const noexcept`

An exception thrown to report an invalid combination of flags.

## Functional utilities ##

* `template <typename F> std::function<...> stdfun(F& lambda)`

Wraps a lambda in a `std::function` with the appropriate signature.

* `struct DoNothing`
    * `void operator()() const noexcept {}`
    * `template <typename T> void operator()(T&) const noexcept {}`
    * `template <typename T> void operator()(const T&) const noexcept {}`
* `constexpr DoNothing do_nothing`
* `struct Identity`
    * `template <typename T> T& operator()(T& t) const noexcept { return t; }`
    * `template <typename T> const T& operator()(const T& t) const noexcept { return t; }`
* `constexpr Identity identity`

Simple function objects.

## Hash functions ##

* `template <typename... Args> void hash_combine(size_t& hash, const Args&... args) noexcept`
* `template <typename... Args> size_t hash_value(const Args&... args) noexcept`
* `template <typename Range> void hash_range(size_t& hash, const Range& range) noexcept`
* `template <typename Range> size_t hash_range(const Range& range) noexcept`

Functions for combining hashes incrementally, or for generating the hash of a
number of objects in one call, for use in implementing hash functions for
compound types. All of these call `std::hash` for the element type.

## I/O utilities ##

Functions that take a file name have overloads that take a UTF-16 string in
native Windows builds (not on Cygwin, where the `_wfopen()` function is not
available).

* `bool is_stdout_redirected() noexcept`

Attempts to detect whether standard output has been redirected to a file or
pipe (`true`), or is going directly to a terminal (`false`). This is not
always possible to detect reliably; this function is fairly reliable on Unix,
less so on Windows.

* `bool load_file(const string& file, string& dst)`
* `bool load_file(const wstring& file, string& dst)` _(Native Windows only)_

Read a file's contents into a string. The return value is true if everything
went well, false if there was an error while opening or reading the file.

* `bool save_file(const string& file, const void* ptr, size_t n, bool append = false)`
* `bool save_file(const string& file, const string& src, bool append = false)`
* `bool save_file(const wstring& file, const void* ptr, size_t n, bool append = false)` _(Native Windows only)_
* `bool save_file(const wstring& file, const string& src, bool append = false)` _(Native Windows only)_

Write a string's contents into a file, optionally appending it to the file
instead of overwriting it if it already exists (if the file does not exist,
the `append` flag has no effect). The return value is true if everything went
well, false if there was an error while opening or writing the file.

* `constexpr const char* xt_up           = "<esc>[A"    = Cursor up`
* `constexpr const char* xt_down         = "<esc>[B"    = Cursor down`
* `constexpr const char* xt_right        = "<esc>[C"    = Cursor right`
* `constexpr const char* xt_left         = "<esc>[D"    = Cursor left`
* `constexpr const char* xt_erase_left   = "<esc>[1K"   = Erase left`
* `constexpr const char* xt_erase_right  = "<esc>[K"    = Erase right`
* `constexpr const char* xt_erase_above  = "<esc>[1J"   = Erase above`
* `constexpr const char* xt_erase_below  = "<esc>[J"    = Erase below`
* `constexpr const char* xt_erase_line   = "<esc>[2K"   = Erase line`
* `constexpr const char* xt_clear        = "<esc>[2J"   = Clear screen`
* `constexpr const char* xt_reset        = "<esc>[0m"   = Reset attributes`
* `constexpr const char* xt_bold         = "<esc>[1m"   = Bold`
* `constexpr const char* xt_under        = "<esc>[4m"   = Underline`
* `constexpr const char* xt_black        = "<esc>[30m"  = Black foreground`
* `constexpr const char* xt_red          = "<esc>[31m"  = Red foreground`
* `constexpr const char* xt_green        = "<esc>[32m"  = Green foreground`
* `constexpr const char* xt_yellow       = "<esc>[33m"  = Yellow foreground`
* `constexpr const char* xt_blue         = "<esc>[34m"  = Blue foreground`
* `constexpr const char* xt_magenta      = "<esc>[35m"  = Magenta foreground`
* `constexpr const char* xt_cyan         = "<esc>[36m"  = Cyan foreground`
* `constexpr const char* xt_white        = "<esc>[37m"  = White foreground`
* `constexpr const char* xt_black_bg     = "<esc>[40m"  = Black background`
* `constexpr const char* xt_red_bg       = "<esc>[41m"  = Red background`
* `constexpr const char* xt_green_bg     = "<esc>[42m"  = Green background`
* `constexpr const char* xt_yellow_bg    = "<esc>[43m"  = Yellow background`
* `constexpr const char* xt_blue_bg      = "<esc>[44m"  = Blue background`
* `constexpr const char* xt_magenta_bg   = "<esc>[45m"  = Magenta background`
* `constexpr const char* xt_cyan_bg      = "<esc>[46m"  = Cyan background`
* `constexpr const char* xt_white_bg     = "<esc>[47m"  = White background`

Xterm cursor movement and output formatting codes.

* `string xt_move_up(int n)      = "<esc>[<n>A"             = Cursor up n spaces`
* `string xt_move_down(int n)    = "<esc>[<n>B"             = Cursor down n spaces`
* `string xt_move_right(int n)   = "<esc>[<n>C"             = Cursor right n spaces`
* `string xt_move_left(int n)    = "<esc>[<n>D"             = Cursor left n spaces`
* `string xt_colour(int rgb)     = "<esc>[38;5;<16-231>m"   = Set foreground colour to an RGB value`
* `string xt_colour_bg(int rgb)  = "<esc>[48;5;<16-231>m"   = Set background colour to an RGB value`
* `string xt_grey(int grey)      = "<esc>[38;5;<232-255>m"  = Set foreground colour to a grey level`
* `string xt_grey_bg(int grey)   = "<esc>[48;5;<232-255>m"  = Set background colour to a grey level`

Functions for generating variable Xterm codes. The RGB levels passed to the
`xt_colour[_bg]()` functions are in the form of a 3 digit number from 111 to
666; each digit is clamped to a range of 0-5, yielding 216 different colours.
The grey level passed to the `xt_grey[_bg]()` functions is clamped to a range
of 1-24.

## Keyword Arguments ##

* `template <typename K> struct Kwarg`
    * `template <typename A> ... Kwarg::operator=(const A& arg) const`
* `template <typename K, typename V, typename... Args> bool kwget(const Kwarg<K>& key, V& var, const Args&... args)`

This provides a simple implementation of variadic keyword arguments for C++
functions.

Define a `Kwarg<K>` object for each keyword argument, where `K` is the
argument type. Functions that will take keyword arguments should be declared
with a variadic argument pack (possibly preceded by ordinary positional
arguments). When calling the function, the keyword arguments should be
supplied in the form `key=value`, where `key` is a `Kwarg<K>` object, and
`value` is the argument value. The value type must be convertible to `K`. If
`K` is `bool`, the keyword alone can be passed as an argument, with the value
defaulting to `true`.

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

## Mixins ##

These are convenience base classes that define members and operators that
would normally just be repetitive boilerplate (similar to the ones in Boost).
They all use the CRTP idiom; a class `T` should derived from `Mixin<T>` to
automatically generate the desired boilerplate code.

<table>
    <thead>
        <tr>
            <th>Mixin</th>
            <th>Requires</th>
            <th>Defines</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><code>EqualityComparable&lt;T&gt;</code></td>
            <td><code>t1==t2</code></td>
            <td><code>t1!=t2</code></td>
        </tr>
        <tr>
            <td><code>LessThanComparable&lt;T&gt;</code></td>
            <td><code>t1==t2<br>t1&lt;t2</code></td>
            <td><code>t1!=t2<br>t1&gt;t2<br>t1&lt;=t2<br>t1&gt;=t2</code></td>
        </tr>
        <tr>
            <td><code>InputIterator&lt;T,CV&gt;</code></td>
            <td><code>\*t<br>++t<br>t1==t2</code></td>
            <td><code>t-&gt;<br>t++<br>t1!=t2</code></td>
        </tr>
        <tr>
            <td><code>OutputIterator&lt;T&gt;</code></td>
            <td><code>t=v</code></td>
            <td><code>\*t<br>++t<br>t++</code></td>
        </tr>
        <tr>
            <td><code>ForwardIterator&lt;T,CV&gt;</code></td>
            <td><code>\*t<br>++t<br>t1==t2</code></td>
            <td><code>t-&gt;<br>t++<br>t1!=t2</code></td>
        </tr>
        <tr>
            <td><code>BidirectionalIterator&lt;T,CV&gt;</code></td>
            <td><code>\*t<br>++t<br>\--t<br>t1==t2</code></td>
            <td><code>t-&gt;<br>t++<br>t\--<br>t1!=t2</code></td>
        </tr>
        <tr>
            <td><code>RandomAccessIterator&lt;T,CV&gt;</code></td>
            <td><code>\*t<br>++t<br>\--t<br>t+=n<br>t1-t2<br>t1==t2<br>t1&lt;t2</code></td>
            <td><code>t-&gt;<br>t[n]<br>t++<br>t\--<br>t-=n<br>t+n<br>n+t<br>t-n<br>t1!=t2<br>t1&gt;t2<br>t1&lt;=t2<br>t1&gt;=t2</code></td>
        </tr>
    </tbody>
</table>

In the iterator mixins, `CV` is either `V` or `const V`, where `V` is the
iterator's value type, depending on whether a mutable or const iterator is
required. In addition to the operators listed in the table above, all iterator
mixins supply the standard member types:

* `using difference_type = ptrdiff_t`
* `using iterator_category = [standard iterator tag type]`
* `using pointer = CV*`
* `using reference = CV&`
* `using value_type = std::remove_const_t<CV>`

## Range utilities ##

* `template <typename Range> using RangeIterator = ...`
* `template <typename Range> using RangeValue = ...`

The iterator and value types of a range.

* `template <typename Iterator> struct Irange`
    * `Iterator Irange::first`
    * `Iterator Irange::second`
    * `constexpr Iterator Irange::begin() const { return first; }`
    * `constexpr Iterator Irange::end() const { return second; }`
* `template <typename Iterator> constexpr Irange<Iterator> irange(const Iterator& i, const Iterator& j)`
* `template <typename Iterator> constexpr Irange<Iterator> irange(const std::pair<Iterator, Iterator>& p)`

A wrapper for a pair of iterators, usable as a range in standard algorithms.

* `template <typename T> constexpr Irange<T*> array_range(T* ptr, size_t len)`

Returns `irange(ptr, ptr + len)`.

* `template <typename Container> [output iterator] append(Container& con)`
* `template <typename Container> [output iterator] overwrite(Container& con)`

These create output iterators that will append elements to a standard
container. The `append()` function is similar to `std::back_inserter()` (but
supports containers without `push_back()`), while `overwrite()` will first
clear the container and then return the append iterator.

* `template <typename T, size_t N> constexpr size_t array_count(T[N])`
* `template <typename Range> size_t range_count(const Range& r)`
* `template <typename Range> bool range_empty(const Range& r)`

Return the length of a range. The `range_count()` function is just shorthand
for `std::distance(begin(r), end(r))`, and `range_empty()` has the obvious
meaning. The `array_count()` version returns the same value, but only works on
C-style arrays and is `constexpr`.

* `template <typename T> Irange<[iterator]> seq(T t2)`
* `template <typename T> Irange<[iterator]> seq(T t1, T t2)`
* `template <typename T> Irange<[iterator]> seq(T t1, T t2, T delta)`

These generate arithmetic sequences. The iterator type is a forward iterator
that dereferences to a `const T&`.

The first two versions use `++t` to generate the next value in the sequence
(the first starts with `T()`); the third uses `t+=delta`. `T` must be totally
ordered and support the relevant arithmetic operators. The sequences are half
open, containing `t1` but not `t2`; they will stop when `t<t2` is no longer
true, or if `delta<T()`, when `t2<t` is no longer true. The sequences will be
empty if `t1<t2` is not true (or if `t2<t1` is not true, when `delta<T()`),
and will always be empty if `delta=T()`.

## Scope guards ##

* `class ScopeExit`
    * `using ScopeExit::callback = std::function<void()>`
    * `explicit ScopeExit::ScopeExit(callback f)`
    * `ScopeExit::~ScopeExit() noexcept`
* `class ScopeSuccess`
    * `using ScopeSuccess::callback = std::function<void()>`
    * `explicit ScopeSuccess::ScopeSuccess(callback f)`
    * `ScopeSuccess::~ScopeSuccess() noexcept`
* `class ScopeFailure`
    * `using ScopeFailure::callback = std::function<void()>`
    * `explicit ScopeFailure::ScopeFailure(callback f)`
    * `ScopeFailure::~ScopeFailure() noexcept`

These store a function object, to be called when the guard is destroyed.
`ScopeExit` calls the function unconditionally, `ScopeSuccess` calls it only
on normal exit (not when unwinding due to an exception), and `ScopeFailure`
calls it only when an exception causes stack unwinding (not on normal exit).
If the constructor throws an exception (this is only possible if the function
object's copy constructor or assignment operator throws), `ScopeExit` and
`ScopeFailure` will call the function before propagating the exception, while
`ScopeSuccess` will not. Any exceptions thrown by the function call in the
destructor are silently ignored (normally the function should be written so as
not to throw anything).

* `class Transaction`
    * `using Transaction::callback = std::function<void()>`
    * `Transaction::Transaction() noexcept`
    * `Transaction::~Transaction() noexcept`
    * `void Transaction::call(callback func, callback undo)`
    * `void Transaction::commit() noexcept`
    * `void Transaction::rollback() noexcept`

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

A single `Transaction` object can be used for multiple transactions. Once
`commit()` or `rollback()` is called, the undo stack is discarded, and any
newly added function pairs become part of a new cycle, equivalent to a newly
constructed `Transaction`.

## String functions ##

* `string ascii_lowercase(const string& s)`
* `string ascii_uppercase(const string& s)`
* `string ascii_titlecase(const string& s)`

Simple ASCII-only case conversion functions. All non-ASCII characters are left
unchanged.

* `template <typename C> basic_string<C> cstr(const C* ptr)`
* `template <typename C> basic_string<C> cstr(const C* ptr, size_t n)`

These construct a string from a pointer to a null-terminated character
sequence, or a pointer and a length. They differ from the corresponding string
constructors in that passing a null pointer will yield an empty string instead
of undefined behaviour.

* `template <typename C> size_t cstr_size(const C* ptr)`

Returns the length of a null-terminated string (a generalized version of
`strlen()`). This will return zero if the pointer is null.

* `template <typename T> u8string dec(T x, size_t digits = 1)`
* `template <typename T> u8string hex(T x, size_t digits = 2 * sizeof(T))`
* `long long decnum(const string& str) noexcept`
* `unsigned long long hexnum(const string& str) noexcept`
* `double fpnum(const string& str) noexcept`

Simple string and number conversion functions. The `dec()` and `hex()`
functions convert an integer to a decimal or hexadecimal string, generating at
least the specified number of digits. The `decnum()` and `hexnum()` functions
perform the reverse conversion, while `fpnum()` converts a string to a
floating point number; these will ignore any trailing characters that are not
part of a number, and will return zero if the string is empty or does not
contain a valid number. Results that are out of range will be clamped to the
nearest end of the return type's range (for `fpnum()` this will normally be
positive or negative infinity).

* `u8string hexdump(const void* ptr, size_t n, size_t block = 0)`
* `u8string hexdump(const string& str, size_t block = 0)`

Converts a block of raw data into hexadecimal bytes. If `block` is not zero, a
line feed is inserted after each block.

* `template <typename InputIterator> string join_words(InputIterator begin, InputIterator end, const string& delim = " ")`
* `template <typename OutputIterator> void split_words(const string& src, OutputIterator dst, const string& delim = ascii_whitespace)`

Join words into a string, using the given delimiter, or split a string into
words, discarding any sequence of delimiter characters. The dereferenced
iterators must be assignment compatible with `string`.

* `string quote(const string& str, bool allow_8bit = false)`

Returns a quoted string; internal quotes, backslashes, control characters, and
non-ASCII characters are escaped. If the `allow_8bit` flag is set, bytes in
the 128-255 range are passed through unchanged instead of being escaped.

* `template <typename T> string to_str(const T& t)`

Formats an object as a string. For most types this uses the type's output
stream operator. For strings, including character arrays and pointers, the
string content is simply copied directly without using an output stream (a
null character pointer is treated as an empty string). Ranges (other than
strings) are serialized in a format similar to a JSON array (e.g.
`"[1,2,3]"`), or an object (e.g. `"{1:a,2:b,3:c}"`) if the range's value type
is a pair; `to_str()` is called recursively on each range element.

* `wstring utf8_to_wstring(const u8string& ustr)` _(Windows only)_
* `u8string wstring_to_utf8(const wstring& wstr)` _(Windows only)_

These are only defined on Windows, and provide conversions between UTF-8 and
the native UTF-16 API. They are minimal wrappers for `MultiByteToWideChar()`
and `WideCharToMultiByte()`. These do not do any error checking; the output
will be garbage if the input was not valid Unicode.

## Threads ##

This section of the library exists because some of the compilers I need to
support don't implement C++11 threads yet.

* `class Mutex`
    * `Mutex::Mutex() noexcept`
    * `Mutex::~Mutex() noexcept`
    * `void Mutex::lock() noexcept`
    * `bool Mutex::try_lock() noexcept`
    * `void Mutex::unlock() noexcept`
* `class MutexLock`
    * `explicit MutexLock::MutexLock(Mutex& m) noexcept`
    * `MutexLock::~MutexLock() noexcept`

Mutex and exclusive lock.

* `class ConditionVariable`
    * `ConditionVariable::ConditionVariable()`
    * `ConditionVariable::~ConditionVariable() noexcept`
    * `void ConditionVariable::notify_all() noexcept`
    * `void ConditionVariable::notify_one() noexcept`
    * `void ConditionVariable::wait(MutexLock& lock)`
    * `template <typename Pred> void ConditionVariable::wait(MutexLock& lock, Pred p)`
    * `template <typename R, typename P, typename Pred> bool ConditionVariable::wait_for(MutexLock& lock, std::chrono::duration<R, P> t, Pred p)`

Condition variable.

* `class Thread`
    * `using Thread::callback = std::function<void()>`
    * `using Thread::id_type = ...`
    * `Thread::Thread() noexcept`
    * `explicit Thread::Thread(callback f)`
    * `Thread::Thread(Thread&& t)`
    * `Thread::~Thread() noexcept`
    * `Thread& Thread::operator=(Thread&& t)`
    * `Thread::id_type Thread::get_id() const noexcept`
    * `bool Thread::poll() noexcept`
    * `void Thread::wait()`
    * `size_t Thread::cpu_threads() noexcept`
    * `static Thread::id_type Thread::current() noexcept`
    * `void Thread::yield() noexcept`


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

The constructor and the `wait()` function may throw `SystemError` or a derived
class if anything goes wrong in the underlying native thread API, and `wait()`
may rethrow as described above.

The `cpu_threads()` function returns the number of hardware threads available.

The `yield()` function causes the current thread to yield its scheduler time
slice.

## Time and date functions ##

In this documentation, the `std::chrono::` prefix on the standard date and
time types is left out for brevity. For all functions here that accept or
return a broken down date, results are unspecified if the date is outside the
range of a `time_t` (1970-2038 on 32-bit systems), or if the fields are set to
values that do not represent a valid date.

* `enum ZoneFlag`
    * `utc_date`
    * `local_date`

This is passed to the conversion functions to indicate whether a broken down
date is expressed in UTC or the local time zone.

* `u8string format_date(std::chrono::system_clock::time_point tp, int prec = 0, ZoneFlag z = utc_date)`
* `u8string format_date(std::chrono::system_clock::time_point tp, const u8string& format, ZoneFlag z = utc_date)`

These convert a time point into a broken down date and format it. The first
version writes the date in ISO 8601 format (`"yyyy-mm-dd hh:mm:ss"`). If
`prec` is greater than zero, the specified number of decimal places will be
added to the seconds field.

The second version writes the date using the conventions of `strftime()`. This
will return an empty string if anything goes wrong; there is no way to
distinguish between a conversion error and a legitimately empty result (this
is a limitation of `strftime()`).

For reference, the portable subset of the `strftime()` formatting codes are:

| Code  | Description                          | Code   | Description                          |
| ----  | -----------                          | ----   | -----------                          |
|       **Date elements**                      ||       **Weekday elements**                   ||
| `%Y`  | Year number                          | `%a`   | Local weekday abbreviation           |
| `%y`  | Last 2 digits of the year (`00-99`)  | `%w`   | Sunday-based weekday number (`0-6`)  |
| `%m`  | Month number (`00-12`)               | `%A`   | Local weekday name                   |
| `%B`  | Local month name                     | `%U`   | Sunday-based week number (`00-53`)   |
| `%b`  | Local month abbreviation             | `%W`   | Monday-based week number (`00-53`)   |
| `%d`  | Day of the month (`01-31`)           |        **Other elements**                     ||
|       **Time of day elements**               || `%c`  | Local standard date/time format      |
| `%H`  | Hour on 24-hour clock (`00-23`)      | `%x`   | Local standard date format           |
| `%I`  | Hour on 12-hour clock (`01-12`)      | `%X`   | Local standard time format           |
| `%p`  | Local equivalent of a.m./p.m.        | `%j`   | Day of the year (`001-366`)          |
| `%M`  | Minute (`00-59`)                     | `%Z`   | Time zone name                       |
| `%S`  | Second (`00-60`)                     | `%z`   | Time zone offset                     |

* `template <typename R, typename P> u8string format_time(const duration<R, P>& time, int prec = 0)`

Formats a time duration in Julian years, days, hours, minutes, seconds, and
(if `prec>0`) fractions of a second. Results are unspecified if the number of
years in `time` does not fit in a signed 32-bit integer.

* `system_clock::time_point make_date(int year, int month, int day, int hour, int min, double sec, ZoneFlag z = utc_date) noexcept`

Converts a broken down date into a time point.

* `template <typename R, typename P> void sleep_for(std::chrono::duration<R, P> t) noexcept`
* `void sleep_for(double t) noexcept`

Suspend the current thread for the specified interval, supplied either as a
duration or a number of seconds. Calling this with a duration less than or
equal to zero will cause the thread to yield its time slice. Resolution is
system dependent. Behaviour is undefined if the duration exceeds 31 days.

* `template <typename R, typename P> double to_seconds(const duration<R, P>& d) noexcept`
* `template <typename R, typename P> void from_seconds(double s, duration<R, P>& d) noexcept`

Convenience functions to convert between a `duration` and a floating point
number of seconds.

* _Unix_
    * `template <typename R, typename P> timespec duration_to_timespec(const duration<R, P>& d) noexcept`
    * `template <typename R, typename P> timeval duration_to_timeval(const duration<R, P>& d) noexcept`
    * `timespec timepoint_to_timespec(const system_clock::time_point& tp) noexcept`
    * `timeval timepoint_to_timeval(const system_clock::time_point& tp) noexcept`
    * `template <typename R, typename P> void timespec_to_duration(const timespec& ts, duration<R, P>& d) noexcept`
    * `system_clock::time_point timespec_to_timepoint(const timespec& ts) noexcept`
    * `template <typename R, typename P> void timeval_to_duration(const timeval& tv, duration<R, P>& d) noexcept`
    * `system_clock::time_point timeval_to_timepoint(const timeval& tv) noexcept`
* _Windows_
    * `system_clock::time_point filetime_to_timepoint(const FILETIME& ft) noexcept`
    * `void timepoint_to_filetime(const system_clock::time_point& tp, FILETIME& ft) noexcept`

Conversion functions between C++ chrono types and system API types. Some of
them return their result through a reference argument instead of a return
value, either to avoid having to explicitly specify the duration type, or to
avoid including system headers in this header. Behaviour is undefined if the
value being represented is out of range for either the source or destination
type.

For reference, the system time types are:

* _Unix_
    * `#include <time.h>`
        * `struct timespec`
            * `time_t timespec::tv_sec [seconds]`
            * `long timespec::tv_nsec [nanoseconds]`
    * `#include <sys/time.h>`
        * `struct timeval`
            * `time_t timeval::tv_sec [seconds]`
            * `suseconds_t timeval::tv_usec [microseconds]`
* _Windows_
    * `#include <windows.h>`
        * `struct FILETIME`
            * `DWORD FILETIME::dwLowDateTime [low 32 bits]`
            * `DWORD FILETIME::dwHighDateTime [high 32 bits]`

## Type properties ##

* `template <typename T1, typename T2> using CopyConst = ...`

Yields a type created by transferring the `const` qualification (or lack of
it) from `T1` to the unqualified type of `T2`. For example, `CopyConst<int,
const string>` yields `string`, while `CopyConst<const int, string>` yields
`const string`.

* `template <typename T2, typename T1> T2 implicit_cast(const T1& t)`

Converts one type to another, only if the conversion is implicit.

* `string demangle(const string& name)`
* `string type_name(const std::type_info& t)`
* `template <typename T> string type_name()`
* `template <typename T> string type_name(const T&)`

Demangle a type name. The original mangled name can be supplied as an explicit
string, as a `std::type_info` object, as a type argument to a template
function (e.g. `type_name<int>()`), or as an object whose type is to be named
(e.g. `type_name(42)`).

## UUID ##

* `class Uuid`
    * `Uuid::Uuid() noexcept`
    * `Uuid::Uuid(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h, uint8_t i, uint8_t j, uint8_t k, uint8_t l, uint8_t m, uint8_t n, uint8_t o, uint8_t p) noexcept`
    * `Uuid::Uuid(uint32_t abcd, uint16_t ef, uint16_t gh, uint8_t i, uint8_t j, uint8_t k, uint8_t l, uint8_t m, uint8_t n, uint8_t o, uint8_t p) noexcept`
    * `explicit Uuid::Uuid(uint128_t u) noexcept`
    * `explicit Uuid::Uuid(const uint8_t* ptr) noexcept`
    * `explicit Uuid::Uuid(const string& s)`
    * `Uuid::Uuid(const Uuid& u) noexcept`
    * `Uuid::Uuid(Uuid&& u) noexcept`
    * `Uuid::~Uuid() noexcept`
    * `Uuid& Uuid::operator=(const Uuid& u) noexcept`
    * `Uuid& Uuid::operator=(Uuid&& u) noexcept`
    * `uint8_t& Uuid::operator[](size_t i) noexcept`
    * `const uint8_t& Uuid::operator[](size_t i) const noexcept`
    * `uint8_t* Uuid::begin() noexcept`
    * `const uint8_t* Uuid::begin() const noexcept`
    * `uint8_t* Uuid::end() noexcept`
    * `const uint8_t* Uuid::end() const noexcept`
    * `uint128_t Uuid::as_integer() const noexcept`
    * `u8string Uuid::str() const`
* `bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool operator!=(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool operator<(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool operator>(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool operator<=(const Uuid& lhs, const Uuid& rhs) noexcept`
* `bool operator>=(const Uuid& lhs, const Uuid& rhs) noexcept`
* `std::ostream& operator<<(std::ostream& o, const Uuid& u)`

This class holds a standard 16 byte universally unique identifier (UUID).

The default constructor sets all bytes to zero. The second and third
constructors accept explicit byte values, either as a list of 16 bytes, or in
the standard breakdown format. The fourth constructor copies a UUID from a 128
bit integer, in little endian order. The fifth constructor copies the next 16
bytes from the location pointed to; a null pointer will set all bytes to zero.

The sixth constructor parses the string representation of a UUID. It expects
the string to hold exactly 32 hex digits, in groups each containing an even
number of digits (optionally prefixed with `"0x"`), and will treat any
characters that are not ASCII alphanumerics as delimiters. It will throw
`std::invalid_argument` if the string is not a valid UUID.

The `begin()`, `end()`, and `operator[]` functions grant access to the byte
representation. Behaviour is undefined if the index to `operator[]` is greater
than 15.

The `as_integer()` function returns the UUID as a 128 bit integer, in little
endian order.

The `str()` function, and the output operator, format the UUID in the standard
broken down hex representation, e.g. `"01234567-89ab-cdef-0123-456789abcdef"`.

The comparison operators perform the natural bytewise lexicographical
comparisons.

* `struct RandomUuid`
    * `using result_type = RandomUuid::Uuid`
    * `template <typename Rng> Uuid RandomUuid::operator()(Rng& rng) const`

Generates a random version 4 UUID.

## Version number ##

* `struct Version`
    * `unsigned Version::major`
    * `unsigned Version::minor`
    * `unsigned Version::patch`
    * `Version(unsigned x = 0, unsigned y = 0, unsigned z = 0)`
    * `unsigned& Version::operator[](size_t i) noexcept`
    * `const unsigned& Version::operator[](size_t i) const noexcept`
    * `unsigned* Version::begin() noexcept`
    * `const unsigned* Version::begin() const noexcept`
    * `unsigned* Version::end() noexcept`
    * `const unsigned* Version::end() const noexcept`
    * `u8string Version::str() const`
    * `static Version Version::from(int n, int scale = 10) noexcept`
    * `static Version Version::parse(const u8string& s)`
* `bool operator==(const Version& lhs, const Version& rhs) noexcept`
* `bool operator!=(const Version& lhs, const Version& rhs) noexcept`
* `bool operator<(const Version& lhs, const Version& rhs) noexcept;`
* `bool operator>(const Version& lhs, const Version& rhs) noexcept`
* `bool operator<=(const Version& lhs, const Version& rhs) noexcept`
* `bool operator>=(const Version& lhs, const Version& rhs) noexcept`
* `std::ostream& operator<<(std::ostream& o, const Version& v)`

A simple three-part version number. The `begin()` and `end()` functions, and
the index and comparison operators, have their usual meanings for a fixed size
array. The `str()` function and the output operator format the version number
as a string, in the usual `"x.y.z"` format.

The `from()` function extracts a version number compressed into an integer,
e.g. 123 will be interpreted as version `1.2.3`. The `scale` argument is
usually 10, 16, 100, or 256. The result is unspecified if `n<0` or `scale<2`.

The `parse()` function parses a version number in the same format; missing
elements are assumed to be zero, and any text after the third number is
ignored. An empty string will be interpreted as version 0.0.0; otherwise, it
will throw `std::invalid_argument` if the string does not start with an
integer.
