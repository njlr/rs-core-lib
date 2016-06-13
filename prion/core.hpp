#pragma once

// Compilation environment identification

// MSVC is not actually supported yet; the macro is supplied for future development.
// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008

#if defined(_MSC_VER)
    #define PRI_COMPILER_MSVC (_MSC_VER >= 1900 ? _MSC_VER - 500 : _MSC_VER - 600)
#elif defined(__GNUC__)
    #if defined(__clang__)
        #define PRI_COMPILER_CLANG 1
    #else
        #define PRI_COMPILER_GCC (100 * __GNUC__ + 10 * __GNUC_MINOR__ + __GNUC_PATCHLEVEL__)
    #endif
#endif

#if defined(__CYGWIN__)
    #define PRI_TARGET_ANYWINDOWS 1
    #define PRI_TARGET_CYGWIN 1
    #define PRI_TARGET_UNIX 1
    #if defined(_WIN32)
        #define PRI_TARGET_WIN32 1
    #endif
#elif defined(_WIN32)
    #define PRI_TARGET_ANYWINDOWS 1
    #define PRI_TARGET_WINDOWS 1
    #define PRI_TARGET_WIN32 1
    #if defined(PRI_COMPILER_GCC)
        #define PRI_TARGET_MINGW 1
    #endif
#else
    #define PRI_TARGET_UNIX 1
    #if defined(__APPLE__)
        // Target condition on Apple must be #if, not #ifdef
        #define PRI_TARGET_APPLE 1
        #include <TargetConditionals.h>
        #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
            #define PRI_TARGET_IOS 1
        #elif TARGET_OS_MAC
            #define PRI_TARGET_MACOSX 1
        #endif
    #elif defined(__linux__)
        #define PRI_TARGET_LINUX 1
    #endif
#endif

#if defined(PRI_TARGET_UNIX)
    #if ! defined(_XOPEN_SOURCE)
        #define _XOPEN_SOURCE 700 // Posix 2008
    #endif
    #if ! defined(_REENTRANT)
        #define _REENTRANT 1
    #endif
#endif

#if defined(PRI_TARGET_WIN32)
    #if ! defined(NOMINMAX)
        #define NOMINMAX 1
    #endif
    #if ! defined(UNICODE)
        #define UNICODE 1
    #endif
    #if ! defined(_UNICODE)
        #define _UNICODE 1
    #endif
    #if ! defined(WINVER)
        #define WINVER 0x601 // Windows 7
    #endif
    #if ! defined(_WIN32_WINNT)
        #define _WIN32_WINNT 0x601
    #endif
#endif

// Includes go here so anything that needs the macros above will see them

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <clocale>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>
#include <cxxabi.h>

#if ! defined(PRI_TARGET_MINGW)
    #include <mutex>
#endif

#if defined(PRI_TARGET_UNIX)
    #include <pthread.h>
    #include <sched.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <unistd.h>
    #if defined(PRI_TARGET_APPLE)
        #include <mach/mach.h>
    #endif
#endif

#if defined(PRI_TARGET_WINDOWS)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <io.h>
#elif defined(PRI_TARGET_WIN32)
    #include <windows.h>
#endif

// Fix GNU brain damage

#if defined(major)
    #undef major
#endif
#if defined(minor)
    #undef minor
#endif
#if defined(_P)
    #undef _P
#endif

// Other preprocessor macros

#define PRI_ASSERT(expr) do { \
    if (! static_cast<bool>(expr)) \
        throw std::logic_error(std::string("Assertion failure [") + __FILE__ + ":" + ::Prion::dec(__LINE__) + "]: " + # expr); \
} while (false)

#define PRI_CHAR(C, T) (::Prion::PrionDetail::select_char<T>(C, u ## C, U ## C, L ## C))
#define PRI_CSTR(S, T) (::Prion::PrionDetail::select_cstr<T>(S, u ## S, U ## S, L ## S))
#define PRI_LDLIB(libs)
#define PRI_OVERLOAD(f) [] (auto&&... args) { return f(std::forward<decltype(args)>(args)...); }
#define PRI_STATIC_ASSERT(expr) static_assert((expr), # expr)

namespace Prion {

    namespace PrionDetail {

        template <typename EnumType>
        void write_enum(std::ostream& out, EnumType t, long first_value, const char* prefix, const char* names) {
            size_t index = long(t) - first_value;
            for (size_t i = 0; i < index; ++i) {
                names = strchr(names, ',');
                if (names == nullptr) {
                    out << long(t);
                    return;
                }
                names += strspn(names, " ,");
            }
            out << prefix;
            out.write(names, strcspn(names, " ,"));
        }

    }

}

