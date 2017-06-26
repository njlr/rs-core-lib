#pragma once

// Compilation environment control macros

#ifdef __APPLE__
    #ifndef _DARWIN_C_SOURCE
        #define _DARWIN_C_SOURCE 1
    #endif
#endif

#if defined(__CYGWIN__) || ! defined(_WIN32)
    #ifndef _XOPEN_SOURCE
        #define _XOPEN_SOURCE 700 // Posix 2008
    #endif
    #ifndef _REENTRANT
        #define _REENTRANT 1
    #endif
#endif

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX 1
    #endif
    #ifndef UNICODE
        #define UNICODE 1
    #endif
    #ifndef _UNICODE
        #define _UNICODE 1
    #endif
    #ifndef WINVER
        #define WINVER 0x601 // Windows 7
    #endif
    #ifndef _WIN32_WINNT
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
#include <deque>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <ostream>
#include <random>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>
#include <cxxabi.h>

#ifdef __APPLE__
    #include <mach/mach.h>
#endif

#ifndef __MINGW32__
    #include <mutex>
#endif

#ifdef _WIN32
    #ifdef __CYGWIN__
        #include <windows.h>
    #else
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #include <windows.h>
        #include <io.h>
    #endif
#endif

#ifdef _XOPEN_SOURCE
    #include <pthread.h>
    #include <sched.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif

// Fix GNU brain damage

#ifdef __GNUC__
    #ifdef major
        #undef major
    #endif
    #ifdef minor
        #undef minor
    #endif
#endif

// Other preprocessor macros

#define RS_ASSERT(expr) do { \
    if (! bool(expr)) \
        throw std::logic_error(std::string("Assertion failure [") + __FILE__ + ":" + ::RS::dec(__LINE__) + "]: " + #expr); \
} while (false)

#ifdef __clang__
    #define RS_ASSUME(expr) __builtin_assume(bool(expr))
#else
    #define RS_ASSUME(expr)
#endif
#define RS_LIKELY(expr) bool(__builtin_expect(bool(expr), 1))
#define RS_UNLIKELY(expr) bool(__builtin_expect(bool(expr), 0))
#define RS_NOTREACHED __builtin_unreachable()

#define RS_BITMASK_OPERATORS(EC) \
    inline constexpr bool operator!(EC x) noexcept { return std::underlying_type_t<EC>(x) == 0; } \
    inline constexpr EC operator~(EC x) noexcept { return EC(~ std::underlying_type_t<EC>(x)); } \
    inline constexpr EC operator&(EC lhs, EC rhs) noexcept { return EC(std::underlying_type_t<EC>(lhs) & std::underlying_type_t<EC>(rhs)); } \
    inline constexpr EC operator|(EC lhs, EC rhs) noexcept { return EC(std::underlying_type_t<EC>(lhs) | std::underlying_type_t<EC>(rhs)); } \
    inline constexpr EC operator^(EC lhs, EC rhs) noexcept { return EC(std::underlying_type_t<EC>(lhs) ^ std::underlying_type_t<EC>(rhs)); } \
    inline constexpr EC& operator&=(EC& lhs, EC rhs) noexcept { return lhs = lhs & rhs; } \
    inline constexpr EC& operator|=(EC& lhs, EC rhs) noexcept { return lhs = lhs | rhs; } \
    inline constexpr EC& operator^=(EC& lhs, EC rhs) noexcept { return lhs = lhs ^ rhs; }

#define RS_CHAR(C, T) (::RS::RS_Detail::select_char<T>(C, u##C, U##C, L##C))
#define RS_CSTR(S, T) (::RS::RS_Detail::select_cstr<T>(S, u##S, U##S, L##S))
#define RS_OVERLOAD(f) [] (auto&&... args) { return f(std::forward<decltype(args)>(args)...); }
#define RS_STATIC_ASSERT(expr) static_assert((expr), #expr)

