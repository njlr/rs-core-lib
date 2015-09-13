#pragma once

// Compilation environment identification

#if defined(__CYGWIN__)
    #define PRI_TARGET_CYGWIN 1
    #define PRI_TARGET_UNIX 1
    #if defined(_WIN32)
        #define PRI_TARGET_WINDOWS 1
    #endif
#elif defined(_WIN32)
    // We don't support MSVC yet, so we can assume Mingw
    #define PRI_TARGET_MINGW 1
    #define PRI_TARGET_NATIVE_WINDOWS 1
    #define PRI_TARGET_WINDOWS 1
#else
    #define PRI_TARGET_UNIX 1
    #if defined(__APPLE__)
        #define PRI_TARGET_DARWIN 1
        #include "TargetConditionals.h"
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

#if defined(PRI_TARGET_WINDOWS)
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

// Includes here so anything that needs the macros above will see them

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
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>
#include <cxxabi.h>

#if defined(PRI_TARGET_UNIX)
    #include <pthread.h>
    #include <sched.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <unistd.h>
    #if defined(PRI_TARGET_DARWIN)
        #include <mach/mach.h>
    #endif
#endif

#if defined(PRI_TARGET_WINDOWS)
    #include <windows.h>
    #include <io.h>
#endif

// Fix GNU brain damage

#if defined(major)
    #undef major
#endif
#if defined(minor)
    #undef minor
#endif

// Other preprocessor macros

#define PRI_BOUNDS(range) std::begin(range), std::end(range)
#define PRI_LDLIB(libs)
#define PRI_STATIC_ASSERT(expr) static_assert((expr), # expr)

// For internal use only

#define PRI_NO_COPY_MOVE(T) \
    T(const T&) = delete; \
    T(T&&) = delete; \
    T& operator=(const T&) = delete; \
    T& operator=(T&&) = delete;

namespace Prion {

    // Basic types

    using std::basic_string;
    using std::string;
    using std::u16string;
    using std::u32string;
    using std::wstring;
    using u8string = std::string;
    using int128_t = __int128;
    using uint128_t = unsigned __int128;

    // Functions needed early