#define PRI_ENUM_IMPLEMENTATION(EnumType, class_tag, name_prefix, first_value, first_name, ...) \
    enum class_tag EnumType { first_name = first_value, __VA_ARGS__ }; \
    inline std::ostream& operator<<(std::ostream& out, EnumType t) { \
        ::Prion::PrionDetail::write_enum(out, t, first_value, name_prefix, # first_name "," # __VA_ARGS__); \
        return out; \
    }

#define PRI_ENUM(EnumType, first_value, first_name, ...) \
    PRI_ENUM_IMPLEMENTATION(EnumType,, "", first_value, first_name, __VA_ARGS__)

#define PRI_ENUM_CLASS(EnumType, first_value, first_name, ...) \
    PRI_ENUM_IMPLEMENTATION(EnumType, class, # EnumType "::", first_value, first_name, __VA_ARGS__)

// For internal use only

#define PRI_NO_COPY_MOVE(T) \
    T(const T&) = delete; \
    T(T&&) = delete; \
    T& operator=(const T&) = delete; \
    T& operator=(T&&) = delete;

// Must be used in the global namespace
#define PRI_DEFINE_STD_HASH(T) \
    namespace std { \
        template <> struct hash<T> { \
            using argument_type = T; \
            using result_type = size_t; \
            size_t operator()(const T& t) const noexcept { return t.hash(); } \
        }; \
    }

namespace Prion {

    // Basic types

    // From <functional>
    using std::function;
    // From <iostream>
    using std::cerr;
    using std::cin;
    using std::clog;
    using std::cout;
    using std::endl;
    using std::flush;
    // From <memory>
    using std::make_shared;
    using std::make_unique;
    using std::shared_ptr;
    using std::unique_ptr;
    // From <string>
    using std::basic_string;
    using std::string;
    using std::u16string;
    using std::u32string;
    using std::wstring;
    // From <tuple>
    using std::make_tuple;
    using std::tie;
    using std::tuple;
    // From <utility>
    using std::make_pair;
    using std::move;
    using std::pair;
    // From <vector>
    using std::vector;

    using u8string = std::string;
    using int128_t = __int128;
    using uint128_t = unsigned __int128;

    // Things needed early

    #if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        constexpr bool big_endian_target = false;
        constexpr bool little_endian_target = true;
    #elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        constexpr bool big_endian_target = true;
        constexpr bool little_endian_target = false;
    #else
        #error Unknown byte order
    #endif

    constexpr const char* ascii_whitespace = "\t\n\v\f\r ";
    constexpr size_t npos = string::npos;

    template <typename Range> using RangeIterator = decltype(std::begin(std::declval<Range&>()));
    template <typename Range> using RangeValue = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;

    template <typename T> constexpr auto as_signed(T t) noexcept { return static_cast<std::make_signed_t<T>>(t); }
    constexpr auto as_signed(int128_t t) noexcept { return int128_t(t); }
    constexpr auto as_signed(uint128_t t) noexcept { return int128_t(t); }
    template <typename T> constexpr auto as_unsigned(T t) noexcept { return static_cast<std::make_unsigned_t<T>>(t); }
    constexpr auto as_unsigned(int128_t t) noexcept { return uint128_t(t); }
    constexpr auto as_unsigned(uint128_t t) noexcept { return uint128_t(t); }

    template <typename C>
    basic_string<C> cstr(const C* ptr) {
        using string_type = basic_string<C>;
        return ptr ? string_type(ptr) : string_type();
    }

    template <typename C>
    basic_string<C> cstr(const C* ptr, size_t n) {
        using string_type = basic_string<C>;
        return ptr ? string_type(ptr, n) : string_type();
    }

    namespace PrionDetail {

        inline void append_hex_byte(uint8_t b, string& s) {
            static constexpr const char* digits = "0123456789abcdef";
            s += digits[b / 16];
            s += digits[b % 16];
        }

        template <typename T>
        constexpr T select_char(char c, char16_t c16, char32_t c32, wchar_t wc) noexcept {
            return std::is_same<T, char>::value ? T(c)
                : std::is_same<T, char16_t>::value ? T(c16)
                : std::is_same<T, char32_t>::value ? T(c32)
                : std::is_same<T, wchar_t>::value ? T(wc)
                : T();
        }

        template <typename T>
        constexpr const T* select_cstr(const char* c, const char16_t* c16, const char32_t* c32, const wchar_t* wc) noexcept {
            return std::is_same<T, char>::value ? reinterpret_cast<const T*>(c)
                : std::is_same<T, char16_t>::value ? reinterpret_cast<const T*>(c16)
                : std::is_same<T, char32_t>::value ? reinterpret_cast<const T*>(c32)
                : std::is_same<T, wchar_t>::value ? reinterpret_cast<const T*>(wc)
                : nullptr;
        }

        template <typename T>
        u8string int_to_string(T x, int base, size_t digits) {
            bool neg = x < T(0);
            auto b = as_unsigned(base);
            auto y = neg ? as_unsigned(- x) : as_unsigned(x);
            u8string s;
            do {
                auto d = int(y % b);
                s += char((d < 10 ? '0' : 'a' - 10) + d);
                y /= b;
            } while (y || s.size() < digits);
            if (neg)
                s += '-';
            std::reverse(s.begin(), s.end());
            return s;
        }

    }

    template <typename T> u8string bin(T x, size_t digits = 8 * sizeof(T)) { return PrionDetail::int_to_string(x, 2, digits); }
    template <typename T> u8string dec(T x, size_t digits = 1) { return PrionDetail::int_to_string(x, 10, digits); }
    template <typename T> u8string hex(T x, size_t digits = 2 * sizeof(T)) { return PrionDetail::int_to_string(x, 16, digits); }

    inline string quote(const string& str, bool allow_8bit = false) {
        string result = "\"";
        for (auto c: str) {
            auto b = uint8_t(c);
            if (c == 0)                        result += "\\0";
            else if (c == '\t')                result += "\\t";
            else if (c == '\n')                result += "\\n";
            else if (c == '\f')                result += "\\f";
            else if (c == '\r')                result += "\\r";
            else if (c == '\"')                result += "\\\"";
            else if (c == '\\')                result += "\\\\";
            else if (c >= 0x20 && c <= 0x7e)   result += c;
            else if (allow_8bit && b >= 0x80)  result += c;
            else {
                result += "\\x";
                PrionDetail::append_hex_byte(b, result);
            }
        }
        result += '\"';
        return result;
    }

    // Unicode functions

    namespace PrionDetail {

        template <typename Src, typename Dst,
            size_t N1 = sizeof(typename Src::value_type),
            size_t N2 = sizeof(typename Dst::value_type)>
        struct UtfConvert {
            Dst operator()(const Src& s) const {
                return UtfConvert<u32string, Dst>()(UtfConvert<Src, u32string>()(s));
            }
        };

        template <typename Src, typename Dst, size_t N>
        struct UtfConvert<Src, Dst, N, N> {
            Dst operator()(const Src& s) const {
                using dc = typename Dst::value_type;
                return Dst(reinterpret_cast<const dc*>(s.data()), s.size());
            }
        };

        template <typename Src, typename Dst>
        struct UtfConvert<Src, Dst, 1, 4> {
            Dst operator()(const Src& s) const {
                using dc = typename Dst::value_type;
                auto p = reinterpret_cast<const uint8_t*>(s.data()), q = p + s.size();
                Dst d;
                while (p < q) {
                    if (*p <= 0x7f) {
                        d += dc(*p++);
                    } else if (p[0] <= 0xc1 || p[0] >= 0xf5 || q - p < 2 || p[1] < 0x80 || p[1] > 0xbf
                            || (p[0] == 0xe0 && p[1] < 0xa0) || (p[0] == 0xed && p[1] > 0x9f)
                            || (p[0] == 0xf0 && p[1] < 0x90) || (p[0] == 0xf4 && p[1] > 0x8f)) {
                        d += dc(0xfffd);
                        ++p;
                    } else if (p[0] <= 0xdf) {
                        d += (dc(p[0] & 0x1f) << 6) | dc(p[1] & 0x3f);
                        p += 2;
                    } else if (p[0] <= 0xef) {
                        if (q - p < 3 || p[2] < 0x80 || p[2] > 0xbf) {
                            d += dc(0xfffd);
                            p += 2;
                        } else {
                            d += (dc(p[0] & 0x0f) << 12) | (dc(p[1] & 0x3f) << 6) | dc(p[2] & 0x3f);
                            p += 3;
                        }
                    } else {
                        if (q - p < 3 || p[2] < 0x80 || p[2] > 0xbf) {
                            d += dc(0xfffd);
                            p += 2;
                        } else if (q - p < 4 || p[3] < 0x80 || p[3] > 0xbf) {
                            d += dc(0xfffd);
                            p += 3;
                        } else {
                            d += (dc(p[0] & 0x07) << 18) | (dc(p[1] & 0x3f) << 12)
                                | (dc(p[2] & 0x3f) << 6) | dc(p[3] & 0x3f);
                            p += 4;
                        }
                    }
                }
                return d;
            }
        };

        template <typename Src, typename Dst>
        struct UtfConvert<Src, Dst, 2, 4> {
            Dst operator()(const Src& s) const {
                using dc = typename Dst::value_type;
                auto p = reinterpret_cast<const uint16_t*>(s.data()), q = p + s.size();
                Dst d;
                while (p < q) {
                    if (*p <= 0xd7ff || *p >= 0xe000) {
                        d += dc(*p++);
                    } else if (q - p >= 2 && p[0] >= 0xd800 && p[0] <= 0xdbff && p[1] >= 0xdc00 && p[1] <= 0xdfff) {
                        d += dc(0x10000) + ((dc(p[0]) & 0x3ff) << 10) + (dc(p[1]) & 0x3ff);
                        p += 2;
                    } else {
                        d += dc(0xfffd);
                        ++p;
                    }
                }
                return d;
            }
        };

        template <typename Src, typename Dst>
        struct UtfConvert<Src, Dst, 4, 1> {
            Dst operator()(const Src& s) const {
                using dc = typename Dst::value_type;
                Dst d;
                for (auto c: s) {
                    auto u = uint32_t(c);
                    if (u <= 0x7f) {
                        d += dc(u);
                    } else if (u <= 0x7ff) {
                        d += dc(0xc0 | (u >> 6));
                        d += dc(0x80 | (u & 0x3f));
                    } else if (u <= 0xd7ff || (u >= 0xe000 && u <= 0xffff)) {
                        d += dc(0xe0 | (u >> 12));
                        d += dc(0x80 | ((u >> 6) & 0x3f));
                        d += dc(0x80 | (u & 0x3f));
                    } else if (u >= 0x10000 && u <= 0x10ffff) {
                        d += dc(0xf0 | (u >> 18));
                        d += dc(0x80 | ((u >> 12) & 0x3f));
                        d += dc(0x80 | ((u >> 6) & 0x3f));
                        d += dc(0x80 | (u & 0x3f));
                    } else {
                        d += dc(0xef);
                        d += dc(0xbf);
                        d += dc(0xbf);
                    }
                }
                return d;
            }
        };

        template <typename Src, typename Dst>
        struct UtfConvert<Src, Dst, 4, 2> {
            Dst operator()(const Src& s) const {
                using dc = typename Dst::value_type;
                Dst d;
                for (auto c: s) {
                    auto u = uint32_t(c);
                    if (u <= 0xd7ff || (u >= 0xe000 && u <= 0xffff)) {
                        d += dc(u);
                    } else if (u >= 0x10000 && u <= 0x10ffff) {
                        d += dc(0xd800 + ((u >> 10) - 0x40));
                        d += dc(0xdc00 + (u & 0x3ff));
                    } else {
                        d += dc(0xfffd);
                    }
                }
                return d;
            }
        };

    }

    template <typename Dst, typename Src>
    Dst uconv(const Src& s) {
        return PrionDetail::UtfConvert<Src, Dst>()(s);
    }

    // [Types]

    // Containers

    namespace PrionDetail {

        struct DeleteArray { template <typename T> void operator()(T* ptr) const noexcept { delete[] ptr; } };
        struct FreeMemory { template <typename T> void operator()(T* ptr) const noexcept { std::free(ptr); } };

    }

    template <typename T>
    class SimpleBuffer {
    public:
        using const_iterator = const T*;
        using const_reference = const T&;
        using delete_function = function<void(T*)>;
        using difference_type = ptrdiff_t;
        using iterator = T*;
        using reference = T&;
        using size_type = size_t;
        using value_type = T;
        SimpleBuffer() noexcept {}
        explicit SimpleBuffer(size_t n): len(n), ptr(new T[n]) {}
        SimpleBuffer(size_t n, T t): len(n), ptr(new T[n]) { std::fill_n(ptr, n, t); }
        SimpleBuffer(T* p, size_t n) noexcept: len(n), ptr(p), del(PrionDetail::FreeMemory()) {}
        SimpleBuffer(T* p, size_t n, delete_function d): len(n), ptr(p), del(d) {}
        SimpleBuffer(const SimpleBuffer& sb): len(sb.len), ptr(new T[len]) { memcpy(ptr, sb.ptr, bytes()); }
        SimpleBuffer(SimpleBuffer&& sb) noexcept: len(sb.len), ptr(sb.ptr), del(sb.del) { sb.abandon(); }
        ~SimpleBuffer() noexcept { clear(); }
        SimpleBuffer& operator=(const SimpleBuffer& sb);
        SimpleBuffer& operator=(SimpleBuffer&& sb) noexcept;
        T& operator[](size_t i) noexcept { return ptr[i]; }
        const T& operator[](size_t i) const noexcept { return ptr[i]; }
        void assign(size_t n);
        void assign(size_t n, T t) { assign(n); std::fill_n(ptr, n, t); }
        void assign(T* p, size_t n) noexcept { clear(); len = n; ptr = p; del = PrionDetail::FreeMemory(); }
        void assign(T* p, size_t n, delete_function d) { SimpleBuffer temp(p, n, d); swap(temp); }
        T& at(size_t i) { check_index(i); return ptr[i]; }
        const T& at(size_t i) const { check_index(i); return ptr[i]; }
        T* begin() noexcept { return ptr; }
        const T* begin() const noexcept { return ptr; }
        const T* cbegin() const noexcept { return ptr; }
        T* data() noexcept { return ptr; }
        const T* data() const noexcept { return ptr; }
        const T* cdata() const noexcept { return ptr; }
        T* end() noexcept { return ptr + len; }
        const T* end() const noexcept { return ptr + len; }
        const T* cend() const noexcept { return ptr + len; }
        size_t bytes() const noexcept { return len * sizeof(T); }
        size_t capacity() const noexcept { return len; }
        void clear() noexcept { if (ptr && del) del(ptr); abandon(); }
        void copy(const T* p, size_t n) { assign(n); memcpy(ptr, p, bytes()); }
        void copy(const T* p1, const T* p2) { copy(p1, p2 - p1); }
        bool empty() const noexcept { return len == 0; }
        size_t max_size() const noexcept { return npos / sizeof(T); }
        size_t size() const noexcept { return len; }
        void swap(SimpleBuffer& sb2) noexcept { std::swap(len, sb2.len); std::swap(ptr, sb2.ptr); std::swap(del, sb2.del); }
        friend void swap(SimpleBuffer& sb1, SimpleBuffer& sb2) noexcept { sb1.swap(sb2); }
        friend bool operator==(const SimpleBuffer& lhs, const SimpleBuffer& rhs) noexcept
            { return lhs.len == rhs.len && memcmp(lhs.ptr, rhs.ptr, lhs.bytes()) == 0; }
        friend bool operator!=(const SimpleBuffer& lhs, const SimpleBuffer& rhs) noexcept { return ! (lhs == rhs); }
        friend bool operator<(const SimpleBuffer& lhs, const SimpleBuffer& rhs) noexcept {
            auto rc = memcmp(lhs.ptr, rhs.ptr, std::min(lhs.bytes(), rhs.bytes()));
            return rc == 0 ? lhs.len < rhs.len : rc < 0;
        }
        friend bool operator>(const SimpleBuffer& lhs, const SimpleBuffer& rhs) noexcept { return rhs < lhs; }
        friend bool operator<=(const SimpleBuffer& lhs, const SimpleBuffer& rhs) noexcept { return ! (rhs < lhs); }
        friend bool operator>=(const SimpleBuffer& lhs, const SimpleBuffer& rhs) noexcept { return ! (lhs < rhs); }
    private:
        #if ! defined(__GNUC__) || __GNUC__ >= 5
            PRI_STATIC_ASSERT(std::is_trivially_copyable<T>::value);
        #endif
        size_t len = 0;
        T* ptr = nullptr;
        delete_function del = PrionDetail::DeleteArray();
        void check_index(size_t i) const { if (i >= len) throw std::out_of_range("Buffer index out of range"); }
        void abandon() noexcept { len = 0; ptr = nullptr; del = nullptr; }
    };

        template <typename T>
        SimpleBuffer<T>& SimpleBuffer<T>::operator=(const SimpleBuffer<T>& sb) {
            SimpleBuffer temp(sb);
            swap(temp);
            return *this;
        }

        template <typename T>
        SimpleBuffer<T>& SimpleBuffer<T>::operator=(SimpleBuffer<T>&& sb) noexcept {
            clear();
            len = sb.len;
            ptr = sb.ptr;
            del = move(sb.del);
            sb.abandon();
            return *this;
        }

        template <typename T>
        void SimpleBuffer<T>::assign(size_t n) {
            if (n != len) {
                T* temp = new T[n];
                clear();
                len = n;
                ptr = temp;
            }
        }

    template <typename T>
    class Stacklike {
    public:
        using iterator = typename vector<T>::iterator;
        using const_iterator = typename vector<T>::const_iterator;
        Stacklike() = default;
        Stacklike(Stacklike&& s) = default;
        ~Stacklike() noexcept { clear(); }
        Stacklike& operator=(Stacklike&& s) { if (&s != this) { clear(); stack = move(s.stack); } return *this; }
        iterator begin() noexcept { return stack.begin(); }
        const_iterator begin() const noexcept { return stack.cbegin(); }
        const_iterator cbegin() const noexcept { return stack.cbegin(); }
        void clear() noexcept { while (! stack.empty()) stack.pop_back(); }
        bool empty() const noexcept { return stack.empty(); }
        iterator end() noexcept { return stack.end(); }
        const_iterator end() const noexcept { return stack.cend(); }
        const_iterator cend() const noexcept { return stack.cend(); }
        void pop() noexcept { if (! stack.empty()) stack.pop(); }
        void push(const T& t) { stack.push_back(t); }
        void push(T&& t) { stack.push_back(move(t)); }
        size_t size() const noexcept { return stack.size(); }
    private:
        vector<T> stack;
        Stacklike(const Stacklike&) = delete;
        Stacklike& operator=(const Stacklike&) = delete;
    };

    // Endian integers

    enum class End { big, little };

    namespace PrionDetail {

        template <typename T>
        constexpr T swap_ends(T t, size_t N = sizeof(T)) noexcept {
            using U = std::make_unsigned_t<T>;
            return N == 1 ? t : T((swap_ends(U(t) & ((U(1) << (4 * N)) - 1), N / 2) << (4 * N)) | swap_ends(U(t) >> (4 * N), N / 2));
        }

        template <End E, typename T>
        constexpr T order_bytes(T t) noexcept {
            return (E == End::big) == big_endian_target ? t : swap_ends(t);
        }

    }

    template <typename T, End E>
    class Endian {
    public:
        constexpr Endian() noexcept: value(0) {}
        constexpr Endian(T t) noexcept: value(PrionDetail::order_bytes<E>(t)) {}
        constexpr operator T() const noexcept { return get(); }
        constexpr T get() const noexcept { return PrionDetail::order_bytes<E>(value); }
        constexpr const T* ptr() const noexcept { return &value; }
        T* ptr() noexcept { return &value; }
        constexpr T rep() const noexcept { return value; }
        T& rep() noexcept { return value; }
    private:
        T value;
    };

    template <typename T> using BigEndian = Endian<T, End::big>;
    template <typename T> using LittleEndian = Endian<T, End::little>;

    template <typename T, End E> std::ostream& operator<<(std::ostream& out, Endian<T, E> t) { return out << t.get(); }
    template <End E> std::ostream& operator<<(std::ostream& out, Endian<int128_t, E> t) { return out << dec(t.get()); }
    template <End E> std::ostream& operator<<(std::ostream& out, Endian<uint128_t, E> t) { return out << dec(t.get()); }

    // Exceptions

    #if defined(PRI_TARGET_WIN32)

        class WindowsCategory:
        public std::error_category {
        public:
            virtual u8string message(int ev) const {
                static constexpr uint32_t flags =
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
                wchar_t* wptr = nullptr;
                auto units = FormatMessageW(flags, nullptr, ev, 0, (wchar_t*)&wptr, 0, nullptr);
                shared_ptr<wchar_t> wshare(wptr, LocalFree);
                int bytes = WideCharToMultiByte(CP_UTF8, 0, wptr, units, nullptr, 0, nullptr, nullptr);
                string text(bytes, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wptr, units, &text[0], bytes, nullptr, nullptr);
                text.resize(text.find_last_not_of(ascii_whitespace) + 1);
                return text;
            }
            virtual const char* name() const noexcept { return "Win32"; }
        };

        inline const std::error_category& windows_category() noexcept {
            static const WindowsCategory cat;
            return cat;
        }

    #endif

    // Metaprogramming and type traits

    namespace PrionDetail {

        template <size_t Bits> struct IntegerType;
        template <> struct IntegerType<8> { using signed_type = int8_t; using unsigned_type = uint8_t; };
        template <> struct IntegerType<16> { using signed_type = int16_t; using unsigned_type = uint16_t; };
        template <> struct IntegerType<32> { using signed_type = int32_t; using unsigned_type = uint32_t; };
        template <> struct IntegerType<64> { using signed_type = int64_t; using unsigned_type = uint64_t; };
        template <> struct IntegerType<128> { using signed_type = int128_t; using unsigned_type = uint128_t; };

    }

    template <typename T> using BinaryType = typename PrionDetail::IntegerType<8 * sizeof(T)>::unsigned_type;
    template <typename T1, typename T2> using CopyConst =
        std::conditional_t<std::is_const<T1>::value, std::add_const_t<T2>, std::remove_const_t<T2>>;
    template <size_t Bits> using SignedInteger = typename PrionDetail::IntegerType<Bits>::signed_type;
    template <size_t Bits> using UnsignedInteger = typename PrionDetail::IntegerType<Bits>::unsigned_type;

    // Mixins

    template <typename T>
    struct EqualityComparable {
        friend bool operator!=(const T& lhs, const T& rhs) noexcept { return ! (lhs == rhs); }
    };

    template <typename T>
    struct LessThanComparable:
    EqualityComparable<T> {
        friend bool operator>(const T& lhs, const T& rhs) noexcept { return rhs < lhs; }
        friend bool operator<=(const T& lhs, const T& rhs) noexcept { return ! (rhs < lhs); }
        friend bool operator>=(const T& lhs, const T& rhs) noexcept { return ! (lhs < rhs); }
    };

    template <typename T, typename CV>
    struct InputIterator:
    EqualityComparable<T> {
        using difference_type = ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using pointer = CV*;
        using reference = CV&;
        using value_type = std::remove_const_t<CV>;
        CV* operator->() const noexcept { return &*static_cast<const T&>(*this); }
        friend T operator++(T& t, int) { T rc = t; ++t; return rc; }
    };

    template <typename T>
    struct OutputIterator {
        using difference_type = void;
        using iterator_category = std::output_iterator_tag;
        using pointer = void;
        using reference = void;
        using value_type = void;
        T& operator*() noexcept { return static_cast<T&>(*this); }
        friend T& operator++(T& t) noexcept { return t; }
        friend T operator++(T& t, int) noexcept { return t; }
    };

    template <typename T, typename CV>
    struct ForwardIterator:
    InputIterator<T, CV> {
        using iterator_category = std::forward_iterator_tag;
    };

    template <typename T, typename CV>
    struct BidirectionalIterator:
    ForwardIterator<T, CV> {
        using iterator_category = std::bidirectional_iterator_tag;
        friend T operator--(T& t, int) { T rc = t; --t; return rc; }
    };

    template <typename T, typename CV>
    struct RandomAccessIterator:
    BidirectionalIterator<T, CV>,
    LessThanComparable<T> {
        using iterator_category = std::random_access_iterator_tag;
        CV& operator[](ptrdiff_t i) const noexcept { T t = static_cast<const T&>(*this); t += i; return *t; }
        friend T& operator++(T& t) { t += 1; return t; }
        friend T& operator--(T& t) { t += -1; return t; }
        friend T& operator-=(T& lhs, ptrdiff_t rhs) { return lhs += - rhs; }
        friend T operator+(const T& lhs, ptrdiff_t rhs) { T t = lhs; t += rhs; return t; }
        friend T operator+(ptrdiff_t lhs, const T& rhs) { T t = rhs; t += lhs; return t; }
        friend T operator-(const T& lhs, ptrdiff_t rhs) { T t = lhs; t -= rhs; return t; }
        friend bool operator==(const T& lhs, const T& rhs) noexcept { return lhs - rhs == 0; }
        friend bool operator<(const T& lhs, const T& rhs) noexcept { return lhs - rhs < 0; }
    };

    // Smart pointers

    class NullPointer:
    public std::runtime_error {
    public:
        NullPointer(): std::runtime_error("Null pointer") {}
    };

    template <typename T> class Nnptr;
    template <typename T, typename... Args> Nnptr<T> make_nnptr(Args&&... args);
    template <typename T1, typename T2> Nnptr<T1> const_pointer_cast(const Nnptr<T2>& r) noexcept;
    template <typename T1, typename T2> Nnptr<T1> dynamic_pointer_cast(const Nnptr<T2>& r) noexcept;
    template <typename T1, typename T2> Nnptr<T1> static_pointer_cast(const Nnptr<T2>& r) noexcept;

    template <typename T>
    class Nnptr {
    public:
        using element_type = T;
        Nnptr(const Nnptr& p) = default;
        Nnptr(Nnptr&& p) noexcept { ptr = p.ptr; }
        template <typename T2> Nnptr(const Nnptr<T2>& p) noexcept: ptr(p.ptr) {}
        template <typename T2> explicit Nnptr(T2* p): ptr(p) { check(p); }
        template <typename T2, typename D> Nnptr(T2* p, D d): ptr(p, d) { check(p); }
        template <typename T2, typename D> Nnptr(unique_ptr<T2, D>&& p) { check(p.get()); ptr.reset(p.release()); }
        ~Nnptr() = default;
        Nnptr& operator=(const Nnptr& p) = default;
        Nnptr& operator=(Nnptr&& p) noexcept { ptr = p.ptr; return *this; }
        template <typename T2> Nnptr& operator=(const Nnptr<T2>& p) noexcept { ptr = p.ptr; return *this; }
        template <typename T2, typename D> Nnptr& operator=(unique_ptr<T2, D>&& p) { check(p.get()); ptr.reset(p.release()); return *this; }
        explicit operator bool() const noexcept { return bool(ptr); }
        bool operator!() const noexcept { return ! ptr; }
        T& operator*() const noexcept { return *ptr; }
        T* operator->() const noexcept { return ptr.get(); }
        T* get() const noexcept { return ptr.get(); }
        template <typename T2> void reset(T2* p) { check(p); ptr.reset(p); }
        template <typename T2, typename D> void reset(T2* p, D d) { check(p); ptr.reset(p, d); }
        bool unique() const noexcept { return ptr.unique(); }
        template <typename T2, typename... Args> Nnptr<T2> friend make_nnptr(Args&&... args)
            { return Nnptr<T2>(make_shared<T2>(args...)); }
        template <typename T1, typename T2> Nnptr<T1> friend const_pointer_cast(const Nnptr<T2>& p) noexcept
            { return Nnptr<T1>(const_pointer_cast<T1>(p.ptr)); }
        template <typename T1, typename T2> Nnptr<T1> friend dynamic_pointer_cast(const Nnptr<T2>& p) noexcept
            { return Nnptr<T1>(dynamic_pointer_cast<T1>(p.ptr)); }
        template <typename T1, typename T2> Nnptr<T1> friend static_pointer_cast(const Nnptr<T2>& p) noexcept
            { return Nnptr<T1>(static_pointer_cast<T1>(p.ptr)); }
    private:
        shared_ptr<T> ptr;
        static void check(const void* p) { if (! p) throw NullPointer(); }
        Nnptr() = delete;
        explicit Nnptr(const shared_ptr<T>& p) noexcept: ptr(p) {}
    };

    template <typename T1, typename T2> bool operator==(const Nnptr<T1>& lhs, const Nnptr<T2>& rhs) noexcept { return lhs.get() == rhs.get(); }
    template <typename T1, typename T2> bool operator!=(const Nnptr<T1>& lhs, const Nnptr<T2>& rhs) noexcept { return lhs.get() != rhs.get(); }
    template <typename T1, typename T2> bool operator<(const Nnptr<T1>& lhs, const Nnptr<T2>& rhs) noexcept { return lhs.get() < rhs.get(); }
    template <typename T1, typename T2> bool operator>(const Nnptr<T1>& lhs, const Nnptr<T2>& rhs) noexcept { return lhs.get() > rhs.get(); }
    template <typename T1, typename T2> bool operator<=(const Nnptr<T1>& lhs, const Nnptr<T2>& rhs) noexcept { return lhs.get() <= rhs.get(); }
    template <typename T1, typename T2> bool operator>=(const Nnptr<T1>& lhs, const Nnptr<T2>& rhs) noexcept { return lhs.get() >= rhs.get(); }
    template <typename T1, typename T2> bool operator==(const Nnptr<T1>& lhs, const T2* rhs) noexcept { return lhs.get() == rhs; }
    template <typename T1, typename T2> bool operator!=(const Nnptr<T1>& lhs, const T2* rhs) noexcept { return lhs.get() != rhs; }
    template <typename T1, typename T2> bool operator<(const Nnptr<T1>& lhs, const T2* rhs) noexcept { return lhs.get() < rhs; }
    template <typename T1, typename T2> bool operator>(const Nnptr<T1>& lhs, const T2* rhs) noexcept { return lhs.get() > rhs; }
    template <typename T1, typename T2> bool operator<=(const Nnptr<T1>& lhs, const T2* rhs) noexcept { return lhs.get() <= rhs; }
    template <typename T1, typename T2> bool operator>=(const Nnptr<T1>& lhs, const T2* rhs) noexcept { return lhs.get() >= rhs; }
    template <typename T1, typename T2> bool operator==(const T1* lhs, const Nnptr<T2>& rhs) noexcept { return lhs == rhs.get(); }
    template <typename T1, typename T2> bool operator!=(const T1* lhs, const Nnptr<T2>& rhs) noexcept { return lhs != rhs.get(); }
    template <typename T1, typename T2> bool operator<(const T1* lhs, const Nnptr<T2>& rhs) noexcept { return lhs < rhs.get(); }
    template <typename T1, typename T2> bool operator>(const T1* lhs, const Nnptr<T2>& rhs) noexcept { return lhs > rhs.get(); }
    template <typename T1, typename T2> bool operator<=(const T1* lhs, const Nnptr<T2>& rhs) noexcept { return lhs <= rhs.get(); }
    template <typename T1, typename T2> bool operator>=(const T1* lhs, const Nnptr<T2>& rhs) noexcept { return lhs >= rhs.get(); }
    template <typename T> bool operator==(const Nnptr<T>&, std::nullptr_t) noexcept { return false; }
    template <typename T> bool operator!=(const Nnptr<T>&, std::nullptr_t) noexcept { return true; }
    template <typename T> bool operator<(const Nnptr<T>& lhs, std::nullptr_t) noexcept { return lhs.get() < static_cast<T*>(nullptr); }
    template <typename T> bool operator>(const Nnptr<T>& lhs, std::nullptr_t) noexcept { return lhs.get() > static_cast<T*>(nullptr); }
    template <typename T> bool operator<=(const Nnptr<T>& lhs, std::nullptr_t) noexcept { return lhs.get() < static_cast<T*>(nullptr); }
    template <typename T> bool operator>=(const Nnptr<T>& lhs, std::nullptr_t) noexcept { return lhs.get() > static_cast<T*>(nullptr); }
    template <typename T> bool operator==(std::nullptr_t, const Nnptr<T>&) noexcept { return false; }
    template <typename T> bool operator!=(std::nullptr_t, const Nnptr<T>&) noexcept { return true; }
    template <typename T> bool operator<(std::nullptr_t, const Nnptr<T>& rhs) noexcept { return static_cast<T*>(nullptr) < rhs.get(); }
    template <typename T> bool operator>(std::nullptr_t, const Nnptr<T>& rhs) noexcept { return static_cast<T*>(nullptr) > rhs.get(); }
    template <typename T> bool operator<=(std::nullptr_t, const Nnptr<T>& rhs) noexcept { return static_cast<T*>(nullptr) < rhs.get(); }
    template <typename T> bool operator>=(std::nullptr_t, const Nnptr<T>& rhs) noexcept { return static_cast<T*>(nullptr) > rhs.get(); }

    // Type related functions

    template <typename T2, typename T1> bool is(const T1& ref) noexcept
        { return dynamic_cast<const T2*>(&ref) != nullptr; }
    template <typename T2, typename T1> bool is(const T1* ptr) noexcept
        { return dynamic_cast<const T2*>(ptr) != nullptr; }
    template <typename T2, typename T1> bool is(const unique_ptr<T1>& ptr) noexcept
        { return dynamic_cast<const T2*>(ptr.get()) != nullptr; }
    template <typename T2, typename T1> bool is(const shared_ptr<T1>& ptr) noexcept
        { return dynamic_cast<const T2*>(ptr.get()) != nullptr; }

    template <typename T2, typename T1> T2& as(T1& ref)
        { return dynamic_cast<T2&>(ref); }
    template <typename T2, typename T1> const T2& as(const T1& ref)
        { return dynamic_cast<const T2&>(ref); }
    template <typename T2, typename T1> T2& as(T1* ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> const T2& as(const T1* ptr)
        { if (ptr) return dynamic_cast<const T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(unique_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(const unique_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(shared_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(const shared_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }

    template <typename T2, typename T1> inline T2 binary_cast(const T1& t) noexcept {
        PRI_STATIC_ASSERT(sizeof(T2) == sizeof(T1));
        T2 t2;
        memcpy(&t2, &t, sizeof(t));
        return t2;
    }

    template <typename T2, typename T1> inline T2 implicit_cast(const T1& t) { return t; }

    inline string demangle(const string& name) {
        auto mangled = name;
        shared_ptr<char> demangled;
        int status = 0;
        for (;;) {
            if (mangled.empty())
                return name;
            demangled.reset(abi::__cxa_demangle(mangled.data(), nullptr, nullptr, &status), free);
            if (status == -1)
                throw std::bad_alloc();
            if (status == 0 && demangled)
                return demangled.get();
            if (mangled[0] != '_')
                return name;
            mangled.erase(0, 1);
        }
    }

    inline string type_name(const std::type_info& t) { return demangle(t.name()); }
    inline string type_name(const std::type_index& t) { return demangle(t.name()); }
    template <typename T> string type_name() { return type_name(typeid(T)); }
    template <typename T> string type_name(const T&) { return type_name(typeid(T)); }

    // [Constants and literals]

    // Arithmetic constants

    #define PRI_DEFINE_CONSTANT(name, value) \
        static constexpr double name = value; \
        static constexpr float name ## _f = value ## f; \
        static constexpr long double name ## _ld = value ## l; \
        template <typename T> constexpr T c_ ## name() noexcept __attribute__((unused)); \
        template <> constexpr float c_ ## name<float>() noexcept __attribute__((unused)); \
        template <> constexpr double c_ ## name<double>() noexcept __attribute__((unused)); \
        template <typename T> constexpr T c_ ## name() noexcept { return static_cast<T>(name ## _ld); } \
        template <> constexpr float c_ ## name<float>() noexcept { return name ## _f; } \
        template <> constexpr double c_ ## name<double>() noexcept { return name; }

    // Mathematical constants

    PRI_DEFINE_CONSTANT(e,         2.71828182845904523536028747135266249775724709369996);
    PRI_DEFINE_CONSTANT(ln_2,      0.69314718055994530941723212145817656807550013436026);
    PRI_DEFINE_CONSTANT(ln_10,     2.30258509299404568401799145468436420760110148862877);
    PRI_DEFINE_CONSTANT(pi,        3.14159265358979323846264338327950288419716939937511);
    PRI_DEFINE_CONSTANT(sqrt_2,    1.41421356237309504880168872420969807856967187537695);
    PRI_DEFINE_CONSTANT(sqrt_3,    1.73205080756887729352744634150587236694280525381038);
    PRI_DEFINE_CONSTANT(sqrt_5,    2.23606797749978969640917366873127623544061835961153);
    PRI_DEFINE_CONSTANT(sqrt_pi,   1.77245385090551602729816748334114518279754945612239);
    PRI_DEFINE_CONSTANT(sqrt_2pi,  2.50662827463100050241576528481104525300698674060994);

    // Conversion factors

    PRI_DEFINE_CONSTANT(inch,           0.0254);              // m
    PRI_DEFINE_CONSTANT(foot,           0.3048);              // m
    PRI_DEFINE_CONSTANT(yard,           0.9144);              // m
    PRI_DEFINE_CONSTANT(mile,           1609.344);            // m
    PRI_DEFINE_CONSTANT(nautical_mile,  1852.0);              // m
    PRI_DEFINE_CONSTANT(ounce,          0.028349523125);      // kg
    PRI_DEFINE_CONSTANT(pound,          0.45359237);          // kg
    PRI_DEFINE_CONSTANT(short_ton,      907.18474);           // kg
    PRI_DEFINE_CONSTANT(long_ton,       1016.0469088);        // kg
    PRI_DEFINE_CONSTANT(pound_force,    4.4482216152605);     // N
    PRI_DEFINE_CONSTANT(erg,            1.0e-7);              // J
    PRI_DEFINE_CONSTANT(foot_pound,     1.3558179483314004);  // J
    PRI_DEFINE_CONSTANT(calorie,        4.184);               // J
    PRI_DEFINE_CONSTANT(ton_tnt,        4.184e9);             // J
    PRI_DEFINE_CONSTANT(horsepower,     745.69987158227022);  // W
    PRI_DEFINE_CONSTANT(mmHg,           133.322387415);       // Pa
    PRI_DEFINE_CONSTANT(atmosphere,     101325.0);            // Pa
    PRI_DEFINE_CONSTANT(zero_celsius,   273.15);              // K

    // Physical constants

    PRI_DEFINE_CONSTANT(atomic_mass_unit,           1.660538921e-27);  // kg
    PRI_DEFINE_CONSTANT(avogadro_constant,          6.02214129e23);    // mol^-1
    PRI_DEFINE_CONSTANT(boltzmann_constant,         1.3806488e-23);    // J K^-1
    PRI_DEFINE_CONSTANT(elementary_charge,          1.602176565e-19);  // C
    PRI_DEFINE_CONSTANT(gas_constant,               8.3144621);        // J mol^-1 K^-1
    PRI_DEFINE_CONSTANT(gravitational_constant,     6.67384e-11);      // m^3 kg^-1 s^-2
    PRI_DEFINE_CONSTANT(planck_constant,            6.62606957e-34);   // J s
    PRI_DEFINE_CONSTANT(speed_of_light,             299792458.0);      // m s^-1
    PRI_DEFINE_CONSTANT(stefan_boltzmann_constant,  5.670373e-8);      // W m^-2 K^-4

    // Astronomical constants

    PRI_DEFINE_CONSTANT(earth_mass,         5.97219e24);          // kg
    PRI_DEFINE_CONSTANT(earth_radius,       6.3710e6);            // m
    PRI_DEFINE_CONSTANT(earth_gravity,      9.80665);             // m s^-2
    PRI_DEFINE_CONSTANT(jupiter_mass,       1.8986e27);           // kg
    PRI_DEFINE_CONSTANT(jupiter_radius,     6.9911e7);            // m
    PRI_DEFINE_CONSTANT(solar_mass,         1.98855e30);          // kg
    PRI_DEFINE_CONSTANT(solar_radius,       6.96342e8);           // m
    PRI_DEFINE_CONSTANT(solar_luminosity,   3.846e26);            // W
    PRI_DEFINE_CONSTANT(solar_temperature,  5778.0);              // K
    PRI_DEFINE_CONSTANT(astronomical_unit,  1.49597870700e11);    // m
    PRI_DEFINE_CONSTANT(light_year,         9.4607304725808e15);  // m
    PRI_DEFINE_CONSTANT(parsec,             3.08567758149e16);    // m
    PRI_DEFINE_CONSTANT(julian_day,         86400.0);             // s
    PRI_DEFINE_CONSTANT(julian_year,        31557600.0);          // s
    PRI_DEFINE_CONSTANT(sidereal_year,      31558149.7635);       // s
    PRI_DEFINE_CONSTANT(tropical_year,      31556925.19);         // s

    // Arithmetic literals

    namespace PrionDetail {

        template <typename T, char C> constexpr T digit_value() noexcept
            { return T(C >= 'A' && C <= 'Z' ? C - 'A' + 10 : C >= 'a' && C <= 'z' ? C - 'a' + 10 : C - '0'); }

        template <typename T, int Base, char C, char... CS>
        struct BaseInteger {
            using prev_type = BaseInteger<T, Base, CS...>;
            static constexpr T scale = T(Base) * prev_type::scale;
            static constexpr T value = digit_value<T, C>() * scale + prev_type::value;
        };

        template <typename T, int Base, char C>
        struct BaseInteger<T, Base, C> {
            static constexpr T scale = T(1);
            static constexpr T value = digit_value<T, C>();
        };

        template <typename T, char... CS> struct MakeInteger: public BaseInteger<T, 10, CS...> {};
        template <typename T, char... CS> struct MakeInteger<T, '0', 'x', CS...>: public BaseInteger<T, 16, CS...> {};
        template <typename T, char... CS> struct MakeInteger<T, '0', 'X', CS...>: public BaseInteger<T, 16, CS...> {};

    }

    namespace Literals {

        constexpr int8_t operator""_s8(unsigned long long n) noexcept { return int8_t(n); }
        constexpr uint8_t operator""_u8(unsigned long long n) noexcept { return uint8_t(n); }
        constexpr int16_t operator""_s16(unsigned long long n) noexcept { return int16_t(n); }
        constexpr uint16_t operator""_u16(unsigned long long n) noexcept { return uint16_t(n); }
        constexpr int32_t operator""_s32(unsigned long long n) noexcept { return int32_t(n); }
        constexpr uint32_t operator""_u32(unsigned long long n) noexcept { return uint32_t(n); }
        constexpr int64_t operator""_s64(unsigned long long n) noexcept { return int64_t(n); }
        constexpr uint64_t operator""_u64(unsigned long long n) noexcept { return uint64_t(n); }

        template <char... CS> constexpr int128_t operator""_s128() noexcept
            { return int128_t(PrionDetail::MakeInteger<uint128_t, CS...>::value); }
        template <char... CS> constexpr uint128_t operator""_u128() noexcept
            { return PrionDetail::MakeInteger<uint128_t, CS...>::value; }
        template <char... CS> constexpr ptrdiff_t operator""_t() noexcept
            { return ptrdiff_t(PrionDetail::MakeInteger<size_t, CS...>::value); }
        template <char... CS> constexpr size_t operator""_z() noexcept
            { return PrionDetail::MakeInteger<size_t, CS...>::value; }

        constexpr unsigned long long operator""_k(unsigned long long n) noexcept { return 1000ull * n; }
        constexpr unsigned long long operator""_M(unsigned long long n) noexcept { return 1000000ull * n; }
        constexpr unsigned long long operator""_G(unsigned long long n) noexcept { return 1000000000ull * n; }
        constexpr unsigned long long operator""_T(unsigned long long n) noexcept { return 1000000000000ull * n; }
        constexpr unsigned long long operator""_kb(unsigned long long n) noexcept { return 1024ull * n; }
        constexpr unsigned long long operator""_MB(unsigned long long n) noexcept { return 1048576ull * n; }
        constexpr unsigned long long operator""_GB(unsigned long long n) noexcept { return 1073741824ull * n; }
        constexpr unsigned long long operator""_TB(unsigned long long n) noexcept { return 1099511627776ull * n; }

        constexpr long double operator""_y(long double x) noexcept { return 1e-24 * x; }
        constexpr long double operator""_z(long double x) noexcept { return 1e-21 * x; }
        constexpr long double operator""_a(long double x) noexcept { return 1e-18 * x; }
        constexpr long double operator""_f(long double x) noexcept { return 1e-15 * x; }
        constexpr long double operator""_p(long double x) noexcept { return 1e-12 * x; }
        constexpr long double operator""_n(long double x) noexcept { return 1e-9 * x; }
        constexpr long double operator""_u(long double x) noexcept { return 1e-6 * x; }
        constexpr long double operator""_m(long double x) noexcept { return 1e-3 * x; }
        constexpr long double operator""_k(long double x) noexcept { return 1e3 * x; }
        constexpr long double operator""_M(long double x) noexcept { return 1e6 * x; }
        constexpr long double operator""_G(long double x) noexcept { return 1e9 * x; }
        constexpr long double operator""_T(long double x) noexcept { return 1e12 * x; }
        constexpr long double operator""_P(long double x) noexcept { return 1e15 * x; }
        constexpr long double operator""_E(long double x) noexcept { return 1e18 * x; }
        constexpr long double operator""_Z(long double x) noexcept { return 1e21 * x; }
        constexpr long double operator""_Y(long double x) noexcept { return 1e24 * x; }

        constexpr float operator""_degf(long double x) noexcept
            { return float(x * (pi_ld / 180.0L)); }
        constexpr float operator""_degf(unsigned long long x) noexcept
            { return float(static_cast<long double>(x) * (pi_ld / 180.0L)); }
        constexpr double operator""_deg(long double x) noexcept
            { return double(x * (pi_ld / 180.0L)); }
        constexpr double operator""_deg(unsigned long long x) noexcept
            { return double(static_cast<long double>(x) * (pi_ld / 180.0L)); }
        constexpr long double operator""_degl(long double x) noexcept
            { return x * (pi_ld / 180.0L); }
        constexpr long double operator""_degl(unsigned long long x) noexcept
            { return static_cast<long double>(x) * (pi_ld / 180.0L); }

    }

    namespace PrionDetail {

        using namespace Prion::Literals;

    }

    // [Algorithms and ranges]

    // Generic algorithms

    template <typename Container>
    class AppendIterator:
    public OutputIterator<AppendIterator<Container>> {
    public:
        using value_type = typename Container::value_type;
        AppendIterator() = default;
        explicit AppendIterator(Container& c): con(&c) {}
        AppendIterator& operator=(const value_type& v) { con->insert(con->end(), v); return *this; }
    private:
        Container* con;
    };

    template <typename Container> AppendIterator<Container> append(Container& con) { return AppendIterator<Container>(con); }
    template <typename Container> AppendIterator<Container> overwrite(Container& con) { con.clear(); return AppendIterator<Container>(con); }

    template <typename Range1, typename Range2, typename Compare>
    int compare_3way(const Range1& r1, const Range2& r2, Compare cmp) {
        using std::begin;
        using std::end;
        auto i = begin(r1), e1 = end(r1);
        auto j = begin(r2), e2 = end(r2);
        for (; i != e1 && j != e2; ++i, ++j) {
            if (cmp(*i, *j))
                return -1;
            else if (cmp(*j, *i))
                return 1;
        }
        return i != e1 ? 1 : j != e2 ? -1 : 0;
    }

    template <typename Range1, typename Range2>
    int compare_3way(const Range1& r1, const Range2& r2) {
        return compare_3way(r1, r2, std::less<>());
    }

    template <typename Range, typename Container>
    void con_append(const Range& src, Container& dst) {
        using std::begin;
        using std::end;
        std::copy(begin(src), end(src), append(dst));
    }

    template <typename Range, typename Container>
    void con_overwrite(const Range& src, Container& dst) {
        using std::begin;
        using std::end;
        std::copy(begin(src), end(src), overwrite(dst));
    }

    template <typename Container, typename T>
    void con_remove(Container& con, const T& t) {
        con.erase(std::remove(con.begin(), con.end(), t), con.end());
    }

    template <typename Container, typename Predicate>
    void con_remove_if(Container& con, Predicate p) {
        con.erase(std::remove_if(con.begin(), con.end(), p), con.end());
    }

    template <typename Container, typename Predicate>
    void con_remove_if_not(Container& con, Predicate p) {
        con.erase(std::remove_if(con.begin(), con.end(), [p] (const auto& x) { return ! p(x); }), con.end());
    }

    template <typename Container>
    void con_unique(Container& con) {
        con.erase(std::unique(con.begin(), con.end()), con.end());
    }

    template <typename Container, typename BinaryPredicate>
    void con_unique(Container& con, BinaryPredicate p) {
        con.erase(std::unique(con.begin(), con.end(), p), con.end());
    }

    template <typename Container>
    void con_sort_unique(Container& con) {
        std::sort(con.begin(), con.end());
        con_unique(con);
    }

    template <typename Container, typename Compare>
    void con_sort_unique(Container& con, Compare cmp) {
        std::sort(con.begin(), con.end(), cmp);
        con_unique(con, [cmp] (const auto& a, const auto& b) { return ! cmp(a, b); });
    }

    template <typename F>
    void do_n(size_t n, F f) {
        for (; n != 0; --n)
            f();
    }

    template <typename F>
    void for_n(size_t n, F f) {
        for (size_t i = 0; i < n; ++i)
            f(i);
    }

    template <typename Range1, typename Range2, typename Compare>
    bool sets_intersect(const Range1& r1, const Range2& r2, Compare c) {
        using std::begin;
        using std::end;
        auto i = begin(r1), end1 = end(r1);
        auto j = begin(r2), end2 = end(r2);
        while (i != end1 && j != end2) {
            if (c(*i, *j))
                ++i;
            else if (c(*j, *i))
                ++j;
            else
                return true;
        }
        return false;
    }

    template <typename Range1, typename Range2>
    bool sets_intersect(const Range1& r1, const Range2& r2) {
        return sets_intersect(r1, r2, std::less<>());
    }

    template <typename Range>
    RangeValue<Range> sum_of(const Range& r) {
        auto t = RangeValue<Range>();
        for (auto& x: r)
            t = t + x;
        return t;
    }

    template <typename Range>
    RangeValue<Range> sum_of(Range&& r) {
        auto t = RangeValue<Range>();
        for (auto& x: r)
            t = t + x;
        return t;
    }

    template <typename Range>
    RangeValue<Range> product_of(const Range& r) {
        auto t = RangeValue<Range>(1);
        for (auto& x: r)
            t = t * x;
        return t;
    }

    template <typename Range>
    RangeValue<Range> product_of(Range&& r) {
        auto t = RangeValue<Range>(1);
        for (auto& x: r)
            t = t * x;
        return t;
    }

    // Memory algorithms

    inline int mem_compare(const void* lhs, size_t n1, const void* rhs, size_t n2) noexcept {
        if (! lhs || ! rhs)
            return rhs ? -1 : lhs ? 1 : 0;
        int rc = memcmp(lhs, rhs, std::min(n1, n2));
        return rc < 0 ? -1 : rc > 0 ? 1 : n1 < n2 ? -1 : n1 > n2 ? 1 : 0;
    }

    inline void mem_swap(void* ptr1, void* ptr2, size_t n) noexcept {
        if (ptr1 == ptr2)
            return;
        uint8_t b;
        auto p = static_cast<uint8_t*>(ptr1), endp = p + n, q = static_cast<uint8_t*>(ptr2);
        while (p != endp) {
            b = *p;
            *p++ = *q;
            *q++ = b;
        }
    }

    // Range traits

    namespace PrionDetail {

        template <typename T> struct ArrayCount;
        template <typename T, size_t N> struct ArrayCount<T[N]> { static constexpr size_t value = N; };

    }

    template <typename T> constexpr size_t array_count(T&&) noexcept { return PrionDetail::ArrayCount<std::remove_reference_t<T>>::value; }

    template <typename Range>
    size_t range_count(const Range& r) {
        using std::begin;
        using std::end;
        return std::distance(begin(r), end(r));
    }

    template <typename Range>
    bool range_empty(const Range& r) {
        using std::begin;
        using std::end;
        return begin(r) == end(r);
    }

    // Range types

    template <typename Iterator>
    struct Irange {
        Iterator first, second;
        constexpr Iterator begin() const { return first; }
        constexpr Iterator end() const { return second; }
        constexpr bool empty() const { return first == second; }
        constexpr size_t size() const { return std::distance(first, second); }
    };

    template <typename Iterator> constexpr Irange<Iterator> irange(const Iterator& i, const Iterator& j) { return {i, j}; }
    template <typename Iterator> constexpr Irange<Iterator> irange(const pair<Iterator, Iterator>& p) { return {p.first, p.second}; }
    template <typename T> constexpr Irange<T*> array_range(T* ptr, size_t len) { return {ptr, ptr + len}; }

    // Integer sequences

    template <typename T>
    class IntegerSequenceIterator:
    public RandomAccessIterator<IntegerSequenceIterator<T>, const T> {
    public:
        IntegerSequenceIterator() = default;
        IntegerSequenceIterator(T init, T delta): cur(init), del(delta) {}
        const T& operator*() const noexcept { return cur; }
        IntegerSequenceIterator& operator+=(ptrdiff_t n) noexcept { cur += n * del; return *this; }
        ptrdiff_t operator-(const IntegerSequenceIterator& rhs) const noexcept
            { return (ptrdiff_t(cur) - ptrdiff_t(rhs.cur)) / ptrdiff_t(del); }
    private:
        T cur, del;
    };

    namespace PrionDetail {

        template <typename T>
        inline void adjust_integer_sequence(T& init, T& stop, T& delta, bool closed) noexcept {
            if (delta == T(0)) {
                stop = init + T(int(closed));
                delta = T(1);
                return;
            }
            if (stop != init && (stop > init) != (delta > T())) {
                stop = init;
                return;
            }
            T rem = (stop - init) % delta;
            if (rem != T())
                stop += delta - rem;
            else if (closed)
                stop += delta;
        }

    }

    template <typename T>
    Irange<IntegerSequenceIterator<T>> iseq(T init, T stop, T delta) noexcept {
        PrionDetail::adjust_integer_sequence(init, stop, delta, true);
        return {{init, delta}, {stop, delta}};
    }

    template <typename T>
    Irange<IntegerSequenceIterator<T>> iseq(T init, T stop) noexcept {
        T delta = stop >= init ? T(1) : T(-1);
        return iseq(init, stop, delta);
    }

    template <typename T>
    Irange<IntegerSequenceIterator<T>> iseq(T stop) noexcept {
        return iseq(T(), stop);
    }

    template <typename T>
    Irange<IntegerSequenceIterator<T>> xseq(T init, T stop, T delta) noexcept {
        PrionDetail::adjust_integer_sequence(init, stop, delta, false);
        return {{init, delta}, {stop, delta}};
    }

    template <typename T>
    Irange<IntegerSequenceIterator<T>> xseq(T init, T stop) noexcept {
        T delta = stop >= init ? T(1) : T(-1);
        return xseq(init, stop, delta);
    }

    template <typename T>
    Irange<IntegerSequenceIterator<T>> xseq(T stop) noexcept {
        return xseq(T(), stop);
    }

    // [Arithmetic functions]

    // Generic arithmetic functions

    template <typename T> inline T abs(T t) noexcept { using std::abs; return abs(t); }
    inline int128_t abs(int128_t t) noexcept { return t < 0 ? - t : t; }
    inline uint128_t abs(uint128_t t) noexcept { return t; }

    namespace PrionDetail {

        enum class NumMode {
            signed_integer,
            unsigned_integer,
            floating_point,
            not_numeric,
        };

        template <typename T>
        struct NumType {
            static constexpr NumMode value =
                std::is_floating_point<T>::value ? NumMode::floating_point :
                std::is_signed<T>::value ? NumMode::signed_integer :
                std::is_unsigned<T>::value ? NumMode::unsigned_integer : NumMode::not_numeric;
        };

        template <> struct NumType<int128_t> { static constexpr NumMode value = NumMode::signed_integer; };
        template <> struct NumType<uint128_t> { static constexpr NumMode value = NumMode::unsigned_integer; };

        template <typename T, NumMode Mode = NumType<T>::value> struct Divide;

        template <typename T>
        struct Divide<T, NumMode::signed_integer> {
            pair<T, T> operator()(T lhs, T rhs) const noexcept {
                auto q = lhs / rhs, r = lhs % rhs;
                if (r < T(0)) {
                    q += rhs < T(0) ? T(1) : T(-1);
                    r += Prion::abs(rhs);
                }
                return {q, r};
            }
        };

        template <typename T>
        struct Divide<T, NumMode::unsigned_integer> {
            pair<T, T> operator()(T lhs, T rhs) const noexcept {
                return {lhs / rhs, lhs % rhs};
            }
        };

        template <typename T>
        struct Divide<T, NumMode::floating_point> {
            pair<T, T> operator()(T lhs, T rhs) const noexcept {
                using std::fabs;
                using std::floor;
                using std::fmod;
                auto q = floor(lhs / rhs), r = fmod(lhs, rhs);
                if (r < T(0))
                    r += fabs(rhs);
                if (rhs < T(0) && r != T(0))
                    q += T(1);
                return {q, r};
            }
        };

        template <typename T> constexpr T unsigned_gcd(T a, T b) noexcept { return b == T(0) ? a : unsigned_gcd(b, a % b); }

        template <typename T, bool I = std::is_integral<T>::value, bool F = std::is_floating_point<T>::value>
        struct ShiftLeft;

        template <typename T>
        struct ShiftLeft<T, true, false> {
            static constexpr int maxbits = 8 * sizeof(T);
            T operator()(T t, int n) const {
                if (n <= - maxbits || n >= maxbits)
                    return 0;
                else if (n >= 0)
                    return t << n;
                else if (t < 0)
                    return - (- t >> - n);
                else
                    return t >> - n;
            }
        };

        template <typename T>
        struct ShiftLeft<T, false, true> {
            T operator()(T t, int n) const {
                using std::ldexp;
                return ldexp(t, n);
            }
        };

        template <typename T, NumMode Mode = NumType<T>::value> struct SignOf;

        template <typename T>
        struct SignOf<T, NumMode::signed_integer> {
            constexpr int operator()(T t) const noexcept
                { return t < T(0) ? -1 : t == T(0) ? 0 : 1; }
        };

        template <typename T>
        struct SignOf<T, NumMode::unsigned_integer> {
            constexpr int operator()(T t) const noexcept
                { return t != T(0); }
        };

        template <typename T>
        struct SignOf<T, NumMode::floating_point> {
            constexpr int operator()(T t) const noexcept
                { return t < T(0) ? -1 : t == T(0) ? 0 : 1; }
        };

    }

    template <typename T> constexpr T static_min(T t) noexcept { return t; }
    template <typename T, typename... Args> constexpr T static_min(T t, Args... args) noexcept
        { return t < static_min(args...) ? t : static_min(args...); }
    template <typename T> constexpr T static_max(T t) noexcept { return t; }
    template <typename T, typename... Args> constexpr T static_max(T t, Args... args) noexcept
        { return static_max(args...) < t ? t : static_max(args...); }
    template <typename T, typename T2, typename T3> constexpr T clamp(const T& x, const T2& min, const T3& max) noexcept
        { return x < T(min) ? T(min) : T(max) < x ? T(max) : x; }
    template <typename T> pair<T, T> divide(T lhs, T rhs) noexcept { return PrionDetail::Divide<T>()(lhs, rhs); }
    template <typename T> T quo(T lhs, T rhs) noexcept { return PrionDetail::Divide<T>()(lhs, rhs).first; }
    template <typename T> T rem(T lhs, T rhs) noexcept { return PrionDetail::Divide<T>()(lhs, rhs).second; }
    template <typename T> T shift_left(T t, int n) noexcept { return PrionDetail::ShiftLeft<T>()(t, n); }
    template <typename T> T shift_right(T t, int n) noexcept { return PrionDetail::ShiftLeft<T>()(t, - n); }
    template <typename T> constexpr int sign_of(T t) noexcept { return PrionDetail::SignOf<T>()(t); }

    // Integer arithmetic functions

    template <typename T>
    T binomial(T a, T b) noexcept {
        if (b < T(0) || b > a)
            return T(0);
        if (b == T(0) || b == a)
            return T(1);
        if (b > a / T(2))
            b = a - b;
        T n = T(1), d = T(1);
        while (b > 0) {
            n *= a--;
            d *= b--;
        }
        return n / d;
    }

    inline double xbinomial(int a, int b) noexcept {
        if (b < 0 || b > a)
            return 0;
        if (b == 0 || b == a)
            return 1;
        if (b > a / 2)
            b = a - b;
        double n = 1, d = 1;
        while (b > 0) {
            n *= a--;
            d *= b--;
        }
        return n / d;
    }

    template <typename T>
    constexpr T gcd(T a, T b) noexcept {
        using std::abs;
        return PrionDetail::unsigned_gcd(abs(a), abs(b));
    }

    template <typename T>
    constexpr T lcm(T a, T b) noexcept {
        using std::abs;
        return a == T(0) || b == T(0) ? T(0) : abs((a / gcd(a, b)) * b);
    }

    template <typename T>
    T int_power(T x, T y) noexcept {
        T z = T(1);
        while (y) {
            if (y & T(1))
                z *= x;
            x *= x;
            y >>= 1;
        }
        return z;
    }

    template <typename T>
    T int_sqrt(T t) noexcept {
        if (std::numeric_limits<T>::digits < std::numeric_limits<double>::digits)
            return T(floor(sqrt(double(t))));
        auto u = as_unsigned(t);
        using U = decltype(u);
        U result = 0, test = U(1) << (8 * sizeof(U) - 2);
        while (test > u)
            test >>= 2;
        while (test) {
            if (u >= result + test) {
                u -= result + test;
                result += test * 2;
            }
            result >>= 1;
            test >>= 2;
        }
        return T(result);
    }

    // Bitwise operations

    namespace PrionDetail {

        template <typename T> constexpr T rotl_helper(T t, int n) noexcept
            { return (t << (n % (8 * sizeof(T)))) | (t >> (8 * sizeof(T) - (n % (8 * sizeof(T))))); }
        template <typename T> constexpr T rotr_helper(T t, int n) noexcept
            { return (t >> (n % (8 * sizeof(T)))) | (t << (8 * sizeof(T) - (n % (8 * sizeof(T))))); }

    }

    constexpr size_t binary_size(uint64_t x) noexcept { return x == 0 ? 0 : 8 * sizeof(x) - __builtin_clzll(x); }
    constexpr size_t bits_set(uint64_t x) noexcept { return __builtin_popcountll(x); }
    constexpr uint64_t letter_to_mask(char c) noexcept
        { return c >= 'A' && c <= 'Z' ? 1ull << (c - 'A') : c >= 'a' && c <= 'z' ? 1ull << (c - 'a' + 26) : 0; }
    template <typename T> constexpr T rotl(T t, int n) noexcept
        { return n == 0 ? t : n < 0 ? PrionDetail::rotr_helper(t, - n) : PrionDetail::rotl_helper(t, n); }
    template <typename T> constexpr T rotr(T t, int n) noexcept
        { return n == 0 ? t : n < 0 ? PrionDetail::rotl_helper(t, - n) : PrionDetail::rotr_helper(t, n); }

    // Floating point arithmetic functions

    namespace PrionDetail {

        template <typename T2, typename T1, char Mode,
            bool FromFloat = std::is_floating_point<T1>::value>
            struct Round;

        template <typename T2, typename T1, char Mode>
        struct Round<T2, T1, Mode, true> {
            T2 operator()(T1 value) const noexcept {
                using std::ceil;
                using std::floor;
                T1 t = Mode == '<' ? floor(value) : Mode == '>' ? ceil(value) : floor(value + T1(1) / T1(2));
                return T2(t);
            }
        };

        template <typename T2, typename T1, char Mode>
        struct Round<T2, T1, Mode, false> {
            T2 operator()(T1 value) const noexcept {
                return T2(value);
            }
        };

    }

    template <typename T> constexpr T degrees(T rad) noexcept { return rad * (T(180) / c_pi<T>()); }
    template <typename T> constexpr T radians(T deg) noexcept { return deg * (c_pi<T>() / T(180)); }
    template <typename T1, typename T2> constexpr T2 interpolate(T1 x1, T2 y1, T1 x2, T2 y2, T1 x) noexcept
        { return x1 == x2 ? (y1 + y2) * (T1(1) / T1(2)) : y1 == y2 ? y1 : y1 + (y2 - y1) * ((x - x1) / (x2 - x1)); }
    template <typename T2, typename T1> T2 iceil(T1 value) noexcept { return PrionDetail::Round<T2, T1, '>'>()(value); }
    template <typename T2, typename T1> T2 ifloor(T1 value) noexcept { return PrionDetail::Round<T2, T1, '<'>()(value); }
    template <typename T2, typename T1> T2 round(T1 value) noexcept { return PrionDetail::Round<T2, T1, '='>()(value); }

    // [Functional utilities]

    // Function traits

    // Based on code by Kennytm and Victor Laskin
    // http://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda
    // http://vitiy.info/c11-functional-decomposition-easy-way-to-do-aop/

    namespace PrionDetail {

        template <typename Function>
        struct FunctionTraits:
        FunctionTraits<decltype(&Function::operator())> {};

        template <typename ReturnType, typename... Args>
        struct FunctionTraits<ReturnType (Args...)> {
            static constexpr size_t arity = sizeof...(Args);
            using argument_tuple = tuple<Args...>;
            using result_type = ReturnType;
            using signature = result_type(Args...);
            using std_function = function<signature>;
        };

        template <typename ReturnType, typename... Args>
        struct FunctionTraits<ReturnType (*)(Args...)>:
        FunctionTraits<ReturnType (Args...)> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct FunctionTraits<ReturnType (ClassType::*)(Args...)>:
        FunctionTraits<ReturnType (Args...)> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct FunctionTraits<ReturnType (ClassType::*)(Args...) const>:
        FunctionTraits<ReturnType (Args...)> {};

    }

    template <typename Function> struct Arity
        { static constexpr size_t value = PrionDetail::FunctionTraits<Function>::arity; };
    template <typename Function> using ArgumentTuple = typename PrionDetail::FunctionTraits<Function>::argument_tuple;
    template <typename Function, size_t Index> using ArgumentType = std::tuple_element_t<Index, ArgumentTuple<Function>>;
    template <typename Function> using ResultType = typename PrionDetail::FunctionTraits<Function>::result_type;
    template <typename Function> using FunctionSignature = typename PrionDetail::FunctionTraits<Function>::signature;
    template <typename Function> using StdFunction = typename PrionDetail::FunctionTraits<Function>::std_function;

    // Function operations

    template <typename Function> StdFunction<Function> stdfun(Function& lambda) { return StdFunction<Function>(lambda); }

    // tuple_invoke() is based on code from Cppreference.com
    // http://en.cppreference.com/w/cpp/utility/integer_sequence

    namespace PrionDetail {

        template <typename Function, typename Tuple, size_t... Indices>
        decltype(auto) invoke_helper(Function&& f, Tuple&& t, std::index_sequence<Indices...>) {
            return f(std::get<Indices>(std::forward<Tuple>(t))...);
        }

    }

    template<typename Function, typename Tuple>
    decltype(auto) tuple_invoke(Function&& f, Tuple&& t) {
        constexpr auto size = std::tuple_size<std::decay_t<Tuple>>::value;
        return PrionDetail::invoke_helper(std::forward<Function>(f), std::forward<Tuple>(t),
            std::make_index_sequence<size>{});
    }

    // Generic function objects

    struct DoNothing {
        void operator()() const noexcept {}
        template <typename T> void operator()(T&) const noexcept {}
        template <typename T> void operator()(const T&) const noexcept {}
    };

    struct Identity {
        template <typename T> T& operator()(T& t) const noexcept { return t; }
        template <typename T> const T& operator()(const T& t) const noexcept { return t; }
    };

    constexpr DoNothing do_nothing {};
    constexpr Identity identity {};

    // Hash functions

    namespace PrionDetail {

        inline void mix_hash(size_t& h1, size_t h2) noexcept {
            h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        }

    }

    class Djb2a {
    public:
        Djb2a& operator()(const void* ptr, size_t n) noexcept {
            auto p = static_cast<const uint8_t*>(ptr);
            if (p)
                for (size_t i = 0; i < n; ++i)
                    hash = ((hash << 5) + hash) ^ p[i];
            return *this;
        }
        Djb2a& operator()(const string& s) noexcept { return (*this)(s.data(), s.size()); }
        operator uint32_t() const noexcept { return hash; }
    private:
        uint32_t hash = 5381;
    };

    inline void hash_combine(size_t&) noexcept {}

    template <typename T>
    void hash_combine(size_t& hash, const T& t) noexcept {
        PrionDetail::mix_hash(hash, std::hash<T>()(t));
    }

    template <typename T, typename... Args>
    void hash_combine(size_t& hash, const T& t, const Args&... args) noexcept {
        hash_combine(hash, t);
        hash_combine(hash, args...);
    }

    template <typename... Args>
    size_t hash_value(const Args&... args) noexcept {
        size_t hash = 0;
        hash_combine(hash, args...);
        return hash;
    }

    inline size_t hash_bytes(const void* ptr, size_t n) {
        #if defined(PRI_COMPILER_CLANG)
            return std::__murmur2_or_cityhash<size_t>()(ptr, n);
        #elif defined(PRI_COMPILER_GCC)
            return std::_Hash_impl::hash(ptr, n);
        #else
            string s(static_cast<const char*>(ptr), n);
            return std::hash<string>()(s);
        #endif
    }

    inline void hash_bytes(size_t& hash, const void* ptr, size_t n) {
        PrionDetail::mix_hash(hash, hash_bytes(ptr, n));
    }

    template <typename Range>
    size_t hash_range(const Range& range) noexcept {
        size_t hash = 0;
        for (auto&& x: range)
            hash_combine(hash, x);
        return hash;
    }

    template <typename Range>
    void hash_range(size_t& hash, const Range& range) noexcept {
        for (auto&& x: range)
            hash_combine(hash, x);
    }

    template <typename... Args> struct TupleHash
        { size_t operator()(const tuple<Args...>& t) const { return tuple_invoke(hash_value<Args...>, t); } };
    template <typename... Args> struct TupleHash<tuple<Args...>>: TupleHash<Args...> {};

    // Keyword arguments

    namespace PrionDetail {

        template <typename K, typename V, bool = std::is_convertible<K, V>::value> struct Kwcopy;
        template <typename K, typename V> struct Kwcopy<K, V, true> { void operator()(const K& a, V& p) const { p = V(a); } };
        template <typename K, typename V> struct Kwcopy<K, V, false> { void operator()(const K&, V&) const {} };

        template <typename K> struct Kwparam {
            const void* key;
            K val;
        };

    }

    template <typename K> struct Kwarg {
        constexpr Kwarg() {}
        template <typename A> PrionDetail::Kwparam<K> operator=(const A& a) const { return {this, K(a)}; }
    };

    template <typename K, typename V, typename K2, typename... Args>
        bool kwget(const Kwarg<K>& k, V& v, const PrionDetail::Kwparam<K2>& p, const Args&... args) {
            if (&k != p.key)
                return kwget(k, v, args...);
            PrionDetail::Kwcopy<K2, V>()(p.val, v);
            return true;
        }

    template <typename K, typename V, typename... Args>
        bool kwget(const Kwarg<K>& k, V& v, const Kwarg<bool>& p, const Args&... args) {
            return kwget(k, v, p = true, args...);
        }

    template <typename K, typename V> bool kwget(const Kwarg<K>&, V&) { return false; }

    // Scope guards

    template <typename T>
    class Resource {
    public:
        using resource_type = T;
        Resource() = default;
        explicit Resource(T t): res(t), del() {}
        template <typename D> Resource(T t, D d): res(t), del() {
            try { del = deleter(d); }
            catch (...) { d(res); throw; }
        }
        Resource(Resource&& r) noexcept: res(r.res), del(r.del) { r.res = T(); r.del = {}; }
        ~Resource() noexcept {
            if (del) {
                try { del(res); }
                catch (...) {}
            }
        }
        Resource& operator=(Resource&& r) noexcept {
            Resource temp(move(r));
            std::swap(res, temp.res);
            std::swap(del, temp.del);
            return *this;
        }
        operator T&() noexcept { return res; }
        operator T() const noexcept { return res; }
        explicit operator bool() const noexcept { return static_cast<bool>(res); }
        bool operator!() const noexcept { return ! static_cast<bool>(res); }
        T& get() noexcept { return res; }
        T get() const noexcept { return res; }
        T release() noexcept { del = {}; return res; }
    private:
        using deleter = function<void(T&)>;
        T res = T();
        deleter del = {};
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
    };

    template <typename T>
    class Resource<T*> {
    public:
        using resource_type = T*;
        using value_type = T;
        Resource() = default;
        explicit Resource(T* t): res(t), del() {}
        template <typename D> Resource(T* t, D d): res(t), del() {
            try { del = deleter(d); }
            catch (...) { if (res) d(res); throw; }
        }
        Resource(Resource&& r) noexcept: res(r.res), del(r.del) { r.res = nullptr; r.del = {}; }
        ~Resource() noexcept {
            if (res && del) {
                try { del(res); }
                catch (...) {}
            }
        }
        Resource& operator=(Resource&& r) noexcept {
            Resource temp(move(r));
            std::swap(res, temp.res);
            std::swap(del, temp.del);
            return *this;
        }
        operator T*&() noexcept { return res; }
        operator T*() const noexcept { return res; }
        explicit operator bool() const noexcept { return res != nullptr; }
        bool operator!() const noexcept { return res == nullptr; }
        T& operator*() noexcept { return *res; }
        const T& operator*() const noexcept { return *res; }
        T* operator->() const noexcept { return res; }
        T*& get() noexcept { return res; }
        T* get() const noexcept { return res; }
        T* release() noexcept { del = {}; return res; }
    private:
        using deleter = function<void(T*)>;
        T* res = nullptr;
        deleter del = {};
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
    };

    template <>
    class Resource<void*> {
    public:
        using resource_type = void*;
        using value_type = void;
        Resource() = default;
        explicit Resource(void* t): res(t), del() {}
        template <typename D> Resource(void* t, const D& d): res(t) {
            try { del = deleter(d); }
            catch (...) { if (res) d(res); throw; }
        }
        Resource(Resource&& r) noexcept: res(r.res), del(r.del) { r.res = nullptr; r.del = {}; }
        ~Resource() noexcept {
            if (res && del) {
                try { del(res); }
                catch (...) {}
            }
        }
        Resource& operator=(Resource&& r) noexcept {
            Resource temp(move(r));
            std::swap(res, temp.res);
            std::swap(del, temp.del);
            return *this;
        }
        operator void*&() noexcept { return res; }
        operator void*() const noexcept { return res; }
        void*& get() noexcept { return res; }
        void* get() const noexcept { return res; }
        void* release() noexcept { del = {}; return res; }
    private:
        using deleter = function<void(void*)>;
        void* res = nullptr;
        deleter del = {};
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
    };

    template <typename T, typename D>
    Resource<T> make_resource(T&& t, D d) {
        return {t, d};
    }

    // Based on ideas from Evgeny Panasyuk (https://github.com/panaseleus/stack_unwinding)
    // and Andrei Alexandrescu (https://isocpp.org/files/papers/N4152.pdf)

    namespace PrionDetail {

        extern "C" char* __cxa_get_globals();
        inline unsigned uncaught_exception_count() noexcept { return *reinterpret_cast<unsigned*>(__cxa_get_globals() + sizeof(void*)); }

        // MSVC implementation (for reference in case we ever support it):
        // extern "C" char* _getptd();
        // inline unsigned uncaught_exception_count() noexcept {
        //     static const size_t offset = sizeof(void*) == 8 ? 0x100 : 0x90;
        //     return *reinterpret_cast<unsigned*>(_getptd() + offset);
        // }

        template <int Mode, bool Conditional = Mode >= 0>
        struct ScopeExitBase {
            bool should_run() const noexcept { return true; }
        };

        template <int Mode>
        struct ScopeExitBase<Mode, true> {
            unsigned exceptions;
            ScopeExitBase(): exceptions(uncaught_exception_count()) {}
            bool should_run() const noexcept { return (exceptions == uncaught_exception_count()) == (Mode > 0); }
        };

        template <int Mode>
        class ConditionalScopeExit:
        private ScopeExitBase<Mode> {
        public:
            using callback = function<void()>;
            explicit ConditionalScopeExit(callback f) {
                if (Mode > 0) {
                    func = f;
                } else {
                    try { func = f; }
                    catch (...) { silent_call(f); throw; }
                }
            }
            ~ConditionalScopeExit() noexcept {
                if (ScopeExitBase<Mode>::should_run())
                    silent_call(func);
            }
            void release() noexcept { func = nullptr; }
        private:
            function<void()> func;
            ConditionalScopeExit(const ConditionalScopeExit&) = delete;
            ConditionalScopeExit(ConditionalScopeExit&&) = delete;
            ConditionalScopeExit& operator=(const ConditionalScopeExit&) = delete;
            ConditionalScopeExit& operator=(ConditionalScopeExit&&) = delete;
            static void silent_call(callback& f) noexcept { if (f) { try { f(); } catch (...) {} } }
        };

    }

    using ScopeExit = PrionDetail::ConditionalScopeExit<-1>;
    using ScopeSuccess = PrionDetail::ConditionalScopeExit<1>;
    using ScopeFailure = PrionDetail::ConditionalScopeExit<0>;

    class ScopedTransaction {
    public:
        using callback = function<void()>;
        ScopedTransaction() noexcept {}
        ~ScopedTransaction() noexcept { rollback(); }
        void call(callback func, callback undo) {
            stack.push_back(nullptr);
            if (func)
                func();
            stack.back().swap(undo);
        }
        void commit() noexcept { stack.clear(); }
        void rollback() noexcept {
            for (auto i = stack.rbegin(); i != stack.rend(); ++i)
                if (*i) try { (*i)(); } catch (...) {}
            stack.clear();
        }
    private:
        vector<callback> stack;
        ScopedTransaction(const ScopedTransaction&) = delete;
        ScopedTransaction(ScopedTransaction&&) = delete;
        ScopedTransaction& operator=(const ScopedTransaction&) = delete;
        ScopedTransaction& operator=(ScopedTransaction&&) = delete;
    };

    // [I/O utilities]

    // File I/O operations

    namespace PrionDetail {

        inline bool load_file_helper(FILE* fp, string& dst, size_t limit) {
            static constexpr size_t bufsize = 64_kb;
            size_t offset = 0;
            while (! (feof(fp) || ferror(fp)) && dst.size() < limit) {
                dst.resize(std::min(offset + bufsize, limit), 0);
                offset += fread(&dst[0] + offset, 1, dst.size() - offset, fp);
            }
            dst.resize(offset);
            return ! ferror(fp);
        }

        inline bool save_file_helper(FILE* fp, const void* ptr, size_t n) {
            auto cptr = static_cast<const char*>(ptr);
            size_t offset = 0;
            while (offset < n && ! ferror(fp))
                offset += fwrite(cptr + offset, 1, n - offset, fp);
            return ! ferror(fp);
        }

    }

    #if defined(PRI_TARGET_UNIX)

        inline bool load_file(const string& file, string& dst, size_t limit = npos) {
            dst.clear();
            FILE* fp = fopen(file.data(), "rb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::load_file_helper(fp, dst, limit);
        }

        inline bool save_file(const string& file, const void* ptr, size_t n, bool append = false) {
            auto fp = fopen(file.data(), append ? "ab" : "wb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::save_file_helper(fp, ptr, n);
        }

    #else

        inline bool load_file(const wstring& file, string& dst, size_t limit = npos) {
            dst.clear();
            FILE* fp = _wfopen(file.data(), L"rb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::load_file_helper(fp, dst, limit);
        }

        inline bool save_file(const wstring& file, const void* ptr, size_t n, bool append = false) {
            auto fp = _wfopen(file.data(), append ? L"ab" : L"wb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::save_file_helper(fp, ptr, n);
        }

        inline bool load_file(const string& file, string& dst, size_t limit = npos)
            { return load_file(uconv<wstring>(file), dst, limit); }
        inline bool save_file(const string& file, const void* ptr, size_t n, bool append)
            { return save_file(uconv<wstring>(file), ptr, n, append); }
        inline bool save_file(const wstring& file, const string& src, bool append = false)
            { return save_file(file, src.data(), src.size(), append); }

    #endif

    inline bool save_file(const string& file, const string& src, bool append = false)
        { return save_file(file, src.data(), src.size(), append); }

    // Terminal I/O operations

    inline bool is_stdout_redirected() noexcept { return ! isatty(1); }

    namespace PrionDetail {

        inline int grey(int n) noexcept {
            return 231 + clamp(n, 1, 24);
        }

        inline int rgb(int rgb) noexcept {
            int r = clamp((rgb / 100) % 10, 1, 6);
            int g = clamp((rgb / 10) % 10, 1, 6);
            int b = clamp(rgb % 10, 1, 6);
            return 36 * r + 6 * g + b - 27;
        }

    }

    static constexpr const char* xt_up           = "\x1b[A";    // Cursor up
    static constexpr const char* xt_down         = "\x1b[B";    // Cursor down
    static constexpr const char* xt_right        = "\x1b[C";    // Cursor right
    static constexpr const char* xt_left         = "\x1b[D";    // Cursor left
    static constexpr const char* xt_erase_left   = "\x1b[1K";   // Erase left
    static constexpr const char* xt_erase_right  = "\x1b[K";    // Erase right
    static constexpr const char* xt_erase_above  = "\x1b[1J";   // Erase above
    static constexpr const char* xt_erase_below  = "\x1b[J";    // Erase below
    static constexpr const char* xt_erase_line   = "\x1b[2K";   // Erase line
    static constexpr const char* xt_clear        = "\x1b[2J";   // Clear screen
    static constexpr const char* xt_reset        = "\x1b[0m";   // Reset attributes
    static constexpr const char* xt_bold         = "\x1b[1m";   // Bold
    static constexpr const char* xt_under        = "\x1b[4m";   // Underline
    static constexpr const char* xt_black        = "\x1b[30m";  // Black fg
    static constexpr const char* xt_red          = "\x1b[31m";  // Red fg
    static constexpr const char* xt_green        = "\x1b[32m";  // Green fg
    static constexpr const char* xt_yellow       = "\x1b[33m";  // Yellow fg
    static constexpr const char* xt_blue         = "\x1b[34m";  // Blue fg
    static constexpr const char* xt_magenta      = "\x1b[35m";  // Magenta fg
    static constexpr const char* xt_cyan         = "\x1b[36m";  // Cyan fg
    static constexpr const char* xt_white        = "\x1b[37m";  // White fg
    static constexpr const char* xt_black_bg     = "\x1b[40m";  // Black bg
    static constexpr const char* xt_red_bg       = "\x1b[41m";  // Red bg
    static constexpr const char* xt_green_bg     = "\x1b[42m";  // Green bg
    static constexpr const char* xt_yellow_bg    = "\x1b[43m";  // Yellow bg
    static constexpr const char* xt_blue_bg      = "\x1b[44m";  // Blue bg
    static constexpr const char* xt_magenta_bg   = "\x1b[45m";  // Magenta bg
    static constexpr const char* xt_cyan_bg      = "\x1b[46m";  // Cyan bg
    static constexpr const char* xt_white_bg     = "\x1b[47m";  // White bg

    inline string xt_move_up(int n) { return "\x1b[" + dec(n) + 'A'; }                                // Cursor up n spaces
    inline string xt_move_down(int n) { return "\x1b[" + dec(n) + 'B'; }                              // Cursor down n spaces
    inline string xt_move_right(int n) { return "\x1b[" + dec(n) + 'C'; }                             // Cursor right n spaces
    inline string xt_move_left(int n) { return "\x1b[" + dec(n) + 'D'; }                              // Cursor left n spaces
    inline string xt_colour(int rgb) { return "\x1b[38;5;" + dec(PrionDetail::rgb(rgb)) + 'm'; }      // Set fg colour to an RGB value (0-5)
    inline string xt_colour_bg(int rgb) { return "\x1b[48;5;" + dec(PrionDetail::rgb(rgb)) + 'm'; }   // Set bg colour to an RGB value (0-5)
    inline string xt_grey(int grey) { return "\x1b[38;5;" + dec(PrionDetail::grey(grey)) + 'm'; }     // Set fg colour to a grey level (1-24)
    inline string xt_grey_bg(int grey) { return "\x1b[48;5;" + dec(PrionDetail::grey(grey)) + 'm'; }  // Set bg colour to a grey level (1-24)

    // [Random numbers]

    // Simple random generators

    template <typename RNG>
    bool random_bool(RNG& rng, double p = 0.5) {
        std::bernoulli_distribution d(clamp(p, 0, 1));
        return d(rng);
    }

    template <typename RNG>
    int random_int(RNG& rng, int n) {
        if (n < 2)
            return 0;
        std::uniform_int_distribution<int> d(0, n - 1);
        return d(rng);
    }

    template <typename RNG>
    int random_int(RNG& rng, int n1, int n2) {
        if (n1 == n2)
            return n1;
        if (n1 > n2)
            std::swap(n1, n2);
        std::uniform_int_distribution<int> d(n1, n2);
        return d(rng);
    }

    template <typename T, typename RNG>
    T random_real(RNG& rng, T x = T(1), T y = T(0)) {
        if (x == y)
            return x;
        if (x > y)
            std::swap(x, y);
        std::uniform_real_distribution<T> d(x, y);
        return d(rng);
    }

    template <typename RNG, typename Range>
    RangeValue<Range> random_choice(RNG& rng, const Range& range) {
        using std::begin;
        using std::end;
        auto i = begin(range), j = end(range);
        size_t n = std::distance(i, j);
        if (n == 0)
            return {};
        std::uniform_int_distribution<size_t> d(0, n - 1);
        std::advance(i, d(rng));
        return *i;
    }

    template <typename RNG, typename T>
    T random_choice(RNG& rng, std::initializer_list<T> list) {
        if (list.size() == 0)
            return {};
        std::uniform_int_distribution<size_t> d(0, list.size() - 1);
        return list.begin()[d(rng)];
    }

    // [Strings and related functions]

    // Character functions

    constexpr bool ascii_iscntrl(char c) noexcept { return uint8_t(c) <= 31 || c == 127; }
    constexpr bool ascii_isdigit(char c) noexcept { return c >= '0' && c <= '9'; }
    constexpr bool ascii_isgraph(char c) noexcept { return c >= '!' && c <= '~'; }
    constexpr bool ascii_islower(char c) noexcept { return c >= 'a' && c <= 'z'; }
    constexpr bool ascii_isprint(char c) noexcept { return c >= ' ' && c <= '~'; }
    constexpr bool ascii_ispunct(char c) noexcept
        { return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~'); }
    constexpr bool ascii_isspace(char c) noexcept { return (c >= '\t' && c <= '\r') || c == ' '; }
    constexpr bool ascii_isupper(char c) noexcept { return c >= 'A' && c <= 'Z'; }
    constexpr bool ascii_isalpha(char c) noexcept { return ascii_islower(c) || ascii_isupper(c); }
    constexpr bool ascii_isalnum(char c) noexcept { return ascii_isalpha(c) || ascii_isdigit(c); }
    constexpr bool ascii_isxdigit(char c) noexcept { return ascii_isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
    constexpr bool ascii_isalnum_w(char c) noexcept { return ascii_isalnum(c) || c == '_'; }
    constexpr bool ascii_isalpha_w(char c) noexcept { return ascii_isalpha(c) || c == '_'; }
    constexpr bool ascii_ispunct_w(char c) noexcept { return ascii_ispunct(c) && c != '_'; }
    constexpr char ascii_tolower(char c) noexcept { return ascii_isupper(c) ? c + 32 : c; }
    constexpr char ascii_toupper(char c) noexcept { return ascii_islower(c) ? c - 32 : c; }
    template <typename T> constexpr T char_to(char c) noexcept { return T(uint8_t(c)); }

    // General string functions

    inline bool ascii_icase_equal(const string& lhs, const string& rhs) noexcept {
        if (lhs.size() != rhs.size())
            return false;
        for (size_t i = 0, n = lhs.size(); i < n; ++i)
            if (ascii_toupper(lhs[i]) != ascii_toupper(rhs[i]))
                return false;
        return true;
    }

    inline bool ascii_icase_less(const string& lhs, const string& rhs) noexcept {
        for (size_t i = 0, n = std::min(lhs.size(), rhs.size()); i < n; ++i) {
            char a = ascii_toupper(lhs[i]), b = ascii_toupper(rhs[i]);
            if (a != b)
                return uint8_t(a) < uint8_t(b);
        }
        return lhs.size() < rhs.size();
    }

    inline string ascii_lowercase(const string& s) {
        auto r = s;
        std::transform(r.begin(), r.end(), r.begin(), ascii_tolower);
        return r;
    }

    inline string ascii_uppercase(const string& s) {
        auto r = s;
        std::transform(r.begin(), r.end(), r.begin(), ascii_toupper);
        return r;
    }

    inline string ascii_titlecase(const string& s) {
        auto r = s;
        bool was_alpha = false;
        for (char& c: r) {
            if (was_alpha)
                c = ascii_tolower(c);
            else
                c = ascii_toupper(c);
            was_alpha = ascii_isalpha(c);
        }
        return r;
    }

    inline string ascii_sentencecase(const string& s) {
        auto r = s;
        bool new_sentence = true, was_break = false;
        for (char& c: r) {
            if (c == '\n' || c == '\f' || c == '\r') {
                if (was_break)
                    new_sentence = true;
                was_break = true;
            } else {
                was_break = false;
                if (c == '.') {
                    new_sentence = true;
                } else if (new_sentence && ascii_isalpha(c)) {
                    c = ascii_toupper(c);
                    new_sentence = false;
                }
            }
        }
        return r;
    }

    template <typename C>
    size_t cstr_size(const C* ptr) {
        if (! ptr)
            return 0;
        if (sizeof(C) == 1)
            return std::strlen(reinterpret_cast<const char*>(ptr));
        if (sizeof(C) == sizeof(wchar_t))
            return std::wcslen(reinterpret_cast<const wchar_t*>(ptr));
        size_t n = 0;
        while (ptr[n] != C(0))
            ++n;
        return n;
    }

    inline u8string dent(size_t depth) { return u8string(4 * depth, ' '); }

    template <typename InputRange>
    string join(const InputRange& range, const string& delim = {}) {
        string result;
        for (auto& s: range) {
            result += s;
            result += delim;
        }
        if (! result.empty() && ! delim.empty())
            result.resize(result.size() - delim.size());
        return result;
    }

    template <typename OutputIterator>
    void split(const string& src, OutputIterator dst, const string& delim = ascii_whitespace) {
        if (delim.empty()) {
            if (! src.empty()) {
                *dst = src;
                ++dst;
            }
            return;
        }
        size_t i = 0, j = 0, size = src.size();
        while (j < size) {
            i = src.find_first_not_of(delim, j);
            if (i == npos)
                break;
            j = src.find_first_of(delim, i);
            if (j == npos)
                j = size;
            *dst = src.substr(i, j - i);
            ++dst;
        }
    }

    inline string trim(const string& str, const string& chars = ascii_whitespace) {
        size_t pos = str.find_first_not_of(chars);
        if (pos == npos)
            return {};
        else
            return str.substr(pos, str.find_last_not_of(chars) + 1 - pos);
    }

    inline string trim_left(const string& str, const string& chars = ascii_whitespace) {
        size_t pos = str.find_first_not_of(chars);
        if (pos == npos)
            return {};
        else
            return str.substr(pos, npos);
    }

    inline string trim_right(const string& str, const string& chars = ascii_whitespace) {
        return str.substr(0, str.find_last_not_of(chars) + 1);
    }

    inline string unqualify(const string& str) {
        size_t pos = str.find_last_of(".:");
        if (pos == npos)
            return str;
        else
            return str.substr(pos + 1, npos);
    }

    // String formatting and parsing functions

    inline unsigned long long binnum(const string& str) noexcept { return strtoull(str.data(), nullptr, 2); }
    inline long long decnum(const string& str) noexcept { return strtoll(str.data(), nullptr, 10); }
    inline unsigned long long hexnum(const string& str) noexcept { return strtoull(str.data(), nullptr, 16); }
    inline double fpnum(const string& str) noexcept { return strtod(str.data(), nullptr); }

    template <typename T>
    u8string fp_format(T t, char mode = 'g', int prec = 6) {
        static const u8string modes = "EFGHefgh";
        if (modes.find(mode) == npos)
            throw std::invalid_argument("Invalid floating point mode: " + quote(u8string{mode}));
        u8string buf(20, '\0'), fmt;
        switch (mode) {
            case 'H':  fmt = "%#.*G"; break;
            case 'h':  fmt = "%#.*g"; break;
            default:   fmt = u8string("%.*") + mode; break;
        }
        auto x = double(t);
        int rc = 0;
        for (;;) {
            rc = snprintf(&buf[0], buf.size(), fmt.data(), prec, x);
            if (rc < 0)
                throw std::system_error(errno, std::generic_category(), "snprintf()");
            if (size_t(rc) < buf.size())
                break;
            buf.resize(2 * buf.size());
        }
        buf.resize(rc);
        if (mode != 'F' && mode != 'f') {
            size_t p = buf.find_first_of("Ee");
            if (p == npos)
                p = buf.size();
            if (buf[p - 1] == '.') {
                buf.erase(p - 1, 1);
                --p;
            }
            if (p < buf.size()) {
                ++p;
                if (buf[p] == '+')
                    buf.erase(p, 1);
                else if (buf[p] == '-')
                    ++p;
                size_t q = std::min(buf.find_first_not_of('0', p), buf.size() - 1);
                if (q > p)
                    buf.erase(p, q - p);
            }
        }
        return buf;
    }

    inline int64_t si_to_int(const u8string& s) {
        using limits = std::numeric_limits<int64_t>;
        static constexpr const char* prefixes = "KMGTPEZY";
        char* endp = nullptr;
        errno = 0;
        int64_t n = strtoll(s.data(), &endp, 10);
        if (errno == ERANGE)
            throw std::range_error("Out of range: " + quote(s, true));
        if (errno || endp == s.data())
            throw std::invalid_argument("Invalid number: " + quote(s, true));
        if (ascii_isspace(*endp))
            ++endp;
        if (n && ascii_isalpha(*endp)) {
            auto pp = strchr(prefixes, ascii_toupper(*endp));
            if (pp) {
                int64_t steps = pp - prefixes + 1;
                double limit = log10(double(limits::max()) / double(abs(n))) / 3;
                if (steps > limit)
                    throw std::range_error("Out of range: " + quote(s, true));
                n *= int_power(int64_t(1000), steps);
            }
        }
        return n;
    }

    inline double si_to_float(const u8string& s) {
        using limits = std::numeric_limits<double>;
        static constexpr const char* prefixes = "yzafpnum kMGTPEZY";
        char* endp = nullptr;
        errno = 0;
        double x = strtod(s.data(), &endp);
        if (errno == ERANGE)
            throw std::range_error("Out of range: " + quote(s, true));
        if (errno || endp == s.data())
            throw std::invalid_argument("Invalid number: " + quote(s, true));
        if (ascii_isspace(*endp))
            ++endp;
        char c = *endp;
        if (x && ascii_isalpha(c)) {
            if (c == 'K')
                c = 'k';
            auto pp = strchr(prefixes, c);
            if (pp) {
                int steps = pp - prefixes - 8;
                double limit = log10(limits::max() / fabs(x)) / 3;
                if (steps > limit)
                    throw std::range_error("Out of range: " + quote(s, true));
                x *= pow(1000.0, steps);
            }
        }
        return x;
    }

    inline u8string hexdump(const void* ptr, size_t n, size_t block = 0) {
        if (ptr == nullptr || n == 0)
            return {};
        u8string result;
        result.reserve(3 * n);
        size_t col = 0;
        auto bptr = static_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < n; ++i) {
            PrionDetail::append_hex_byte(bptr[i], result);
            if (++col == block) {
                result += '\n';
                col = 0;
            } else {
                result += ' ';
            }
        }
        result.pop_back();
        return result;
    }

    inline u8string hexdump(const string& str, size_t block = 0) { return hexdump(str.data(), str.size(), block); }

    template <typename T> string to_str(const T& t);

    namespace PrionDetail {

        template <typename> struct SfinaeTrue: std::true_type {};
        template <typename T> auto check_std_begin(int) -> SfinaeTrue<decltype(std::begin(std::declval<T>()))>;
        template <typename T> auto check_std_begin(long) -> std::false_type;
        template <typename T> auto check_std_end(int) -> SfinaeTrue<decltype(std::end(std::declval<T>()))>;
        template <typename T> auto check_std_end(long) -> std::false_type;
        template <typename T> struct CheckStdBegin: decltype(check_std_begin<T>(0)) {};
        template <typename T> struct CheckStdEnd: decltype(check_std_end<T>(0)) {};
        template <typename T> struct IsRangeType { static constexpr bool value = CheckStdBegin<T>::value && CheckStdEnd<T>::value; };

        template <typename R, typename I = decltype(std::begin(std::declval<R>())),
            typename V = typename std::iterator_traits<I>::value_type>
        struct RangeToString {
            string operator()(const R& r) const {
                string s = "[";
                for (const V& v: r) {
                    s += to_str(v);
                    s += ',';
                }
                if (s.size() > 1)
                    s.pop_back();
                s += ']';
                return s;
            }
        };

        template <typename R, typename I, typename K, typename V>
        struct RangeToString<R, I, pair<K, V>> {
            string operator()(const R& r) const {
                string s = "{";
                for (const auto& kv: r) {
                    s += to_str(kv.first);
                    s += ':';
                    s += to_str(kv.second);
                    s += ',';
                }
                if (s.size() > 1)
                    s.pop_back();
                s += '}';
                return s;
            }
        };

        template <typename R, typename I>
        struct RangeToString<R, I, char> {
            string operator()(const R& r) const {
                using std::begin;
                using std::end;
                return string(begin(r), end(r));
            }
        };

        template <typename T, bool I = std::is_integral<T>::value,
            bool R = IsRangeType<T>::value>
        struct ObjectToString {
            string operator()(const T& t) const {
                std::ostringstream out;
                out << t;
                return out.str();
            }
        };

        template <typename T> struct ObjectToString<T, true, false> { string operator()(T t) const { return dec(t); } };
        template <> struct ObjectToString<int128_t> { string operator()(int128_t t) const { return dec(t); } };
        template <> struct ObjectToString<uint128_t> { string operator()(uint128_t t) const { return dec(t); } };
        template <typename T> struct ObjectToString<T, false, true>: RangeToString<T> {};
        template <> struct ObjectToString<string> { string operator()(const string& t) const { return t; } };
        template <> struct ObjectToString<char*> { string operator()(char* t) const { return t ? string(t) : string(); } };
        template <> struct ObjectToString<const char*> { string operator()(const char* t) const { return t ? string(t) : string(); } };
        template <> struct ObjectToString<char> { string operator()(char t) const { return {t}; } };
        template <> struct ObjectToString<bool> { string operator()(bool t) const { return t ? "true" : "false"; } };

        template <typename T1, typename T2> struct ObjectToString<pair<T1, T2>, false, false> {
            string operator()(const pair<T1, T2>& t) const {
                return '{' + ObjectToString<T1>()(t.first) + ',' + ObjectToString<T2>()(t.second) + '}';
            }
        };

    }

    template <typename T> inline string to_str(const T& t) { return PrionDetail::ObjectToString<T>()(t); }

    // HTML/XML tags

    class Tag {
    public:
        Tag() = default;
        Tag(const u8string& text, std::ostream& out) {
            u8string start = trim_right(text, "\n");
            if (start.empty() || start[0] != '<' || ! ascii_isalnum_w(start[1]) || start.back() != '>')
                throw std::invalid_argument("Invalid HTML tag: " + quote(text));
            if (start.end()[-2] == '/') {
                out << text;
                return;
            }
            os = &out;
            size_t lines = text.size() - start.size();
            if (lines >= 2)
                start += '\n';
            out << start;
            auto cut = std::find_if_not(text.begin() + 2, text.end(), ascii_isalnum_w);
            end = "</" + u8string(text.begin() + 1, cut) + ">";
            if (lines >= 1)
                end += '\n';
        }
        Tag(Tag&& t) noexcept: end(move(t.end)), os(t.os) { t.os = nullptr; }
        ~Tag() noexcept { if (os) try { *os << end; } catch (...) {} }
        Tag& operator=(Tag&& t) noexcept {
            if (os)
                *os << end;
            end = move(t.end);
            os = t.os;
            t.os = nullptr;
            return *this;
        }
    private:
        u8string end;
        std::ostream* os = nullptr;
        Tag(const Tag&) = delete;
        Tag& operator=(const Tag&) = delete;
    };

    // [Threads]

    // Thread class

    #if defined(PRI_TARGET_UNIX)

        class Thread {
        public:
            using callback = function<void()>;
            using id_type = pthread_t;
            Thread() = default;
            explicit Thread(callback f) {
                if (f) {
                    impl = make_unique<impl_type>();
                    impl->payload = f;
                    impl->state = thread_running;
                    int rc = pthread_create(&impl->thread, nullptr, thread_callback, impl.get());
                    if (rc)
                        throw std::system_error(rc, std::generic_category(), "pthread_create()");
                }
            }
            ~Thread() noexcept { if (impl && impl->state != thread_joined) pthread_join(impl->thread, nullptr); }
            Thread(Thread&&) = default;
            Thread& operator=(Thread&&) = default;
            id_type get_id() const noexcept { return impl ? impl->thread : id_type(); }
            bool poll() noexcept { return ! impl || impl->state != thread_running; }
            void wait() {
                if (! impl || impl->state == thread_joined)
                    return;
                int rc = pthread_join(impl->thread, nullptr);
                impl->state = thread_joined;
                if (rc)
                    throw std::system_error(rc, std::generic_category(), "pthread_join()");
                if (impl->except)
                    std::rethrow_exception(impl->except);
            }
            static size_t cpu_threads() noexcept {
                size_t n = 0;
                #if defined(PRI_TARGET_APPLE)
                    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
                    host_basic_info_data_t info;
                    if (host_info(mach_host_self(), HOST_BASIC_INFO,
                            reinterpret_cast<host_info_t>(&info), &count) == 0)
                        n = info.avail_cpus;
                #else
                    char buf[256];
                    FILE* f = fopen("/proc/cpuinfo", "r");
                    while (fgets(buf, sizeof(buf), f))
                        if (memcmp(buf, "processor", 9) == 0)
                            ++n;
                    fclose(f);
                #endif
                if (n == 0)
                    n = 1;
                return n;
            }
            static id_type current() noexcept { return pthread_self(); }
            static void yield() noexcept { sched_yield(); }
        private:
            enum state_type {
                thread_running,
                thread_complete,
                thread_joined,
            };
            struct impl_type {
                std::exception_ptr except;
                Thread::callback payload;
                std::atomic<int> state;
                pthread_t thread;
            };
            unique_ptr<impl_type> impl;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            static void* thread_callback(void* ptr) noexcept {
                auto t = static_cast<impl_type*>(ptr);
                if (t->payload) {
                    try { t->payload(); }
                    catch (...) { t->except = std::current_exception(); }
                }
                t->state = thread_complete;
                return nullptr;
            }
        };

    #else

        class Thread {
        public:
            using callback = function<void()>;
            using id_type = uint32_t;
            Thread() = default;
            explicit Thread(callback f) {
                if (f) {
                    impl = make_unique<impl_type>();
                    impl->payload = f;
                    impl->state = thread_running;
                    impl->thread = CreateThread(nullptr, 0, thread_callback, impl.get(), 0, &impl->key);
                    if (! impl->thread)
                        throw std::system_error(GetLastError(), windows_category(), "CreateThread()");
                }
            }
            ~Thread() noexcept {
                if (impl) {
                    if (impl->state != thread_joined)
                        WaitForSingleObject(impl->thread, INFINITE);
                    CloseHandle(impl->thread);
                }
            }
            Thread(Thread&&) = default;
            Thread& operator=(Thread&&) = default;
            id_type get_id() const noexcept { return impl ? impl->key : 0; }
            bool poll() noexcept { return ! impl || impl->state != thread_running; }
            void wait() {
                if (! impl || impl->state == thread_joined)
                    return;
                auto rc = WaitForSingleObject(impl->thread, INFINITE);
                impl->state = thread_joined;
                if (rc == WAIT_FAILED)
                    throw std::system_error(GetLastError(), windows_category(), "WaitForSingleObject()");
                if (impl->except)
                    std::rethrow_exception(impl->except);
            }
            static size_t cpu_threads() noexcept {
                SYSTEM_INFO sysinfo;
                memset(&sysinfo, 0, sizeof(sysinfo));
                GetSystemInfo(&sysinfo);
                return sysinfo.dwNumberOfProcessors;
            }
            static id_type current() noexcept { return GetCurrentThreadId(); }
            static void yield() noexcept { Sleep(0); }
        private:
            enum state_type {
                thread_running,
                thread_complete,
                thread_joined,
            };
            struct impl_type {
                std::exception_ptr except;
                Thread::callback payload;
                std::atomic<int> state;
                DWORD key;
                HANDLE thread;
            };
            unique_ptr<impl_type> impl;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            static DWORD WINAPI thread_callback(void* ptr) noexcept {
                auto t = static_cast<impl_type*>(ptr);
                if (t->payload) {
                    try { t->payload(); }
                    catch (...) { t->except = std::current_exception(); }
                }
                t->state = thread_complete;
                return 0;
            }
        };

    #endif

    // Synchronisation objects

    #if defined(PRI_TARGET_UNIX)

        class Mutex {
        public:
            Mutex() noexcept: pmutex(PTHREAD_MUTEX_INITIALIZER) {}
            ~Mutex() noexcept { pthread_mutex_destroy(&pmutex); }
            void lock() noexcept { pthread_mutex_lock(&pmutex); }
            bool try_lock() noexcept { return pthread_mutex_trylock(&pmutex) == 0; }
            void unlock() noexcept { pthread_mutex_unlock(&pmutex); }
        private:
            friend class ConditionVariable;
            pthread_mutex_t pmutex;
            PRI_NO_COPY_MOVE(Mutex)
        };

    #else

        class Mutex {
        public:
            Mutex() noexcept { InitializeCriticalSection(&wcrit); }
            ~Mutex() noexcept { DeleteCriticalSection(&wcrit); }
            void lock() noexcept { EnterCriticalSection(&wcrit); }
            bool try_lock() noexcept { return TryEnterCriticalSection(&wcrit); }
            void unlock() noexcept { LeaveCriticalSection(&wcrit); }
        private:
            friend class ConditionVariable;
            CRITICAL_SECTION wcrit;
            PRI_NO_COPY_MOVE(Mutex)
        };

    #endif

    class MutexLock {
    public:
        MutexLock() = default;
        explicit MutexLock(Mutex& m) noexcept: mx(&m) { mx->lock(); }
        ~MutexLock() noexcept { if (mx) mx->unlock(); }
        MutexLock(MutexLock&& lock) noexcept: mx(lock.mx) { lock.mx = nullptr; }
        MutexLock& operator=(MutexLock&& lock) noexcept { if (mx) mx->unlock(); mx = lock.mx; lock.mx = nullptr; return *this; }
    private:
        friend class ConditionVariable;
        Mutex* mx = nullptr;
    };

    inline auto make_lock(Mutex& m) noexcept { return MutexLock(m); }

    #if ! defined(PRI_TARGET_MINGW)
        template <typename T> inline auto make_lock(T& t) { return std::unique_lock<T>(t); }
    #endif

    #if defined(PRI_TARGET_UNIX)

        // The Posix standard recommends using clock_gettime() with
        // CLOCK_REALTIME to find the current time for a CV wait. OSX doesn't
        // have clock_gettime(), but the Apple man page for
        // pthread_cond_timedwait() recommends using gettimeofday() instead,
        // and manually converting timeval to timespec.

        class ConditionVariable {
        public:
            ConditionVariable() {
                int rc = pthread_cond_init(&pcond, nullptr);
                if (rc)
                    throw std::system_error(rc, std::generic_category(), "pthread_cond_init()");
            }
            ~ConditionVariable() noexcept { pthread_cond_destroy(&pcond); }
            void notify_all() noexcept { pthread_cond_broadcast(&pcond); }
            void notify_one() noexcept { pthread_cond_signal(&pcond); }
            void wait(MutexLock& ml) {
                int rc = pthread_cond_wait(&pcond, &ml.mx->pmutex);
                if (rc)
                    throw std::system_error(rc, std::generic_category(), "pthread_cond_wait()");
            }
            template <typename Pred> void wait(MutexLock& lock, Pred p) { while (! p()) wait(lock); }
            template <typename R, typename P, typename Pred> bool wait_for(MutexLock& lock, std::chrono::duration<R, P> t, Pred p) {
                using namespace std::chrono;
                return wait_impl(lock, duration_cast<nanoseconds>(t), predicate(p));
            }
        private:
            using predicate = function<bool()>;
            pthread_cond_t pcond;
            PRI_NO_COPY_MOVE(ConditionVariable)
            bool wait_impl(MutexLock& ml, std::chrono::nanoseconds ns, predicate p) {
                using namespace Prion::Literals;
                using namespace std::chrono;
                if (p())
                    return true;
                auto nsc = ns.count();
                if (nsc <= 0)
                    return false;
                timespec ts;
                #if defined(PRI_TARGET_APPLE)
                    timeval tv;
                    gettimeofday(&tv, nullptr);
                    ts = {tv.tv_sec, 1000 * tv.tv_usec};
                #else
                    clock_gettime(CLOCK_REALTIME, &ts);
                #endif
                ts.tv_nsec += nsc % 1_G;
                ts.tv_sec += nsc / 1_G + ts.tv_nsec / 1_G;
                ts.tv_nsec %= 1_G;
                for (;;) {
                    int rc = pthread_cond_timedwait(&pcond, &ml.mx->pmutex, &ts);
                    if (rc == ETIMEDOUT)
                        return p();
                    if (rc != 0)
                        throw std::system_error(rc, std::generic_category(), "pthread_cond_timedwait()");
                    if (p())
                        return true;
                }
            }
        };

    #else

        // Windows only has millisecond resolution in their API. The actual
        // resolution varies from 1ms to 15ms depending on the version of
        // Windows, what APIs have been called lately, and the phase of the
        // moon.

        class ConditionVariable {
        public:
            ConditionVariable() { InitializeConditionVariable(&wcond); }
            ~ConditionVariable() noexcept {}
            void notify_all() noexcept { WakeAllConditionVariable(&wcond); }
            void notify_one() noexcept { WakeConditionVariable(&wcond); }
            void wait(MutexLock& lock) {
                if (! SleepConditionVariableCS(&wcond, &lock.mx->wcrit, INFINITE))
                    throw std::system_error(GetLastError(), windows_category(), "SleepConditionVariableCS()");
            }
            template <typename Pred> void wait(MutexLock& lock, Pred p) { while (! p()) wait(lock); }
            template <typename R, typename P, typename Pred> bool wait_for(MutexLock& lock, std::chrono::duration<R, P> t, Pred p) {
                using namespace std::chrono;
                return wait_impl(lock, duration_cast<nanoseconds>(t), predicate(p));
            }
        private:
            using predicate = function<bool()>;
            CONDITION_VARIABLE wcond;
            PRI_NO_COPY_MOVE(ConditionVariable)
            bool wait_impl(MutexLock& lock, std::chrono::nanoseconds ns, predicate p) {
                using namespace std::chrono;
                if (p())
                    return true;
                if (ns <= nanoseconds())
                    return false;
                auto timeout = system_clock::now() + ns;
                for (;;) {
                    auto remain = timeout - system_clock::now();
                    auto ms = duration_cast<milliseconds>(remain).count();
                    if (ms < 0)
                        ms = 0;
                    else if (ms >= INFINITE)
                        ms = INFINITE - 1;
                    if (SleepConditionVariableCS(&wcond, &lock.mx->wcrit, ms)) {
                        if (p())
                            return true;
                    } else {
                        auto status = GetLastError();
                        if (status == ERROR_TIMEOUT)
                            return p();
                        else
                            throw std::system_error(status, windows_category(), "SleepConditionVariableCS()");
                    }
                }
            }
        };

    #endif

    // [Time and date operations]

    // General time and date operations

    PRI_ENUM_CLASS(Zone, 1, utc, local)

    namespace PrionDetail {

        using SleepUnit =
            #if defined(PRI_TARGET_UNIX)
                std::chrono::microseconds;
            #else
                std::chrono::milliseconds;
            #endif

        inline void sleep_for(SleepUnit t) noexcept {
            #if defined(PRI_TARGET_UNIX)
                auto usec = t.count();
                if (usec > 0) {
                    timeval tv;
                    tv.tv_sec = usec / 1_M;
                    tv.tv_usec = usec % 1_M;
                    select(0, nullptr, nullptr, nullptr, &tv);
                } else {
                    sched_yield();
                }
            #else
                auto msec = t.count();
                if (msec > 0)
                    Sleep(uint32_t(msec));
                else
                    Sleep(0);
            #endif
        }

    }

    template <typename R, typename P>
    void from_seconds(double s, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        d = duration_cast<duration<R, P>>(duration<double>(s));
    }

    template <typename R, typename P>
    double to_seconds(const std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        return duration_cast<duration<double>>(d).count();
    }

    inline std::chrono::system_clock::time_point make_date(int year, int month, int day,
            int hour, int min, double sec, Zone z = Zone::utc) noexcept {
        using namespace std::chrono;
        double isec = 0, fsec = modf(sec, &isec);
        if (fsec < 0) {
            isec -= 1;
            fsec += 1;
        }
        tm stm;
        memset(&stm, 0, sizeof(stm));
        stm.tm_sec = int(isec);
        stm.tm_min = min;
        stm.tm_hour = hour;
        stm.tm_mday = day;
        stm.tm_mon = month - 1;
        stm.tm_year = year - 1900;
        stm.tm_isdst = -1;
        time_t t;
        if (z == Zone::local)
            t = mktime(&stm);
        else
            #if defined(PRI_TARGET_UNIX)
                t = timegm(&stm);
            #else
                t = _mkgmtime(&stm);
            #endif
        system_clock::time_point::rep extra(fsec * system_clock::time_point::duration(seconds(1)).count());
        return system_clock::from_time_t(t) + system_clock::time_point::duration(extra);
    }

    template <typename R, typename P>
    void sleep_for(std::chrono::duration<R, P> t) noexcept {
        using namespace std::chrono;
        PrionDetail::sleep_for(duration_cast<PrionDetail::SleepUnit>(t));
    }

    inline void sleep_for(double t) noexcept {
        using namespace std::chrono;
        PrionDetail::sleep_for(duration_cast<PrionDetail::SleepUnit>(duration<double>(t)));
    }

    // Time and date formatting

    namespace PrionDetail {

        inline u8string format_time(int64_t sec, double frac, int prec) {
            u8string result;
            if (sec < 0 || frac < 0)
                result += '-';
            sec = std::abs(sec);
            int y = sec / 31557600;
            sec -= 31557600 * y;
            int d = sec / 86400;
            sec -= 86400 * d;
            int h = sec / 3600;
            sec -= 3600 * h;
            int m = sec / 60;
            sec -= 60 * m;
            int rc, s = sec;
            vector<char> buf(64);
            for (;;) {
                if (y > 0)
                    rc = snprintf(buf.data(), buf.size(), "%dy%03dd%02dh%02dm%02d", y, d, h, m, s);
                else if (d > 0)
                    rc = snprintf(buf.data(), buf.size(), "%dd%02dh%02dm%02d", d, h, m, s);
                else if (h > 0)
                    rc = snprintf(buf.data(), buf.size(), "%dh%02dm%02d", h, m, s);
                else if (m > 0)
                    rc = snprintf(buf.data(), buf.size(), "%dm%02d", m, s);
                else
                    rc = snprintf(buf.data(), buf.size(), "%d", s);
                if (rc < int(buf.size()))
                    break;
                buf.resize(2 * buf.size());
            }
            result += buf.data();
            if (prec > 0) {
                buf.resize(prec + 3);
                snprintf(buf.data(), buf.size(), "%.*f", prec, fabs(frac));
                result += buf.data() + 1;
            }
            result += 's';
            return result;
        }

    }

    // Unfortunately strftime() doesn't set errno and simply returns zero on
    // any error. This means that there is no way to distinguish between an
    // invalid format string, an output buffer that is too small, and a
    // legitimately empty result. Here we try first with a reasonable output
    // length, and if that fails, try again with a much larger one; if it
    // still fails, give up. This could theoretically fail in the face of a
    // very long localized date format, but there doesn't seem to be a better
    // solution.

    inline u8string format_date(std::chrono::system_clock::time_point tp, const u8string& format, Zone z = Zone::utc) {
        using namespace std::chrono;
        auto t = system_clock::to_time_t(tp);
        tm stm = z == Zone::local ? *localtime(&t) : *gmtime(&t);
        u8string result(std::max(2 * format.size(), size_t(100)), '\0');
        auto rc = strftime(&result[0], result.size(), format.data(), &stm);
        if (rc == 0) {
            result.resize(10 * result.size(), '\0');
            rc = strftime(&result[0], result.size(), format.data(), &stm);
        }
        result.resize(rc);
        return result;
    }

    inline u8string format_date(std::chrono::system_clock::time_point tp, int prec = 0, Zone z = Zone::utc) {
        using namespace std::chrono;
        using namespace std::literals;
        u8string result = format_date(tp, "%Y-%m-%d %H:%M:%S"s, z);
        if (prec > 0) {
            double sec = to_seconds(tp.time_since_epoch());
            double isec;
            double fsec = modf(sec, &isec);
            u8string buf(prec + 3, '\0');
            snprintf(&buf[0], buf.size(), "%.*f", prec, fsec);
            result += buf.data() + 1;
        }
        return result;
    }

    template <typename R, typename P>
    u8string format_time(const std::chrono::duration<R, P>& time, int prec = 0) {
        using namespace std::chrono;
        auto whole = duration_cast<seconds>(time);
        auto frac = time - duration_cast<duration<R, P>>(whole);
        return PrionDetail::format_time(whole.count(), duration_cast<duration<double>>(frac).count(), prec);
    }

    // System specific time and date conversions

    template <typename R, typename P>
    timespec duration_to_timespec(const std::chrono::duration<R, P>& d) noexcept {
        using namespace Prion::Literals;
        using namespace std::chrono;
        int64_t nsec = duration_cast<nanoseconds>(d).count();
        return {time_t(nsec / 1_G), long(nsec % 1_G)};
    }

    template <typename R, typename P>
    timeval duration_to_timeval(const std::chrono::duration<R, P>& d) noexcept {
        using namespace Prion::Literals;
        using namespace std::chrono;
        #if defined(PRI_TARGET_UNIX)
            using sec_type = time_t;
            using usec_type = suseconds_t;
        #else
            using sec_type = long;
            using usec_type = long;
        #endif
        int64_t usec = duration_cast<microseconds>(d).count();
        return {sec_type(usec / 1_M), usec_type(usec % 1_M)};
    }

    inline timespec timepoint_to_timespec(const std::chrono::system_clock::time_point& tp) noexcept {
        using namespace std::chrono;
        return duration_to_timespec(tp - system_clock::time_point());
    }

    inline timeval timepoint_to_timeval(const std::chrono::system_clock::time_point& tp) noexcept {
        using namespace std::chrono;
        return duration_to_timeval(tp - system_clock::time_point());
    }

    template <typename R, typename P>
    void timespec_to_duration(const timespec& ts, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        using D = duration<R, P>;
        d = duration_cast<D>(seconds(ts.tv_sec)) + duration_cast<D>(nanoseconds(ts.tv_nsec));
    }

    template <typename R, typename P>
    void timeval_to_duration(const timeval& tv, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        using D = duration<R, P>;
        d = duration_cast<D>(seconds(tv.tv_sec)) + duration_cast<D>(microseconds(tv.tv_usec));
    }

    inline std::chrono::system_clock::time_point timespec_to_timepoint(const timespec& ts) noexcept {
        using namespace std::chrono;
        system_clock::duration d;
        timespec_to_duration(ts, d);
        return system_clock::time_point() + d;
    }

    inline std::chrono::system_clock::time_point timeval_to_timepoint(const timeval& tv) noexcept {
        using namespace std::chrono;
        system_clock::duration d;
        timeval_to_duration(tv, d);
        return system_clock::time_point() + d;
    }

    #if defined(PRI_TARGET_WIN32)

        inline std::chrono::system_clock::time_point filetime_to_timepoint(const FILETIME& ft) noexcept {
            using namespace Prion::Literals;
            using namespace std::chrono;
            static constexpr int64_t filetime_freq = 10_M; // FILETIME ticks (100 ns) per second
            static constexpr int64_t windows_epoch = 11644473600ll; // Windows epoch (1601) to Unix epoch (1970)
            int64_t ticks = (int64_t(ft.dwHighDateTime) << 32) + int64_t(ft.dwLowDateTime);
            int64_t sec = ticks / filetime_freq - windows_epoch;
            int64_t nsec = 100ll * (ticks % filetime_freq);
            return system_clock::from_time_t(time_t(sec)) + duration_cast<system_clock::duration>(nanoseconds(nsec));
        }

        inline FILETIME timepoint_to_filetime(const std::chrono::system_clock::time_point& tp) noexcept {
            using namespace std::chrono;
            auto unix_time = tp - system_clock::from_time_t(0);
            uint64_t nsec = duration_cast<nanoseconds>(unix_time).count();
            uint64_t ticks = nsec / 100ll;
            return {uint32_t(ticks & 0xfffffffful), uint32_t(ticks >> 32)};
        }

    #endif

    // [Things that need to go at the end because of dependencies]

    // Logging

    namespace PrionDetail {

        template <typename T>
        int hash_xcolour(T t) noexcept {
            using namespace std::chrono;
            static const auto init = uint32_t(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
            uint32_t x = init, y = 0;
            auto pt = reinterpret_cast<const uint8_t*>(&t);
            for (size_t i = 0; i < sizeof(T); i += 4) {
                memcpy(&y, pt + i, 4);
                x ^= y;
            }
            std::minstd_rand rng(x);
            std::uniform_int_distribution<> dist(0, 123);
            y = dist(rng);
            return 100 * (y / 25) + 10 * (y / 5 % 5) + y % 5 + 222;
        }

        inline void log_message(const u8string& msg, bool timestamp) {
            using namespace std::chrono;
            static Mutex mtx;
            auto lock = make_lock(mtx);
            u8string text = xt_colour(hash_xcolour(Thread::current()));
            if (timestamp)
                text += "[" + format_date(system_clock::now(), 3) + "] ";
            else
                text += "# ";
            text += msg + xt_reset;
            cout << text << endl;
        }

    }

    inline void logx(const u8string& msg) { PrionDetail::log_message(msg, false); }
    inline void logx(const char* msg) { PrionDetail::log_message(cstr(msg), false); }
    template <typename... Args> inline void logx(Args... args)
        { PrionDetail::log_message(join(vector<u8string>{to_str(args)...}, " "), false); }
    inline void logt(const u8string& msg) { PrionDetail::log_message(msg, true); }
    inline void logt(const char* msg) { PrionDetail::log_message(cstr(msg), true); }
    template <typename... Args> inline void logt(Args... args)
        { PrionDetail::log_message(join(vector<u8string>{to_str(args)...}, " "), true); }

    // UUID

    namespace PrionDetail {

        inline int decode_hex_byte(string::const_iterator& i, string::const_iterator end) {
            auto j = i;
            if (end - i >= 2 && j[0] == '0' && (j[1] == 'X' || j[1] == 'x'))
                j += 2;
            if (end - j < 2)
                return -1;
            int n = 0;
            for (auto k = j + 2; j != k; ++j) {
                n <<= 4;
                if (*j >= '0' && *j <= '9')
                    n += *j - '0';
                else if (*j >= 'A' && *j <= 'F')
                    n += *j - 'A' + 10;
                else if (*j >= 'a' && *j <= 'f')
                    n += *j - 'a' + 10;
                else
                    return -1;
            }
            i = j;
            return n;
        }

    }

    struct RandomUuid;

    class Uuid:
    public LessThanComparable<Uuid> {
    public:
        Uuid() noexcept { memset(bytes, 0, 16); }
        Uuid(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h,
            uint8_t i, uint8_t j, uint8_t k, uint8_t l, uint8_t m, uint8_t n, uint8_t o, uint8_t p) noexcept:
            bytes{a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p} {}
        Uuid(uint32_t abcd, uint16_t ef, uint16_t gh, uint8_t i, uint8_t j, uint8_t k, uint8_t l,
            uint8_t m, uint8_t n, uint8_t o, uint8_t p) noexcept:
            bytes{uint8_t((abcd >> 24) & 0xff), uint8_t((abcd >> 16) & 0xff), uint8_t((abcd >> 8) & 0xff), uint8_t(abcd & 0xff),
                uint8_t((ef >> 8) & 0xff), uint8_t(ef & 0xff), uint8_t((gh >> 8) & 0xff), uint8_t(gh & 0xff),
                i, j, k, l, m, n, o, p} {}
        explicit Uuid(uint128_t u) noexcept { whole = u; }
        explicit Uuid(const uint8_t* ptr) noexcept { if (ptr) memcpy(bytes, ptr, 16); else memset(bytes, 0, 16); }
        explicit Uuid(const string& s);
        uint8_t& operator[](size_t i) noexcept { return bytes[i]; }
        const uint8_t& operator[](size_t i) const noexcept { return bytes[i]; }
        uint8_t* begin() noexcept { return bytes; }
        const uint8_t* begin() const noexcept { return bytes; }
        uint8_t* end() noexcept { return bytes + 16; }
        const uint8_t* end() const noexcept { return bytes + 16; }
        uint128_t as_integer() const noexcept { return whole; }
        size_t hash() const noexcept { return hash_bytes(bytes, 16); }
        u8string str() const;
        friend bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept { return memcmp(lhs.bytes, rhs.bytes, 16) == 0; }
        friend bool operator<(const Uuid& lhs, const Uuid& rhs) noexcept { return memcmp(lhs.bytes, rhs.bytes, 16) == -1; }
    private:
        friend struct RandomUuid;
        union {
            uint8_t bytes[16];
            uint32_t words[4];
            BigEndian<uint128_t> whole;
        };
    };

    inline Uuid::Uuid(const string& s) {
        auto begins = s.begin(), i = begins, ends = s.end();
        auto j = begin(), endu = end();
        int rc = 0;
        while (i != ends && j != endu && rc != -1) {
            i = std::find_if(i, ends, ascii_isxdigit);
            if (i == ends)
                break;
            rc = PrionDetail::decode_hex_byte(i, ends);
            if (rc == -1)
                break;
            *j++ = uint8_t(rc);
        }
        if (rc == -1 || j != endu || std::find_if(i, ends, ascii_isalnum) != ends)
            throw std::invalid_argument("Invalid UUID: " + s);
    }

    inline u8string Uuid::str() const {
        u8string s;
        s.reserve(36);
        int i = 0;
        for (; i < 4; ++i)
            PrionDetail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 6; ++i)
            PrionDetail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 8; ++i)
            PrionDetail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 10; ++i)
            PrionDetail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 16; ++i)
            PrionDetail::append_hex_byte(bytes[i], s);
        return s;
    }

    inline std::ostream& operator<<(std::ostream& o, const Uuid& u) { return o << u.str(); }
    inline u8string to_str(const Uuid& u) { return u.str(); }

    struct RandomUuid {
        using result_type = Uuid;
        template <typename Rng> Uuid operator()(Rng& rng) const {
            std::uniform_int_distribution<uint32_t> dist;
            Uuid result;
            for (auto& w: result.words)
                w = dist(rng);
            result[6] = (result[6] & 0x0f) | 0x40;
            result[8] = (result[8] & 0x3f) | 0x80;
            return result;
        }
    };

    class Version:
    public LessThanComparable<Version> {
    public:
        using value_type = unsigned;
        Version() noexcept {}
        template <typename... Args> Version(unsigned n, Args... args) { append(n, args...); trim(); }
        explicit Version(const u8string& s);
        unsigned operator[](size_t i) const noexcept { return i < ver.size() ? ver[i] : 0; }
        const unsigned* begin() const noexcept { return ver.data(); }
        const unsigned* end() const noexcept { return ver.data() + ver.size(); }
        unsigned major() const noexcept { return (*this)[0]; }
        unsigned minor() const noexcept { return (*this)[1]; }
        unsigned patch() const noexcept { return (*this)[2]; }
        string str(size_t min_elements = 2) const;
        string suffix() const { return suf; }
        uint32_t to32() const noexcept;
        static Version from32(uint32_t n) noexcept;
        friend bool operator==(const Version& lhs, const Version& rhs) noexcept
            { return lhs.ver == rhs.ver && lhs.suf == rhs.suf; }
        friend bool operator<(const Version& lhs, const Version& rhs) noexcept
            { int c = compare_3way(lhs.ver, rhs.ver); return c == 0 ? lhs.suf < rhs.suf : c == -1; }
    private:
        vector<unsigned> ver;
        u8string suf;
        template <typename... Args> void append(unsigned n, Args... args) { ver.push_back(n); append(args...); }
        void append(const u8string& s) { suf = s; }
        void append() {}
        void trim() { while (! ver.empty() && ver.back() == 0) ver.pop_back(); }
    };

    inline Version::Version(const u8string& s) {
        auto i = s.begin(), end = s.end();
        while (i != end) {
            auto j = std::find_if_not(i, end, ascii_isdigit);
            if (i == j)
                break;
            ver.push_back(unsigned(decnum(u8string(i, j))));
            i = j;
            if (i == end || *i != '.')
                break;
            ++i;
            if (i == end || ! ascii_isdigit(*i))
                break;
        }
        suf.assign(i, end);
        trim();
    }

    inline string Version::str(size_t min_elements) const {
        string s;
        for (auto& v: ver)
            s += to_str(v) + '.';
        for (size_t i = ver.size(); i < min_elements; ++i)
            s += "0.";
        if (! s.empty())
            s.pop_back();
        s += suf;
        return s;
    }

    inline uint32_t Version::to32() const noexcept {
        uint32_t v = 0;
        for (size_t i = 0; i < 4; ++i)
            v = (v << 8) | ((*this)[i] & 0xff);
        return v;
    }

    inline Version Version::from32(uint32_t n) noexcept {
        Version v;
        for (int i = 24; i >= 0 && n != 0; i -= 8)
            v.ver.push_back((n >> i) & 0xff);
        v.trim();
        return v;
    }

    inline std::ostream& operator<<(std::ostream& o, const Version& v) {
        return o << v.str();
    }

    inline u8string to_str(const Version& v) {
        return v.str();
    }

}

PRI_DEFINE_STD_HASH(Prion::Uuid)