#define RS_ENUM_IMPLEMENTATION(EnumType, IntType, class_tag, name_prefix, first_value, first_name, ...) \
    enum class_tag EnumType: IntType { first_name = first_value, __VA_ARGS__, RS_enum_sentinel }; \
    inline __attribute__((unused)) std::ostream& operator<<(std::ostream& out, EnumType t) { \
        ::RS::RS_Detail::write_enum(out, t, first_value, name_prefix, #first_name "," #__VA_ARGS__); \
        return out; \
    } \
    constexpr __attribute__((unused)) bool enum_is_valid(EnumType t) noexcept { \
        return IntType(t) >= IntType(first_value) && IntType(t) < IntType(EnumType::RS_enum_sentinel); \
    } \
    inline __attribute__((unused)) std::vector<EnumType> RS_enum_values(EnumType) { \
        IntType n = IntType(EnumType::RS_enum_sentinel) - IntType(first_value); \
        std::vector<EnumType> v(n); \
        for (IntType i = 0; i < n; ++i) \
            v[i] = EnumType(first_value + i); \
        return v; \
    }

#define RS_ENUM(EnumType, IntType, first_value, first_name, ...) \
    RS_ENUM_IMPLEMENTATION(EnumType, IntType,, "", first_value, first_name, __VA_ARGS__)
#define RS_ENUM_CLASS(EnumType, IntType, first_value, first_name, ...) \
    RS_ENUM_IMPLEMENTATION(EnumType, IntType, class, #EnumType "::", first_value, first_name, __VA_ARGS__)

#define RS_LDLIB(libs)

#define RS_MOVE_ONLY(T) \
    T(const T&) = delete; \
    T(T&&) = default; \
    T& operator=(const T&) = delete; \
    T& operator=(T&&) = default;
#define RS_NO_COPY_MOVE(T) \
    T(const T&) = delete; \
    T(T&&) = delete; \
    T& operator=(const T&) = delete; \
    T& operator=(T&&) = delete;

// For internal use only
// Must be used in the global namespace
#define RS_DEFINE_STD_HASH(T) \
    namespace std { \
        template <> struct hash<T> { \
            using argument_type = T; \
            using result_type = size_t; \
            size_t operator()(const T& t) const noexcept { return t.hash(); } \
        }; \
    }

namespace RS {

    // Things needed early

    namespace RS_Detail {

        template <typename EnumType>
        void write_enum(std::ostream& out, EnumType t, long first_value, const char* prefix, const char* names) {
            using U = std::underlying_type_t<EnumType>;
            size_t index = U(t) - first_value;
            for (size_t i = 0; i < index; ++i) {
                names = strchr(names, ',');
                if (names == nullptr) {
                    out << U(t);
                    return;
                }
                names += strspn(names, " ,");
            }
            out << prefix;
            out.write(names, strcspn(names, " ,"));
        }

    }

    using U8string = std::string;

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
    constexpr size_t npos = std::string::npos;

    template <typename Range> using RangeIterator = decltype(std::begin(std::declval<Range&>()));
    template <typename Range> using RangeValue = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;

    template <typename T> std::vector<T> enum_values() { return RS_enum_values(T()); }

    template <typename T> constexpr auto as_signed(T t) noexcept { return static_cast<std::make_signed_t<T>>(t); }
    template <typename T> constexpr auto as_unsigned(T t) noexcept { return static_cast<std::make_unsigned_t<T>>(t); }

    template <typename C>
    std::basic_string<C> cstr(const C* ptr) {
        using string_type = std::basic_string<C>;
        return ptr ? string_type(ptr) : string_type();
    }

    template <typename C>
    std::basic_string<C> cstr(const C* ptr, size_t n) {
        using string_type = std::basic_string<C>;
        return ptr ? string_type(ptr, n) : string_type();
    }

    namespace RS_Detail {

        inline void append_hex_byte(uint8_t b, std::string& s) {
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
        U8string int_to_string(T x, int base, size_t digits) {
            bool neg = x < T(0);
            auto b = as_unsigned(base);
            auto y = neg ? as_unsigned(- x) : as_unsigned(x);
            U8string s;
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

    template <typename T> U8string bin(T x, size_t digits = 8 * sizeof(T)) { return RS_Detail::int_to_string(x, 2, digits); }
    template <typename T> U8string dec(T x, size_t digits = 1) { return RS_Detail::int_to_string(x, 10, digits); }
    template <typename T> U8string hex(T x, size_t digits = 2 * sizeof(T)) { return RS_Detail::int_to_string(x, 16, digits); }

    // Unicode functions

    namespace RS_Detail {

        template <typename Src, typename Dst,
            size_t N1 = sizeof(typename Src::value_type),
            size_t N2 = sizeof(typename Dst::value_type)>
        struct UtfConvert {
            Dst operator()(const Src& s) const {
                return UtfConvert<std::u32string, Dst>()(UtfConvert<Src, std::u32string>()(s));
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
                auto p = reinterpret_cast<const uint8_t*>(s.data());
                auto q = p + s.size();
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
                auto p = reinterpret_cast<const uint16_t*>(s.data());
                auto q = p + s.size();
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

        inline size_t utf8_check_ahead(const uint8_t* ptr, size_t len, size_t pos, size_t n, uint8_t a1, uint8_t b1) {
            if (pos + n > len)
                return 0;
            if (ptr[pos + 1] < a1 || ptr[pos + 1] > b1)
                return 0;
            for (size_t i = 2; i < n; ++i)
                if (ptr[pos + i] < 0x80 || ptr[pos + i] > 0xbf)
                    return 0;
            return n;
        }

        template <typename C, size_t N = sizeof(C)>
        struct UtfValidate;

        template <typename C>
        struct UtfValidate<C, 1> {
            size_t operator()(const std::basic_string<C>& s) const noexcept {
                auto ptr = reinterpret_cast<const uint8_t*>(s.data());
                size_t units = s.size(), i = 0;
                while (i < units) {
                    size_t n = 0;
                    if (ptr[i] <= 0x7f)
                        n = 1;
                    else if (ptr[i] <= 0xc1)
                        break;
                    else if (ptr[i] <= 0xdf)
                        n = utf8_check_ahead(ptr, units, i, 2, 0x80, 0xbf);
                    else if (ptr[i] <= 0xe0)
                        n = utf8_check_ahead(ptr, units, i, 3, 0xa0, 0xbf);
                    else if (ptr[i] <= 0xec)
                        n = utf8_check_ahead(ptr, units, i, 3, 0x80, 0xbf);
                    else if (ptr[i] <= 0xed)
                        n = utf8_check_ahead(ptr, units, i, 3, 0x80, 0x9f);
                    else if (ptr[i] <= 0xef)
                        n = utf8_check_ahead(ptr, units, i, 3, 0x80, 0xbf);
                    else if (ptr[i] <= 0xf0)
                        n = utf8_check_ahead(ptr, units, i, 4, 0x90, 0xbf);
                    else if (ptr[i] <= 0xf3)
                        n = utf8_check_ahead(ptr, units, i, 4, 0x80, 0xbf);
                    else if (ptr[i] <= 0xf4)
                        n = utf8_check_ahead(ptr, units, i, 4, 0x80, 0x8f);
                    if (n == 0)
                        break;
                    i += n;
                }
                return i;
            }
        };

        template <typename C>
        struct UtfValidate<C, 2> {
            size_t operator()(const std::basic_string<C>& s) const noexcept {
                auto ptr = reinterpret_cast<const uint16_t*>(s.data());
                size_t units = s.size(), i = 0;
                while (i < units) {
                    if (ptr[i] <= 0xd7ff || ptr[i] >= 0xe000) {
                        ++i;
                    } else if (ptr[i] <= 0xdbff) {
                        if (units - i < 2 || ptr[i + 1] <= 0xdbff || ptr[i + 1] >= 0xe000)
                            break;
                        i += 2;
                    } else {
                        break;
                    }
                }
                return i;
            }
        };

        template <typename C>
        struct UtfValidate<C, 4> {
            size_t operator()(const std::basic_string<C>& s) const noexcept {
                auto ptr = reinterpret_cast<const uint32_t*>(s.data());
                size_t units = s.size(), i = 0;
                for (; i < units; ++i)
                    if ((ptr[i] >= 0xd800 && ptr[i] <= 0xdfff) || ptr[i] >= 0x110000)
                        break;
                return i;
            }
        };

    }

    template <typename Dst, typename Src>
    Dst uconv(const Src& s) {
        return RS_Detail::UtfConvert<Src, Dst>()(s);
    }

    template <typename C>
    bool uvalid(const std::basic_string<C>& s, size_t& n) noexcept {
        n = RS_Detail::UtfValidate<C>()(s);
        return n == s.size();
    }

    template <typename C>
    bool uvalid(const std::basic_string<C>& s) noexcept {
        size_t n = 0;
        return uvalid(s, n);
    }

    namespace RS_Detail {

        inline U8string quote_string(const std::string& str, bool check_utf8) {
            bool allow_8bit = check_utf8 && uvalid(str);
            U8string result = "\"";
            for (auto c: str) {
                switch (c) {
                    case 0:     result += "\\0"; break;
                    case '\t':  result += "\\t"; break;
                    case '\n':  result += "\\n"; break;
                    case '\f':  result += "\\f"; break;
                    case '\r':  result += "\\r"; break;
                    case '\"':  result += "\\\""; break;
                    case '\\':  result += "\\\\"; break;
                    default: {
                        auto b = uint8_t(c);
                        if ((b >= 0x20 && b <= 0x7e) || (allow_8bit && b >= 0x80)) {
                            result += c;
                        } else {
                            result += "\\x";
                            RS_Detail::append_hex_byte(b, result);
                        }
                        break;
                    }
                }
            }
            result += '\"';
            return result;
        }

    }

    inline U8string quote(const std::string& str) { return RS_Detail::quote_string(str, true); }
    inline U8string bquote(const std::string& str) { return RS_Detail::quote_string(str, false); }

    // [Types]

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

    template <typename T, typename CV>
    struct FlexibleRandomAccessIterator:
    BidirectionalIterator<T, CV>,
    LessThanComparable<T> {
        using iterator_category = std::random_access_iterator_tag;
        CV& operator[](ptrdiff_t i) const noexcept { T t = static_cast<const T&>(*this); t += i; return *t; }
        friend T& operator-=(T& lhs, ptrdiff_t rhs) { return lhs += - rhs; }
        friend T operator+(const T& lhs, ptrdiff_t rhs) { T t = lhs; t += rhs; return t; }
        friend T operator+(ptrdiff_t lhs, const T& rhs) { T t = rhs; t += lhs; return t; }
        friend T operator-(const T& lhs, ptrdiff_t rhs) { T t = lhs; t -= rhs; return t; }
        friend bool operator<(const T& lhs, const T& rhs) noexcept { return lhs - rhs < 0; }
    };

    // Endian integers

    enum ByteOrder {
        big_endian = 1,
        little_endian = 2,
    };

    namespace RS_Detail {

        template <typename T>
        constexpr T swap_ends(T t, size_t N = sizeof(T)) noexcept {
            using U = std::make_unsigned_t<T>;
            return N == 1 ? t : T((swap_ends(U(t) & ((U(1) << (4 * N)) - 1), N / 2) << (4 * N)) | swap_ends(U(t) >> (4 * N), N / 2));
        }

        template <ByteOrder B, typename T>
        constexpr T order_bytes(T t) noexcept {
            return (B == big_endian) == big_endian_target ? t : swap_ends(t);
        }

    }

    template <typename T, ByteOrder B>
    class Endian {
    public:
        using value_type = T;
        static constexpr auto byte_order = B;
        constexpr Endian() noexcept: value(0) {}
        constexpr Endian(T t) noexcept: value(RS_Detail::order_bytes<B>(t)) {}
        explicit Endian(const void* p) noexcept { memcpy(&value, p, sizeof(T)); }
        constexpr operator T() const noexcept { return get(); }
        constexpr T get() const noexcept { return RS_Detail::order_bytes<B>(value); }
        constexpr const T* ptr() const noexcept { return &value; }
        T* ptr() noexcept { return &value; }
        constexpr T rep() const noexcept { return value; }
        T& rep() noexcept { return value; }
    private:
        T value;
    };

    template <typename T> using BigEndian = Endian<T, big_endian>;
    template <typename T> using LittleEndian = Endian<T, little_endian>;

    template <typename T, ByteOrder B> inline std::ostream& operator<<(std::ostream& out, Endian<T, B> t) { return out << t.get(); }

    // Containers

    template <typename T>
    class Stack {
    public:
        Stack() = default;
        ~Stack() noexcept { clear(); }
        Stack(const Stack&) = delete;
        Stack(Stack&& s) = default;
        Stack& operator=(const Stack&) = delete;
        Stack& operator=(Stack&& s) { if (&s != this) { clear(); con = std::move(s.con); } return *this; }
        void clear() noexcept { while (! empty()) pop(); }
        bool empty() const noexcept { return con.empty(); }
        void pop() noexcept { con.pop_back(); }
        void push(const T& t) { con.push_back(t); }
        void push(T&& t) { con.push_back(std::move(t)); }
        size_t size() const noexcept { return con.size(); }
        const T& top() const noexcept { return con.back(); }
    private:
        std::deque<T> con;
    };

    // Exceptions

    inline void rethrow(std::exception_ptr p) { if (p) std::rethrow_exception(p); }

    #ifdef _WIN32

        class WindowsCategory:
        public std::error_category {
        public:
            virtual U8string message(int ev) const {
                static constexpr uint32_t flags =
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
                wchar_t* wptr = nullptr;
                auto units = FormatMessageW(flags, nullptr, ev, 0, reinterpret_cast<wchar_t*>(&wptr), 0, nullptr);
                std::shared_ptr<wchar_t> wshare(wptr, LocalFree);
                int bytes = WideCharToMultiByte(CP_UTF8, 0, wptr, units, nullptr, 0, nullptr, nullptr);
                std::string text(bytes, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wptr, units, &text[0], bytes, nullptr, nullptr);
                text.resize(text.find_last_not_of(ascii_whitespace) + 1);
                text.shrink_to_fit();
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

    namespace RS_Detail {

        template <size_t Bits> struct IntegerType;
        template <> struct IntegerType<8> { using signed_type = int8_t; using unsigned_type = uint8_t; };
        template <> struct IntegerType<16> { using signed_type = int16_t; using unsigned_type = uint16_t; };
        template <> struct IntegerType<32> { using signed_type = int32_t; using unsigned_type = uint32_t; };
        template <> struct IntegerType<64> { using signed_type = int64_t; using unsigned_type = uint64_t; };

    }

    template <typename T> using BinaryType = typename RS_Detail::IntegerType<8 * sizeof(T)>::unsigned_type;
    template <typename T1, typename T2> using CopyConst =
        std::conditional_t<std::is_const<T1>::value, std::add_const_t<T2>, std::remove_const_t<T2>>;
    template <size_t Bits> using SignedInteger = typename RS_Detail::IntegerType<Bits>::signed_type;
    template <size_t Bits> using UnsignedInteger = typename RS_Detail::IntegerType<Bits>::unsigned_type;

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
        Nnptr() = delete;
        ~Nnptr() = default;
        Nnptr(const Nnptr& p) = default;
        Nnptr(Nnptr&& p) noexcept { ptr = p.ptr; }
        Nnptr& operator=(const Nnptr& p) = default;
        Nnptr& operator=(Nnptr&& p) noexcept { ptr = p.ptr; return *this; }
        template <typename T2> Nnptr(const Nnptr<T2>& p) noexcept: ptr(p.ptr) {}
        template <typename T2> explicit Nnptr(T2* p): ptr(p) { check(p); }
        template <typename T2, typename D> Nnptr(T2* p, D d): ptr(p, d) { check(p); }
        template <typename T2, typename D> Nnptr(std::unique_ptr<T2, D>&& p) { check(p.get()); ptr.reset(p.release()); }
        template <typename T2> Nnptr& operator=(const Nnptr<T2>& p) noexcept { ptr = p.ptr; return *this; }
        template <typename T2, typename D> Nnptr& operator=(std::unique_ptr<T2, D>&& p) { check(p.get()); ptr.reset(p.release()); return *this; }
        explicit operator bool() const noexcept { return bool(ptr); }
        bool operator!() const noexcept { return ! ptr; }
        T& operator*() const noexcept { return *ptr; }
        T* operator->() const noexcept { return ptr.get(); }
        T* get() const noexcept { return ptr.get(); }
        template <typename T2> void reset(T2* p) { check(p); ptr.reset(p); }
        template <typename T2, typename D> void reset(T2* p, D d) { check(p); ptr.reset(p, d); }
        bool unique() const noexcept { return ptr.unique(); }
        template <typename T2, typename... Args> Nnptr<T2> friend make_nnptr(Args&&... args);
        template <typename T1, typename T2> Nnptr<T1> friend const_pointer_cast(const Nnptr<T2>& p) noexcept;
        template <typename T1, typename T2> Nnptr<T1> friend dynamic_pointer_cast(const Nnptr<T2>& p) noexcept;
        template <typename T1, typename T2> Nnptr<T1> friend static_pointer_cast(const Nnptr<T2>& p) noexcept;
    private:
        std::shared_ptr<T> ptr;
        static void check(const void* p) { if (! p) throw NullPointer(); }
        explicit Nnptr(const std::shared_ptr<T>& p) noexcept: ptr(p) {}
    };

    template <typename T2, typename... Args> Nnptr<T2> make_nnptr(Args&&... args) { return Nnptr<T2>(std::make_shared<T2>(args...)); }
    template <typename T1, typename T2> Nnptr<T1> const_pointer_cast(const Nnptr<T2>& p) noexcept { return Nnptr<T1>(const_pointer_cast<T1>(p.ptr)); }
    template <typename T1, typename T2> Nnptr<T1> dynamic_pointer_cast(const Nnptr<T2>& p) noexcept { return Nnptr<T1>(dynamic_pointer_cast<T1>(p.ptr)); }
    template <typename T1, typename T2> Nnptr<T1> static_pointer_cast(const Nnptr<T2>& p) noexcept { return Nnptr<T1>(static_pointer_cast<T1>(p.ptr)); }

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

    template <typename T> Nnptr<T> nnptr(const T& t) { return make_nnptr<T>(t); }
    template <typename T> std::shared_ptr<T> shptr(const T& t) { return std::make_shared<T>(t); }
    template <typename T> std::unique_ptr<T> unptr(const T& t) { return std::make_unique<T>(t); }

    // Type related functions

    template <typename T2, typename T1> bool is(const T1& ref) noexcept
        { return dynamic_cast<const T2*>(&ref) != nullptr; }
    template <typename T2, typename T1> bool is(const T1* ptr) noexcept
        { return dynamic_cast<const T2*>(ptr) != nullptr; }
    template <typename T2, typename T1> bool is(const std::unique_ptr<T1>& ptr) noexcept
        { return dynamic_cast<const T2*>(ptr.get()) != nullptr; }
    template <typename T2, typename T1> bool is(const std::shared_ptr<T1>& ptr) noexcept
        { return dynamic_cast<const T2*>(ptr.get()) != nullptr; }

    template <typename T2, typename T1> T2& as(T1& ref)
        { return dynamic_cast<T2&>(ref); }
    template <typename T2, typename T1> const T2& as(const T1& ref)
        { return dynamic_cast<const T2&>(ref); }
    template <typename T2, typename T1> T2& as(T1* ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> const T2& as(const T1* ptr)
        { if (ptr) return dynamic_cast<const T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(std::unique_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(const std::unique_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(std::shared_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }
    template <typename T2, typename T1> T2& as(const std::shared_ptr<T1>& ptr)
        { if (ptr) return dynamic_cast<T2&>(*ptr); else throw std::bad_cast(); }

    template <typename T2, typename T1> inline T2 binary_cast(const T1& t) noexcept {
        RS_STATIC_ASSERT(sizeof(T2) == sizeof(T1));
        T2 t2;
        memcpy(&t2, &t, sizeof(t));
        return t2;
    }

    template <typename T2, typename T1> inline T2 implicit_cast(const T1& t) { return t; }

    inline std::string demangle(const std::string& name) {
        auto mangled = name;
        std::shared_ptr<char> demangled;
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

    inline std::string type_name(const std::type_info& t) { return demangle(t.name()); }
    inline std::string type_name(const std::type_index& t) { return demangle(t.name()); }
    template <typename T> std::string type_name() { return type_name(typeid(T)); }
    template <typename T> std::string type_name(const T& t) { return type_name(typeid(t)); }

    // [Constants and literals]

    // Arithmetic constants

    #define RS_DEFINE_CONSTANT(name, value) \
        constexpr float name##_f = value##f; \
        constexpr double name##_d = value; \
        constexpr long double name##_ld = value##l; \
        template <typename T> constexpr T name##_c = T(name##_ld);

    // Mathematical constants

    RS_DEFINE_CONSTANT(e,           2.71828'18284'59045'23536'02874'71352'66249'77572'47093'69996);
    RS_DEFINE_CONSTANT(ln2,         0.69314'71805'59945'30941'72321'21458'17656'80755'00134'36026);
    RS_DEFINE_CONSTANT(ln10,        2.30258'50929'94045'68401'79914'54684'36420'76011'01488'62877);
    RS_DEFINE_CONSTANT(log2e,       1.44269'50408'88963'40735'99246'81001'89213'74266'45954'15299);
    RS_DEFINE_CONSTANT(log10e,      0.43429'44819'03251'82765'11289'18916'60508'22943'97005'80367);
    RS_DEFINE_CONSTANT(pi,          3.14159'26535'89793'23846'26433'83279'50288'41971'69399'37511);
    RS_DEFINE_CONSTANT(pi_2,        1.57079'63267'94896'61923'13216'91639'75144'20985'84699'68755);
    RS_DEFINE_CONSTANT(pi_4,        0.78539'81633'97448'30961'56608'45819'87572'10492'92349'84378);
    RS_DEFINE_CONSTANT(one_pi,      0.31830'98861'83790'67153'77675'26745'02872'40689'19291'48091);
    RS_DEFINE_CONSTANT(two_pi,      0.63661'97723'67581'34307'55350'53490'05744'81378'38582'96183);
    RS_DEFINE_CONSTANT(two_sqrtpi,  1.12837'91670'95512'57389'61589'03121'54517'16881'01258'65800);
    RS_DEFINE_CONSTANT(sqrtpi,      1.77245'38509'05516'02729'81674'83341'14518'27975'49456'12239);
    RS_DEFINE_CONSTANT(sqrt2pi,     2.50662'82746'31000'50241'57652'84811'04525'30069'86740'60994);
    RS_DEFINE_CONSTANT(sqrt2,       1.41421'35623'73095'04880'16887'24209'69807'85696'71875'37695);
    RS_DEFINE_CONSTANT(sqrt3,       1.73205'08075'68877'29352'74463'41505'87236'69428'05253'81038);
    RS_DEFINE_CONSTANT(sqrt5,       2.23606'79774'99789'69640'91736'68731'27623'54406'18359'61153);
    RS_DEFINE_CONSTANT(one_sqrt2,   0.70710'67811'86547'52440'08443'62104'84903'92848'35937'68847);

    // Conversion factors

    RS_DEFINE_CONSTANT(inch,           0.0254);              // m
    RS_DEFINE_CONSTANT(foot,           0.3048);              // m
    RS_DEFINE_CONSTANT(yard,           0.9144);              // m
    RS_DEFINE_CONSTANT(mile,           1609.344);            // m
    RS_DEFINE_CONSTANT(nautical_mile,  1852.0);              // m
    RS_DEFINE_CONSTANT(ounce,          0.028349523125);      // kg
    RS_DEFINE_CONSTANT(pound,          0.45359237);          // kg
    RS_DEFINE_CONSTANT(short_ton,      907.18474);           // kg
    RS_DEFINE_CONSTANT(long_ton,       1016.0469088);        // kg
    RS_DEFINE_CONSTANT(pound_force,    4.4482216152605);     // N
    RS_DEFINE_CONSTANT(erg,            1.0e-7);              // J
    RS_DEFINE_CONSTANT(foot_pound,     1.3558179483314004);  // J
    RS_DEFINE_CONSTANT(calorie,        4.184);               // J
    RS_DEFINE_CONSTANT(ton_tnt,        4.184e9);             // J
    RS_DEFINE_CONSTANT(horsepower,     745.69987158227022);  // W
    RS_DEFINE_CONSTANT(mmHg,           133.322387415);       // Pa
    RS_DEFINE_CONSTANT(atmosphere,     101325.0);            // Pa
    RS_DEFINE_CONSTANT(zero_celsius,   273.15);              // K

    // Physical constants

    RS_DEFINE_CONSTANT(atomic_mass_unit,           1.660538921e-27);  // kg
    RS_DEFINE_CONSTANT(avogadro_constant,          6.02214129e23);    // mol^-1
    RS_DEFINE_CONSTANT(boltzmann_constant,         1.3806488e-23);    // J K^-1
    RS_DEFINE_CONSTANT(elementary_charge,          1.602176565e-19);  // C
    RS_DEFINE_CONSTANT(gas_constant,               8.3144621);        // J mol^-1 K^-1
    RS_DEFINE_CONSTANT(gravitational_constant,     6.67384e-11);      // m^3 kg^-1 s^-2
    RS_DEFINE_CONSTANT(planck_constant,            6.62606957e-34);   // J s
    RS_DEFINE_CONSTANT(speed_of_light,             299792458.0);      // m s^-1
    RS_DEFINE_CONSTANT(stefan_boltzmann_constant,  5.670373e-8);      // W m^-2 K^-4

    // Astronomical constants

    RS_DEFINE_CONSTANT(earth_mass,         5.97219e24);          // kg
    RS_DEFINE_CONSTANT(earth_radius,       6.3710e6);            // m
    RS_DEFINE_CONSTANT(earth_gravity,      9.80665);             // m s^-2
    RS_DEFINE_CONSTANT(jupiter_mass,       1.8986e27);           // kg
    RS_DEFINE_CONSTANT(jupiter_radius,     6.9911e7);            // m
    RS_DEFINE_CONSTANT(solar_mass,         1.98855e30);          // kg
    RS_DEFINE_CONSTANT(solar_radius,       6.96342e8);           // m
    RS_DEFINE_CONSTANT(solar_luminosity,   3.846e26);            // W
    RS_DEFINE_CONSTANT(solar_temperature,  5778.0);              // K
    RS_DEFINE_CONSTANT(astronomical_unit,  1.49597870700e11);    // m
    RS_DEFINE_CONSTANT(light_year,         9.4607304725808e15);  // m
    RS_DEFINE_CONSTANT(parsec,             3.08567758149e16);    // m
    RS_DEFINE_CONSTANT(julian_day,         86'400.0);            // s
    RS_DEFINE_CONSTANT(julian_year,        31'557'600.0);        // s
    RS_DEFINE_CONSTANT(sidereal_year,      31'558'149.7635);     // s
    RS_DEFINE_CONSTANT(tropical_year,      31'556'925.19);       // s

    // Arithmetic literals

    namespace RS_Detail {

        template <typename T, char C> constexpr T digit_value() noexcept {
            return T(C >= '0' && C <= '9' ? C - '0'
                : C >= 'A' && C <= 'Z' ? C - 'A' + 10
                : C >= 'a' && C <= 'z' ? C - 'a' + 10 : -1);
        }

        template <typename T, int Base, char C, char... CS>
        struct BaseInteger {
            using prev_type = BaseInteger<T, Base, CS...>;
            static constexpr T digit = digit_value<T, C>();
            static constexpr T scale = prev_type::scale * (digit == T(-1) ? T(1) : T(Base));
            static constexpr T value = prev_type::value + (digit == T(-1) ? T(0) : digit * scale);
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
        constexpr wchar_t operator""_wc(unsigned long long n) noexcept { return wchar_t(n); }
        constexpr char16_t operator""_c16(unsigned long long n) noexcept { return char16_t(n); }
        constexpr char32_t operator""_c32(unsigned long long n) noexcept { return char32_t(n); }
        constexpr ptrdiff_t operator""_t(unsigned long long n) noexcept { return ptrdiff_t(n); }
        constexpr size_t operator""_z(unsigned long long n) noexcept { return size_t(n); }

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

    namespace RS_Detail {

        using namespace RS::Literals;

    }

    // Other constants

    constexpr unsigned KB = 1024u;
    constexpr unsigned long MB = 1'048'576ul;
    constexpr unsigned long GB = 1'073'741'824ul;
    constexpr unsigned long long TB = 1'099'511'627'776ull;

    // [Algorithms and ranges]

    // Generic algorithms

    template <typename Container, typename T>
    void append_to(Container& con, const T& t) {
        con.insert(con.end(), t);
    }

    template <typename Container>
    class AppendIterator:
    public OutputIterator<AppendIterator<Container>> {
    public:
        using value_type = typename Container::value_type;
        AppendIterator() = default;
        explicit AppendIterator(Container& c): con(&c) {}
        AppendIterator& operator=(const value_type& v) { append_to(*con, v); return *this; }
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

    template <typename M, typename K, typename T>
    typename M::mapped_type find_in_map(const M& map, const K& key, const T& def) {
        auto i = map.find(key);
        if (i == map.end())
            return def;
        else
            return i->second;
    }

    template <typename M, typename K>
    typename M::mapped_type find_in_map(const M& map, const K& key) {
        typename M::mapped_type t{};
        return find_in_map(map, key, t);
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

    inline size_t mem_match(const void* lhs, const void* rhs, size_t n) noexcept {
        if (! (lhs && rhs))
            return 0;
        auto cp1 = static_cast<const char*>(lhs);
        auto cp2 = static_cast<const char*>(rhs);
        size_t i = 0;
        while (i < n && cp1[i] == cp2[i])
            ++i;
        return i;
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

    namespace RS_Detail {

        template <typename T> struct ArrayCount;
        template <typename T, size_t N> struct ArrayCount<T[N]> { static constexpr size_t value = N; };

    }

    template <typename T> constexpr size_t array_count(T&&) noexcept { return RS_Detail::ArrayCount<std::remove_reference_t<T>>::value; }

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
        using iterator = Iterator;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        Iterator first, second;
        constexpr Iterator begin() const { return first; }
        constexpr Iterator end() const { return second; }
        constexpr bool empty() const { return first == second; }
        constexpr size_t size() const { return std::distance(first, second); }
    };

    template <typename Iterator> constexpr Irange<Iterator> irange(const Iterator& i, const Iterator& j) { return {i, j}; }
    template <typename Iterator> constexpr Irange<Iterator> irange(const std::pair<Iterator, Iterator>& p) { return {p.first, p.second}; }
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

    namespace RS_Detail {

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
        RS_Detail::adjust_integer_sequence(init, stop, delta, true);
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
        RS_Detail::adjust_integer_sequence(init, stop, delta, false);
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

    namespace RS_Detail {

        template <typename T>
        struct NumberMode {
            static constexpr char value =
                std::is_floating_point<T>::value ? 'F' :
                std::is_signed<T>::value ? 'S' :
                std::is_unsigned<T>::value ? 'U' : 'X';
        };

        template <typename T, bool Symmetric = false, char Mode = NumberMode<T>::value>
        struct Divide {
            std::pair<T, T> operator()(T x, T y) const noexcept {
                return {x / y, x % y};
            }
        };

        template <typename T>
        struct Divide<T, false, 'S'> {
            std::pair<T, T> operator()(T x, T y) const noexcept {
                auto q = x / y, r = x % y;
                if (r < T(0)) {
                    q += y < T(0) ? T(1) : T(-1);
                    r += std::abs(y);
                }
                return {q, r};
            }
        };

        template <typename T>
        struct Divide<T, false, 'F'> {
            std::pair<T, T> operator()(T x, T y) const noexcept {
                using std::abs;
                using std::floor;
                using std::fmod;
                auto q = floor(x / y), r = fmod(x, y);
                if (r < T(0))
                    r += abs(y);
                if (y < T(0) && r != T(0))
                    q += T(1);
                return {q, r};
            }
        };

        template <typename T, char Mode>
        struct Divide<T, true, Mode> {
            std::pair<T, T> operator()(T x, T y) const noexcept {
                static_assert(Mode != 'U', "Symmetric division on unsigned type");
                using std::abs;
                auto qr = Divide<T>()(x, y);
                if (qr.second > abs(y) / T(2)) {
                    qr.first += y > T(0) ? T(1) : T(-1);
                    qr.second -= abs(y);
                }
                return qr;
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

        template <typename T, char Mode = NumberMode<T>::value> struct SignOf;

        template <typename T>
        struct SignOf<T, 'S'> {
            constexpr int operator()(T t) const noexcept
                { return t < T(0) ? -1 : t == T(0) ? 0 : 1; }
        };

        template <typename T>
        struct SignOf<T, 'U'> {
            constexpr int operator()(T t) const noexcept
                { return t != T(0); }
        };

        template <typename T>
        struct SignOf<T, 'F'> {
            constexpr int operator()(T t) const noexcept
                { return t < T(0) ? -1 : t == T(0) ? 0 : 1; }
        };

    }

    template <typename T> constexpr T static_min(T t) noexcept { return t; }
    template <typename T, typename... Args> constexpr T static_min(T t, Args... args) noexcept { return t < static_min(args...) ? t : static_min(args...); }
    template <typename T> constexpr T static_max(T t) noexcept { return t; }
    template <typename T, typename... Args> constexpr T static_max(T t, Args... args) noexcept { return static_max(args...) < t ? t : static_max(args...); }
    template <typename T, typename T2, typename T3> constexpr T clamp(const T& x, const T2& min, const T3& max) noexcept { return x < T(min) ? T(min) : T(max) < x ? T(max) : x; }
    template <typename T> std::pair<T, T> divide(T x, T y) noexcept { return RS_Detail::Divide<T>()(x, y); }
    template <typename T> T quo(T x, T y) noexcept { return divide(x, y).first; }
    template <typename T> T rem(T x, T y) noexcept { return divide(x, y).second; }
    template <typename T> std::pair<T, T> symmetric_divide(T x, T y) noexcept { return RS_Detail::Divide<T, true>()(x, y); }
    template <typename T> T symmetric_quotient(T x, T y) noexcept { return symmetric_divide(x, y).first; }
    template <typename T> T symmetric_remainder(T x, T y) noexcept { return symmetric_divide(x, y).second; }
    template <typename T> T shift_left(T t, int n) noexcept { return RS_Detail::ShiftLeft<T>()(t, n); }
    template <typename T> T shift_right(T t, int n) noexcept { return RS_Detail::ShiftLeft<T>()(t, - n); }

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
        return RS_Detail::unsigned_gcd(abs(a), abs(b));
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

    namespace RS_Detail {

        template <typename T>
        constexpr T rotl_helper(T t, int n) noexcept {
            return (t << (n % (8 * sizeof(T)))) | (t >> (8 * sizeof(T) - (n % (8 * sizeof(T)))));
        }

        template <typename T>
        constexpr T rotr_helper(T t, int n) noexcept {
            return (t >> (n % (8 * sizeof(T)))) | (t << (8 * sizeof(T) - (n % (8 * sizeof(T)))));
        }

    }

    template <typename T>
    constexpr size_t ibits(T t) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return __builtin_popcountll(uint64_t(t));
    }

    template <typename T>
    constexpr size_t ilog2p1(T t) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return t ? 64 - __builtin_clzll(uint64_t(t)) : 0;
    }

    template <typename T>
    constexpr T ifloor2(T t) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return t ? T(1) << (ilog2p1(t) - 1) : 0;
    }

    template <typename T>
    constexpr T iceil2(T t) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return t > 1 ? T(1) << (ilog2p1(t - 1) - 1) << 1 : t;
    }

    template <typename T>
    constexpr bool ispow2(T t) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return ibits(t) == 1;
    }

    constexpr uint64_t letter_to_mask(char c) noexcept {
        return c >= 'A' && c <= 'Z' ? 1ull << (c - 'A') : c >= 'a' && c <= 'z' ? 1ull << (c - 'a' + 26) : 0;
    }

    template <typename T>
    constexpr T rotl(T t, int n) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return n == 0 ? t : n < 0 ? RS_Detail::rotr_helper(t, - n) : RS_Detail::rotl_helper(t, n);
    }

    template <typename T>
    constexpr T rotr(T t, int n) noexcept {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return n == 0 ? t : n < 0 ? RS_Detail::rotl_helper(t, - n) : RS_Detail::rotr_helper(t, n);
    }

    // Floating point arithmetic functions

    namespace RS_Detail {

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

    template <typename T> constexpr T degrees(T rad) noexcept { return rad * (T(180) / pi_c<T>); }
    template <typename T> constexpr T radians(T deg) noexcept { return deg * (pi_c<T> / T(180)); }
    template <typename T1, typename T2> constexpr T2 interpolate(T1 x1, T2 y1, T1 x2, T2 y2, T1 x) noexcept
        { return x1 == x2 ? (y1 + y2) * (T1(1) / T1(2)) : y1 == y2 ? y1 : y1 + (y2 - y1) * ((x - x1) / (x2 - x1)); }
    template <typename T2, typename T1> T2 iceil(T1 value) noexcept { return RS_Detail::Round<T2, T1, '>'>()(value); }
    template <typename T2, typename T1> T2 ifloor(T1 value) noexcept { return RS_Detail::Round<T2, T1, '<'>()(value); }
    template <typename T2, typename T1> T2 iround(T1 value) noexcept { return RS_Detail::Round<T2, T1, '='>()(value); }

    // [Functional utilities]

    // Function traits

    // Based on code by Kennytm and Victor Laskin
    // http://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda
    // http://vitiy.info/c11-functional-decomposition-easy-way-to-do-aop/

    namespace RS_Detail {

        template <typename Function>
        struct FunctionTraits:
        FunctionTraits<decltype(&Function::operator())> {};

        template <typename ReturnType, typename... Args>
        struct FunctionTraits<ReturnType (Args...)> {
            static constexpr size_t arity = sizeof...(Args);
            using argument_tuple = std::tuple<Args...>;
            using return_type = ReturnType;
            using signature = return_type(Args...);
            using std_function = std::function<signature>;
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
        { static constexpr size_t value = RS_Detail::FunctionTraits<Function>::arity; };
    template <typename Function> using ArgumentTuple = typename RS_Detail::FunctionTraits<Function>::argument_tuple;
    template <typename Function, size_t Index> using ArgumentType = std::tuple_element_t<Index, ArgumentTuple<Function>>;
    template <typename Function> using ReturnType = typename RS_Detail::FunctionTraits<Function>::return_type;
    template <typename Function> using FunctionSignature = typename RS_Detail::FunctionTraits<Function>::signature;
    template <typename Function> using StdFunction = typename RS_Detail::FunctionTraits<Function>::std_function;

    // Function operations

    template <typename Function> StdFunction<Function> stdfun(Function& lambda) { return StdFunction<Function>(lambda); }

    // tuple_invoke() is based on code from Cppreference.com
    // http://en.cppreference.com/w/cpp/utility/integer_sequence

    namespace RS_Detail {

        template <typename Function, typename Tuple, size_t... Indices>
        decltype(auto) invoke_helper(Function&& f, Tuple&& t, std::index_sequence<Indices...>) {
            return f(std::get<Indices>(std::forward<Tuple>(t))...);
        }

    }

    template<typename Function, typename Tuple>
    decltype(auto) tuple_invoke(Function&& f, Tuple&& t) {
        constexpr auto size = std::tuple_size<std::decay_t<Tuple>>::value;
        return RS_Detail::invoke_helper(std::forward<Function>(f), std::forward<Tuple>(t),
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

    class Djb2a {
    public:
        Djb2a& operator()(const void* ptr, size_t n) noexcept {
            auto p = static_cast<const uint8_t*>(ptr);
            if (p)
                for (size_t i = 0; i < n; ++i)
                    hash = ((hash << 5) + hash) ^ p[i];
            return *this;
        }
        Djb2a& operator()(const std::string& s) noexcept { return (*this)(s.data(), s.size()); }
        operator uint32_t() const noexcept { return hash; }
    private:
        uint32_t hash = 5381;
    };

    inline uint32_t djb2a(const void* ptr, size_t n) noexcept {
        Djb2a d;
        d(ptr, n);
        return d;
    }

    inline void hash_combine(size_t&) noexcept {}

    template <typename T>
    void hash_combine(size_t& hash, const T& t) noexcept {
        hash ^= std::hash<T>()(t) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
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
        { size_t operator()(const std::tuple<Args...>& t) const { return tuple_invoke(hash_value<Args...>, t); } };
    template <typename... Args> struct TupleHash<std::tuple<Args...>>: TupleHash<Args...> {};

    // Keyword arguments

    namespace RS_Detail {

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
        template <typename A> RS_Detail::Kwparam<K> operator=(const A& a) const { return {this, K(a)}; }
    };

    template <typename K, typename V, typename K2, typename... Args>
        bool kwget(const Kwarg<K>& k, V& v, const RS_Detail::Kwparam<K2>& p, const Args&... args) {
            if (&k != p.key)
                return kwget(k, v, args...);
            RS_Detail::Kwcopy<K2, V>()(p.val, v);
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
            Resource temp(std::move(r));
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
        using deleter = std::function<void(T&)>;
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
            Resource temp(std::move(r));
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
        using deleter = std::function<void(T*)>;
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
            Resource temp(std::move(r));
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
        using deleter = std::function<void(void*)>;
        void* res = nullptr;
        deleter del = {};
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
    };

    template <typename T, typename D>
    Resource<T> make_resource(T t, D d) {
        return {t, d};
    }

    // Based on ideas from Evgeny Panasyuk (https://github.com/panaseleus/stack_unwinding)
    // and Andrei Alexandrescu (https://isocpp.org/files/papers/N4152.pdf)

    namespace RS_Detail {

        #if defined(__GNUC__) && __GNUC__ < 6
            extern "C" char* __cxa_get_globals();
            inline int uncaught_exceptions() noexcept { return int(*reinterpret_cast<const uint32_t*>(__cxa_get_globals() + sizeof(void*))); }
        #else
            using std::uncaught_exceptions;
        #endif

        template <int Mode, bool Conditional = Mode >= 0>
        struct ScopeExitBase {
            bool should_run() const noexcept { return true; }
        };

        template <int Mode>
        struct ScopeExitBase<Mode, true> {
            int exceptions;
            ScopeExitBase(): exceptions(uncaught_exceptions()) {}
            bool should_run() const noexcept { return (exceptions == uncaught_exceptions()) == (Mode > 0); }
        };

        template <int Mode>
        class ConditionalScopeExit:
        private ScopeExitBase<Mode> {
        public:
            RS_NO_COPY_MOVE(ConditionalScopeExit)
            using callback = std::function<void()>;
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
            std::function<void()> func;
            static void silent_call(callback& f) noexcept { if (f) { try { f(); } catch (...) {} } }
        };

    }

    using ScopeExit = RS_Detail::ConditionalScopeExit<-1>;
    using ScopeSuccess = RS_Detail::ConditionalScopeExit<1>;
    using ScopeFailure = RS_Detail::ConditionalScopeExit<0>;

    class SizeGuard:
    public ScopeFailure {
    public:
        template <typename T> explicit SizeGuard(T& t): ScopeFailure([&t,n=t.size()] { t.resize(n); }) {}
    };

    class ValueGuard:
    public ScopeFailure {
    public:
        template <typename T> explicit ValueGuard(T& t): ScopeFailure([&t,v=t] { t = std::move(v); }) {}
    };

    class ScopedTransaction {
    public:
        RS_NO_COPY_MOVE(ScopedTransaction)
        using callback = std::function<void()>;
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
        std::vector<callback> stack;
    };

    // [I/O utilities]

    // File I/O operations

    namespace RS_Detail {

        inline bool load_file_helper(FILE* fp, std::string& dst, size_t limit) {
            static constexpr size_t bufsize = 65536;
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

    #ifdef _XOPEN_SOURCE

        inline bool load_file(const std::string& file, std::string& dst, size_t limit = npos) {
            using namespace RS_Detail;
            dst.clear();
            if (file.empty() || file == "-") {
                return load_file_helper(stdin, dst, limit);
            } else {
                FILE* fp = fopen(file.data(), "rb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return load_file_helper(fp, dst, limit);
            }
        }

        inline bool save_file(const std::string& file, const void* ptr, size_t n, bool append = false) {
            using namespace RS_Detail;
            if (file.empty() || file == "-") {
                return save_file_helper(stdout, ptr, n);
            } else {
                auto fp = fopen(file.data(), append ? "ab" : "wb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return save_file_helper(fp, ptr, n);
            }
        }

    #else

        inline bool load_file(const std::wstring& file, std::string& dst, size_t limit = npos) {
            using namespace RS_Detail;
            dst.clear();
            if (file.empty() || file == L"-") {
                return load_file_helper(stdin, dst, limit);
            } else {
                FILE* fp = _wfopen(file.data(), L"rb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return load_file_helper(fp, dst, limit);
            }
        }

        inline bool save_file(const std::wstring& file, const void* ptr, size_t n, bool append = false) {
            using namespace RS_Detail;
            if (file.empty() || file == L"-") {
                return save_file_helper(stdout, ptr, n);
            } else {
                auto fp = _wfopen(file.data(), append ? L"ab" : L"wb");
                if (! fp)
                    return false;
                ScopeExit guard([fp] { fclose(fp); });
                return save_file_helper(fp, ptr, n);
            }
        }

        inline bool load_file(const std::string& file, std::string& dst, size_t limit = npos) { return load_file(uconv<std::wstring>(file), dst, limit); }
        inline bool save_file(const std::string& file, const void* ptr, size_t n, bool append) { return save_file(uconv<std::wstring>(file), ptr, n, append); }
        inline bool save_file(const std::wstring& file, const std::string& src, bool append = false) { return save_file(file, src.data(), src.size(), append); }

    #endif

    inline bool save_file(const std::string& file, const std::string& src, bool append = false) { return save_file(file, src.data(), src.size(), append); }

    // Process I/O operations

    inline std::string run_command(const U8string& cmd) {
        static constexpr size_t block = 1024;
        std::shared_ptr<FILE> pipe;
        #ifdef _XOPEN_SOURCE
            pipe.reset(popen(cmd.data(), "r"), pclose);
        #else
            auto wcmd = uconv<std::wstring>(cmd);
            pipe.reset(_wpopen(wcmd.data(), L"rb"), pclose);
        #endif
        if (! pipe)
            return {};
        U8string result;
        size_t bytes = 0;
        do {
            size_t offset = result.size();
            result.resize(offset + block);
            bytes = fread(&result[0] + offset, 1, block, pipe.get());
            result.resize(offset + bytes);
        } while (bytes);
        return result;
    }

    // Terminal I/O operations

    inline bool is_stdout_redirected() noexcept { return ! isatty(1); }

    namespace RS_Detail {

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

    constexpr const char* xt_up           = "\x1b[A";    // Cursor up
    constexpr const char* xt_down         = "\x1b[B";    // Cursor down
    constexpr const char* xt_right        = "\x1b[C";    // Cursor right
    constexpr const char* xt_left         = "\x1b[D";    // Cursor left
    constexpr const char* xt_erase_left   = "\x1b[1K";   // Erase left
    constexpr const char* xt_erase_right  = "\x1b[K";    // Erase right
    constexpr const char* xt_erase_above  = "\x1b[1J";   // Erase above
    constexpr const char* xt_erase_below  = "\x1b[J";    // Erase below
    constexpr const char* xt_erase_line   = "\x1b[2K";   // Erase line
    constexpr const char* xt_clear        = "\x1b[2J";   // Clear screen
    constexpr const char* xt_reset        = "\x1b[0m";   // Reset attributes
    constexpr const char* xt_bold         = "\x1b[1m";   // Bold
    constexpr const char* xt_under        = "\x1b[4m";   // Underline
    constexpr const char* xt_bold_off     = "\x1b[22m";  // Bold off
    constexpr const char* xt_under_off    = "\x1b[24m";  // Underline off
    constexpr const char* xt_colour_off   = "\x1b[39m";  // Colour off
    constexpr const char* xt_black        = "\x1b[30m";  // Black fg
    constexpr const char* xt_red          = "\x1b[31m";  // Red fg
    constexpr const char* xt_green        = "\x1b[32m";  // Green fg
    constexpr const char* xt_yellow       = "\x1b[33m";  // Yellow fg
    constexpr const char* xt_blue         = "\x1b[34m";  // Blue fg
    constexpr const char* xt_magenta      = "\x1b[35m";  // Magenta fg
    constexpr const char* xt_cyan         = "\x1b[36m";  // Cyan fg
    constexpr const char* xt_white        = "\x1b[37m";  // White fg
    constexpr const char* xt_black_bg     = "\x1b[40m";  // Black bg
    constexpr const char* xt_red_bg       = "\x1b[41m";  // Red bg
    constexpr const char* xt_green_bg     = "\x1b[42m";  // Green bg
    constexpr const char* xt_yellow_bg    = "\x1b[43m";  // Yellow bg
    constexpr const char* xt_blue_bg      = "\x1b[44m";  // Blue bg
    constexpr const char* xt_magenta_bg   = "\x1b[45m";  // Magenta bg
    constexpr const char* xt_cyan_bg      = "\x1b[46m";  // Cyan bg
    constexpr const char* xt_white_bg     = "\x1b[47m";  // White bg

    inline std::string xt_move_up(int n) { return "\x1b[" + dec(n) + 'A'; }                                // Cursor up n spaces
    inline std::string xt_move_down(int n) { return "\x1b[" + dec(n) + 'B'; }                              // Cursor down n spaces
    inline std::string xt_move_right(int n) { return "\x1b[" + dec(n) + 'C'; }                             // Cursor right n spaces
    inline std::string xt_move_left(int n) { return "\x1b[" + dec(n) + 'D'; }                              // Cursor left n spaces
    inline std::string xt_colour(int rgb) { return "\x1b[38;5;" + dec(RS_Detail::rgb(rgb)) + 'm'; }      // Set fg colour to an RGB value (1-6)
    inline std::string xt_colour_bg(int rgb) { return "\x1b[48;5;" + dec(RS_Detail::rgb(rgb)) + 'm'; }   // Set bg colour to an RGB value (1-6)
    inline std::string xt_grey(int grey) { return "\x1b[38;5;" + dec(RS_Detail::grey(grey)) + 'm'; }     // Set fg colour to a grey level (1-24)
    inline std::string xt_grey_bg(int grey) { return "\x1b[48;5;" + dec(RS_Detail::grey(grey)) + 'm'; }  // Set bg colour to a grey level (1-24)

    // [Random numbers]

    // Simple random generators

    template <typename T, typename RNG>
    T random_integer(RNG& rng, T a, T b) {
        static_assert(std::is_integral<T>::value, "Random integer type is not an integer");
        // We need an unsigned integer type big enough for both the RNG and
        // output ranges.
        using R = typename RNG::result_type;
        using UT = std::make_unsigned_t<T>;
        using U2 = std::conditional_t<(sizeof(UT) > sizeof(R)), UT, R>;
        using U = std::conditional_t<(sizeof(U2) > sizeof(unsigned)), U2, unsigned>;
        // Compare the input range (max-min of the values generated by the
        // RNG) with the output range (max-min of the possible results).
        if (a > b)
            std::swap(a, b);
        U rmin = U(rng.min()), rng_range = U(rng.max() - rmin),
            out_range = U(UT(b) - UT(a)), result = U(0);
        if (out_range < rng_range) {
            // The RNG range is larger than the output range. Divide the
            // output of the RNG by the rounded down quotient of the ranges.
            // If one range is not an exact multiple of the other, this may
            // yield a value too large; try again.
            U ratio = (rng_range - out_range) / (out_range + U(1)) + U(1);
            U crit = ratio * out_range + (ratio - U(1));
            do result = U(rng() - rmin);
                while (result > crit);
            result /= ratio;
        } else if (out_range == rng_range) {
            // The trivial case where the two ranges are equal.
            result = U(rng() - rmin);
        } else {
            // The output range is larger than the RNG range. Split the output
            // range into a quotient and remainder modulo the RNG range +1;
            // call this function recursively for the quotient, then call the
            // RNG directly for the remainder. Try again if the result is too
            // large.
            U hi = U(0), lo = U(0);
            U ratio = (out_range - rng_range) / (rng_range + U(1));
            do {
                hi = random_integer(rng, U(0), ratio) * (rng_range + U(1));
                lo = U(rng() - rmin);
            } while (lo > out_range - hi);
            result = hi + lo;
        }
        return a + T(result);
    }

    template <typename T, typename RNG>
    T random_integer(RNG& rng, T t) {
        static_assert(std::is_integral<T>::value,
            "Random integer type is not an integer");
        if (t <= T(1))
            return T(0);
        else
            return random_integer(rng, T(0), t - T(1));
    }

    template <typename T, typename RNG>
    T random_dice(RNG& rng, T n = T(1), T faces = T(6)) {
        static_assert(std::is_integral<T>::value,
            "Random dice type is not an integer");
        if (n < T(1) || faces < T(1))
            return T(0);
        T sum = T(0);
        for (T i = T(0); i < n; ++i)
            sum += random_integer(rng, T(1), faces);
        return sum;
    }

    template <typename T, typename RNG>
    T random_float(RNG& rng, T a = T(1), T b = T(0)) {
        static_assert(std::is_floating_point<T>::value,
            "Random float type is not floating point");
        return a + (b - a) * (T(rng() - rng.min()) / (T(rng.max() - rng.min()) + T(1)));
    }

    template <typename T, typename RNG>
    T random_normal(RNG& rng) {
        static_assert(std::is_floating_point<T>::value,
            "Random normal type is not floating point");
        T u1 = random_float<T>(rng);
        T u2 = random_float<T>(rng);
        return std::sqrt(T(-2) * std::log(u1)) * std::cos(T(2) * pi_c<T> * u2);
    }

    template <typename T, typename RNG>
    T random_normal(RNG& rng, T m, T s) {
        static_assert(std::is_floating_point<T>::value,
            "Random float type is not floating point");
        return m + s * random_normal<T>(rng);
    }

    template <typename RNG>
    bool random_bool(RNG& rng) {
        using R = typename RNG::result_type;
        R min = rng.min();
        R range = rng.max() - min;
        R cutoff = min + range / R(2);
        // Test for odd range is reversed because range is one less than the
        // number of values.
        bool odd = range % R(1) == R(0);
        R x = R();
        do x = rng();
            while (odd && x == min);
        return x > cutoff;
    }

    template <typename RNG>
    bool random_bool(RNG& rng, double p) {
        return random_float<double>(rng) <= p;
    }

    template <typename RNG, typename T>
    bool random_bool(RNG& rng, T num, T den) {
        RS_STATIC_ASSERT(std::is_integral<T>::value);
        return random_integer(rng, den) < num;
    }

    template <typename Range, typename RNG>
    RangeValue<Range> random_choice(RNG& rng, const Range& range) {
        using std::begin;
        using std::end;
        using T = RangeValue<Range>;
        auto i = begin(range), j = end(range);
        if (i == j)
            return T();
        size_t n = std::distance(i, j);
        std::advance(i, random_integer(rng, n));
        return *i;
    }

    template <typename T, typename RNG>
    T random_choice(RNG& rng, std::initializer_list<T> list) {
        if (list.size() == 0)
            return T();
        else
            return list.begin()[random_integer(rng, list.size())];
    }

    // [Strings and related functions]

    // Character functions

    constexpr bool ascii_iscntrl(char c) noexcept { return uint8_t(c) <= 31 || c == 127; }
    constexpr bool ascii_isdigit(char c) noexcept { return c >= '0' && c <= '9'; }
    constexpr bool ascii_isgraph(char c) noexcept { return c >= '!' && c <= '~'; }
    constexpr bool ascii_islower(char c) noexcept { return c >= 'a' && c <= 'z'; }
    constexpr bool ascii_isprint(char c) noexcept { return c >= ' ' && c <= '~'; }
    constexpr bool ascii_ispunct(char c) noexcept { return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~'); }
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

    inline bool ascii_icase_equal(const std::string& lhs, const std::string& rhs) noexcept {
        if (lhs.size() != rhs.size())
            return false;
        for (size_t i = 0, n = lhs.size(); i < n; ++i)
            if (ascii_toupper(lhs[i]) != ascii_toupper(rhs[i]))
                return false;
        return true;
    }

    inline bool ascii_icase_less(const std::string& lhs, const std::string& rhs) noexcept {
        for (size_t i = 0, n = std::min(lhs.size(), rhs.size()); i < n; ++i) {
            char a = ascii_toupper(lhs[i]), b = ascii_toupper(rhs[i]);
            if (a != b)
                return uint8_t(a) < uint8_t(b);
        }
        return lhs.size() < rhs.size();
    }

    inline std::string ascii_lowercase(const std::string& s) {
        auto r = s;
        std::transform(r.begin(), r.end(), r.begin(), ascii_tolower);
        return r;
    }

    inline std::string ascii_uppercase(const std::string& s) {
        auto r = s;
        std::transform(r.begin(), r.end(), r.begin(), ascii_toupper);
        return r;
    }

    inline std::string ascii_titlecase(const std::string& s) {
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

    inline std::string ascii_sentencecase(const std::string& s) {
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

    inline U8string dent(size_t depth) { return U8string(4 * depth, ' '); }

    template <typename InputRange>
    std::string join(const InputRange& range, const std::string& delim = {}, bool term = false) {
        std::string result;
        for (auto& s: range) {
            result += s;
            result += delim;
        }
        if (! term && ! result.empty() && ! delim.empty())
            result.resize(result.size() - delim.size());
        return result;
    }

    inline std::string linearize(const std::string& str) {
        std::string result;
        size_t i = 0, j = 0, size = str.size();
        while (j < size) {
            i = str.find_first_not_of(ascii_whitespace, j);
            if (i == npos)
                break;
            j = str.find_first_of(ascii_whitespace, i);
            if (j == npos)
                j = size;
            if (! result.empty())
                result += ' ';
            result.append(str, i, j - i);
        }
        return result;
    }

    template <typename C>
    void null_term(std::basic_string<C>& str) noexcept {
        auto p = str.find(C(0));
        if (p != npos)
            str.resize(p);
    }

    template <typename OutputIterator>
    void split(const std::string& src, OutputIterator dst, const std::string& delim = ascii_whitespace) {
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

    inline std::vector<std::string> splitv(const std::string& src, const std::string& delim = ascii_whitespace) {
        std::vector<std::string> v;
        split(src, append(v), delim);
        return v;
    }

    inline bool starts_with(const std::string& str, const std::string& prefix) noexcept {
        return str.size() >= prefix.size()
            && memcmp(str.data(), prefix.data(), prefix.size()) == 0;
    }

    inline bool ends_with(const std::string& str, const std::string& suffix) noexcept {
        return str.size() >= suffix.size()
            && memcmp(str.data() + str.size() - suffix.size(), suffix.data(), suffix.size()) == 0;
    }

    inline bool string_is_ascii(const std::string& str) noexcept {
        return std::find_if(str.begin(), str.end(), [] (char c) { return c & 0x80; }) == str.end();
    }

    inline std::string trim(const std::string& str, const std::string& chars = ascii_whitespace) {
        size_t pos = str.find_first_not_of(chars);
        if (pos == npos)
            return {};
        else
            return str.substr(pos, str.find_last_not_of(chars) + 1 - pos);
    }

    inline std::string trim_left(const std::string& str, const std::string& chars = ascii_whitespace) {
        size_t pos = str.find_first_not_of(chars);
        if (pos == npos)
            return {};
        else
            return str.substr(pos, npos);
    }

    inline std::string trim_right(const std::string& str, const std::string& chars = ascii_whitespace) {
        return str.substr(0, str.find_last_not_of(chars) + 1);
    }

    inline std::string unqualify(const std::string& str, const std::string& delims = ".:") {
        if (delims.empty())
            return str;
        size_t pos = str.find_last_of(delims);
        if (pos == npos)
            return str;
        else
            return str.substr(pos + 1, npos);
    }

    // String formatting and parsing functions

    inline unsigned long long binnum(const std::string& str) noexcept { return strtoull(str.data(), nullptr, 2); }
    inline long long decnum(const std::string& str) noexcept { return strtoll(str.data(), nullptr, 10); }
    inline unsigned long long hexnum(const std::string& str) noexcept { return strtoull(str.data(), nullptr, 16); }
    inline double fpnum(const std::string& str) noexcept { return strtod(str.data(), nullptr); }

    template <typename T>
    U8string fp_format(T t, char mode = 'g', int prec = 6) {
        static const U8string modes = "EFGZefgz";
        if (modes.find(mode) == npos)
            throw std::invalid_argument("Invalid floating point mode: " + quote(U8string{mode}));
        U8string buf(20, '\0'), fmt;
        switch (mode) {
            case 'Z':  fmt = "%#.*G"; break;
            case 'z':  fmt = "%#.*g"; break;
            default:   fmt = U8string("%.*") + mode; break;
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

    inline int64_t si_to_int(const U8string& s) {
        using limits = std::numeric_limits<int64_t>;
        static constexpr const char* prefixes = "KMGTPEZY";
        char* endp = nullptr;
        errno = 0;
        int64_t n = strtoll(s.data(), &endp, 10);
        if (errno == ERANGE)
            throw std::range_error("Out of range: " + quote(s));
        if (errno || endp == s.data())
            throw std::invalid_argument("Invalid number: " + quote(s));
        if (ascii_isspace(*endp))
            ++endp;
        if (n && ascii_isalpha(*endp)) {
            auto pp = strchr(prefixes, ascii_toupper(*endp));
            if (pp) {
                int64_t steps = pp - prefixes + 1;
                double limit = log10(double(limits::max()) / double(std::abs(n))) / 3;
                if (steps > limit)
                    throw std::range_error("Out of range: " + quote(s));
                n *= int_power(int64_t(1000), steps);
            }
        }
        return n;
    }

    inline double si_to_float(const U8string& s) {
        using limits = std::numeric_limits<double>;
        static constexpr const char* prefixes = "yzafpnum kMGTPEZY";
        char* endp = nullptr;
        errno = 0;
        double x = strtod(s.data(), &endp);
        if (errno == ERANGE)
            throw std::range_error("Out of range: " + quote(s));
        if (errno || endp == s.data())
            throw std::invalid_argument("Invalid number: " + quote(s));
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
                    throw std::range_error("Out of range: " + quote(s));
                x *= pow(1000.0, steps);
            }
        }
        return x;
    }

    inline U8string hexdump(const void* ptr, size_t n, size_t block = 0) {
        if (ptr == nullptr || n == 0)
            return {};
        U8string result;
        result.reserve(3 * n);
        size_t col = 0;
        auto bptr = static_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < n; ++i) {
            RS_Detail::append_hex_byte(bptr[i], result);
            if (++col == block) {
                result += '\n';
                col = 0;
            } else {
                result += ' ';
            }
        }
        result.pop_back();
        if (block)
            result += '\n';
        return result;
    }

    inline U8string hexdump(const std::string& str, size_t block = 0) { return hexdump(str.data(), str.size(), block); }
    inline U8string tf(bool b) { return b ? "true" : "false"; }
    inline U8string yn(bool b) { return b ? "yes" : "no"; }

    template <typename T> U8string to_str(const T& t);

    namespace RS_Detail {

        template <typename R, typename I = decltype(std::begin(std::declval<R>())),
            typename V = typename std::iterator_traits<I>::value_type>
        struct RangeToString {
            U8string operator()(const R& r) const {
                U8string s = "[";
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
        struct RangeToString<R, I, std::pair<K, V>> {
            U8string operator()(const R& r) const {
                U8string s = "{";
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
            U8string operator()(const R& r) const {
                using std::begin;
                using std::end;
                return U8string(begin(r), end(r));
            }
        };

        template <typename> struct SfinaeTrue: std::true_type {};
        template <typename T> auto check_std_begin(int) -> SfinaeTrue<decltype(std::begin(std::declval<T>()))>;
        template <typename T> auto check_std_begin(long) -> std::false_type;
        template <typename T> auto check_std_end(int) -> SfinaeTrue<decltype(std::end(std::declval<T>()))>;
        template <typename T> auto check_std_end(long) -> std::false_type;
        template <typename T> struct CheckStdBegin: decltype(check_std_begin<T>(0)) {};
        template <typename T> struct CheckStdEnd: decltype(check_std_end<T>(0)) {};
        template <typename T> struct IsRangeType { static constexpr bool value = CheckStdBegin<T>::value && CheckStdEnd<T>::value; };

        template <typename T>
        struct ObjectToStringCategory {
            static constexpr char value =
                std::is_integral<T>::value ? 'I' :
                std::is_floating_point<T>::value ? 'F' :
                std::is_convertible<T, std::string>::value ? 'S' :
                IsRangeType<T>::value ? 'R' : 'X';
        };

        template <typename T, char C = ObjectToStringCategory<T>::value>
        struct ObjectToString {
            U8string operator()(const T& t) const {
                std::ostringstream out;
                out << t;
                return out.str();
            }
        };

        template <> struct ObjectToString<U8string> { U8string operator()(const U8string& t) const { return t; } };
        template <> struct ObjectToString<char*> { U8string operator()(char* t) const { return t ? U8string(t) : U8string(); } };
        template <> struct ObjectToString<const char*> { U8string operator()(const char* t) const { return t ? U8string(t) : U8string(); } };
        template <> struct ObjectToString<char> { U8string operator()(char t) const { return {t}; } };
        template <> struct ObjectToString<bool> { U8string operator()(bool t) const { return t ? "true" : "false"; } };
        template <> struct ObjectToString<std::nullptr_t> { U8string operator()(std::nullptr_t) const { return "null"; } };
        template <typename T> struct ObjectToString<T, 'I'> { U8string operator()(T t) const { return dec(t); } };
        template <typename T> struct ObjectToString<T, 'F'> { U8string operator()(T t) const { return fp_format(t); } };
        template <typename T> struct ObjectToString<T, 'S'> { U8string operator()(T t) const { return static_cast<std::string>(*&t); } };
        template <typename T> struct ObjectToString<T, 'R'>: RangeToString<T> {};
        template <typename T> struct ObjectToString<std::atomic<T>, 'X'> { U8string operator()(const std::atomic<T>& t) const { return ObjectToString<T>()(t); } };
        template <typename T1, typename T2> struct ObjectToString<std::pair<T1, T2>, 'X'>
            { U8string operator()(const std::pair<T1, T2>& t) const { return '{' + ObjectToString<T1>()(t.first) + ',' + ObjectToString<T2>()(t.second) + '}'; } };

    }

    template <typename T> inline U8string to_str(const T& t) { return RS_Detail::ObjectToString<T>()(t); }

    // HTML/XML tags

    class Tag {
    public:
        Tag() = default;
        Tag(std::ostream& out, const std::string& element) {
            std::string content = trim_right(element, "\n");
            size_t lfs = element.size() - content.size();
            content = trim(content, "\t\n\f\r <>");
            if (content.empty())
                return;
            std::string start = '<' + content + '>';
            if (content.back() == '/') {
                if (lfs)
                    start += '\n';
                out << start;
                return;
            }
            auto cut = std::find_if_not(content.begin(), content.end(), ascii_isalnum_w);
            std::string tag(content.begin(), cut);
            end = "</" + tag + '>';
            if (lfs >= 2)
                start += '\n';
            if (lfs)
                end += '\n';
            out << start;
            os = &out;
        }
        ~Tag() noexcept {
            if (os)
                try { *os << end; }
                    catch (...) {}
        }
        Tag(const Tag&) = delete;
        Tag(Tag&& t) noexcept {
            if (&t != this) {
                end = std::move(t.end);
                os = t.os;
                t.os = nullptr;
            }
        }
        Tag& operator=(const Tag&) = delete;
        Tag& operator=(Tag&& t) noexcept {
            if (&t != this) {
                if (os)
                    *os << end;
                end = std::move(t.end);
                os = t.os;
                t.os = nullptr;
            }
            return *this;
        }
    private:
        std::string end;
        std::ostream* os = nullptr;
    };

    template <typename T>
    void tagged(std::ostream& out, const std::string& element, const T& t) {
        Tag html(out, element);
        out << t;
    }

    template <typename... Args>
    void tagged(std::ostream& out, const std::string& element, const Args&... args) {
        Tag html(out, element);
        tagged(out, args...);
    }

    // [Threads]

    // Thread class

    #ifdef _XOPEN_SOURCE

        class Thread {
        public:
            using callback = std::function<void()>;
            using id_type = pthread_t;
            Thread() = default;
            explicit Thread(callback f) {
                if (f) {
                    impl = std::make_unique<impl_type>();
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
                rethrow(impl->except);
            }
            static size_t cpu_threads() noexcept {
                size_t n = 0;
                #ifdef __APPLE__
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
            std::unique_ptr<impl_type> impl;
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
            using callback = std::function<void()>;
            using id_type = uint32_t;
            Thread() = default;
            explicit Thread(callback f) {
                if (f) {
                    impl = std::make_unique<impl_type>();
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
                rethrow(impl->except);
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
            std::unique_ptr<impl_type> impl;
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

    #ifdef _XOPEN_SOURCE

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
            RS_NO_COPY_MOVE(Mutex)
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
            RS_NO_COPY_MOVE(Mutex)
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

    #ifndef __MINGW32__
        template <typename T> inline auto make_lock(T& t) { return std::unique_lock<T>(t); }
    #endif

    #ifdef _XOPEN_SOURCE

        // The Posix standard recommends using clock_gettime() with
        // CLOCK_REALTIME to find the current time for a CV wait. Darwin
        // doesn't have clock_gettime(), but the Apple man page for
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
            template <typename C, typename D, typename Pred> bool wait_until(MutexLock& lock, std::chrono::time_point<C, D> t, Pred p) {
                using namespace std::chrono;
                nanoseconds ns;
                auto now = C::now();
                if (now < t)
                    ns = duration_cast<nanoseconds>(t - now);
                return wait_impl(lock, ns, predicate(p));
            }
        private:
            using predicate = std::function<bool()>;
            pthread_cond_t pcond;
            RS_NO_COPY_MOVE(ConditionVariable)
            bool wait_impl(MutexLock& ml, std::chrono::nanoseconds ns, predicate p) {
                using namespace RS::Literals;
                using namespace std::chrono;
                static constexpr unsigned long G = 1'000'000'000ul;
                if (p())
                    return true;
                auto nsc = ns.count();
                if (nsc <= 0)
                    return false;
                timespec ts;
                #ifdef __APPLE__
                    timeval tv;
                    gettimeofday(&tv, nullptr);
                    ts = {tv.tv_sec, 1000 * tv.tv_usec};
                #else
                    clock_gettime(CLOCK_REALTIME, &ts);
                #endif
                ts.tv_nsec += nsc % G;
                ts.tv_sec += nsc / G + ts.tv_nsec / G;
                ts.tv_nsec %= G;
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
            template <typename C, typename D, typename Pred> bool wait_until(MutexLock& lock, std::chrono::time_point<C, D> t, Pred p) {
                using namespace std::chrono;
                nanoseconds ns;
                auto now = C::now();
                if (now < t)
                    ns = duration_cast<nanoseconds>(t - now);
                return wait_impl(lock, ns, predicate(p));
            }
        private:
            using predicate = std::function<bool()>;
            CONDITION_VARIABLE wcond;
            RS_NO_COPY_MOVE(ConditionVariable)
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

    // Time and date types

    using Dseconds = std::chrono::duration<double>;
    using Ddays = std::chrono::duration<double, std::ratio<86400>>;
    using Dyears = std::chrono::duration<double, std::ratio<31557600>>;
    using ReliableClock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock, std::chrono::steady_clock>;

    // General time and date operations

    RS_ENUM_CLASS(Zone, int, 1, utc, local)

    namespace RS_Detail {

        #ifdef _XOPEN_SOURCE

            using SleepUnit = std::chrono::microseconds;

            inline void sleep_for(SleepUnit t) noexcept {
                static constexpr unsigned long M = 1'000'000;
                auto usec = t.count();
                if (usec > 0) {
                    timeval tv;
                    tv.tv_sec = usec / M;
                    tv.tv_usec = usec % M;
                    select(0, nullptr, nullptr, nullptr, &tv);
                } else {
                    sched_yield();
                }
            }

        #else

            using SleepUnit = std::chrono::milliseconds;

            inline void sleep_for(SleepUnit t) noexcept {
                auto msec = t.count();
                if (msec > 0)
                    Sleep(uint32_t(msec));
                else
                    Sleep(0);
            }

        #endif

    }

    template <typename R, typename P>
    void from_seconds(double s, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        d = duration_cast<duration<R, P>>(Dseconds(s));
    }

    template <typename R, typename P>
    double to_seconds(const std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        return duration_cast<Dseconds>(d).count();
    }

    inline std::chrono::system_clock::time_point make_date(int year, int month, int day, int hour = 0, int min = 0, double sec = 0, Zone z = Zone::utc) noexcept {
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
            #ifdef _XOPEN_SOURCE
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
        RS_Detail::sleep_for(duration_cast<RS_Detail::SleepUnit>(t));
    }

    inline void sleep_for(double t) noexcept {
        using namespace std::chrono;
        RS_Detail::sleep_for(duration_cast<RS_Detail::SleepUnit>(Dseconds(t)));
    }

    template <typename C, typename D>
    void sleep_until(std::chrono::time_point<C, D> t) noexcept {
        sleep_for(t - C::now());
    }

    // Time and date formatting

    // Unfortunately strftime() doesn't set errno and simply returns zero on
    // any error. This means that there is no way to distinguish between an
    // invalid format string, an output buffer that is too small, and a
    // legitimately empty result. Here we try first with a reasonable output
    // length, and if that fails, try again with a much larger one; if it
    // still fails, give up. This could theoretically fail in the face of a
    // very long localized date format, but there doesn't seem to be a better
    // solution.

    inline U8string format_date(std::chrono::system_clock::time_point tp, const U8string& format, Zone z = Zone::utc) {
        using namespace std::chrono;
        auto t = system_clock::to_time_t(tp);
        tm stm = z == Zone::local ? *localtime(&t) : *gmtime(&t);
        U8string result(std::max(2 * format.size(), size_t(100)), '\0');
        auto rc = strftime(&result[0], result.size(), format.data(), &stm);
        if (rc == 0) {
            result.resize(10 * result.size(), '\0');
            rc = strftime(&result[0], result.size(), format.data(), &stm);
        }
        result.resize(rc);
        result.shrink_to_fit();
        return result;
    }

    inline U8string format_date(std::chrono::system_clock::time_point tp, int prec = 0, Zone z = Zone::utc) {
        using namespace std::chrono;
        using namespace std::literals;
        U8string result = format_date(tp, "%Y-%m-%d %H:%M:%S"s, z);
        if (prec > 0) {
            double sec = to_seconds(tp.time_since_epoch());
            double isec;
            double fsec = modf(sec, &isec);
            U8string buf(prec + 3, '\0');
            snprintf(&buf[0], buf.size(), "%.*f", prec, fsec);
            result += buf.data() + 1;
        }
        return result;
    }

    template <typename R, typename P>
    U8string format_time(const std::chrono::duration<R, P>& time, int prec = 0) {
        using namespace std::chrono;
        auto whole = duration_cast<seconds>(time);
        int64_t s = whole.count();
        auto frac = time - duration_cast<duration<R, P>>(whole);
        double f = duration_cast<Dseconds>(frac).count();
        U8string result;
        if (s < 0 || f < 0)
            result += '-';
        s = std::abs(s);
        f = std::abs(f);
        int64_t d = s / 86400;
        s -= 86400 * d;
        if (d)
            result += dec(d) + 'd';
        int64_t h = s / 3600;
        s -= 3600 * h;
        if (d || h)
            result += dec(h, d ? 2 : 1) + 'h';
        int64_t m = s / 60;
        if (d || h || m)
            result += dec(m, d || h ? 2 : 1) + 'm';
        s -= 60 * m;
        result += dec(s, d || h || m ? 2 : 1);
        if (prec > 0) {
            size_t pos = result.size();
            result += fp_format(f, 'f', prec);
            result.erase(pos, 1);
        }
        result += 's';
        return result;
    }

    // System specific time and date conversions

    template <typename R, typename P>
    timespec duration_to_timespec(const std::chrono::duration<R, P>& d) noexcept {
        using namespace RS::Literals;
        using namespace std::chrono;
        static constexpr int64_t G = 1'000'000'000ll;
        int64_t nsec = duration_cast<nanoseconds>(d).count();
        return {time_t(nsec / G), long(nsec % G)};
    }

    template <typename R, typename P>
    timeval duration_to_timeval(const std::chrono::duration<R, P>& d) noexcept {
        using namespace RS::Literals;
        using namespace std::chrono;
        #ifdef _XOPEN_SOURCE
            using sec_type = time_t;
            using usec_type = suseconds_t;
        #else
            using sec_type = long;
            using usec_type = long;
        #endif
        static constexpr int64_t M = 1'000'000;
        int64_t usec = duration_cast<microseconds>(d).count();
        return {sec_type(usec / M), usec_type(usec % M)};
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

    #ifdef _WIN32

        inline std::chrono::system_clock::time_point filetime_to_timepoint(const FILETIME& ft) noexcept {
            using namespace RS::Literals;
            using namespace std::chrono;
            static constexpr int64_t filetime_freq = 10'000'000; // FILETIME ticks (100 ns) per second
            static constexpr int64_t windows_epoch = 11'644'473'600ll; // Windows epoch (1601) to Unix epoch (1970)
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

    // Blob class

    class Blob:
    public LessThanComparable<Blob> {
    public:
        Blob() = default;
        explicit Blob(size_t n) { init_size(n); }
        Blob(size_t n, uint8_t x) { init_size(n); memset(ptr, x, len); }
        Blob(void* p, size_t n): Blob(p, n, &std::free) {}
        template <typename F> Blob(void* p, size_t n, F f) {
            if (p && n) {
                ptr = p;
                len = n;
                del = f;
            }
        }
        ~Blob() noexcept { if (ptr && del) try { del(ptr); } catch (...) {} }
        Blob(const Blob& b) { init_copy(b.data(), b.size()); }
        Blob(Blob&& b) noexcept: ptr(b.ptr), len(b.len) { del.swap(b.del); }
        Blob& operator=(const Blob& b) { copy(b.data(), b.size()); return *this; }
        Blob& operator=(Blob&& b) noexcept { Blob b2(std::move(b)); swap(b2); return *this; }
        void* data() noexcept { return ptr; }
        const void* data() const noexcept { return ptr; }
        uint8_t* bdata() noexcept { return static_cast<uint8_t*>(ptr); }
        const uint8_t* bdata() const noexcept { return static_cast<const uint8_t*>(ptr); }
        char* cdata() noexcept { return static_cast<char*>(ptr); }
        const char* cdata() const noexcept { return static_cast<const char*>(ptr); }
        Irange<uint8_t*> bytes() noexcept { return {bdata(), bdata() + len}; }
        Irange<const uint8_t*> bytes() const noexcept { return {bdata(), bdata() + len}; }
        Irange<char*> chars() noexcept { return {cdata(), cdata() + len}; }
        Irange<const char*> chars() const noexcept { return {cdata(), cdata() + len}; }
        void clear() noexcept { Blob b; swap(b); }
        void copy(const void* p, size_t n) { Blob b; b.init_copy(p, n); swap(b); }
        bool empty() const noexcept { return len == 0; }
        void fill(uint8_t x) noexcept { memset(ptr, x, len); }
        size_t hash() const noexcept { return djb2a(ptr, len); }
        U8string hex(size_t block = 0) const { return hexdump(ptr, len, block); }
        void reset(size_t n) { Blob b(n); swap(b); }
        void reset(size_t n, uint8_t x) { Blob b(n, x); swap(b); }
        void reset(void* p, size_t n) { Blob b(p, n); swap(b); }
        template <typename F> void reset(void* p, size_t n, F f) { Blob b(p, n, f); swap(b); }
        size_t size() const noexcept { return len; }
        std::string str() const { return empty() ? std::string() : std::string(cdata(), len); }
        void swap(Blob& b) noexcept {
            std::swap(ptr, b.ptr);
            std::swap(len, b.len);
            del.swap(b.del);
        }
    private:
        void* ptr = nullptr;
        size_t len = 0;
        std::function<void(void*)> del;
        void init_copy(const void* p, size_t n) {
            if (p && n) {
                init_size(n);
                memcpy(ptr, p, n);
            }
        }
        void init_size(size_t n) {
            if (n) {
                ptr = std::malloc(n);
                if (! ptr)
                    throw std::bad_alloc();
                len = n;
                del = &std::free;
            }
        }
    };

    inline bool operator==(const Blob& lhs, const Blob& rhs) noexcept {
        return lhs.size() == rhs.size() && memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
    }

    inline bool operator<(const Blob& lhs, const Blob& rhs) noexcept {
        auto cmp = memcmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
        return cmp < 0 || (cmp == 0 && lhs.size() < rhs.size());
    }

    inline void swap(Blob& b1, Blob& b2) noexcept { b1.swap(b2); }

    // Logging

    namespace RS_Detail {

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

        inline void log_message(const U8string& msg, bool timestamp) {
            using namespace std::chrono;
            static Mutex mtx;
            auto lock = make_lock(mtx);
            U8string text = xt_colour(hash_xcolour(Thread::current()));
            text += "# ";
            if (timestamp) {
                text += "[";
                text += format_date(system_clock::now(), 3);
                text += "] ";
            }
            text += trim_right(msg);
            text += xt_reset;
            text += '\n';
            fwrite(text.data(), 1, text.size(), stdout);
            fflush(stdout);
        }

    }

    inline void logx(const U8string& msg) noexcept {
        try { RS_Detail::log_message(msg, false); }
        catch (...) {}
    }

    inline void logx(const char* msg) noexcept {
        try { RS_Detail::log_message(cstr(msg), false); }
        catch (...) {}
    }

    template <typename... Args> inline void logx(Args... args) noexcept {
        try { RS_Detail::log_message(join(std::vector<U8string>{to_str(args)...}, " "), false); }
        catch (...) {}
    }

    inline void logt(const U8string& msg) noexcept {
        try { RS_Detail::log_message(msg, true); }
        catch (...) {}
    }

    inline void logt(const char* msg) noexcept {
        try { RS_Detail::log_message(cstr(msg), true); }
        catch (...) {}
    }

    template <typename... Args> inline void logt(Args... args) noexcept {
        try { RS_Detail::log_message(join(std::vector<U8string>{to_str(args)...}, " "), true); }
        catch (...) {}
    }

    class Stopwatch {
    public:
        RS_NO_COPY_MOVE(Stopwatch)
        explicit Stopwatch(const U8string& name, int precision = 3) noexcept:
            Stopwatch(name.data(), precision) {}
        explicit Stopwatch(const char* name, int precision = 3) noexcept {
            try {
                prefix = name;
                prefix += " : ";
                prec = precision;
                start = ReliableClock::now();
            }
            catch (...) {}
        }
        ~Stopwatch() noexcept {
            try {
                auto t = ReliableClock::now() - start;
                logx(prefix + format_time(t, prec));
            }
            catch (...) {}
        }
    private:
        U8string prefix;
        int prec;
        ReliableClock::time_point start;
    };

    // UUID

    namespace RS_Detail {

        inline int decode_hex_byte(std::string::const_iterator& i, std::string::const_iterator end) {
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
        explicit Uuid(const uint8_t* ptr) noexcept { if (ptr) memcpy(bytes, ptr, 16); else memset(bytes, 0, 16); }
        explicit Uuid(const std::string& s);
        uint8_t& operator[](size_t i) noexcept { return bytes[i]; }
        const uint8_t& operator[](size_t i) const noexcept { return bytes[i]; }
        uint8_t* begin() noexcept { return bytes; }
        const uint8_t* begin() const noexcept { return bytes; }
        uint8_t* end() noexcept { return bytes + 16; }
        const uint8_t* end() const noexcept { return bytes + 16; }
        size_t hash() const noexcept { return djb2a(bytes, 16); }
        U8string str() const;
        friend bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept { return memcmp(lhs.bytes, rhs.bytes, 16) == 0; }
        friend bool operator<(const Uuid& lhs, const Uuid& rhs) noexcept { return memcmp(lhs.bytes, rhs.bytes, 16) == -1; }
    private:
        friend struct RandomUuid;
        union {
            uint8_t bytes[16];
            uint32_t words[4];
        };
    };

    inline Uuid::Uuid(const std::string& s) {
        auto begins = s.begin(), i = begins, ends = s.end();
        auto j = begin(), endu = end();
        int rc = 0;
        while (i != ends && j != endu && rc != -1) {
            i = std::find_if(i, ends, ascii_isxdigit);
            if (i == ends)
                break;
            rc = RS_Detail::decode_hex_byte(i, ends);
            if (rc == -1)
                break;
            *j++ = uint8_t(rc);
        }
        if (rc == -1 || j != endu || std::find_if(i, ends, ascii_isalnum) != ends)
            throw std::invalid_argument("Invalid UUID: " + s);
    }

    inline U8string Uuid::str() const {
        U8string s;
        s.reserve(36);
        int i = 0;
        for (; i < 4; ++i)
            RS_Detail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 6; ++i)
            RS_Detail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 8; ++i)
            RS_Detail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 10; ++i)
            RS_Detail::append_hex_byte(bytes[i], s);
        s += '-';
        for (; i < 16; ++i)
            RS_Detail::append_hex_byte(bytes[i], s);
        return s;
    }

    inline std::ostream& operator<<(std::ostream& o, const Uuid& u) { return o << u.str(); }
    inline U8string to_str(const Uuid& u) { return u.str(); }

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
        explicit Version(const U8string& s);
        unsigned operator[](size_t i) const noexcept { return i < ver.size() ? ver[i] : 0; }
        const unsigned* begin() const noexcept { return ver.data(); }
        const unsigned* end() const noexcept { return ver.data() + ver.size(); }
        unsigned major() const noexcept { return (*this)[0]; }
        unsigned minor() const noexcept { return (*this)[1]; }
        unsigned patch() const noexcept { return (*this)[2]; }
        std::string str(size_t min_elements = 2) const;
        std::string suffix() const { return suf; }
        uint32_t to32() const noexcept;
        static Version from32(uint32_t n) noexcept;
        friend bool operator==(const Version& lhs, const Version& rhs) noexcept
            { return lhs.ver == rhs.ver && lhs.suf == rhs.suf; }
        friend bool operator<(const Version& lhs, const Version& rhs) noexcept
            { int c = compare_3way(lhs.ver, rhs.ver); return c == 0 ? lhs.suf < rhs.suf : c == -1; }
    private:
        std::vector<unsigned> ver;
        U8string suf;
        template <typename... Args> void append(unsigned n, Args... args) { ver.push_back(n); append(args...); }
        void append(const U8string& s) { suf = s; }
        void append() {}
        void trim() { while (! ver.empty() && ver.back() == 0) ver.pop_back(); }
    };

    inline Version::Version(const U8string& s) {
        auto i = s.begin(), end = s.end();
        while (i != end) {
            auto j = std::find_if_not(i, end, ascii_isdigit);
            if (i == j)
                break;
            ver.push_back(unsigned(decnum(U8string(i, j))));
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

    inline std::string Version::str(size_t min_elements) const {
        std::string s;
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

    inline U8string to_str(const Version& v) {
        return v.str();
    }

}

RS_DEFINE_STD_HASH(RS::Blob)
RS_DEFINE_STD_HASH(RS::Uuid)

namespace std {
    template <typename T, RS::ByteOrder B> struct hash<RS::Endian<T, B>> {
        using argument_type = RS::Endian<T, B>;
        using result_type = size_t;
        size_t operator()(RS::Endian<T, B> t) const noexcept { return std::hash<T>()(t.get()); }
    };
}