    template <typename T> inline auto as_signed(T t) noexcept { return static_cast<std::make_signed_t<T>>(t); }
    inline auto as_signed(int128_t t) noexcept { return int128_t(t); }
    inline auto as_signed(uint128_t t) noexcept { return int128_t(t); }
    template <typename T> inline auto as_unsigned(T t) noexcept { return static_cast<std::make_unsigned_t<T>>(t); }
    inline auto as_unsigned(int128_t t) noexcept { return uint128_t(t); }
    inline auto as_unsigned(uint128_t t) noexcept { return uint128_t(t); }

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
            std::reverse(PRI_BOUNDS(s));
            return s;
        }

    }

    template <typename T> u8string dec(T x, size_t digits = 1) { return PrionDetail::int_to_string(x, 10, digits); }
    template <typename T> u8string hex(T x, size_t digits = 2 * sizeof(T)) { return PrionDetail::int_to_string(x, 16, digits); }

    #if defined(PRI_TARGET_WINDOWS)

        inline wstring utf8_to_wstring(const u8string& ustr) {
            using namespace PrionDetail;
            if (ustr.empty())
                return {};
            int rc = MultiByteToWideChar(CP_UTF8, 0, ustr.data(), ustr.size(), nullptr, 0);
            if (rc <= 0)
                return {};
            wstring result(rc, 0);
            MultiByteToWideChar(CP_UTF8, 0, ustr.data(), ustr.size(), &result[0], rc);
            return result;
        }

        inline u8string wstring_to_utf8(const wstring& wstr) {
            using namespace PrionDetail;
            if (wstr.empty())
                return {};
            int rc = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
            if (rc <= 0)
                return {};
            u8string result(rc, 0);
            WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), &result[0], rc, nullptr, nullptr);
            return result;
        }

    #endif

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
        friend T& operator-=(T& lhs, ptrdiff_t rhs) noexcept { return lhs += - rhs; }
        friend T operator+(const T& lhs, ptrdiff_t rhs) { T t = lhs; t += rhs; return t; }
        friend T operator+(ptrdiff_t lhs, const T& rhs) { T t = rhs; t += lhs; return t; }
        friend T operator-(const T& lhs, ptrdiff_t rhs) { T t = lhs; t -= rhs; return t; }
    };

    // Constants

    constexpr const char* ascii_whitespace = "\t\n\v\f\r ";
    constexpr size_t npos = string::npos;

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

    PRI_DEFINE_CONSTANT(e,        2.71828182845904523536028747135266249775724709369996);
    PRI_DEFINE_CONSTANT(ln2,      0.69314718055994530941723212145817656807550013436026);
    PRI_DEFINE_CONSTANT(ln10,     2.30258509299404568401799145468436420760110148862877);
    PRI_DEFINE_CONSTANT(pi,       3.14159265358979323846264338327950288419716939937511);
    PRI_DEFINE_CONSTANT(sqrt2,    1.41421356237309504880168872420969807856967187537695);
    PRI_DEFINE_CONSTANT(sqrt3,    1.73205080756887729352744634150587236694280525381038);
    PRI_DEFINE_CONSTANT(sqrt5,    2.23606797749978969640917366873127623544061835961153);
    PRI_DEFINE_CONSTANT(sqrt2pi,  2.50662827463100050241576528481104525300698674060994);

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

        template <char C> constexpr uint128_t digit_value() noexcept
            { return uint128_t(C >= 'A' && C <= 'Z' ? C - 'A' + 10 : C >= 'a' && C <= 'z' ? C - 'a' + 10 : C - '0'); }

        template <uint128_t Base, char C, char... CS>
        struct BaseInteger {
            using prev_type = BaseInteger<Base, CS...>;
            static constexpr uint128_t scale = Base * prev_type::scale;
            static constexpr uint128_t value = digit_value<C>() * scale + prev_type::value;
        };

        template <uint128_t Base, char C>
        struct BaseInteger<Base, C> {
            static constexpr uint128_t scale = 1;
            static constexpr uint128_t value = digit_value<C>();
        };

        template <char... CS> struct MakeInteger: public BaseInteger<10, CS...> {};
        template <char... CS> struct MakeInteger<'0', 'x', CS...>: public BaseInteger<16, CS...> {};
        template <char... CS> struct MakeInteger<'0', 'X', CS...>: public BaseInteger<16, CS...> {};

    }

    namespace Literals {

        template <char... CS> inline constexpr int128_t operator"" _s128() noexcept
            { return int128_t(PrionDetail::MakeInteger<CS...>::value); }
        template <char... CS> inline constexpr uint128_t operator"" _u128() noexcept
            { return PrionDetail::MakeInteger<CS...>::value; }
        inline constexpr float operator"" _degf(long double x) noexcept
            { return float(x * (pi_ld / 180.0L)); }
        inline constexpr float operator"" _degf(unsigned long long x) noexcept
            { return float(static_cast<long double>(x) * (pi_ld / 180.0L)); }
        inline constexpr double operator"" _deg(long double x) noexcept
            { return double(x * (pi_ld / 180.0L)); }
        inline constexpr double operator"" _deg(unsigned long long x) noexcept
            { return double(static_cast<long double>(x) * (pi_ld / 180.0L)); }
        inline constexpr long double operator"" _degl(long double x) noexcept
            { return x * (pi_ld / 180.0L); }
        inline constexpr long double operator"" _degl(unsigned long long x) noexcept
            { return static_cast<long double>(x) * (pi_ld / 180.0L); }

    }

    // Arithmetic functions

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
            std::pair<T, T> operator()(T lhs, T rhs) const noexcept {
                auto q = lhs / rhs, r = lhs % rhs;
                if (r < T(0)) {
                    q += rhs < T(0) ? T(1) : T(-1);
                    r += abs(rhs);
                }
                return {q, r};
            }
        };

        template <typename T>
        struct Divide<T, NumMode::unsigned_integer> {
            std::pair<T, T> operator()(T lhs, T rhs) const noexcept {
                return {lhs / rhs, lhs % rhs};
            }
        };

        template <typename T>
        struct Divide<T, NumMode::floating_point> {
            std::pair<T, T> operator()(T lhs, T rhs) const noexcept {
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

        template <typename T2, typename T1, bool FromFloat = std::is_floating_point<T1>::value> struct Round;

        template <typename T2, typename T1>
        struct Round<T2, T1, true> {
            T2 operator()(T1 value) const noexcept {
                using std::floor;
                return static_cast<T2>(floor(value + T1(1) / T1(2)));
            }
        };

        template <typename T2, typename T1>
        struct Round<T2, T1, false> {
            T2 operator()(T1 value) const noexcept {
                return static_cast<T2>(value);
            }
        };

    }

    template <typename T> constexpr T static_min(T t) noexcept { return t; }
    template <typename T, typename... Args> constexpr T static_min(T t, Args... args) noexcept
        { return t < static_min(args...) ? t : static_min(args...); }
    template <typename T> constexpr T static_max(T t) noexcept { return t; }
    template <typename T, typename... Args> constexpr T static_max(T t, Args... args) noexcept
        { return static_max(args...) < t ? t : static_max(args...); }
    template <typename T, typename T2, typename T3> constexpr T clamp(const T& x, const T2& min, const T3& max) noexcept
        { return x < static_cast<T>(min) ? static_cast<T>(min) : static_cast<T>(max) < x ? static_cast<T>(max) : x; }
    template <typename T> constexpr T degrees(T rad) noexcept { return rad * (T(180) / c_pi<T>()); }
    template <typename T> constexpr T radians(T deg) noexcept { return deg * (c_pi<T>() / T(180)); }
    template <typename T> std::pair<T, T> divide(T lhs, T rhs) noexcept { return PrionDetail::Divide<T>()(lhs, rhs); }
    template <typename T> T quo(T lhs, T rhs) noexcept { return PrionDetail::Divide<T>()(lhs, rhs).first; }
    template <typename T> T rem(T lhs, T rhs) noexcept { return PrionDetail::Divide<T>()(lhs, rhs).second; }
    template <typename T1, typename T2> constexpr T2 interpolate(T1 x1, T2 y1, T1 x2, T2 y2, T1 x)
        { return y1 == y2 ? y1 : y1 + (y2 - y1) * ((x - x1) / (x2 - x1)); }

    template <typename T> constexpr T rotl(T t, int n) noexcept { return (t << n) | (t >> (8 * sizeof(T) - n)); }
    template <typename T> constexpr T rotr(T t, int n) noexcept { return (t >> n) | (t << (8 * sizeof(T) - n)); }
    template <typename T2, typename T1> T2 round(T1 value) noexcept { return PrionDetail::Round<T2, T1>()(value); }
    template <typename T> constexpr int sign_of(T t) noexcept { return PrionDetail::SignOf<T>()(t); }

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
        if (sign_of(t) < 0)
            return 0;
        if (std::numeric_limits<T>::digits < std::numeric_limits<double>::digits)
            return static_cast<T>(floor(sqrt(double(t))));
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
        return static_cast<T>(result);
    }

    // Byte order

    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        constexpr bool big_endian_target = false;
        constexpr bool little_endian_target = true;
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        constexpr bool big_endian_target = true;
        constexpr bool little_endian_target = false;
    #else
        #error Unknown byte order
    #endif

    namespace PrionDetail {

        template <typename T, size_t N = sizeof(T)> struct ByteSwap {
            void operator()(T& t) const noexcept {
                auto ptr = reinterpret_cast<uint8_t*>(&t);
                std::reverse(ptr, ptr + N);
            }
        };

        template <typename T> struct ByteSwap<T, 1> {
            void operator()(T& /*t*/) const noexcept {}
        };

        template <typename T> struct ByteSwap<T, 2> {
            void operator()(T& t) const noexcept {
                auto p = reinterpret_cast<uint8_t*>(&t);
                auto b = p[0]; p[0] = p[1]; p[1] = b;
            }
        };

        template <typename T> struct ByteSwap<T, 4> {
            void operator()(T& t) const noexcept {
                auto p = reinterpret_cast<uint8_t*>(&t);
                auto a = p[0]; p[0] = p[3]; p[3] = a;
                auto b = p[1]; p[1] = p[2]; p[2] = b;
            }
        };

    }

    template <typename T>
    inline T big_endian(T t) noexcept {
        if (little_endian_target)
            PrionDetail::ByteSwap<T>()(t);
        return t;
    }

    template <typename T>
    inline T little_endian(T t) noexcept {
        if (big_endian_target)
            PrionDetail::ByteSwap<T>()(t);
        return t;
    }

    template <typename T>
    void read_be(T& t, const void* ptr, size_t ofs = 0) noexcept {
        memcpy(&t, static_cast<const uint8_t*>(ptr) + ofs, sizeof(t));
        t = big_endian(t);
    }

    template <typename T>
    void read_be(T& t, const void* ptr, size_t ofs, size_t len) noexcept {
        auto bp = static_cast<const uint8_t*>(ptr) + ofs;
        t = 0;
        for (; len > 0; --len)
            t = (t << 8) + static_cast<T>(*bp++);
    }

    template <typename T>
    T read_be(const void* ptr, size_t ofs = 0) noexcept {
        T t;
        read_be(t, ptr, ofs);
        return t;
    }

    template <typename T>
    T read_be(const void* ptr, size_t ofs, size_t len) noexcept {
        T t;
        read_be(t, ptr, ofs, len);
        return t;
    }

    template <typename T>
    void read_le(T& t, const void* ptr, size_t ofs = 0) noexcept {
        memcpy(&t, static_cast<const uint8_t*>(ptr) + ofs, sizeof(t));
        t = little_endian(t);
    }

    template <typename T>
    void read_le(T& t, const void* ptr, size_t ofs, size_t len) noexcept {
        auto bp = static_cast<const uint8_t*>(ptr) + ofs + len;
        t = 0;
        for (; len > 0; --len)
            t = (t << 8) + static_cast<T>(*--bp);
    }

    template <typename T>
    T read_le(const void* ptr, size_t ofs = 0) noexcept {
        T t;
        read_le(t, ptr, ofs);
        return t;
    }

    template <typename T>
    T read_le(const void* ptr, size_t ofs, size_t len) noexcept {
        T t;
        read_le(t, ptr, ofs, len);
        return t;
    }

    template <typename T>
    void write_be(T t, void* ptr, size_t ofs = 0) noexcept {
        t = big_endian(t);
        memcpy(static_cast<uint8_t*>(ptr) + ofs, &t, sizeof(T));
    }

    template <typename T>
    void write_be(T t, void* ptr, size_t ofs, size_t len) noexcept {
        auto bp = static_cast<uint8_t*>(ptr) + ofs + len;
        for (; len > 0; --len, t >>= 8)
            *--bp = uint8_t(t & 0xff);
    }

    template <typename T>
    void write_le(T t, void* ptr, size_t ofs = 0) noexcept {
        t = little_endian(t);
        memcpy(static_cast<uint8_t*>(ptr) + ofs, &t, sizeof(T));
    }

    template <typename T>
    void write_le(T t, void* ptr, size_t ofs, size_t len) noexcept {
        auto bp = static_cast<uint8_t*>(ptr) + ofs;
        for (; len > 0; --len, t >>= 8)
            *bp++ = uint8_t(t & 0xff);
    }

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
    template <typename T> constexpr T char_to(char c) noexcept { return static_cast<T>(static_cast<unsigned char>(c)); }

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
        using delete_function = std::function<void(T*)>;
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
        SimpleBuffer& operator=(const SimpleBuffer& sb) {
            SimpleBuffer temp(sb);
            swap(temp);
            return *this;
        }
        SimpleBuffer& operator=(SimpleBuffer&& sb) noexcept {
            clear();
            len = sb.len;
            ptr = sb.ptr;
            del = std::move(sb.del);
            sb.abandon();
            return *this;
        }
        T& operator[](size_t i) noexcept { return ptr[i]; }
        const T& operator[](size_t i) const noexcept { return ptr[i]; }
        void assign(size_t n) {
            if (n != len) {
                T* temp = new T[n];
                clear();
                len = n;
                ptr = temp;
            }
        }
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

    // Exceptions

    namespace PrionDetail {

        #if defined(PRI_TARGET_WINDOWS) || defined(PRI_TARGET_CYGWIN)

            class ForceUtf8 {
            public:
                ForceUtf8(): prev(setlocale(LC_CTYPE, "en_US.UTF-8")) {}
                ~ForceUtf8() { if (prev) setlocale(LC_CTYPE, prev); }
            private:
                const char* prev;
            };

        #else

            class ForceUtf8 {
            public:
                ForceUtf8(): prev(uselocale(nullptr)) { uselocale(newlocale(LC_CTYPE_MASK, "en_US.UTF-8", duplocale(prev))); }
                ~ForceUtf8() { uselocale(prev); }
            private:
                locale_t prev;
            };

        #endif

        inline string trim_ws(const string& s) {
            size_t i = s.find_first_not_of(ascii_whitespace);
            if (i == npos)
                return {};
            size_t j = s.find_last_not_of(ascii_whitespace);
            return s.substr(i, j + 1 - i);
        }

    }

    class SystemError:
    public std::runtime_error {
    public:
        int error() const noexcept { return err; }
        const char* function() const noexcept { return fun->data(); }
    protected:
        SystemError(int error, const u8string& function, const u8string& message): std::runtime_error(message), err(error), fun(std::make_shared<u8string>(function)) {}
        static u8string assemble(int error, const u8string& function, const u8string& details);
    private:
        int err;
        std::shared_ptr<u8string> fun;
    };

    inline u8string SystemError::assemble(int error, const u8string& function, const u8string& details) {
        u8string msg;
        if (! function.empty())
            msg += function;
        if (error != 0) {
            if (! msg.empty())
                msg += ": ";
            msg += "Error " + dec(error);
        }
        if (! details.empty()) {
            if (! msg.empty())
                msg += ": ";
            msg += details;
        }
        if (msg.empty())
            msg = "System call failed";
        return msg;
    }

    class CrtError:
    public SystemError {
    public:
        CrtError(int error, const char* function): SystemError(error, cstr(function), assemble(error, cstr(function), translate(error))) {}
        CrtError(int error, const u8string& function): SystemError(error, function, assemble(error, function, translate(error))) {}
        static u8string translate(int error) { PrionDetail::ForceUtf8 fu; return PrionDetail::trim_ws(cstr(strerror(error))); }
    };

    #if defined(PRI_TARGET_WINDOWS)

        namespace PrionDetail {

            class LocalBuffer {
            public:
                LocalBuffer(): ptr(nullptr) {}
                ~LocalBuffer() { LocalFree(ptr); }
                wchar_t* get() const { return static_cast<wchar_t*>(ptr); }
                wchar_t* indirect() { return reinterpret_cast<wchar_t*>(&ptr); }
            private:
                void* ptr;
            };

        }

        class WindowsError:
        public SystemError {
        public:
            WindowsError(uint32_t error, const char* function):
                SystemError(int(error), cstr(function), assemble(int(error), cstr(function), translate(error))) {}
            WindowsError(uint32_t error, const u8string& function):
                SystemError(int(error), function, assemble(int(error), function, translate(error))) {}
            static u8string translate(uint32_t error);
        };

        inline u8string WindowsError::translate(uint32_t error) {
            using namespace PrionDetail;
            static constexpr uint32_t flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
            LocalBuffer buf;
            auto rc = FormatMessageW(flags, nullptr, error, 0, buf.indirect(), 0, nullptr);
            return trim_ws(wstring_to_utf8(wstring(buf.get(), rc)));
        }

    #endif

    // Flag sets

    class Flagset:
    public EqualityComparable<Flagset> {
    public:
        static constexpr char cvalue(uint64_t flag) noexcept {
            return __builtin_popcountll(flag) != 1 ? '\0'
                : flag < (1ull << 26) ? 'A' + __builtin_ctzll(flag)
                : flag < (1ull << 52) ? 'a' + __builtin_ctzll(flag) - 26
                : flag == (1ull << 52) ? '#'
                : flag == (1ull << 53) ? '$'
                : flag == (1ull << 54) ? '%'
                : flag == (1ull << 55) ? '&'
                : flag == (1ull << 56) ? '*'
                : flag == (1ull << 57) ? '+'
                : flag == (1ull << 58) ? '-'
                : flag == (1ull << 59) ? '/'
                : flag == (1ull << 60) ? '<'
                : flag == (1ull << 61) ? '='
                : flag == (1ull << 62) ? '>'
                : flag == (1ull << 63) ? '@'
                : '\0';
        }
        template <typename C> static constexpr uint64_t value(C flag) noexcept {
            return (flag >= C('A') && flag <= C('Z')) ? 1ull << (flag - 65)
                : (flag >= C('a') && flag <= C('z')) ? 1ull << (flag - 71)
                : (flag == C('#')) ? 1ull << 52
                : (flag == C('$')) ? 1ull << 53
                : (flag == C('%')) ? 1ull << 54
                : (flag == C('&')) ? 1ull << 55
                : (flag == C('*')) ? 1ull << 56
                : (flag == C('+')) ? 1ull << 57
                : (flag == C('-')) ? 1ull << 58
                : (flag == C('/')) ? 1ull << 59
                : (flag == C('<')) ? 1ull << 60
                : (flag == C('=')) ? 1ull << 61
                : (flag == C('>')) ? 1ull << 62
                : (flag == C('@')) ? 1ull << 63
                : 0;
        }
        constexpr Flagset() noexcept: bits(0) {}
        constexpr Flagset(std::nullptr_t) noexcept: bits(0) {}
        constexpr Flagset(bool flags) noexcept: bits(flags) {}
        constexpr Flagset(char flags) noexcept: bits(flags) {}
        constexpr Flagset(signed char flags) noexcept: bits(flags) {}
        constexpr Flagset(unsigned char flags) noexcept: bits(flags) {}
        constexpr Flagset(short flags) noexcept: bits(flags) {}
        constexpr Flagset(unsigned short flags) noexcept: bits(flags) {}
        constexpr Flagset(int flags) noexcept: bits(flags) {}
        constexpr Flagset(unsigned flags) noexcept: bits(flags) {}
        constexpr Flagset(long flags) noexcept: bits(flags) {}
        constexpr Flagset(unsigned long flags) noexcept: bits(flags) {}
        constexpr Flagset(long long flags) noexcept: bits(flags) {}
        constexpr Flagset(unsigned long long flags) noexcept: bits(flags) {}
        template <typename C> Flagset(const C* flags) noexcept: bits(0) { for (; flags && *flags; ++flags) setc(*flags); }
        template <typename C> Flagset(const basic_string<C>& flags) noexcept: bits(0) { for (auto c: flags) setc(c); }
        void allow(Flagset allowed, const char* domain) const;
        constexpr bool empty() const noexcept { return bits == 0; }
        void exclusive(Flagset xgroup, const char* domain) const;
        constexpr uint64_t get() const noexcept { return bits; }
        constexpr bool get(uint64_t flags) const noexcept { return bits & flags; }
        template <typename C> constexpr bool getc(C flag) const noexcept { return get(value(flag)); }
        template <typename C> bool gets(const C* flags) const noexcept
            { for (; flags && *flags; ++flags) if (getc(*flags)) return true; return false; }
        template <typename C> bool gets(const basic_string<C>& flags) const noexcept
            { for (auto c: flags) if (getc(c)) return true; return false; }
        void set(uint64_t flags, bool state = true) noexcept { if (state) bits |= flags; else bits &= ~ flags; }
        template <typename C> void setc(C flag, bool state = true) noexcept { set(value(flag), state); }
        u8string str() const {
            static const u8string punct = "#$%&*+-/<=>@";
            if (bits == 0)
                return "0";
            u8string s;
            uint64_t mask = 1;
            for (char c = 'A'; c <= 'Z'; ++c, mask <<= 1)
                if (bits & mask)
                    s += c;
            for (char c = 'a'; c <= 'z'; ++c, mask <<= 1)
                if (bits & mask)
                    s += c;
            for (char c: punct) {
                if (bits & mask)
                    s += c;
                mask <<= 1;
            }
            return s;
        }
        template <typename C> basic_string<C> ustr() const { auto s = str(); return basic_string<C>(s.begin(), s.end()); }
        constexpr Flagset operator~() const noexcept { return {~ bits}; }
        Flagset& operator&=(Flagset rhs) noexcept { bits &= rhs.bits; return *this; }
        Flagset& operator|=(Flagset rhs) noexcept { bits |= rhs.bits; return *this; }
        Flagset& operator^=(Flagset rhs) noexcept { bits ^= rhs.bits; return *this; }
        friend constexpr Flagset operator&(Flagset lhs, Flagset rhs) noexcept { return {lhs.bits & rhs.bits}; }
        friend constexpr Flagset operator|(Flagset lhs, Flagset rhs) noexcept { return {lhs.bits | rhs.bits}; }
        friend constexpr Flagset operator^(Flagset lhs, Flagset rhs) noexcept { return {lhs.bits ^ rhs.bits}; }
        friend constexpr bool operator==(Flagset lhs, Flagset rhs) noexcept { return lhs.bits == rhs.bits; }
        friend std::ostream& operator<<(std::ostream& out, Flagset f) { return out << f.str(); }
    private:
        uint64_t bits;
    };

    class FlagError:
    public std::invalid_argument {
    public:
        FlagError(Flagset flags, const char* domain): std::invalid_argument(assemble(flags.str(), domain)), fs(flags) {}
        FlagError(const u8string& flags, const char* domain): std::invalid_argument(assemble(flags, domain)), fs(flags) {}
        FlagError(const char* flags, const char* domain): std::invalid_argument(assemble(flags, domain)), fs(flags) {}
        Flagset flags() const noexcept { return fs; }
    private:
        Flagset fs;
        static u8string assemble(const u8string& flags, const char* domain) {
            u8string s = "Invalid ";
            if (domain && *domain) s += u8string(domain) + " ";
            s += "flags: \"" + flags + "\"";
            return s;
        }
    };

    inline void Flagset::allow(Flagset allowed, const char* domain) const
        { if (bits & ~ allowed.bits) throw FlagError(*this, domain); }
    inline void Flagset::exclusive(Flagset xgroup, const char* domain) const
        { if (__builtin_popcountll(bits & xgroup.bits) > 1) throw FlagError(*this, domain); }

    // Functional utilities

    // stdfun() is based on an idea by Victor Laskin
    // http://vitiy.info/c11-functional-decomposition-easy-way-to-do-aop/

    namespace PrionDetail {

        template <typename F> struct FunctionTraits:
            FunctionTraits<decltype(&F::operator())> {};
        template <typename T, typename RT, typename... Args> struct FunctionTraits<RT(T::*)(Args...) const>
            { using function = std::function<RT(Args...)>; };

    }

    template <typename F> typename PrionDetail::FunctionTraits<F>::function stdfun(F& lambda)
        { return static_cast<typename PrionDetail::FunctionTraits<F>::function>(lambda); }

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

    template <typename Range>
    void hash_range(size_t& hash, const Range& range) noexcept {
        for (auto&& x: range)
            hash_combine(hash, x);
    }

    template <typename Range>
    size_t hash_range(const Range& range) noexcept {
        size_t hash(0);
        hash_range(hash, range);
        return hash;
    }

    template <typename... Args>
    size_t hash_value(const Args&... args) noexcept {
        size_t hash(0);
        hash_combine(hash, args...);
        return hash;
    }

    // Keyword arguments

    namespace PrionDetail {

        template <typename K, typename V, bool = std::is_convertible<K, V>::value> struct Kwcopy;
        template <typename K, typename V> struct Kwcopy<K, V, true> { void operator()(const K& a, V& p) const { p = static_cast<V>(a); } };
        template <typename K, typename V> struct Kwcopy<K, V, false> { void operator()(const K&, V&) const {} };

        template <typename K> struct Kwparam {
            const void* key;
            K val;
        };

    }

    template <typename K> struct Kwarg {
        constexpr Kwarg() {}
        template <typename A> PrionDetail::Kwparam<K> operator=(const A& a) const { return {this, static_cast<K>(a)}; }
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

    // Range utilities

    namespace PrionDetail {

        template <typename T> struct ArrayCount;
        template <typename T, size_t N> struct ArrayCount<T[N]> { static constexpr size_t value = N; };

    }

    template <typename Range> using RangeIterator = decltype(std::begin(std::declval<Range&>()));
    template <typename Range> using RangeValue = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;

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

    template <typename Iterator>
    struct Irange {
        Iterator first, second;
        constexpr Iterator begin() const { return first; }
        constexpr Iterator end() const { return second; }
    };

    template <typename Iterator> constexpr Irange<Iterator> irange(const Iterator& i, const Iterator& j) { return {i, j}; }
    template <typename Iterator> constexpr Irange<Iterator> irange(const std::pair<Iterator, Iterator>& p) { return {p.first, p.second}; }
    template <typename T> constexpr Irange<T*> array_range(T* ptr, size_t len) { return {ptr, ptr + len}; }
    template <typename T> constexpr size_t array_count(T&&) { return PrionDetail::ArrayCount<std::remove_reference_t<T>>::value; }
    template <typename Range> size_t range_count(const Range& r) { return std::distance(PRI_BOUNDS(r)); }
    template <typename Range> bool range_empty(const Range& r) { return std::begin(r) == std::end(r); }

    namespace PrionDetail {

        template <typename T>
        class UnitSequenceIterator:
        public ForwardIterator<UnitSequenceIterator<T>, const T> {
        public:
            UnitSequenceIterator(): current(), target(), ok(false) {}
            UnitSequenceIterator(const T& t1, const T& t2): current(t1), target(t2), ok(t1 < t2) {}
            const T& operator*() const { return current; }
            UnitSequenceIterator& operator++() {
                if (ok) {
                    ++current;
                    ok = current < target;
                }
                return *this;
            }
            bool operator==(const UnitSequenceIterator& rhs) const { return ok == rhs.ok && (! ok || current == rhs.current); }
        private:
            T current;
            T target;
            bool ok;
        };

        template <typename T>
        class DeltaSequenceIterator:
        public ForwardIterator<DeltaSequenceIterator<T>, const T> {
        public:
            DeltaSequenceIterator(): current(), target(), delta(), sign(0) {}
            DeltaSequenceIterator(const T& t1, const T& t2, const T& dt):
                current(t1), target(t2), delta(dt), sign(0) {
                    if (t1 < t2 && T() < dt)
                        sign = 1;
                    else if (t2 < t1 && dt < T())
                        sign = -1;
                }
            const T& operator*() const { return current; }
            DeltaSequenceIterator& operator++() {
                if (sign != 0) {
                    current += delta;
                    if ((sign == 1 && ! (current < target))
                            || (sign == -1 && ! (target < current)))
                        sign = 0;
                }
                return *this;
            }
            bool operator==(const DeltaSequenceIterator& rhs) const { return sign == rhs.sign && (sign == 0 || current == rhs.current); }
        private:
            T current;
            T target;
            T delta;
            int sign;
        };

    }

    template <typename T> inline Irange<PrionDetail::UnitSequenceIterator<T>> seq(T t2) { return {{T(), t2}, {}}; }
    template <typename T> inline Irange<PrionDetail::UnitSequenceIterator<T>> seq(T t1, T t2) { return {{t1, t2}, {}}; }
    template <typename T> inline Irange<PrionDetail::DeltaSequenceIterator<T>> seq(T t1, T t2, T delta) { return {{t1, t2, delta}, {}}; }

    // Scope guards

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
        private:
            std::function<void()> func;
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

    class Transaction {
    public:
        using callback = std::function<void()>;
        Transaction() noexcept {}
        ~Transaction() noexcept { rollback(); }
        void call(callback func, callback undo) {
            stack.push_back(nullptr);
            if (func)
                func();
            stack.back().swap(undo);
        }
        void commit() noexcept { stack.clear(); }
        void rollback() noexcept {
            for (auto i = stack.rbegin(); i != stack.rend(); ++i) {
                if (*i) {
                    try { (*i)(); }
                    catch (...) {}
                }
            }
            stack.clear();
        }
    private:
        std::vector<callback> stack;
        Transaction(const Transaction&) = delete;
        Transaction(Transaction&&) = delete;
        Transaction& operator=(const Transaction&) = delete;
        Transaction& operator=(Transaction&&) = delete;
    };

    // String functions

    inline string ascii_lowercase(const string& str) {
        auto result = str;
        std::transform(PRI_BOUNDS(result), std::begin(result), ascii_tolower);
        return result;
    }

    inline string ascii_uppercase(const string& str) {
        auto result = str;
        std::transform(PRI_BOUNDS(result), std::begin(result), ascii_toupper);
        return result;
    }

    inline string ascii_titlecase(const string& str) {
        auto result = str;
        bool was_alpha = false;
        for (char& c: result) {
            c = was_alpha ? ascii_tolower(c) : ascii_toupper(c);
            was_alpha = ascii_isalpha(c);
        }
        return result;
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

    inline long long decnum(const string& str) noexcept { return strtoll(str.data(), nullptr, 10); }
    inline unsigned long long hexnum(const string& str) noexcept { return strtoull(str.data(), nullptr, 16); }
    inline double fpnum(const string& str) noexcept { return strtod(str.data(), nullptr); }

    namespace PrionDetail {

        inline void append_hex_byte(uint8_t b, string& s) {
            static constexpr const char* digits = "0123456789abcdef";
            s += digits[b / 16];
            s += digits[b % 16];
        }

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

    template <typename InputIterator>
    string join_words(InputIterator begin, InputIterator end, const string& delim = " ") {
        string result;
        for (; begin != end; ++begin) {
            result += *begin;
            result += delim;
        }
        if (! result.empty() && ! delim.empty())
            result.resize(result.size() - delim.size());
        return result;
    }

    template <typename OutputIterator>
    void split_words(const string& src, OutputIterator dst, const string& delim = ascii_whitespace) {
        if (delim.empty()) {
            if (! src.empty())
                *dst = src;
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
                for (auto& v: r) {
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
            string operator()(const R& r) const {
                string s = "{";
                for (auto& kv: r) {
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

        template <typename T, bool = std::is_integral<T>::value,
            bool = IsRangeType<T>::value>
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

    }

    template <typename T> string to_str(const T& t) { return PrionDetail::ObjectToString<T>()(t); }

    // Threads

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
        explicit MutexLock(Mutex& m) noexcept: mx(&m) { mx->lock(); }
        ~MutexLock() noexcept { mx->unlock(); }
    private:
        friend class ConditionVariable;
        Mutex* mx;
        PRI_NO_COPY_MOVE(MutexLock)
    };

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
                    throw CrtError(rc, "pthread_cond_init()");
            }
            ~ConditionVariable() noexcept { pthread_cond_destroy(&pcond); }
            void notify_all() noexcept { pthread_cond_broadcast(&pcond); }
            void notify_one() noexcept { pthread_cond_signal(&pcond); }
            void wait(MutexLock& ml) {
                int rc = pthread_cond_wait(&pcond, &ml.mx->pmutex);
                if (rc)
                    throw CrtError(rc, "pthread_cond_wait()");
            }
            template <typename Pred> void wait(MutexLock& lock, Pred p) { while (! p()) wait(lock); }
            template <typename R, typename P, typename Pred> bool wait_for(MutexLock& lock, std::chrono::duration<R, P> t, Pred p) {
                using namespace std::chrono;
                return wait_impl(lock, duration_cast<nanoseconds>(t), predicate(p));
            }
        private:
            using predicate = std::function<bool()>;
            pthread_cond_t pcond;
            PRI_NO_COPY_MOVE(ConditionVariable)
            bool wait_impl(MutexLock& ml, std::chrono::nanoseconds ns, predicate p) {
                using namespace std::chrono;
                if (p())
                    return true;
                auto nsc = ns.count();
                if (nsc <= 0)
                    return false;
                timespec ts;
                #if defined(PRI_TARGET_DARWIN)
                    timeval tv;
                    gettimeofday(&tv, nullptr);
                    ts = {tv.tv_sec, 1000 * tv.tv_usec};
                #else
                    clock_gettime(CLOCK_REALTIME, &ts);
                #endif
                static constexpr long billion = 1000000000L;
                ts.tv_nsec += nsc % billion;
                ts.tv_sec += nsc / billion + ts.tv_nsec / billion;
                ts.tv_nsec %= billion;
                for (;;) {
                    int rc = pthread_cond_timedwait(&pcond, &ml.mx->pmutex, &ts);
                    if (rc == ETIMEDOUT)
                        return p();
                    if (rc != 0)
                        throw CrtError(rc, "pthread_cond_timedwait()");
                    if (p())
                        return true;
                }
            }
        };

        class Thread {
        public:
            using callback = std::function<void()>;
            using id_type = pthread_t;
            Thread() noexcept: except(), payload(), state(thread_joined), thread() {}
            explicit Thread(callback f): Thread() {
                if (f) {
                    payload = f;
                    state = thread_running;
                    int rc = pthread_create(&thread, nullptr, thread_callback, this);
                    if (rc)
                        throw CrtError(rc, "pthread_create()");
                }
            }
            ~Thread() noexcept { if (state != thread_joined) pthread_join(thread, nullptr); }
            Thread(Thread&&) = default;
            Thread& operator=(Thread&&) = default;
            id_type get_id() const noexcept { return thread; }
            bool poll() noexcept { return state != thread_running; }
            void wait() {
                if (state == thread_joined)
                    return;
                int rc = pthread_join(thread, nullptr);
                state = thread_joined;
                if (rc)
                    throw CrtError(rc, "pthread_join()");
                if (except)
                    std::rethrow_exception(except);
            }
            static size_t cpu_threads() noexcept {
                size_t n = 0;
                #if defined(PRI_TARGET_DARWIN)
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
            std::exception_ptr except;
            Thread::callback payload;
            std::atomic<int> state;
            pthread_t thread;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            static void* thread_callback(void* ptr) noexcept {
                auto t = static_cast<Thread*>(ptr);
                if (t->payload) {
                    try { t->payload(); }
                    catch (...) { t->except = std::current_exception(); }
                }
                t->state = thread_complete;
                return nullptr;
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
                    throw WindowsError(GetLastError(), "SleepConditionVariableCS()");
            }
            template <typename Pred> void wait(MutexLock& lock, Pred p) { while (! p()) wait(lock); }
            template <typename R, typename P, typename Pred> bool wait_for(MutexLock& lock, std::chrono::duration<R, P> t, Pred p) {
                using namespace std::chrono;
                return wait_impl(lock, duration_cast<nanoseconds>(t), predicate(p));
            }
        private:
            using predicate = std::function<bool()>;
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
                            throw WindowsError(status, "SleepConditionVariableCS()");
                    }
                }
            }
        };

        class Thread {
        public:
            using callback = std::function<void()>;
            using id_type = uint32_t;
            Thread() noexcept: except(), payload(), state(thread_joined), key(0), thread(nullptr) {}
            explicit Thread(callback f): Thread() {
                if (f) {
                    payload = f;
                    state = thread_running;
                    thread = CreateThread(nullptr, 0, thread_callback, this, 0, &key);
                    if (! thread)
                        throw WindowsError(GetLastError(), "CreateThread()");
                }
            }
            ~Thread() noexcept {
                if (state != thread_joined)
                    WaitForSingleObject(thread, INFINITE);
                CloseHandle(thread);
            }
            Thread(Thread&&) = default;
            Thread& operator=(Thread&&) = default;
            id_type get_id() const noexcept { return key; }
            bool poll() noexcept { return state != thread_running; }
            void wait() {
                if (state == thread_joined)
                    return;
                auto rc = WaitForSingleObject(thread, INFINITE);
                state = thread_joined;
                if (rc == WAIT_FAILED)
                    throw WindowsError(GetLastError(), "WaitForSingleObject()");
                if (except)
                    std::rethrow_exception(except);
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
            std::exception_ptr except;
            Thread::callback payload;
            std::atomic<int> state;
            DWORD key;
            HANDLE thread;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            static DWORD WINAPI thread_callback(void* ptr) noexcept {
                auto t = static_cast<Thread*>(ptr);
                if (t->payload) {
                    try { t->payload(); }
                    catch (...) { t->except = std::current_exception(); }
                }
                t->state = thread_complete;
                return 0;
            }
        };

    #endif

    // Time and date functions

    namespace PrionDetail {

        using SleepUnit =
            #if defined(PRI_TARGET_UNIX)
                std::chrono::microseconds;
            #else
                std::chrono::milliseconds;
            #endif

        inline u8string format_time(int64_t sec, double frac, int prec) {
            u8string result;
            if (sec < 0 || frac < 0)
                result += '-';
            sec = abs(sec);
            int y = sec / 31557600;
            sec -= 31557600 * y;
            int d = sec / 86400;
            sec -= 86400 * d;
            int h = sec / 3600;
            sec -= 3600 * h;
            int m = sec / 60;
            sec -= 60 * m;
            int rc, s = sec;
            std::vector<char> buf(64);
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

        inline void sleep_for(SleepUnit t) noexcept {
            #if defined(PRI_TARGET_UNIX)
                auto usec = t.count();
                if (usec > 0) {
                    timeval tv;
                    tv.tv_sec = usec / 1000000;
                    tv.tv_usec = usec % 1000000;
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
    double to_seconds(const std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        return duration_cast<duration<double>>(d).count();
    }

    template <typename R, typename P>
    void from_seconds(double s, std::chrono::duration<R, P>& d) noexcept {
        using namespace std::chrono;
        d = duration_cast<duration<R, P>>(duration<double>(s));
    }

    enum ZoneFlag {
        utc_date,
        local_date
    };

    // Unfortunately strftime() doesn't set errno and simply returns zero on
    // any error. This means that there is no way to distinguish between an
    // invalid format string, an output buffer that is too small, and a
    // legitimately empty result. Here we try first with a reasonable output
    // length, and if that fails, try again with a much larger one; if it
    // still fails, give up. This could theoretically fail in the face of a
    // very long localized date format, but there doesn't seem to be a better
    // solution.

    inline u8string format_date(std::chrono::system_clock::time_point tp, const u8string& format, ZoneFlag z = utc_date) {
        using namespace std::chrono;
        auto t = system_clock::to_time_t(tp);
        tm stm = z == local_date ? *localtime(&t) : *gmtime(&t);
        u8string result(std::max(2 * format.size(), size_t(100)), '\0');
        auto rc = strftime(&result[0], result.size(), format.data(), &stm);
        if (rc == 0) {
            result.resize(10 * result.size(), '\0');
            rc = strftime(&result[0], result.size(), format.data(), &stm);
        }
        result.resize(rc);
        return result;
    }

    inline u8string format_date(std::chrono::system_clock::time_point tp, int prec = 0, ZoneFlag z = utc_date) {
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

    inline std::chrono::system_clock::time_point make_date(int year, int month, int day,
            int hour, int min, double sec, ZoneFlag z = utc_date) noexcept {
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
        if (z == local_date)
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
    u8string format_time(const std::chrono::duration<R, P>& time, int prec = 0) {
        using namespace std::chrono;
        auto whole = duration_cast<seconds>(time);
        auto frac = time - duration_cast<duration<R, P>>(whole);
        return PrionDetail::format_time(whole.count(), duration_cast<duration<double>>(frac).count(), prec);
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

    #if defined(PRI_TARGET_UNIX)

        template <typename R, typename P>
        timespec duration_to_timespec(const std::chrono::duration<R, P>& d) noexcept {
            using namespace std::chrono;
            static constexpr int64_t billion = 1000000000ll;
            int64_t nsec = duration_cast<nanoseconds>(d).count();
            return {time_t(nsec / billion), long(nsec % billion)};
        }

        template <typename R, typename P>
        timeval duration_to_timeval(const std::chrono::duration<R, P>& d) noexcept {
            using namespace std::chrono;
            static constexpr int64_t million = 1000000ll;
            int64_t usec = duration_cast<microseconds>(d).count();
            return {time_t(usec / million), suseconds_t(usec % million)};
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

    #endif

    #if defined(PRI_TARGET_WINDOWS)

        inline std::chrono::system_clock::time_point filetime_to_timepoint(const FILETIME& ft) noexcept {
            using namespace std::chrono;
            static constexpr int64_t filetime_freq = 10000000ll;     // FILETIME ticks (100 ns) per second
            static constexpr int64_t windows_epoch = 11644473600ll;  // Windows epoch (1601) to Unix epoch (1970)
            int64_t ticks = (int64_t(ft.dwHighDateTime) << 32) + int64_t(ft.dwLowDateTime);
            int64_t sec = ticks / filetime_freq - windows_epoch;
            int64_t nsec = 100ll * (ticks % filetime_freq);
            return system_clock::from_time_t(time_t(sec)) + duration_cast<system_clock::duration>(nanoseconds(nsec));
        }

        inline void timepoint_to_filetime(const std::chrono::system_clock::time_point& tp, FILETIME& ft) noexcept {
            using namespace std::chrono;
            auto unix_time = tp - system_clock::from_time_t(0);
            uint64_t nsec = duration_cast<nanoseconds>(unix_time).count();
            uint64_t ticks = nsec / 100ll;
            ft = {uint32_t(ticks), uint32_t(ticks >> 32)};
        }

    #endif

    // Type properties

    template <typename T1, typename T2> using CopyConst =
        std::conditional_t<std::is_const<T1>::value, std::add_const_t<T2>, std::remove_const_t<T2>>;

    template <typename T2, typename T1> inline T2 implicit_cast(const T1& t) { return t; }

    inline string demangle(const string& name) {
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

    inline string type_name(const std::type_info& t) { return demangle(t.name()); }
    template <typename T> string type_name() { return type_name(typeid(T)); }
    template <typename T> string type_name(const T&) { return type_name(typeid(T)); }

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
        explicit Uuid(uint128_t u) noexcept { write_be(u, bytes); }
        explicit Uuid(const uint8_t* ptr) noexcept { if (ptr) memcpy(bytes, ptr, 16); else memset(bytes, 0, 16); }
        explicit Uuid(const string& s);
        uint8_t& operator[](size_t i) noexcept { return bytes[i]; }
        const uint8_t& operator[](size_t i) const noexcept { return bytes[i]; }
        uint8_t* begin() noexcept { return bytes; }
        const uint8_t* begin() const noexcept { return bytes; }
        uint8_t* end() noexcept { return bytes + 16; }
        const uint8_t* end() const noexcept { return bytes + 16; }
        uint128_t as_integer() const noexcept { return read_be<uint128_t>(bytes); }
        u8string str() const;
        friend bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept { return memcmp(lhs.bytes, rhs.bytes, 16) == 0; }
        friend bool operator<(const Uuid& lhs, const Uuid& rhs) noexcept { return memcmp(lhs.bytes, rhs.bytes, 16) == -1; }
    private:
        friend struct RandomUuid;
        union {
            uint8_t bytes[16];
            uint32_t words[4];
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

    // Version number

    struct Version:
    public LessThanComparable<Version> {
        unsigned major;
        unsigned minor;
        unsigned patch;
        Version(unsigned x = 0, unsigned y = 0, unsigned z = 0): major(x), minor(y), patch(z) {}
        unsigned& operator[](size_t i) noexcept { return begin()[i]; }
        const unsigned& operator[](size_t i) const noexcept { return begin()[i]; }
        unsigned* begin() noexcept { return &major; }
        const unsigned* begin() const noexcept { return &major; }
        unsigned* end() noexcept { return begin() + 3; }
        const unsigned* end() const noexcept { return begin() + 3; }
        u8string str() const { return dec(major) + '.' + dec(minor) + '.' + dec(patch); }
        static Version from(int n, int scale = 10) noexcept;
        static Version parse(const u8string& s);
    };

    inline Version Version::from(int n, int scale) noexcept {
        Version v{0, 0, 0};
        if (scale < 1)
            return v;
        v.patch = n % scale;
        n /= scale;
        v.minor = n % scale;
        n /= scale;
        v.major = n;
        return v;
    }

    inline Version Version::parse(const u8string& s) {
        Version v{0, 0, 0};
        if (s.empty())
            return v;
        auto i = std::begin(s), e = std::end(s);
        unsigned n = 0;
        while (n < 3) {
            auto j = std::find_if_not(i, e, ascii_isdigit);
            if (j == i)
                break;
            v[n++] = unsigned(decnum(string(i, j)));
            if (j == e || *j != '.')
                break;
            i = j + 1;
        }
        if (n == 0)
            throw std::invalid_argument("Invalid version: " + s);
        return v;
    }

    inline bool operator==(const Version& lhs, const Version& rhs) noexcept {
        return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
    }

    inline bool operator<(const Version& lhs, const Version& rhs) noexcept {
        if (lhs.major != rhs.major)
            return lhs.major < rhs.major;
        else if (lhs.minor != rhs.minor)
            return lhs.minor < rhs.minor;
        else
            return lhs.patch < rhs.patch;
    }

    inline std::ostream& operator<<(std::ostream& o, const Version& v) { return o << v.str(); }
    inline u8string to_str(const Version& v) { return v.str(); }

    // I/O utilities

    // (These are at the end of the file because of internal dependencies)

    inline bool is_stdout_redirected() noexcept { return ! isatty(1); }

    namespace PrionDetail {

        #if defined(PRI_TARGET_NATIVE_WINDOWS)
            extern "C" int _isatty(int fd);
        #endif

        inline bool load_file_helper(FILE* fp, string& dst) {
            static constexpr size_t bufsize = 65536;
            size_t offset = 0;
            while (! (feof(fp) || ferror(fp))) {
                dst.resize(offset + bufsize, 0);
                offset += fread(&dst[0] + offset, 1, bufsize, fp);
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

        inline bool load_file(const string& file, string& dst) {
            dst.clear();
            FILE* fp = fopen(file.data(), "rb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::load_file_helper(fp, dst);
        }

        inline bool save_file(const string& file, const void* ptr, size_t n, bool append = false) {
            auto fp = fopen(file.data(), append ? "ab" : "wb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::save_file_helper(fp, ptr, n);
        }

    #else

        inline bool load_file(const wstring& file, string& dst) {
            dst.clear();
            FILE* fp = _wfopen(file.data(), L"rb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::load_file_helper(fp, dst);
        }

        inline bool save_file(const wstring& file, const void* ptr, size_t n, bool append = false) {
            auto fp = _wfopen(file.data(), append ? L"ab" : L"wb");
            if (! fp)
                return false;
            ScopeExit guard([fp] { fclose(fp); });
            return PrionDetail::save_file_helper(fp, ptr, n);
        }

        inline bool load_file(const string& file, string& dst) { return load_file(utf8_to_wstring(file), dst); }
        inline bool save_file(const string& file, const void* ptr, size_t n, bool append)
            { return save_file(utf8_to_wstring(file), ptr, n, append); }
        inline bool save_file(const wstring& file, const string& src, bool append = false)
            { return save_file(file, src.data(), src.size(), append); }

    #endif

    inline bool save_file(const string& file, const string& src, bool append = false) { return save_file(file, src.data(), src.size(), append); }

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

    static constexpr const char* xt_up           = "\e[A";    // Cursor up
    static constexpr const char* xt_down         = "\e[B";    // Cursor down
    static constexpr const char* xt_right        = "\e[C";    // Cursor right
    static constexpr const char* xt_left         = "\e[D";    // Cursor left
    static constexpr const char* xt_erase_left   = "\e[1K";   // Erase left
    static constexpr const char* xt_erase_right  = "\e[K";    // Erase right
    static constexpr const char* xt_erase_above  = "\e[1J";   // Erase above
    static constexpr const char* xt_erase_below  = "\e[J";    // Erase below
    static constexpr const char* xt_erase_line   = "\e[2K";   // Erase line
    static constexpr const char* xt_clear        = "\e[2J";   // Clear screen
    static constexpr const char* xt_reset        = "\e[0m";   // Reset attributes
    static constexpr const char* xt_bold         = "\e[1m";   // Bold
    static constexpr const char* xt_under        = "\e[4m";   // Underline
    static constexpr const char* xt_black        = "\e[30m";  // Black fg
    static constexpr const char* xt_red          = "\e[31m";  // Red fg
    static constexpr const char* xt_green        = "\e[32m";  // Green fg
    static constexpr const char* xt_yellow       = "\e[33m";  // Yellow fg
    static constexpr const char* xt_blue         = "\e[34m";  // Blue fg
    static constexpr const char* xt_magenta      = "\e[35m";  // Magenta fg
    static constexpr const char* xt_cyan         = "\e[36m";  // Cyan fg
    static constexpr const char* xt_white        = "\e[37m";  // White fg
    static constexpr const char* xt_black_bg     = "\e[40m";  // Black bg
    static constexpr const char* xt_red_bg       = "\e[41m";  // Red bg
    static constexpr const char* xt_green_bg     = "\e[42m";  // Green bg
    static constexpr const char* xt_yellow_bg    = "\e[43m";  // Yellow bg
    static constexpr const char* xt_blue_bg      = "\e[44m";  // Blue bg
    static constexpr const char* xt_magenta_bg   = "\e[45m";  // Magenta bg
    static constexpr const char* xt_cyan_bg      = "\e[46m";  // Cyan bg
    static constexpr const char* xt_white_bg     = "\e[47m";  // White bg

    inline string xt_move_up(int n) { return "\x1b[" + dec(n) + 'A'; }                                // Cursor up n spaces
    inline string xt_move_down(int n) { return "\x1b[" + dec(n) + 'B'; }                              // Cursor down n spaces
    inline string xt_move_right(int n) { return "\x1b[" + dec(n) + 'C'; }                             // Cursor right n spaces
    inline string xt_move_left(int n) { return "\x1b[" + dec(n) + 'D'; }                              // Cursor left n spaces
    inline string xt_colour(int rgb) { return "\x1b[38;5;" + dec(PrionDetail::rgb(rgb)) + 'm'; }      // Set fg colour to an RGB value (0-5)
    inline string xt_colour_bg(int rgb) { return "\x1b[48;5;" + dec(PrionDetail::rgb(rgb)) + 'm'; }   // Set bg colour to an RGB value (0-5)
    inline string xt_grey(int grey) { return "\x1b[38;5;" + dec(PrionDetail::grey(grey)) + 'm'; }     // Set fg colour to a grey level (1-24)
    inline string xt_grey_bg(int grey) { return "\x1b[48;5;" + dec(PrionDetail::grey(grey)) + 'm'; }  // Set bg colour to a grey level (1-24)

}
