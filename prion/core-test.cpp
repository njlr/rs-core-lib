#include "prion/core.hpp"
#include "prion/unit-test.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <random>
#include <ratio>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(PRI_TARGET_UNIX)
    #include <sys/time.h>
#endif

#if defined(PRI_TARGET_WINDOWS)
    #include <windows.h>
#endif

using namespace std::chrono;
using namespace std::literals;
using namespace Prion;
using namespace Prion::Literals;

TEST_MAIN;

// These tests are a bit incestuous since the unit test module itself uses
// some of the facilities in the core module. The shared functions are mostly
// simple enough that it should be clear where the error lies if anything goes
// wrong, and in any case the alternative of duplicating large parts of the
// core module in the unit test module would not be an improvement.

namespace {

    u8string f1(int n) { return u8string(n, '*'); }
    u8string f1(u8string s) { return '[' + s + ']'; }
    u8string f2() { return "Hello"; }
    u8string f2(u8string s) { return '[' + s + ']'; }
    u8string f2(int x, int y) { return dec(x * y); }

    void check_macros() {

        std::vector<int> iv = {1, 2, 3, 4, 5}, iv2 = {2, 3, 5, 7, 11};
        std::vector<u8string> result, sv = {"Neddie", "Eccles", "Bluebottle"};

        TRY(std::transform(PRI_BOUNDS(iv), overwrite(result), PRI_OVERLOAD(f1)));
        TEST_EQUAL_RANGE(result, (std::vector<u8string>{"*", "**", "***", "****", "*****"}));
        TRY(std::transform(PRI_BOUNDS(sv), overwrite(result), PRI_OVERLOAD(f1)));
        TEST_EQUAL_RANGE(result, (std::vector<u8string>{"[Neddie]", "[Eccles]", "[Bluebottle]"}));

        TRY(std::generate_n(overwrite(result), 3, PRI_OVERLOAD(f2)));
        TEST_EQUAL_RANGE(result, (std::vector<u8string>{"Hello", "Hello", "Hello"}));
        TRY(std::transform(PRI_BOUNDS(sv), overwrite(result), PRI_OVERLOAD(f2)));
        TEST_EQUAL_RANGE(result, (std::vector<u8string>{"[Neddie]", "[Eccles]", "[Bluebottle]"}));
        auto of2 = PRI_OVERLOAD(f2);
        auto out = overwrite(result);
        for (size_t i = 0; i < iv.size() && i < iv2.size(); ++i)
            TRY(*out++ = of2(iv[i], iv2[i]));
        TEST_EQUAL_RANGE(result, (std::vector<u8string>{"2", "6", "15", "28", "55"}));

    }

    void check_arithmetic_literals() {

        int128_t i = 0;
        uint128_t u = 0;

        TRY(i = 0_s128);
        TEST_EQUAL(i, 0_s128);
        TEST_EQUAL(dec(i), "0");
        TEST_EQUAL(hex(i, 1), "0");
        TEST_EQUAL(hex(i), "00000000000000000000000000000000");
        TRY(i = 12345_s128);
        TEST_EQUAL(i, 12345_s128);
        TEST_EQUAL(dec(i), "12345");
        TRY(i = 170141183460469231731687303715884105727_s128); // 2^127-1
        TEST_EQUAL(i, 170141183460469231731687303715884105727_s128);
        TEST_EQUAL(dec(i), "170141183460469231731687303715884105727");
        TRY(i = -170141183460469231731687303715884105727_s128); // -(2^127-1)
        TEST_EQUAL(i, -170141183460469231731687303715884105727_s128);
        TEST_EQUAL(dec(i), "-170141183460469231731687303715884105727");
        TRY(i = -170141183460469231731687303715884105728_s128); // -2^127
        TEST_EQUAL(i, -170141183460469231731687303715884105728_s128);
        TEST_EQUAL(dec(i), "-170141183460469231731687303715884105728");

        TRY(i = 0x0_s128);
        TEST_EQUAL(dec(i), "0");
        TRY(i = 0x12345_s128);
        TEST_EQUAL(hex(i, 1), "12345");
        TEST_EQUAL(dec(i), "74565");
        TRY(i = 0x7fffffffffffffffffffffffffffffff_s128);
        TEST_EQUAL(hex(i), "7fffffffffffffffffffffffffffffff");
        TEST_EQUAL(dec(i), "170141183460469231731687303715884105727");

        TRY(u = 0_u128);
        TEST_EQUAL(u, 0_u128);
        TEST_EQUAL(dec(u), "0");
        TEST_EQUAL(hex(u, 1), "0");
        TEST_EQUAL(hex(u), "00000000000000000000000000000000");
        TRY(u = 12345_u128);
        TEST_EQUAL(u, 12345_u128);
        TEST_EQUAL(dec(u), "12345");
        TRY(u = 340282366920938463463374607431768211455_u128); // 2^128-1
        TEST_EQUAL(u, 340282366920938463463374607431768211455_u128);
        TEST_EQUAL(dec(u), "340282366920938463463374607431768211455");

        TRY(u = 0x0_u128);
        TEST_EQUAL(dec(u), "0");
        TRY(u = 0x12345_u128);
        TEST_EQUAL(hex(u, 1), "12345");
        TEST_EQUAL(dec(u), "74565");
        TRY(u = 0xffffffffffffffffffffffffffffffff_u128);
        TEST_EQUAL(hex(u), "ffffffffffffffffffffffffffffffff");
        TEST_EQUAL(dec(u), "340282366920938463463374607431768211455");

        TEST_EQUAL(0_deg, 0.0);
        TEST_NEAR(45_deg, 0.785398);
        TEST_NEAR(90_deg, 1.570796);
        TEST_NEAR(180_deg, 3.141593);
        TEST_NEAR(360_deg, 6.283185);
        TEST_NEAR(-45_deg, -0.785398);
        TEST_NEAR(-90_deg, -1.570796);
        TEST_NEAR(-180_deg, -3.141593);
        TEST_NEAR(-360_deg, -6.283185);

        TEST_EQUAL(0.0_deg, 0.0);
        TEST_NEAR(45.0_deg, 0.785398);
        TEST_NEAR(90.0_deg, 1.570796);
        TEST_NEAR(180.0_deg, 3.141593);
        TEST_NEAR(360.0_deg, 6.283185);
        TEST_NEAR(-45.0_deg, -0.785398);
        TEST_NEAR(-90.0_deg, -1.570796);
        TEST_NEAR(-180.0_deg, -3.141593);
        TEST_NEAR(-360.0_deg, -6.283185);

    }

    void check_arithmetic_functions() {

        static constexpr int n1 = 10, n2 = 20, n3 = 30, n4 = 40;

        static constexpr int m1 = static_min(n1);
        static constexpr int m2 = static_min(n1, n2);
        static constexpr int m3 = static_min(n1, n2, n3);
        static constexpr int m4 = static_min(n1, n2, n3, n4);
        static constexpr int m5 = static_max(n1);
        static constexpr int m6 = static_max(n1, n2);
        static constexpr int m7 = static_max(n1, n2, n3);
        static constexpr int m8 = static_max(n1, n2, n3, n4);

        TEST_EQUAL(m1, 10);
        TEST_EQUAL(m2, 10);
        TEST_EQUAL(m3, 10);
        TEST_EQUAL(m4, 10);
        TEST_EQUAL(m5, 10);
        TEST_EQUAL(m6, 20);
        TEST_EQUAL(m7, 30);
        TEST_EQUAL(m8, 40);

        int8_t s8 = -42;
        uint8_t u8 = 42;
        int16_t s16 = -42;
        uint16_t u16 = 42, v16;
        int32_t s32 = -42;
        uint32_t u32 = 42, v32;
        int64_t s64 = -42;
        uint64_t u64 = 42;
        int128_t s128 = -42;
        uint128_t u128 = 42;

        TEST_EQUAL(sizeof(as_signed(s8)), 1);
        TEST_EQUAL(sizeof(as_signed(u8)), 1);
        TEST_EQUAL(sizeof(as_unsigned(s8)), 1);
        TEST_EQUAL(sizeof(as_unsigned(u8)), 1);
        TEST_EQUAL(sizeof(as_signed(s16)), 2);
        TEST_EQUAL(sizeof(as_signed(u16)), 2);
        TEST_EQUAL(sizeof(as_unsigned(s16)), 2);
        TEST_EQUAL(sizeof(as_unsigned(u16)), 2);
        TEST_EQUAL(sizeof(as_signed(s32)), 4);
        TEST_EQUAL(sizeof(as_signed(u32)), 4);
        TEST_EQUAL(sizeof(as_unsigned(s32)), 4);
        TEST_EQUAL(sizeof(as_unsigned(u32)), 4);
        TEST_EQUAL(sizeof(as_signed(s64)), 8);
        TEST_EQUAL(sizeof(as_signed(u64)), 8);
        TEST_EQUAL(sizeof(as_unsigned(s64)), 8);
        TEST_EQUAL(sizeof(as_unsigned(u64)), 8);
        TEST_EQUAL(sizeof(as_signed(s128)), 16);
        TEST_EQUAL(sizeof(as_signed(u128)), 16);
        TEST_EQUAL(sizeof(as_unsigned(s128)), 16);
        TEST_EQUAL(sizeof(as_unsigned(u128)), 16);

        TEST(std::is_signed<decltype(as_signed(s8))>::value);
        TEST(std::is_signed<decltype(as_signed(u8))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(s8))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(u8))>::value);
        TEST(std::is_signed<decltype(as_signed(s16))>::value);
        TEST(std::is_signed<decltype(as_signed(u16))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(s16))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(u16))>::value);
        TEST(std::is_signed<decltype(as_signed(s32))>::value);
        TEST(std::is_signed<decltype(as_signed(u32))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(s32))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(u32))>::value);
        TEST(std::is_signed<decltype(as_signed(s64))>::value);
        TEST(std::is_signed<decltype(as_signed(u64))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(s64))>::value);
        TEST(std::is_unsigned<decltype(as_unsigned(u64))>::value);

        TEST_EQUAL(as_signed(s8), -42);
        TEST_EQUAL(as_signed(u8), 42);
        TEST_EQUAL(as_unsigned(s8), 214);
        TEST_EQUAL(as_unsigned(u8), 42);
        TEST_EQUAL(as_signed(s16), -42);
        TEST_EQUAL(as_signed(u16), 42);
        TEST_EQUAL(as_unsigned(s16), 65494);
        TEST_EQUAL(as_unsigned(u16), 42);
        TEST_EQUAL(as_signed(s32), -42);
        TEST_EQUAL(as_signed(u32), 42);
        TEST_EQUAL(as_unsigned(s32), 4294967254ul);
        TEST_EQUAL(as_unsigned(u32), 42);
        TEST_EQUAL(as_signed(s64), -42);
        TEST_EQUAL(as_signed(u64), 42);
        TEST_EQUAL(as_unsigned(s64), 18446744073709551574ull);
        TEST_EQUAL(as_unsigned(u64), 42);
        TEST_EQUAL(to_str(as_signed(s128)), "-42");
        TEST_EQUAL(to_str(as_signed(u128)), "42");
        TEST_EQUAL(to_str(as_unsigned(s128)), "340282366920938463463374607431768211414");
        TEST_EQUAL(to_str(as_unsigned(u128)), "42");

        TEST_EQUAL(clamp(1, 2, 4), 2);
        TEST_EQUAL(clamp(2, 2, 4), 2);
        TEST_EQUAL(clamp(3, 2, 4), 3);
        TEST_EQUAL(clamp(4, 2, 4), 4);
        TEST_EQUAL(clamp(5, 2, 4), 4);
        TEST_EQUAL(clamp(1.0, 2, 4), 2);
        TEST_EQUAL(clamp(1.5, 2, 4), 2);
        TEST_EQUAL(clamp(2.0, 2, 4), 2);
        TEST_EQUAL(clamp(2.5, 2, 4), 2.5);
        TEST_EQUAL(clamp(3.0, 2, 4), 3);
        TEST_EQUAL(clamp(3.5, 2, 4), 3.5);
        TEST_EQUAL(clamp(4.0, 2, 4), 4);
        TEST_EQUAL(clamp(4.5, 2, 4), 4);
        TEST_EQUAL(clamp(5.0, 2, 4), 4);

        TEST_EQUAL(degrees(0.0), 0);
        TEST_NEAR(degrees(1.0), 57.295780);
        TEST_NEAR(degrees(2.0), 114.591559);
        TEST_NEAR(degrees(-1.0), -57.295780);
        TEST_NEAR(degrees(-2.0), -114.591559);

        TEST_EQUAL(radians(0.0), 0);
        TEST_NEAR(radians(45.0), 0.785398);
        TEST_NEAR(radians(90.0), 1.570796);
        TEST_NEAR(radians(180.0), 3.141593);
        TEST_NEAR(radians(360.0), 6.283185);
        TEST_NEAR(radians(-45.0), -0.785398);
        TEST_NEAR(radians(-90.0), -1.570796);
        TEST_NEAR(radians(-180.0), -3.141593);
        TEST_NEAR(radians(-360.0), -6.283185);

        int i1, i2, i3, i4;

        i1 = -6;  i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -2);  TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -5;  i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -2);  TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -4;  i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -2);  TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -3;  i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -1);  TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -2;  i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -1);  TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -1;  i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -1);  TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 0;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 0);   TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 1;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 0);   TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 2;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 0);   TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 3;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 1);   TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 4;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 1);   TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 5;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 1);   TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 6;   i2 = 3;   TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 2);   TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -6;  i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 2);   TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -5;  i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 2);   TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -4;  i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 2);   TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -3;  i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 1);   TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -2;  i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 1);   TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = -1;  i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 1);   TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 0;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 0);   TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 1;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 0);   TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 2;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, 0);   TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 3;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -1);  TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 4;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -1);  TEST_EQUAL(i4, 1);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 5;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -1);  TEST_EQUAL(i4, 2);  TEST_EQUAL(i2 * i3 + i4, i1);
        i1 = 6;   i2 = -3;  TRY(i3 = quo(i1, i2));  TRY(i4 = rem(i1, i2));  TEST_EQUAL(i3, -2);  TEST_EQUAL(i4, 0);  TEST_EQUAL(i2 * i3 + i4, i1);

        unsigned u1, u2, u3, u4;

        u1 = 0;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 0);  TEST_EQUAL(u4, 0);  TEST_EQUAL(u2 * u3 + u4, u1);
        u1 = 1;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 0);  TEST_EQUAL(u4, 1);  TEST_EQUAL(u2 * u3 + u4, u1);
        u1 = 2;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 0);  TEST_EQUAL(u4, 2);  TEST_EQUAL(u2 * u3 + u4, u1);
        u1 = 3;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 1);  TEST_EQUAL(u4, 0);  TEST_EQUAL(u2 * u3 + u4, u1);
        u1 = 4;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 1);  TEST_EQUAL(u4, 1);  TEST_EQUAL(u2 * u3 + u4, u1);
        u1 = 5;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 1);  TEST_EQUAL(u4, 2);  TEST_EQUAL(u2 * u3 + u4, u1);
        u1 = 6;  u2 = 3;  TRY(u3 = quo(u1, u2));  TRY(u4 = rem(u1, u2));  TEST_EQUAL(u3, 2);  TEST_EQUAL(u4, 0);  TEST_EQUAL(u2 * u3 + u4, u1);

        int128_t x1, x2, x3, x4;

        x1 = -6;  x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -2);  TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -5;  x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -2);  TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -4;  x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -2);  TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -3;  x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -1);  TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -2;  x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -1);  TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -1;  x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -1);  TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 0;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 0);   TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 1;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 0);   TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 2;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 0);   TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 3;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 1);   TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 4;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 1);   TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 5;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 1);   TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 6;   x2 = 3;   TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 2);   TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -6;  x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 2);   TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -5;  x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 2);   TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -4;  x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 2);   TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -3;  x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 1);   TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -2;  x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 1);   TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = -1;  x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 1);   TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 0;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 0);   TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 1;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 0);   TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 2;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, 0);   TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 3;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -1);  TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 4;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -1);  TEST_EQUAL(x4, 1);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 5;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -1);  TEST_EQUAL(x4, 2);  TEST_EQUAL(x2 * x3 + x4, x1);
        x1 = 6;   x2 = -3;  TRY(x3 = quo(x1, x2));  TRY(x4 = rem(x1, x2));  TEST_EQUAL(x3, -2);  TEST_EQUAL(x4, 0);  TEST_EQUAL(x2 * x3 + x4, x1);

        uint128_t y1, y2, y3, y4;

        y1 = 0;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 0);  TEST_EQUAL(y4, 0);  TEST_EQUAL(y2 * y3 + y4, y1);
        y1 = 1;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 0);  TEST_EQUAL(y4, 1);  TEST_EQUAL(y2 * y3 + y4, y1);
        y1 = 2;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 0);  TEST_EQUAL(y4, 2);  TEST_EQUAL(y2 * y3 + y4, y1);
        y1 = 3;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 1);  TEST_EQUAL(y4, 0);  TEST_EQUAL(y2 * y3 + y4, y1);
        y1 = 4;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 1);  TEST_EQUAL(y4, 1);  TEST_EQUAL(y2 * y3 + y4, y1);
        y1 = 5;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 1);  TEST_EQUAL(y4, 2);  TEST_EQUAL(y2 * y3 + y4, y1);
        y1 = 6;  y2 = 3;  TRY(y3 = quo(y1, y2));  TRY(y4 = rem(y1, y2));  TEST_EQUAL(y3, 2);  TEST_EQUAL(y4, 0);  TEST_EQUAL(y2 * y3 + y4, y1);

        double d1, d2, d3, d4;

        d1 = -6;  d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -2);  TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -5;  d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -2);  TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -4;  d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -2);  TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -3;  d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -1);  TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -2;  d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -1);  TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -1;  d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -1);  TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 0;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 0);   TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 1;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 0);   TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 2;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 0);   TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 3;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 1);   TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 4;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 1);   TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 5;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 1);   TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 6;   d2 = 3;   TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 2);   TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -6;  d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 2);   TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -5;  d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 2);   TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -4;  d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 2);   TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -3;  d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 1);   TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -2;  d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 1);   TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = -1;  d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 1);   TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 0;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 0);   TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 1;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 0);   TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 2;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, 0);   TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 3;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -1);  TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 4;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -1);  TEST_EQUAL(d4, 1);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 5;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -1);  TEST_EQUAL(d4, 2);  TEST_EQUAL(d2 * d3 + d4, d1);
        d1 = 6;   d2 = -3;  TRY(d3 = quo(d1, d2));  TRY(d4 = rem(d1, d2));  TEST_EQUAL(d3, -2);  TEST_EQUAL(d4, 0);  TEST_EQUAL(d2 * d3 + d4, d1);

        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 0.00), 25.0);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 0.25), 22.5);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 0.50), 20.0);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 0.75), 17.5);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 1.00), 15.0);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 1.25), 12.5);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 1.50), 10.0);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 1.75), 7.5);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 2.00), 5.0);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 2.25), 2.5);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 2.50), 0.0);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 2.75), -2.5);
        TEST_EQUAL(interpolate(1.0, 15.0, 2.0, 5.0, 3.00), -5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 0.50), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 1.00), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 1.50), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 2.00), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 2.50), 5.0);

        TEST_EQUAL(int_power(0, 1), 0);
        TEST_EQUAL(int_power(0, 2), 0);
        TEST_EQUAL(int_power(0, 3), 0);

        TEST_EQUAL(int_power(1, 0), 1);
        TEST_EQUAL(int_power(1, 1), 1);
        TEST_EQUAL(int_power(1, 2), 1);
        TEST_EQUAL(int_power(1, 3), 1);

        TEST_EQUAL(int_power(-1, 0), 1);
        TEST_EQUAL(int_power(-1, 1), -1);
        TEST_EQUAL(int_power(-1, 2), 1);
        TEST_EQUAL(int_power(-1, 3), -1);

        TEST_EQUAL(int_power(2, 0), 1);
        TEST_EQUAL(int_power(2, 1), 2);
        TEST_EQUAL(int_power(2, 2), 4);
        TEST_EQUAL(int_power(2, 3), 8);
        TEST_EQUAL(int_power(2, 4), 16);
        TEST_EQUAL(int_power(2, 5), 32);
        TEST_EQUAL(int_power(2, 6), 64);
        TEST_EQUAL(int_power(2, 7), 128);
        TEST_EQUAL(int_power(2, 8), 256);
        TEST_EQUAL(int_power(2, 9), 512);
        TEST_EQUAL(int_power(2, 10), 1024);

        TEST_EQUAL(int_power(-2, 0), 1);
        TEST_EQUAL(int_power(-2, 1), -2);
        TEST_EQUAL(int_power(-2, 2), 4);
        TEST_EQUAL(int_power(-2, 3), -8);
        TEST_EQUAL(int_power(-2, 4), 16);
        TEST_EQUAL(int_power(-2, 5), -32);
        TEST_EQUAL(int_power(-2, 6), 64);
        TEST_EQUAL(int_power(-2, 7), -128);
        TEST_EQUAL(int_power(-2, 8), 256);
        TEST_EQUAL(int_power(-2, 9), -512);
        TEST_EQUAL(int_power(-2, 10), 1024);

        TEST_EQUAL(int_power(10ll, 0ll), 1ll);
        TEST_EQUAL(int_power(10ll, 1ll), 10ll);
        TEST_EQUAL(int_power(10ll, 2ll), 100ll);
        TEST_EQUAL(int_power(10ll, 3ll), 1000ll);
        TEST_EQUAL(int_power(10ll, 4ll), 10000ll);
        TEST_EQUAL(int_power(10ll, 5ll), 100000ll);
        TEST_EQUAL(int_power(10ll, 6ll), 1000000ll);
        TEST_EQUAL(int_power(10ll, 7ll), 10000000ll);
        TEST_EQUAL(int_power(10ll, 8ll), 100000000ll);
        TEST_EQUAL(int_power(10ll, 9ll), 1000000000ll);
        TEST_EQUAL(int_power(10ll, 10ll), 10000000000ll);

        TEST_EQUAL(int_power(-10ll, 0ll), 1ll);
        TEST_EQUAL(int_power(-10ll, 1ll), -10ll);
        TEST_EQUAL(int_power(-10ll, 2ll), 100ll);
        TEST_EQUAL(int_power(-10ll, 3ll), -1000ll);
        TEST_EQUAL(int_power(-10ll, 4ll), 10000ll);
        TEST_EQUAL(int_power(-10ll, 5ll), -100000ll);
        TEST_EQUAL(int_power(-10ll, 6ll), 1000000ll);
        TEST_EQUAL(int_power(-10ll, 7ll), -10000000ll);
        TEST_EQUAL(int_power(-10ll, 8ll), 100000000ll);
        TEST_EQUAL(int_power(-10ll, 9ll), -1000000000ll);
        TEST_EQUAL(int_power(-10ll, 10ll), 10000000000ll);

        TEST_EQUAL(int_power(10ull, 0ull), 1ull);
        TEST_EQUAL(int_power(10ull, 1ull), 10ull);
        TEST_EQUAL(int_power(10ull, 2ull), 100ull);
        TEST_EQUAL(int_power(10ull, 3ull), 1000ull);
        TEST_EQUAL(int_power(10ull, 4ull), 10000ull);
        TEST_EQUAL(int_power(10ull, 5ull), 100000ull);
        TEST_EQUAL(int_power(10ull, 6ull), 1000000ull);
        TEST_EQUAL(int_power(10ull, 7ull), 10000000ull);
        TEST_EQUAL(int_power(10ull, 8ull), 100000000ull);
        TEST_EQUAL(int_power(10ull, 9ull), 1000000000ull);
        TEST_EQUAL(int_power(10ull, 10ull), 10000000000ull);

        TEST_EQUAL(int_power(-10_s128, 0_s128), 1_s128);
        TEST_EQUAL(int_power(-10_s128, 1_s128), -10_s128);
        TEST_EQUAL(int_power(-10_s128, 2_s128), 100_s128);
        TEST_EQUAL(int_power(-10_s128, 3_s128), -1000_s128);
        TEST_EQUAL(int_power(-10_s128, 4_s128), 10000_s128);
        TEST_EQUAL(int_power(-10_s128, 5_s128), -100000_s128);
        TEST_EQUAL(int_power(-10_s128, 6_s128), 1000000_s128);
        TEST_EQUAL(int_power(-10_s128, 7_s128), -10000000_s128);
        TEST_EQUAL(int_power(-10_s128, 8_s128), 100000000_s128);
        TEST_EQUAL(int_power(-10_s128, 9_s128), -1000000000_s128);
        TEST_EQUAL(int_power(-10_s128, 10_s128), 10000000000_s128);

        TEST_EQUAL(int_power(10_u128, 0_u128), 1_u128);
        TEST_EQUAL(int_power(10_u128, 1_u128), 10_u128);
        TEST_EQUAL(int_power(10_u128, 2_u128), 100_u128);
        TEST_EQUAL(int_power(10_u128, 3_u128), 1000_u128);
        TEST_EQUAL(int_power(10_u128, 4_u128), 10000_u128);
        TEST_EQUAL(int_power(10_u128, 5_u128), 100000_u128);
        TEST_EQUAL(int_power(10_u128, 6_u128), 1000000_u128);
        TEST_EQUAL(int_power(10_u128, 7_u128), 10000000_u128);
        TEST_EQUAL(int_power(10_u128, 8_u128), 100000000_u128);
        TEST_EQUAL(int_power(10_u128, 9_u128), 1000000000_u128);
        TEST_EQUAL(int_power(10_u128, 10_u128), 10000000000_u128);

        TEST_EQUAL(int_sqrt(0), 0);
        TEST_EQUAL(int_sqrt(1), 1);
        TEST_EQUAL(int_sqrt(2), 1);
        TEST_EQUAL(int_sqrt(3), 1);
        TEST_EQUAL(int_sqrt(4), 2);
        TEST_EQUAL(int_sqrt(5), 2);
        TEST_EQUAL(int_sqrt(6), 2);
        TEST_EQUAL(int_sqrt(7), 2);
        TEST_EQUAL(int_sqrt(8), 2);
        TEST_EQUAL(int_sqrt(9), 3);
        TEST_EQUAL(int_sqrt(10), 3);
        TEST_EQUAL(int_sqrt(11), 3);
        TEST_EQUAL(int_sqrt(12), 3);
        TEST_EQUAL(int_sqrt(13), 3);
        TEST_EQUAL(int_sqrt(14), 3);
        TEST_EQUAL(int_sqrt(15), 3);
        TEST_EQUAL(int_sqrt(16), 4);
        TEST_EQUAL(int_sqrt(17), 4);
        TEST_EQUAL(int_sqrt(18), 4);
        TEST_EQUAL(int_sqrt(19), 4);
        TEST_EQUAL(int_sqrt(20), 4);
        TEST_EQUAL(int_sqrt(21), 4);
        TEST_EQUAL(int_sqrt(22), 4);
        TEST_EQUAL(int_sqrt(23), 4);
        TEST_EQUAL(int_sqrt(24), 4);
        TEST_EQUAL(int_sqrt(25), 5);

        TEST_EQUAL(int_sqrt(123u), 11u);
        TEST_EQUAL(int_sqrt(123), 11);
        TEST_EQUAL(int_sqrt(1234u), 35u);
        TEST_EQUAL(int_sqrt(1234), 35);
        TEST_EQUAL(int_sqrt(12345u), 111u);
        TEST_EQUAL(int_sqrt(12345), 111);
        TEST_EQUAL(int_sqrt(123456ul), 351ul);
        TEST_EQUAL(int_sqrt(123456l), 351l);
        TEST_EQUAL(int_sqrt(1234567ul), 1111ul);
        TEST_EQUAL(int_sqrt(1234567l), 1111l);
        TEST_EQUAL(int_sqrt(12345678ul), 3513ul);
        TEST_EQUAL(int_sqrt(12345678l), 3513l);
        TEST_EQUAL(int_sqrt(123456789ul), 11111ul);
        TEST_EQUAL(int_sqrt(123456789l), 11111l);
        TEST_EQUAL(int_sqrt(1234567890ul), 35136ul);
        TEST_EQUAL(int_sqrt(1234567890l), 35136l);
        TEST_EQUAL(int_sqrt(12345678901ull), 111111ull);
        TEST_EQUAL(int_sqrt(12345678901ll), 111111ll);
        TEST_EQUAL(int_sqrt(123456789012ull), 351364ull);
        TEST_EQUAL(int_sqrt(123456789012ll), 351364ll);
        TEST_EQUAL(int_sqrt(1234567890123ull), 1111111ull);
        TEST_EQUAL(int_sqrt(1234567890123ll), 1111111ll);
        TEST_EQUAL(int_sqrt(12345678901234ull), 3513641ull);
        TEST_EQUAL(int_sqrt(12345678901234ll), 3513641ll);
        TEST_EQUAL(int_sqrt(123456789012345ull), 11111111ull);
        TEST_EQUAL(int_sqrt(123456789012345ll), 11111111ll);
        TEST_EQUAL(int_sqrt(1234567890123456ull), 35136418ull);
        TEST_EQUAL(int_sqrt(1234567890123456ll), 35136418ll);
        TEST_EQUAL(int_sqrt(12345678901234567ull), 111111110ull);
        TEST_EQUAL(int_sqrt(12345678901234567ll), 111111110ll);
        TEST_EQUAL(int_sqrt(123456789012345678ull), 351364182ull);
        TEST_EQUAL(int_sqrt(123456789012345678ll), 351364182ll);
        TEST_EQUAL(int_sqrt(1234567890123456789ull), 1111111106ull);
        TEST_EQUAL(int_sqrt(1234567890123456789ll), 1111111106ll);
        TEST_EQUAL(int_sqrt(12345678901234567890ull), 3513641828ull);
        TEST_EQUAL(int_sqrt(12345678901234567890_s128), 3513641828_s128);
        TEST_EQUAL(int_sqrt(123456789012345678901_u128), 11111111061_u128);
        TEST_EQUAL(int_sqrt(123456789012345678901_s128), 11111111061_s128);
        TEST_EQUAL(int_sqrt(1234567890123456789012_u128), 35136418288_u128);
        TEST_EQUAL(int_sqrt(1234567890123456789012_s128), 35136418288_s128);
        TEST_EQUAL(int_sqrt(12345678901234567890123_u128), 111111110611_u128);
        TEST_EQUAL(int_sqrt(12345678901234567890123_s128), 111111110611_s128);
        TEST_EQUAL(int_sqrt(123456789012345678901234_u128), 351364182882_u128);
        TEST_EQUAL(int_sqrt(123456789012345678901234_s128), 351364182882_s128);
        TEST_EQUAL(int_sqrt(1234567890123456789012345_u128), 1111111106111_u128);
        TEST_EQUAL(int_sqrt(1234567890123456789012345_s128), 1111111106111_s128);
        TEST_EQUAL(int_sqrt(12345678901234567890123456_u128), 3513641828820_u128);
        TEST_EQUAL(int_sqrt(12345678901234567890123456_s128), 3513641828820_s128);
        TEST_EQUAL(int_sqrt(123456789012345678901234567_u128), 11111111061111_u128);
        TEST_EQUAL(int_sqrt(123456789012345678901234567_s128), 11111111061111_s128);
        TEST_EQUAL(int_sqrt(1234567890123456789012345678_u128), 35136418288201_u128);
        TEST_EQUAL(int_sqrt(1234567890123456789012345678_s128), 35136418288201_s128);
        TEST_EQUAL(int_sqrt(12345678901234567890123456789_u128), 111111110611111_u128);
        TEST_EQUAL(int_sqrt(12345678901234567890123456789_s128), 111111110611111_s128);
        TEST_EQUAL(int_sqrt(123456789012345678901234567890_u128), 351364182882014_u128);
        TEST_EQUAL(int_sqrt(123456789012345678901234567890_s128), 351364182882014_s128);

        u16 = 0x1234;

        TRY(v16 = rotl(u16, 0));   TEST_EQUAL(v16, 0x1234);
        TRY(v16 = rotl(u16, 4));   TEST_EQUAL(v16, 0x2341);
        TRY(v16 = rotl(u16, 8));   TEST_EQUAL(v16, 0x3412);
        TRY(v16 = rotl(u16, 12));  TEST_EQUAL(v16, 0x4123);
        TRY(v16 = rotr(u16, 0));   TEST_EQUAL(v16, 0x1234);
        TRY(v16 = rotr(u16, 4));   TEST_EQUAL(v16, 0x4123);
        TRY(v16 = rotr(u16, 8));   TEST_EQUAL(v16, 0x3412);
        TRY(v16 = rotr(u16, 12));  TEST_EQUAL(v16, 0x2341);

        u32 = 0x12345678;

        TRY(v32 = rotl(u32, 0));    TEST_EQUAL(v32, 0x12345678);
        TRY(v32 = rotl(u32, 4));    TEST_EQUAL(v32, 0x23456781);
        TRY(v32 = rotl(u32, 8));    TEST_EQUAL(v32, 0x34567812);
        TRY(v32 = rotl(u32, 12));   TEST_EQUAL(v32, 0x45678123);
        TRY(v32 = rotl(u32, 16));   TEST_EQUAL(v32, 0x56781234);
        TRY(v32 = rotl(u32, 20));   TEST_EQUAL(v32, 0x67812345);
        TRY(v32 = rotl(u32, 24));   TEST_EQUAL(v32, 0x78123456);
        TRY(v32 = rotl(u32, 28));   TEST_EQUAL(v32, 0x81234567);
        TRY(v32 = rotr(u32, 0));    TEST_EQUAL(v32, 0x12345678);
        TRY(v32 = rotr(u32, 4));    TEST_EQUAL(v32, 0x81234567);
        TRY(v32 = rotr(u32, 8));    TEST_EQUAL(v32, 0x78123456);
        TRY(v32 = rotr(u32, 12));   TEST_EQUAL(v32, 0x67812345);
        TRY(v32 = rotr(u32, 16));   TEST_EQUAL(v32, 0x56781234);
        TRY(v32 = rotr(u32, 20));   TEST_EQUAL(v32, 0x45678123);
        TRY(v32 = rotr(u32, 24));   TEST_EQUAL(v32, 0x34567812);
        TRY(v32 = rotr(u32, 28));   TEST_EQUAL(v32, 0x23456781);

        TEST_EQUAL(round<int>(42), 42);
        TEST_EQUAL(round<double>(42), 42.0);
        TEST_EQUAL(round<double>(42.0), 42.0);
        TEST_EQUAL(round<double>(42.42), 42.0);
        TEST_EQUAL(round<float>(42.0), 42.0f);
        TEST_EQUAL(round<float>(42.42), 42.0f);
        TEST_EQUAL(round<int>(0.0), 0);
        TEST_EQUAL(round<int>(0.25), 0);
        TEST_EQUAL(round<int>(0.5), 1);
        TEST_EQUAL(round<int>(0.75), 1);
        TEST_EQUAL(round<int>(1), 1);
        TEST_EQUAL(round<int>(-0.25), 0);
        TEST_EQUAL(round<int>(-0.5), 0);
        TEST_EQUAL(round<int>(-0.75), -1);
        TEST_EQUAL(round<int>(-1), -1);
        TEST_EQUAL(round<double>(0.0), 0.0);
        TEST_EQUAL(round<double>(0.25), 0.0);
        TEST_EQUAL(round<double>(0.5), 1.0);
        TEST_EQUAL(round<double>(0.75), 1.0);
        TEST_EQUAL(round<double>(1), 1.0);
        TEST_EQUAL(round<double>(-0.25), 0.0);
        TEST_EQUAL(round<double>(-0.5), 0.0);
        TEST_EQUAL(round<double>(-0.75), -1.0);
        TEST_EQUAL(round<double>(-1), -1.0);
        TEST_EQUAL(round<int128_t>(0.0), 0);
        TEST_EQUAL(round<int128_t>(0.25), 0);
        TEST_EQUAL(round<int128_t>(0.5), 1);
        TEST_EQUAL(round<int128_t>(0.75), 1);
        TEST_EQUAL(round<int128_t>(1), 1);
        TEST_EQUAL(round<int128_t>(-0.25), 0);
        TEST_EQUAL(round<int128_t>(-0.5), 0);
        TEST_EQUAL(round<int128_t>(-0.75), -1);
        TEST_EQUAL(round<int128_t>(-1), -1);

        TEST_EQUAL(sign_of(0), 0);
        TEST_EQUAL(sign_of(42), 1);
        TEST_EQUAL(sign_of(-42), -1);
        TEST_EQUAL(sign_of(0u), 0);
        TEST_EQUAL(sign_of(42u), 1);
        TEST_EQUAL(sign_of(0.0), 0);
        TEST_EQUAL(sign_of(42.0), 1);
        TEST_EQUAL(sign_of(-42.0), -1);
        TEST_EQUAL(sign_of(0_s128), 0);
        TEST_EQUAL(sign_of(42_s128), 1);
        TEST_EQUAL(sign_of(-42_s128), -1);
        TEST_EQUAL(sign_of(0_u128), 0);
        TEST_EQUAL(sign_of(42_u128), 1);

    }

    void check_byte_order() {

        union { uint16_t u; uint8_t b[2]; } union16;
        union16.u = 1;
        uint32_t u32 = 0x12345678;

        TEST_COMPARE(big_endian_target, !=, little_endian_target);

        if (little_endian_target) {
            TEST_EQUAL(union16.b[0], 1);
            TEST_EQUAL(union16.b[1], 0);
            TEST_EQUAL(big_endian(u32), 0x78563412);
            TEST_EQUAL(little_endian(u32), 0x12345678);
        } else {
            TEST_EQUAL(union16.b[0], 0);
            TEST_EQUAL(union16.b[1], 1);
            TEST_EQUAL(big_endian(u32), 0x12345678);
            TEST_EQUAL(little_endian(u32), 0x78563412);
        }

        string s = "\x12\x34\x56\x78\x9a\xbc\xde\xf0";

        TRY(read_be(u32, s.data()));                   TEST_EQUAL(u32, 0x12345678);
        TRY(read_be(u32, s.data(), 0, 4));             TEST_EQUAL(u32, 0x12345678);
        TRY(read_be(u32, s.data(), 0, 8));             TEST_EQUAL(u32, 0x9abcdef0);
        TRY(read_be(u32, s.data(), 0, 2));             TEST_EQUAL(u32, 0x1234);
        TRY(read_be(u32, s.data(), 2, 2));             TEST_EQUAL(u32, 0x5678);
        TRY(read_le(u32, s.data()));                   TEST_EQUAL(u32, 0x78563412);
        TRY(read_le(u32, s.data(), 0, 4));             TEST_EQUAL(u32, 0x78563412);
        TRY(read_le(u32, s.data(), 0, 8));             TEST_EQUAL(u32, 0x78563412);
        TRY(read_le(u32, s.data(), 0, 2));             TEST_EQUAL(u32, 0x3412);
        TRY(read_le(u32, s.data(), 2, 2));             TEST_EQUAL(u32, 0x7856);
        TRY(u32 = read_be<uint32_t>(s.data()));        TEST_EQUAL(u32, 0x12345678);
        TRY(u32 = read_be<uint32_t>(s.data(), 0, 4));  TEST_EQUAL(u32, 0x12345678);
        TRY(u32 = read_be<uint32_t>(s.data(), 0, 8));  TEST_EQUAL(u32, 0x9abcdef0);
        TRY(u32 = read_be<uint32_t>(s.data(), 0, 2));  TEST_EQUAL(u32, 0x1234);
        TRY(u32 = read_be<uint32_t>(s.data(), 2, 2));  TEST_EQUAL(u32, 0x5678);
        TRY(u32 = read_le<uint32_t>(s.data()));        TEST_EQUAL(u32, 0x78563412);
        TRY(u32 = read_le<uint32_t>(s.data(), 0, 4));  TEST_EQUAL(u32, 0x78563412);
        TRY(u32 = read_le<uint32_t>(s.data(), 0, 8));  TEST_EQUAL(u32, 0x78563412);
        TRY(u32 = read_le<uint32_t>(s.data(), 0, 2));  TEST_EQUAL(u32, 0x3412);
        TRY(u32 = read_le<uint32_t>(s.data(), 2, 2));  TEST_EQUAL(u32, 0x7856);

        s.assign(4, 0);

        TRY(write_be(uint32_t(0x12345678), &s[0]));                   TEST_EQUAL(s, "\x12\x34\x56\x78");
        TRY(write_be(uint32_t(0x12345678), &s[0], 0, 4));             TEST_EQUAL(s, "\x12\x34\x56\x78");
        TRY(write_be(uint32_t(0x123456789abcdef0ull), &s[0], 0, 4));  TEST_EQUAL(s, "\x9a\xbc\xde\xf0");
        TRY(write_le(uint32_t(0x12345678), &s[0]));                   TEST_EQUAL(s, "\x78\x56\x34\x12");
        TRY(write_le(uint32_t(0x12345678), &s[0], 0, 4));             TEST_EQUAL(s, "\x78\x56\x34\x12");
        TRY(write_le(uint32_t(0x123456789abcdef0ull), &s[0], 0, 4));  TEST_EQUAL(s, "\xf0\xde\xbc\x9a");

        s.assign(4, '*');  TRY(write_be(uint32_t(0x12345678), &s[0], 0, 2));  TEST_EQUAL(s, "\x56\x78**");
        s.assign(4, '*');  TRY(write_be(uint32_t(0x12345678), &s[0], 2, 2));  TEST_EQUAL(s, "**\x56\x78");
        s.assign(4, '*');  TRY(write_le(uint32_t(0x12345678), &s[0], 0, 2));  TEST_EQUAL(s, "\x78\x56**");
        s.assign(4, '*');  TRY(write_le(uint32_t(0x12345678), &s[0], 2, 2));  TEST_EQUAL(s, "**\x78\x56");

    }

    void check_character_functions() {

        TEST(! ascii_isalnum('\0'));
        TEST(! ascii_isalnum('\t'));
        TEST(! ascii_isalnum('\n'));
        TEST(! ascii_isalnum('\x1f'));
        TEST(! ascii_isalnum(' '));
        TEST(! ascii_isalnum('!'));
        TEST(! ascii_isalnum('/'));
        TEST(ascii_isalnum('0'));
        TEST(ascii_isalnum('9'));
        TEST(! ascii_isalnum(':'));
        TEST(! ascii_isalnum('@'));
        TEST(ascii_isalnum('A'));
        TEST(ascii_isalnum('Z'));
        TEST(! ascii_isalnum('['));
        TEST(! ascii_isalnum('_'));
        TEST(! ascii_isalnum('`'));
        TEST(ascii_isalnum('a'));
        TEST(ascii_isalnum('z'));
        TEST(! ascii_isalnum('{'));
        TEST(! ascii_isalnum('~'));
        TEST(! ascii_isalnum('\x7f'));
        TEST(! ascii_isalnum('\x80'));

        TEST(! ascii_isalpha('\0'));
        TEST(! ascii_isalpha('\t'));
        TEST(! ascii_isalpha('\n'));
        TEST(! ascii_isalpha('\x1f'));
        TEST(! ascii_isalpha(' '));
        TEST(! ascii_isalpha('!'));
        TEST(! ascii_isalpha('/'));
        TEST(! ascii_isalpha('0'));
        TEST(! ascii_isalpha('9'));
        TEST(! ascii_isalpha(':'));
        TEST(! ascii_isalpha('@'));
        TEST(ascii_isalpha('A'));
        TEST(ascii_isalpha('Z'));
        TEST(! ascii_isalpha('['));
        TEST(! ascii_isalpha('_'));
        TEST(! ascii_isalpha('`'));
        TEST(ascii_isalpha('a'));
        TEST(ascii_isalpha('z'));
        TEST(! ascii_isalpha('{'));
        TEST(! ascii_isalpha('~'));
        TEST(! ascii_isalpha('\x7f'));
        TEST(! ascii_isalpha('\x80'));

        TEST(ascii_iscntrl('\0'));
        TEST(ascii_iscntrl('\t'));
        TEST(ascii_iscntrl('\n'));
        TEST(ascii_iscntrl('\x1f'));
        TEST(! ascii_iscntrl(' '));
        TEST(! ascii_iscntrl('!'));
        TEST(! ascii_iscntrl('/'));
        TEST(! ascii_iscntrl('0'));
        TEST(! ascii_iscntrl('9'));
        TEST(! ascii_iscntrl(':'));
        TEST(! ascii_iscntrl('@'));
        TEST(! ascii_iscntrl('A'));
        TEST(! ascii_iscntrl('Z'));
        TEST(! ascii_iscntrl('['));
        TEST(! ascii_iscntrl('_'));
        TEST(! ascii_iscntrl('`'));
        TEST(! ascii_iscntrl('a'));
        TEST(! ascii_iscntrl('z'));
        TEST(! ascii_iscntrl('{'));
        TEST(! ascii_iscntrl('~'));
        TEST(ascii_iscntrl('\x7f'));
        TEST(! ascii_iscntrl('\x80'));

        TEST(! ascii_isdigit('\0'));
        TEST(! ascii_isdigit('\t'));
        TEST(! ascii_isdigit('\n'));
        TEST(! ascii_isdigit('\x1f'));
        TEST(! ascii_isdigit(' '));
        TEST(! ascii_isdigit('!'));
        TEST(! ascii_isdigit('/'));
        TEST(ascii_isdigit('0'));
        TEST(ascii_isdigit('9'));
        TEST(! ascii_isdigit(':'));
        TEST(! ascii_isdigit('@'));
        TEST(! ascii_isdigit('A'));
        TEST(! ascii_isdigit('Z'));
        TEST(! ascii_isdigit('['));
        TEST(! ascii_isdigit('_'));
        TEST(! ascii_isdigit('`'));
        TEST(! ascii_isdigit('a'));
        TEST(! ascii_isdigit('z'));
        TEST(! ascii_isdigit('{'));
        TEST(! ascii_isdigit('~'));
        TEST(! ascii_isdigit('\x7f'));
        TEST(! ascii_isdigit('\x80'));

        TEST(! ascii_isgraph('\0'));
        TEST(! ascii_isgraph('\t'));
        TEST(! ascii_isgraph('\n'));
        TEST(! ascii_isgraph('\x1f'));
        TEST(! ascii_isgraph(' '));
        TEST(ascii_isgraph('!'));
        TEST(ascii_isgraph('/'));
        TEST(ascii_isgraph('0'));
        TEST(ascii_isgraph('9'));
        TEST(ascii_isgraph(':'));
        TEST(ascii_isgraph('@'));
        TEST(ascii_isgraph('A'));
        TEST(ascii_isgraph('Z'));
        TEST(ascii_isgraph('['));
        TEST(ascii_isgraph('_'));
        TEST(ascii_isgraph('`'));
        TEST(ascii_isgraph('a'));
        TEST(ascii_isgraph('z'));
        TEST(ascii_isgraph('{'));
        TEST(ascii_isgraph('~'));
        TEST(! ascii_isgraph('\x7f'));
        TEST(! ascii_isgraph('\x80'));

        TEST(! ascii_islower('\0'));
        TEST(! ascii_islower('\t'));
        TEST(! ascii_islower('\n'));
        TEST(! ascii_islower('\x1f'));
        TEST(! ascii_islower(' '));
        TEST(! ascii_islower('!'));
        TEST(! ascii_islower('/'));
        TEST(! ascii_islower('0'));
        TEST(! ascii_islower('9'));
        TEST(! ascii_islower(':'));
        TEST(! ascii_islower('@'));
        TEST(! ascii_islower('A'));
        TEST(! ascii_islower('Z'));
        TEST(! ascii_islower('['));
        TEST(! ascii_islower('_'));
        TEST(! ascii_islower('`'));
        TEST(ascii_islower('a'));
        TEST(ascii_islower('z'));
        TEST(! ascii_islower('{'));
        TEST(! ascii_islower('~'));
        TEST(! ascii_islower('\x7f'));
        TEST(! ascii_islower('\x80'));

        TEST(! ascii_isprint('\0'));
        TEST(! ascii_isprint('\t'));
        TEST(! ascii_isprint('\n'));
        TEST(! ascii_isprint('\x1f'));
        TEST(ascii_isprint(' '));
        TEST(ascii_isprint('!'));
        TEST(ascii_isprint('/'));
        TEST(ascii_isprint('0'));
        TEST(ascii_isprint('9'));
        TEST(ascii_isprint(':'));
        TEST(ascii_isprint('@'));
        TEST(ascii_isprint('A'));
        TEST(ascii_isprint('Z'));
        TEST(ascii_isprint('['));
        TEST(ascii_isprint('_'));
        TEST(ascii_isprint('`'));
        TEST(ascii_isprint('a'));
        TEST(ascii_isprint('z'));
        TEST(ascii_isprint('{'));
        TEST(ascii_isprint('~'));
        TEST(! ascii_isprint('\x7f'));
        TEST(! ascii_isprint('\x80'));

        TEST(! ascii_ispunct('\0'));
        TEST(! ascii_ispunct('\t'));
        TEST(! ascii_ispunct('\n'));
        TEST(! ascii_ispunct('\x1f'));
        TEST(! ascii_ispunct(' '));
        TEST(ascii_ispunct('!'));
        TEST(ascii_ispunct('/'));
        TEST(! ascii_ispunct('0'));
        TEST(! ascii_ispunct('9'));
        TEST(ascii_ispunct(':'));
        TEST(ascii_ispunct('@'));
        TEST(! ascii_ispunct('A'));
        TEST(! ascii_ispunct('Z'));
        TEST(ascii_ispunct('['));
        TEST(ascii_ispunct('_'));
        TEST(ascii_ispunct('`'));
        TEST(! ascii_ispunct('a'));
        TEST(! ascii_ispunct('z'));
        TEST(ascii_ispunct('{'));
        TEST(ascii_ispunct('~'));
        TEST(! ascii_ispunct('\x7f'));
        TEST(! ascii_ispunct('\x80'));

        TEST(! ascii_isspace('\0'));
        TEST(ascii_isspace('\t'));
        TEST(ascii_isspace('\n'));
        TEST(! ascii_isspace('\x1f'));
        TEST(ascii_isspace(' '));
        TEST(! ascii_isspace('!'));
        TEST(! ascii_isspace('/'));
        TEST(! ascii_isspace('0'));
        TEST(! ascii_isspace('9'));
        TEST(! ascii_isspace(':'));
        TEST(! ascii_isspace('@'));
        TEST(! ascii_isspace('A'));
        TEST(! ascii_isspace('Z'));
        TEST(! ascii_isspace('['));
        TEST(! ascii_isspace('_'));
        TEST(! ascii_isspace('`'));
        TEST(! ascii_isspace('a'));
        TEST(! ascii_isspace('z'));
        TEST(! ascii_isspace('{'));
        TEST(! ascii_isspace('~'));
        TEST(! ascii_isspace('\x7f'));
        TEST(! ascii_isspace('\x80'));

        TEST(! ascii_isupper('\0'));
        TEST(! ascii_isupper('\t'));
        TEST(! ascii_isupper('\n'));
        TEST(! ascii_isupper('\x1f'));
        TEST(! ascii_isupper(' '));
        TEST(! ascii_isupper('!'));
        TEST(! ascii_isupper('/'));
        TEST(! ascii_isupper('0'));
        TEST(! ascii_isupper('9'));
        TEST(! ascii_isupper(':'));
        TEST(! ascii_isupper('@'));
        TEST(ascii_isupper('A'));
        TEST(ascii_isupper('Z'));
        TEST(! ascii_isupper('['));
        TEST(! ascii_isupper('_'));
        TEST(! ascii_isupper('`'));
        TEST(! ascii_isupper('a'));
        TEST(! ascii_isupper('z'));
        TEST(! ascii_isupper('{'));
        TEST(! ascii_isupper('~'));
        TEST(! ascii_isupper('\x7f'));
        TEST(! ascii_isupper('\x80'));

        TEST(! ascii_isxdigit('\0'));
        TEST(! ascii_isxdigit('\t'));
        TEST(! ascii_isxdigit('\n'));
        TEST(! ascii_isxdigit('\x1f'));
        TEST(! ascii_isxdigit(' '));
        TEST(! ascii_isxdigit('!'));
        TEST(! ascii_isxdigit('/'));
        TEST(ascii_isxdigit('0'));
        TEST(ascii_isxdigit('9'));
        TEST(! ascii_isxdigit(':'));
        TEST(! ascii_isxdigit('@'));
        TEST(ascii_isxdigit('A'));
        TEST(!ascii_isxdigit('Z'));
        TEST(!ascii_isxdigit('['));
        TEST(!ascii_isxdigit('_'));
        TEST(!ascii_isxdigit('`'));
        TEST(ascii_isxdigit('a'));
        TEST(! ascii_isxdigit('z'));
        TEST(! ascii_isxdigit('{'));
        TEST(! ascii_isxdigit('~'));
        TEST(! ascii_isxdigit('\x7f'));
        TEST(! ascii_isxdigit('\x80'));

        TEST(! ascii_isalnum_w('\0'));
        TEST(! ascii_isalnum_w('\t'));
        TEST(! ascii_isalnum_w('\n'));
        TEST(! ascii_isalnum_w('\x1f'));
        TEST(! ascii_isalnum_w(' '));
        TEST(! ascii_isalnum_w('!'));
        TEST(! ascii_isalnum_w('/'));
        TEST(ascii_isalnum_w('0'));
        TEST(ascii_isalnum_w('9'));
        TEST(! ascii_isalnum_w(':'));
        TEST(! ascii_isalnum_w('@'));
        TEST(ascii_isalnum_w('A'));
        TEST(ascii_isalnum_w('Z'));
        TEST(! ascii_isalnum_w('['));
        TEST(ascii_isalnum_w('_'));
        TEST(! ascii_isalnum_w('`'));
        TEST(ascii_isalnum_w('a'));
        TEST(ascii_isalnum_w('z'));
        TEST(! ascii_isalnum_w('{'));
        TEST(! ascii_isalnum_w('~'));
        TEST(! ascii_isalnum_w('\x7f'));
        TEST(! ascii_isalnum_w('\x80'));

        TEST(! ascii_isalpha_w('\0'));
        TEST(! ascii_isalpha_w('\t'));
        TEST(! ascii_isalpha_w('\n'));
        TEST(! ascii_isalpha_w('\x1f'));
        TEST(! ascii_isalpha_w(' '));
        TEST(! ascii_isalpha_w('!'));
        TEST(! ascii_isalpha_w('/'));
        TEST(! ascii_isalpha_w('0'));
        TEST(! ascii_isalpha_w('9'));
        TEST(! ascii_isalpha_w(':'));
        TEST(! ascii_isalpha_w('@'));
        TEST(ascii_isalpha_w('A'));
        TEST(ascii_isalpha_w('Z'));
        TEST(! ascii_isalpha_w('['));
        TEST(ascii_isalpha_w('_'));
        TEST(! ascii_isalpha_w('`'));
        TEST(ascii_isalpha_w('a'));
        TEST(ascii_isalpha_w('z'));
        TEST(! ascii_isalpha_w('{'));
        TEST(! ascii_isalpha_w('~'));
        TEST(! ascii_isalpha_w('\x7f'));
        TEST(! ascii_isalpha_w('\x80'));

        TEST(! ascii_ispunct_w('\0'));
        TEST(! ascii_ispunct_w('\t'));
        TEST(! ascii_ispunct_w('\n'));
        TEST(! ascii_ispunct_w('\x1f'));
        TEST(! ascii_ispunct_w(' '));
        TEST(ascii_ispunct_w('!'));
        TEST(ascii_ispunct_w('/'));
        TEST(! ascii_ispunct_w('0'));
        TEST(! ascii_ispunct_w('9'));
        TEST(ascii_ispunct_w(':'));
        TEST(ascii_ispunct_w('@'));
        TEST(! ascii_ispunct_w('A'));
        TEST(! ascii_ispunct_w('Z'));
        TEST(ascii_ispunct_w('['));
        TEST(! ascii_ispunct_w('_'));
        TEST(ascii_ispunct_w('`'));
        TEST(! ascii_ispunct_w('a'));
        TEST(! ascii_ispunct_w('z'));
        TEST(ascii_ispunct_w('{'));
        TEST(ascii_ispunct_w('~'));
        TEST(! ascii_ispunct_w('\x7f'));
        TEST(! ascii_ispunct_w('\x80'));

        TEST_EQUAL(ascii_tolower('A'), 'a');
        TEST_EQUAL(ascii_tolower('a'), 'a');
        TEST_EQUAL(ascii_tolower('\0'), '\0');
        TEST_EQUAL(ascii_tolower('@'), '@');
        TEST_EQUAL(ascii_tolower('\x7f'), '\x7f');
        TEST_EQUAL(ascii_tolower('\x80'), '\x80');
        TEST_EQUAL(ascii_tolower('\xff'), '\xff');

        TEST_EQUAL(ascii_toupper('A'), 'A');
        TEST_EQUAL(ascii_toupper('a'), 'A');
        TEST_EQUAL(ascii_toupper('\0'), '\0');
        TEST_EQUAL(ascii_toupper('@'), '@');
        TEST_EQUAL(ascii_toupper('\x7f'), '\x7f');
        TEST_EQUAL(ascii_toupper('\x80'), '\x80');
        TEST_EQUAL(ascii_toupper('\xff'), '\xff');

        TEST_EQUAL(char_to<int>(0), 0);
        TEST_EQUAL(char_to<int>('A'), 65);
        TEST_EQUAL(char_to<int>(char(127)), 127);
        TEST_EQUAL(char_to<int>(char(128)), 128);
        TEST_EQUAL(char_to<int>(char(-128)), 128);
        TEST_EQUAL(char_to<int>(char(255)), 255);
        TEST_EQUAL(char_to<int>(char(-1)), 255);

        TEST_EQUAL(char_to<uint32_t>(0), 0);
        TEST_EQUAL(char_to<uint32_t>('A'), 65);
        TEST_EQUAL(char_to<uint32_t>(char(127)), 127);
        TEST_EQUAL(char_to<uint32_t>(char(128)), 128);
        TEST_EQUAL(char_to<uint32_t>(char(-128)), 128);
        TEST_EQUAL(char_to<uint32_t>(char(255)), 255);
        TEST_EQUAL(char_to<uint32_t>(char(-1)), 255);

    }

    void check_containers() {

        using buf = SimpleBuffer<int32_t>;

        std::shared_ptr<buf> bp;
        int32_t array[] = {1,2,3,4,5};

        TRY(bp.reset(new buf));
        TEST(bp->empty());

        TRY(bp.reset(new buf(10)));
        TEST_EQUAL(bp->size(), 10);

        TRY(bp.reset(new buf(20, 42)));
        TEST_EQUAL(bp->size(), 20);
        TEST_EQUAL(bp->at(0), 42);
        TEST_THROW(bp->at(20), std::out_of_range);

        TRY(bp->copy(array, 5));
        TEST_EQUAL(bp->size(), 5);
        TEST_EQUAL(bp->at(0), 1);
        TEST_EQUAL(bp->at(4), 5);

        TRY(bp->copy(array + 0, array + 5));
        TEST_EQUAL(bp->size(), 5);
        TEST_EQUAL(bp->at(0), 1);
        TEST_EQUAL(bp->at(4), 5);

        buf b1, b2;

        TRY(b1.assign(10));
        TEST_EQUAL(b1.size(), 10);
        TEST_EQUAL(b1.bytes(), 40);

        TRY(b1.assign(20, 42));
        TEST_EQUAL(b1.size(), 20);
        TEST_EQUAL(b1.bytes(), 80);
        TEST_EQUAL(b1[0], 42);
        TEST_EQUAL(b1.at(0), 42);
        TEST_THROW(b1.at(20), std::out_of_range);

        TRY(b1.copy(array, 5));
        TEST_EQUAL(b1.size(), 5);
        TEST_EQUAL(b1.bytes(), 20);
        TEST_EQUAL(b1[0], 1);
        TEST_EQUAL(b1[4], 5);

        TRY(b1.copy(array + 0, array + 5));
        TEST_EQUAL(b1.size(), 5);
        TEST_EQUAL(b1.bytes(), 20);
        TEST_EQUAL(b1[0], 1);
        TEST_EQUAL(b1[4], 5);

        TRY(b2.assign(20, 42));
        TRY(b1 = b2);
        TEST_EQUAL(b1.size(), 20);
        TEST_EQUAL(b1[0], 42);

        TRY(b2.assign(10, 99));
        TRY(b1 = std::move(b2));
        TEST_EQUAL(b1.size(), 10);
        TEST_EQUAL(b1[0], 99);
        TEST_EQUAL(b2.size(), 0);

        TRY(bp.reset(new buf(b1)));
        TEST_EQUAL(bp->size(), 10);
        TEST_EQUAL(bp->at(0), 99);

        TRY(bp.reset(new buf(std::move(b1))));
        TEST_EQUAL(bp->size(), 10);
        TEST_EQUAL(bp->at(0), 99);
        TEST_EQUAL(b1.size(), 0);

    }

    void check_exceptions() {

        u8string s;
        TRY(s = CrtError::translate(EDOM));
        TEST_MATCH(s, "[Dd]omain");
        try {
            throw CrtError(EDOM, "SomeFunction()");
        }
        catch (const std::exception& ex) {
            TEST_MATCH(string(ex.what()), "^SomeFunction\\(\\): Error \\d+: .*[Dd]omain");
        }

        #if defined(PRI_TARGET_WINDOWS)

            TRY(s = WindowsError::translate(ERROR_INVALID_FUNCTION));
            TEST_EQUAL(s, "Incorrect function.");
            try {
                throw WindowsError(ERROR_INVALID_FUNCTION, "SomeFunction()");
            }
            catch (const std::exception& ex) {
                TEST_EQUAL(string(ex.what()), "SomeFunction(): Error 1: Incorrect function.");
            }

        #endif

    }

    void check_flagset() {

        TEST_EQUAL(Flagset::value('A'), 1);
        TEST_EQUAL(Flagset::value('B'), 2);
        TEST_EQUAL(Flagset::value('C'), 4);
        TEST_EQUAL(Flagset::value('Z'), 0x2000000ull);
        TEST_EQUAL(Flagset::value('a'), 0x4000000ull);
        TEST_EQUAL(Flagset::value('b'), 0x8000000ull);
        TEST_EQUAL(Flagset::value('c'), 0x10000000ull);
        TEST_EQUAL(Flagset::value('z'), 0x8000000000000ull);
        TEST_EQUAL(Flagset::value('#'), 0x10000000000000ull);
        TEST_EQUAL(Flagset::value('$'), 0x20000000000000ull);
        TEST_EQUAL(Flagset::value('%'), 0x40000000000000ull);
        TEST_EQUAL(Flagset::value('&'), 0x80000000000000ull);
        TEST_EQUAL(Flagset::value('*'), 0x100000000000000ull);
        TEST_EQUAL(Flagset::value('+'), 0x200000000000000ull);
        TEST_EQUAL(Flagset::value('-'), 0x400000000000000ull);
        TEST_EQUAL(Flagset::value('/'), 0x800000000000000ull);
        TEST_EQUAL(Flagset::value('<'), 0x1000000000000000ull);
        TEST_EQUAL(Flagset::value('='), 0x2000000000000000ull);
        TEST_EQUAL(Flagset::value('>'), 0x4000000000000000ull);
        TEST_EQUAL(Flagset::value('@'), 0x8000000000000000ull);

        TEST_EQUAL(Flagset::cvalue(1), 'A');
        TEST_EQUAL(Flagset::cvalue(2), 'B');
        TEST_EQUAL(Flagset::cvalue(4), 'C');
        TEST_EQUAL(Flagset::cvalue(0x2000000ull), 'Z');
        TEST_EQUAL(Flagset::cvalue(0x4000000ull), 'a');
        TEST_EQUAL(Flagset::cvalue(0x8000000ull), 'b');
        TEST_EQUAL(Flagset::cvalue(0x10000000ull), 'c');
        TEST_EQUAL(Flagset::cvalue(0x8000000000000ull), 'z');
        TEST_EQUAL(Flagset::cvalue(0x10000000000000ull), '#');
        TEST_EQUAL(Flagset::cvalue(0x20000000000000ull), '$');
        TEST_EQUAL(Flagset::cvalue(0x40000000000000ull), '%');
        TEST_EQUAL(Flagset::cvalue(0x80000000000000ull), '&');
        TEST_EQUAL(Flagset::cvalue(0x100000000000000ull), '*');
        TEST_EQUAL(Flagset::cvalue(0x200000000000000ull), '+');
        TEST_EQUAL(Flagset::cvalue(0x400000000000000ull), '-');
        TEST_EQUAL(Flagset::cvalue(0x800000000000000ull), '/');
        TEST_EQUAL(Flagset::cvalue(0x1000000000000000ull), '<');
        TEST_EQUAL(Flagset::cvalue(0x2000000000000000ull), '=');
        TEST_EQUAL(Flagset::cvalue(0x4000000000000000ull), '>');
        TEST_EQUAL(Flagset::cvalue(0x8000000000000000ull), '@');

        constexpr uint64_t ALPHA = Flagset::value('A');
        constexpr uint64_t BRAVO = Flagset::value('B');
        constexpr uint64_t CHARLIE = Flagset::value('C');
        constexpr uint64_t DELTA = Flagset::value('D');
        constexpr uint64_t ZULU = Flagset::value('Z');
        constexpr uint64_t alpha = Flagset::value('a');
        constexpr uint64_t bravo = Flagset::value('b');
        constexpr uint64_t charlie = Flagset::value('c');
        constexpr uint64_t delta = Flagset::value('d');
        constexpr uint64_t zulu = Flagset::value('z');

        TEST_EQUAL(ALPHA, 1);
        TEST_EQUAL(BRAVO, 2);
        TEST_EQUAL(CHARLIE, 4);
        TEST_EQUAL(ZULU, 0x2000000ull);
        TEST_EQUAL(alpha, 0x4000000ull);
        TEST_EQUAL(bravo, 0x8000000ull);
        TEST_EQUAL(charlie, 0x10000000ull);
        TEST_EQUAL(zulu, 0x8000000000000ull);

        Flagset f;

        TEST(f.empty());
        TEST(! f.get(ALPHA));
        TEST(! f.get(alpha));
        TEST(! f.getc('A'));
        TEST(! f.getc('a'));
        TEST(! f.gets("abc"));
        TEST(! f.gets("abc"s));
        TEST_EQUAL(f.get(), 0);
        TEST_EQUAL(f.str(), "0");
        TEST_EQUAL(f.ustr<char32_t>(), U"0");

        TRY(f = ALPHA);
        TEST(! f.empty());
        TEST(f.get(ALPHA));
        TEST(! f.get(alpha));
        TEST(f.getc('A'));
        TEST(! f.getc('a'));
        TEST(! f.gets("abc"));
        TEST(! f.gets("abc"s));
        TEST_EQUAL(f.get(), ALPHA);
        TEST_EQUAL(f.str(), "A");
        TEST_EQUAL(f.ustr<char32_t>(), U"A");

        TRY(f = alpha);
        TEST(! f.empty());
        TEST(! f.get(ALPHA));
        TEST(f.get(alpha));
        TEST(! f.getc('A'));
        TEST(f.getc('a'));
        TEST(f.gets("abc"));
        TEST(f.gets("abc"s));
        TEST_EQUAL(f.get(), alpha);
        TEST_EQUAL(f.str(), "a");
        TEST_EQUAL(f.ustr<char32_t>(), U"a");

        TRY(f = ALPHA | alpha);
        TEST(! f.empty());
        TEST(f.get(ALPHA));
        TEST(f.get(alpha));
        TEST(f.getc('A'));
        TEST(f.getc('a'));
        TEST(f.gets("abc"));
        TEST(f.gets("abc"s));
        TEST_EQUAL(f.get(), ALPHA | alpha);
        TEST_EQUAL(f.str(), "Aa");
        TEST_EQUAL(f.ustr<char32_t>(), U"Aa");

        TRY(f = "abc");
        TEST(! f.empty());
        TEST(! f.get(ALPHA));
        TEST(! f.get(BRAVO));
        TEST(! f.get(CHARLIE));
        TEST(! f.get(DELTA));
        TEST(f.get(alpha));
        TEST(f.get(bravo));
        TEST(f.get(charlie));
        TEST(! f.get(delta));
        TEST(! f.getc('A'));
        TEST(! f.getc('B'));
        TEST(! f.getc('C'));
        TEST(! f.getc('D'));
        TEST(f.getc('a'));
        TEST(f.getc('b'));
        TEST(f.getc('c'));
        TEST(! f.getc('d'));
        TEST(f.gets("abc"));
        TEST(f.gets("abc"s));
        TEST_EQUAL(f.get(), alpha | bravo | charlie);
        TEST_EQUAL(f.str(), "abc");
        TEST_EQUAL(f.ustr<char32_t>(), U"abc");

        TRY(f = 0);
        TEST(f.empty());
        TEST(! f.get(ALPHA));
        TEST(! f.get(alpha));
        TEST(! f.getc('A'));
        TEST(! f.getc('a'));
        TEST(! f.gets("abc"));
        TEST(! f.gets("abc"s));
        TEST_EQUAL(f.get(), 0);
        TEST_EQUAL(f.str(), "0");
        TEST_EQUAL(f.ustr<char32_t>(), U"0");

        TRY(f.set(ALPHA));
        TEST(f.get(ALPHA));
        TEST(! f.get(alpha));
        TEST(f.getc('A'));
        TEST(! f.getc('a'));
        TEST_EQUAL(f.get(), ALPHA);
        TEST_EQUAL(f.str(), "A");
        TEST_EQUAL(f.ustr<char32_t>(), U"A");

        TRY(f.setc('a'));
        TEST(f.get(ALPHA));
        TEST(f.get(alpha));
        TEST(f.getc('A'));
        TEST(f.getc('a'));
        TEST_EQUAL(f.get(), ALPHA | alpha);
        TEST_EQUAL(f.str(), "Aa");
        TEST_EQUAL(f.ustr<char32_t>(), U"Aa");

        TRY(f.set(ALPHA, false));
        TEST(! f.get(ALPHA));
        TEST(f.get(alpha));
        TEST(! f.getc('A'));
        TEST(f.getc('a'));
        TEST_EQUAL(f.get(), alpha);
        TEST_EQUAL(f.str(), "a");
        TEST_EQUAL(f.ustr<char32_t>(), U"a");

        TRY(f.setc('a', false));
        TEST(! f.get(ALPHA));
        TEST(! f.get(alpha));
        TEST(! f.getc('A'));
        TEST(! f.getc('a'));
        TEST_EQUAL(f.get(), 0);
        TEST_EQUAL(f.str(), "0");
        TEST_EQUAL(f.ustr<char32_t>(), U"0");

        TRY(f = "abc+-*/");
        TEST(f.getc('+'));
        TEST(f.getc('-'));
        TEST(f.getc('*'));
        TEST(f.getc('/'));
        TEST(! f.getc('<'));
        TEST(! f.getc('='));
        TEST(! f.getc('>'));
        TEST(! f.getc(' '));
        TEST_EQUAL(f.str(), "abc*+-/");
        TEST_EQUAL(f.ustr<char32_t>(), U"abc*+-/");

        TRY(f = "xyz<=>");
        TEST(! f.getc('+'));
        TEST(! f.getc('-'));
        TEST(! f.getc('*'));
        TEST(! f.getc('/'));
        TEST(f.getc('<'));
        TEST(f.getc('='));
        TEST(f.getc('>'));
        TEST(! f.getc(' '));
        TEST_EQUAL(f.str(), "xyz<=>");
        TEST_EQUAL(f.ustr<char32_t>(), U"xyz<=>");
        TEST_COMPARE(Flagset(""), ==, Flagset(""));
        TEST_COMPARE(Flagset("ABC"), ==, Flagset("ABC"));
        TEST_COMPARE(Flagset("abc"), ==, Flagset("abc"));
        TEST_COMPARE(Flagset("abc"), ==, Flagset("cba"));
        TEST_COMPARE(Flagset("[abc]"), ==, Flagset("{abc}"));
        TEST_COMPARE(Flagset("abc"), ==, Flagset("aaabbbccc"));

        TRY(f = "(a, b, c)");
        TEST(f.getc('a'));
        TEST(f.getc('b'));
        TEST(f.getc('c'));
        TEST(! f.getc('d'));
        TEST(! f.getc(' '));
        TEST(! f.getc('('));
        TEST(! f.getc(')'));
        TEST(! f.getc(','));

        TRY(f = "abc");
        TRY(f.allow("abc", "test"));
        TRY(f.allow("abcdef", "test"));
        TEST_THROW_EQUAL(f.allow("", "test"), FlagError, "Invalid test flags: \"abc\"");
        TEST_THROW_EQUAL(f.allow("bcd", "test"), FlagError, "Invalid test flags: \"abc\"");
        TEST_THROW_EQUAL(f.allow("cde", "test"), FlagError, "Invalid test flags: \"abc\"");
        TEST_THROW_EQUAL(f.allow("def", "test"), FlagError, "Invalid test flags: \"abc\"");
        TRY(f.exclusive("", "test"));
        TRY(f.exclusive("cde", "test"));
        TRY(f.exclusive("def", "test"));
        TEST_THROW_EQUAL(f.exclusive("abc", "test"), FlagError, "Invalid test flags: \"abc\"");
        TEST_THROW_EQUAL(f.exclusive("bcd", "test"), FlagError, "Invalid test flags: \"abc\"");

    }

    class Counted {
    public:
        Counted() { ++num; }
        Counted(const Counted&) { ++num; }
        Counted(Counted&&) = default;
        ~Counted() { --num; }
        Counted& operator=(const Counted&) = default;
        Counted& operator=(Counted&&) = default;
        static int count() { return num; }
        static void reset() { num = 0; }
    private:
        static int num;
    };

    int Counted::num = 0;

    void check_functional_utilities() {

        auto f1 = [] (int x) { return x * x; };
        auto sf1 = stdfun(f1);
        TEST_TYPE_OF(sf1, std::function<int(int)>);
        TEST_EQUAL(sf1(5), 25);

        int z = 0;
        auto f2 = [&] (int x, int y) { z = x * y; };
        auto sf2 = stdfun(f2);
        TEST_TYPE_OF(sf2, std::function<void(int, int)>);
        TRY(sf2(6, 7));
        TEST_EQUAL(z, 42);

        Counted::reset();
        TEST_EQUAL(Counted::count(), 0);
        Counted* cp = nullptr;
        std::shared_ptr<Counted> csp;
        TRY(csp.reset(new Counted));
        TEST_EQUAL(Counted::count(), 1);
        TRY(csp.reset());
        TEST_EQUAL(Counted::count(), 0);
        TRY(csp.reset(new Counted, do_nothing));
        TEST_EQUAL(Counted::count(), 1);
        cp = csp.get();
        TRY(csp.reset());
        TEST_EQUAL(Counted::count(), 1);
        delete cp;
        TEST_EQUAL(Counted::count(), 0);

        int n1 = 42, n2 = 0;
        u8string s1 = "Hello world", s2;
        u8string& sr(s1);
        const u8string& csr(s1);
        TRY(n2 = identity(n1));  TEST_EQUAL(n2, 42);
        TRY(n2 = identity(100));  TEST_EQUAL(n2, 100);
        TRY(s2 = identity(s1));  TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity(sr));  TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity(csr));  TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity("Goodbye"));  TEST_EQUAL(s2, "Goodbye");
        TRY(s2 = identity("Hello again"s));  TEST_EQUAL(s2, "Hello again");

    }

    void check_hash_functions() {

        size_t h1 = 0, h2 = 0, h3 = 0, h4 = 0, h5 = 0;
        string s = "hello";
        TRY(hash_combine(h1, 42));
        TRY(hash_combine(h2, 42, s));
        TRY(hash_combine(h3, 86, 99, s));
        TEST_COMPARE(h1, !=, 0);
        TEST_COMPARE(h2, !=, 0);
        TEST_COMPARE(h3, !=, 0);
        TEST_COMPARE(h1, !=, h2);
        TEST_COMPARE(h1, !=, h3);
        TEST_COMPARE(h2, !=, h3);
        std::vector<string> sv {"hello", "world", "goodbye"};
        for (auto& s: sv)
            TRY(hash_combine(h4, s));
        TRY(h5 = hash_range(sv));
        TEST_EQUAL(h4, h5);

    }

    constexpr Kwarg<int> kw_alpha;
    constexpr Kwarg<bool> kw_bravo;
    constexpr Kwarg<string> kw_charlie;

    struct Kwtest {
        int a = 0;
        bool b = false;
        string c;
        template <typename... Args> int fun(const Args&... args) {
            auto i = int(kwget(kw_alpha, a, args...));
            auto j = int(kwget(kw_bravo, b, args...));
            auto k = int(kwget(kw_charlie, c, args...));
            return (i << 2) + (j << 1) + k;
        }
    };

    class TempFile {
    public:
        explicit TempFile(const u8string& file): f(file) {}
        ~TempFile() { remove(f.data()); }
    private:
        u8string f;
        TempFile(const TempFile&) = delete;
        TempFile(TempFile&&) = delete;
        TempFile& operator=(const TempFile&) = delete;
        TempFile& operator=(TempFile&&) = delete;
    };

    void check_io_utilities() {

        string file = "__test__", s;
        TempFile tempfile(file);

        TEST(load_file("README.md"s, s));
        TEST_EQUAL(s.substr(0, 18), "# Prion Library #\n");
        TEST(save_file(file, "Hello world\n"s));
        TEST(load_file(file, s));
        TEST_EQUAL(s, "Hello world\n");
        TEST(save_file(file, "Goodbye\n"s, true));
        TEST(load_file(file, s));
        TEST_EQUAL(s, "Hello world\nGoodbye\n");
        TEST(! load_file("__no_such_file__", s));

        #if defined(PRI_TARGET_NATIVE_WINDOWS)

            wstring wfile = L"__test__";

            TEST(load_file(L"README.md"s, s));
            TEST_EQUAL(s.substr(0, 18), "# Prion Library #\n");
            TEST(save_file(wfile, "Hello world\n"s));
            TEST(load_file(wfile, s));
            TEST_EQUAL(s, "Hello world\n");
            TEST(save_file(wfile, "Goodbye\n"s, true));
            TEST(load_file(wfile, s));
            TEST_EQUAL(s, "Hello world\nGoodbye\n");
            TEST(! load_file(L"__no_such_file__", s));

        #endif

        TRY(s = xt_up);              TEST_EQUAL(s, "\x1b[A");
        TRY(s = xt_down);            TEST_EQUAL(s, "\x1b[B");
        TRY(s = xt_right);           TEST_EQUAL(s, "\x1b[C");
        TRY(s = xt_left);            TEST_EQUAL(s, "\x1b[D");
        TRY(s = xt_erase_left);      TEST_EQUAL(s, "\x1b[1K");
        TRY(s = xt_erase_right);     TEST_EQUAL(s, "\x1b[K");
        TRY(s = xt_erase_above);     TEST_EQUAL(s, "\x1b[1J");
        TRY(s = xt_erase_below);     TEST_EQUAL(s, "\x1b[J");
        TRY(s = xt_erase_line);      TEST_EQUAL(s, "\x1b[2K");
        TRY(s = xt_clear);           TEST_EQUAL(s, "\x1b[2J");
        TRY(s = xt_reset);           TEST_EQUAL(s, "\x1b[0m");
        TRY(s = xt_bold);            TEST_EQUAL(s, "\x1b[1m");
        TRY(s = xt_under);           TEST_EQUAL(s, "\x1b[4m");
        TRY(s = xt_black);           TEST_EQUAL(s, "\x1b[30m");
        TRY(s = xt_red);             TEST_EQUAL(s, "\x1b[31m");
        TRY(s = xt_green);           TEST_EQUAL(s, "\x1b[32m");
        TRY(s = xt_yellow);          TEST_EQUAL(s, "\x1b[33m");
        TRY(s = xt_blue);            TEST_EQUAL(s, "\x1b[34m");
        TRY(s = xt_magenta);         TEST_EQUAL(s, "\x1b[35m");
        TRY(s = xt_cyan);            TEST_EQUAL(s, "\x1b[36m");
        TRY(s = xt_white);           TEST_EQUAL(s, "\x1b[37m");
        TRY(s = xt_black_bg);        TEST_EQUAL(s, "\x1b[40m");
        TRY(s = xt_red_bg);          TEST_EQUAL(s, "\x1b[41m");
        TRY(s = xt_green_bg);        TEST_EQUAL(s, "\x1b[42m");
        TRY(s = xt_yellow_bg);       TEST_EQUAL(s, "\x1b[43m");
        TRY(s = xt_blue_bg);         TEST_EQUAL(s, "\x1b[44m");
        TRY(s = xt_magenta_bg);      TEST_EQUAL(s, "\x1b[45m");
        TRY(s = xt_cyan_bg);         TEST_EQUAL(s, "\x1b[46m");
        TRY(s = xt_white_bg);        TEST_EQUAL(s, "\x1b[47m");
        TRY(s = xt_move_up(42));     TEST_EQUAL(s, "\x1b[42A");
        TRY(s = xt_move_down(42));   TEST_EQUAL(s, "\x1b[42B");
        TRY(s = xt_move_right(42));  TEST_EQUAL(s, "\x1b[42C");
        TRY(s = xt_move_left(42));   TEST_EQUAL(s, "\x1b[42D");
        TRY(s = xt_colour(111));     TEST_EQUAL(s, "\x1b[38;5;16m");
        TRY(s = xt_colour(234));     TEST_EQUAL(s, "\x1b[38;5;67m");
        TRY(s = xt_colour(654));     TEST_EQUAL(s, "\x1b[38;5;223m");
        TRY(s = xt_colour(666));     TEST_EQUAL(s, "\x1b[38;5;231m");
        TRY(s = xt_colour_bg(111));  TEST_EQUAL(s, "\x1b[48;5;16m");
        TRY(s = xt_colour_bg(234));  TEST_EQUAL(s, "\x1b[48;5;67m");
        TRY(s = xt_colour_bg(654));  TEST_EQUAL(s, "\x1b[48;5;223m");
        TRY(s = xt_colour_bg(666));  TEST_EQUAL(s, "\x1b[48;5;231m");
        TRY(s = xt_grey(1));         TEST_EQUAL(s, "\x1b[38;5;232m");
        TRY(s = xt_grey(10));        TEST_EQUAL(s, "\x1b[38;5;241m");
        TRY(s = xt_grey(24));        TEST_EQUAL(s, "\x1b[38;5;255m");
        TRY(s = xt_grey_bg(1));      TEST_EQUAL(s, "\x1b[48;5;232m");
        TRY(s = xt_grey_bg(10));     TEST_EQUAL(s, "\x1b[48;5;241m");
        TRY(s = xt_grey_bg(24));     TEST_EQUAL(s, "\x1b[48;5;255m");

    }

    void check_keyword_arguments() {

        Kwtest k;
        TEST_EQUAL(k.fun(), 0b000);
        TEST_EQUAL(k.a, 0);
        TEST_EQUAL(k.b, false);
        TEST_EQUAL(k.c, "");

        k = {};
        TEST_EQUAL(k.fun(kw_alpha = 42), 0b100);
        TEST_EQUAL(k.a, 42);
        TEST_EQUAL(k.b, false);
        TEST_EQUAL(k.c, "");

        k = {};
        TEST_EQUAL(k.fun(kw_bravo = true), 0b010);
        TEST_EQUAL(k.a, 0);
        TEST_EQUAL(k.b, true);
        TEST_EQUAL(k.c, "");

        k = {};
        TEST_EQUAL(k.fun(kw_bravo), 0b010);
        TEST_EQUAL(k.a, 0);
        TEST_EQUAL(k.b, true);
        TEST_EQUAL(k.c, "");

        k = {};
        TEST_EQUAL(k.fun(kw_alpha = 42, kw_bravo = true, kw_charlie = "hello"s), 0b111);
        TEST_EQUAL(k.a, 42);
        TEST_EQUAL(k.b, true);
        TEST_EQUAL(k.c, "hello");

        k = {};
        TEST_EQUAL(k.fun(kw_alpha = 42L, kw_bravo = 1, kw_charlie = "hello"), 0b111);
        TEST_EQUAL(k.a, 42);
        TEST_EQUAL(k.b, true);
        TEST_EQUAL(k.c, "hello");

        k = {};
        TEST_EQUAL(k.fun(kw_alpha = 42, kw_bravo, kw_charlie = "hello"), 0b111);
        TEST_EQUAL(k.a, 42);
        TEST_EQUAL(k.b, true);
        TEST_EQUAL(k.c, "hello");

        k = {};
        TEST_EQUAL(k.fun(kw_charlie = "hello", kw_alpha = 42), 0b101);
        TEST_EQUAL(k.a, 42);
        TEST_EQUAL(k.b, false);
        TEST_EQUAL(k.c, "hello");

    }

    void check_range_utilities() {

        string s1, s2;

        TEST_TYPE(RangeIterator<string>, string::iterator);
        TEST_TYPE(RangeIterator<const string>, string::const_iterator);
        TEST_TYPE(RangeIterator<std::vector<int>>, std::vector<int>::iterator);
        TEST_TYPE(RangeIterator<const std::vector<int>>, std::vector<int>::const_iterator);
        TEST_TYPE(RangeIterator<Irange<int*>>, int*);
        TEST_TYPE(RangeIterator<Irange<const int*>>, const int*);

        TEST_TYPE(RangeValue<string>, char);
        TEST_TYPE(RangeValue<std::vector<int>>, int);
        TEST_TYPE(RangeValue<Irange<int*>>, int);
        TEST_TYPE(RangeValue<Irange<const int*>>, int);

        s1 = "Hello";
        TRY(std::copy(PRI_BOUNDS(s1), append(s2)));
        TEST_EQUAL(s2, "Hello");
        s1 = " world";
        TRY(std::copy(PRI_BOUNDS(s1), append(s2)));
        TEST_EQUAL(s2, "Hello world");
        s1 = "Goodbye";
        TRY(std::copy(PRI_BOUNDS(s1), overwrite(s2)));
        TEST_EQUAL(s2, "Goodbye");

        std::set<int> set1, set2;
        set1 = {1, 2, 3};
        TRY(std::copy(PRI_BOUNDS(set1), append(set2)));
        TEST_EQUAL(to_str(set2), "[1,2,3]");
        set1 = {4, 5, 6};
        TRY(std::copy(PRI_BOUNDS(set1), append(set2)));
        TEST_EQUAL(to_str(set2), "[1,2,3,4,5,6]");
        set1 = {1, 2, 3};
        TRY(std::copy(PRI_BOUNDS(set1), append(set2)));
        TEST_EQUAL(to_str(set2), "[1,2,3,4,5,6]");
        set1 = {7, 8, 9};
        TRY(std::copy(PRI_BOUNDS(set1), overwrite(set2)));
        TEST_EQUAL(to_str(set2), "[7,8,9]");

        char array[] {'H','e','l','l','o'};
        const char* cp = array;
        auto av = array_range(array, sizeof(array));
        TEST_EQUAL(std::distance(PRI_BOUNDS(av)), 5);
        TRY(std::copy(PRI_BOUNDS(av), overwrite(s1)));
        TEST_EQUAL(s1, "Hello");
        auto avc = array_range(cp, sizeof(array));
        TEST_EQUAL(std::distance(PRI_BOUNDS(avc)), 5);
        TRY(std::copy(PRI_BOUNDS(avc), overwrite(s1)));
        TEST_EQUAL(s1, "Hello");

        TEST_EQUAL(array_count(""), 1);
        TEST_EQUAL(array_count("Hello"), 6);
        TEST_EQUAL(array_count(array), 5);
        TEST_EQUAL(range_count(""s), 0);
        TEST_EQUAL(range_count("Hello"s), 5);
        TEST_EQUAL(range_count(""), 1);
        TEST_EQUAL(range_count("Hello"), 6);
        TEST(range_empty(""s));
        TEST(! range_empty("Hello"s));

        TRY(s1 = to_str(seq(0)));           TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(1)));           TEST_EQUAL(s1, "[0]");
        TRY(s1 = to_str(seq(5)));           TEST_EQUAL(s1, "[0,1,2,3,4]");
        TRY(s1 = to_str(seq(1, 5)));        TEST_EQUAL(s1, "[1,2,3,4]");
        TRY(s1 = to_str(seq(1, 2)));        TEST_EQUAL(s1, "[1]");
        TRY(s1 = to_str(seq(1, 1)));        TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(1, 0)));        TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(0, 10, 2)));    TEST_EQUAL(s1, "[0,2,4,6,8]");
        TRY(s1 = to_str(seq(1, 10, 2)));    TEST_EQUAL(s1, "[1,3,5,7,9]");
        TRY(s1 = to_str(seq(0, -10, -2)));  TEST_EQUAL(s1, "[0,-2,-4,-6,-8]");
        TRY(s1 = to_str(seq(1, -10, -2)));  TEST_EQUAL(s1, "[1,-1,-3,-5,-7,-9]");
        TRY(s1 = to_str(seq(0, 10, -2)));   TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(0, -10, 2)));   TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(0, 2, 2)));     TEST_EQUAL(s1, "[0]");
        TRY(s1 = to_str(seq(0, 1, 2)));     TEST_EQUAL(s1, "[0]");
        TRY(s1 = to_str(seq(0, 0, 2)));     TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(0, -1, 2)));    TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(0, -2, -2)));   TEST_EQUAL(s1, "[0]");
        TRY(s1 = to_str(seq(0, -1, -2)));   TEST_EQUAL(s1, "[0]");
        TRY(s1 = to_str(seq(0, 0, -2)));    TEST_EQUAL(s1, "[]");
        TRY(s1 = to_str(seq(0, 1, -2)));    TEST_EQUAL(s1, "[]");

    }

    void check_scope_guards() {

        int n = 1;
        {
            ScopeExit x([&] { n = 2; });
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 2);

        n = 1;
        try {
            ScopeExit x([&] { n = 2; });
            TEST_EQUAL(n, 1);
            throw std::runtime_error("Hello");
        }
        catch (...) {}
        TEST_EQUAL(n, 2);

        n = 1;
        {
            ScopeSuccess x([&] { n = 2; });
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 2);

        n = 1;
        try {
            ScopeSuccess x([&] { n = 2; });
            TEST_EQUAL(n, 1);
            throw std::runtime_error("Hello");
        }
        catch (...) {}
        TEST_EQUAL(n, 1);

        n = 1;
        {
            ScopeFailure x([&] { n = 2; });
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 1);

        n = 1;
        try {
            ScopeFailure x([&] { n = 2; });
            TEST_EQUAL(n, 1);
            throw std::runtime_error("Hello");
        }
        catch (...) {}
        TEST_EQUAL(n, 2);

        {
            ScopeExit x(nullptr);
        }

        try {
            ScopeExit x(nullptr);
            throw std::runtime_error("Hello");
        }
        catch (...) {}

        {
            ScopeSuccess x(nullptr);
        }

        try {
            ScopeSuccess x(nullptr);
            throw std::runtime_error("Hello");
        }
        catch (...) {}

        {
            ScopeFailure x(nullptr);
        }

        try {
            ScopeFailure x(nullptr);
            throw std::runtime_error("Hello");
        }
        catch (...) {}

        string s;
        {
            Transaction t;
            TRY(t.call([&] { s += 'a'; }, [&] { s += 'z'; }));
            TRY(t.call([&] { s += 'b'; }, [&] { s += 'y'; }));
            TRY(t.call([&] { s += 'c'; }, [&] { s += 'x'; }));
            TEST_EQUAL(s, "abc");
        }
        TEST_EQUAL(s, "abcxyz");

        s.clear();
        {
            Transaction t;
            TRY(t.call([&] { s += 'a'; }, [&] { s += 'z'; }));
            TRY(t.call([&] { s += 'b'; }, [&] { s += 'y'; }));
            TRY(t.call([&] { s += 'c'; }, [&] { s += 'x'; }));
            TEST_EQUAL(s, "abc");
            TRY(t.rollback());
            TEST_EQUAL(s, "abcxyz");
        }
        TEST_EQUAL(s, "abcxyz");

        s.clear();
        {
            Transaction t;
            TRY(t.call([&] { s += 'a'; }, [&] { s += 'z'; }));
            TRY(t.call([&] { s += 'b'; }, [&] { s += 'y'; }));
            TRY(t.call([&] { s += 'c'; }, [&] { s += 'x'; }));
            TEST_EQUAL(s, "abc");
            TRY(t.commit());
            TEST_EQUAL(s, "abc");
            TRY(t.rollback());
            TEST_EQUAL(s, "abc");
        }
        TEST_EQUAL(s, "abc");

    }

    void check_string_functions() {

        string s;
        std::vector<string> sv;
        std::vector<int> iv;
        std::map<int, string> ism;

        const char* p0 = nullptr;
        const char* p1 = "";
        const char* p2 = "Hello";
        const char16_t* q0 = nullptr;
        const char16_t* q1 = u"";
        const char16_t* q2 = u"Hello";
        const char32_t* r0 = nullptr;
        const char32_t* r1 = U"";
        const char32_t* r2 = U"Hello";

        TEST_EQUAL(ascii_lowercase("Hello World"s), "hello world");
        TEST_EQUAL(ascii_uppercase("Hello World"s), "HELLO WORLD");
        TEST_EQUAL(ascii_titlecase("hello world"s), "Hello World");
        TEST_EQUAL(ascii_titlecase("HELLO WORLD"s), "Hello World");

        TEST_EQUAL(cstr(p0), ""s);
        TEST_EQUAL(cstr(p1), ""s);
        TEST_EQUAL(cstr(p2), "Hello"s);
        TEST_EQUAL(cstr(p0, 0), ""s);
        TEST_EQUAL(cstr(p1, 0), ""s);
        TEST_EQUAL(cstr(p2, 0), ""s);
        TEST_EQUAL(cstr(p0, 5), ""s);
        TEST_EQUAL(cstr(p2, 5), "Hello"s);
        TEST_EQUAL(cstr(q0), u""s);
        TEST_EQUAL(cstr(q1), u""s);
        TEST_EQUAL(cstr(q2), u"Hello"s);
        TEST_EQUAL(cstr(q0, 0), u""s);
        TEST_EQUAL(cstr(q1, 0), u""s);
        TEST_EQUAL(cstr(q2, 0), u""s);
        TEST_EQUAL(cstr(q0, 5), u""s);
        TEST_EQUAL(cstr(q2, 5), u"Hello"s);
        TEST_EQUAL(cstr(r0), U""s);
        TEST_EQUAL(cstr(r1), U""s);
        TEST_EQUAL(cstr(r2), U"Hello"s);
        TEST_EQUAL(cstr(r0, 0), U""s);
        TEST_EQUAL(cstr(r1, 0), U""s);
        TEST_EQUAL(cstr(r2, 0), U""s);
        TEST_EQUAL(cstr(r0, 5), U""s);
        TEST_EQUAL(cstr(r2, 5), U"Hello"s);

        TEST_EQUAL(cstr_size(p0), 0);
        TEST_EQUAL(cstr_size(p1), 0);
        TEST_EQUAL(cstr_size(p2), 5);
        TEST_EQUAL(cstr_size(q0), 0);
        TEST_EQUAL(cstr_size(q1), 0);
        TEST_EQUAL(cstr_size(q2), 5);
        TEST_EQUAL(cstr_size(r0), 0);
        TEST_EQUAL(cstr_size(r1), 0);
        TEST_EQUAL(cstr_size(r2), 5);

        TEST_EQUAL(dec(0), "0");
        TEST_EQUAL(dec(42), "42");
        TEST_EQUAL(dec(-42), "-42");
        TEST_EQUAL(dec(0, 4), "0000");
        TEST_EQUAL(dec(42, 4), "0042");
        TEST_EQUAL(dec(-42, 4), "-0042");

        TEST_EQUAL(hex(0, 1), "0");
        TEST_EQUAL(hex(0, 2), "00");
        TEST_EQUAL(hex(42, 1), "2a");
        TEST_EQUAL(hex(42, 2), "2a");
        TEST_EQUAL(hex(42, 3), "02a");
        TEST_EQUAL(hex(0xabcdef, 3), "abcdef");
        TEST_EQUAL(hex(0xabcdef, 6), "abcdef");
        TEST_EQUAL(hex(0xabcdef, 9), "000abcdef");
        TEST_EQUAL(hex(int8_t(0)), "00");
        TEST_EQUAL(hex(int16_t(0)), "0000");
        TEST_EQUAL(hex(int32_t(0)), "00000000");
        TEST_EQUAL(hex(int8_t(42)), "2a");
        TEST_EQUAL(hex(int16_t(42)), "002a");
        TEST_EQUAL(hex(int32_t(42)), "0000002a");
        TEST_EQUAL(hex(int8_t(-42)), "-2a");
        TEST_EQUAL(hex(int16_t(-42)), "-002a");
        TEST_EQUAL(hex(int32_t(-42)), "-0000002a");
        TEST_EQUAL(hex(uint32_t(0xabcdef)), "00abcdef");
        TEST_EQUAL(hex(uint64_t(0x123456789abcdefull)), "0123456789abcdef");

        TEST_EQUAL(decnum(""), 0);
        TEST_EQUAL(decnum("0"), 0);
        TEST_EQUAL(decnum("42"), 42);
        TEST_EQUAL(decnum("0042"), 42);
        TEST_EQUAL(decnum("+42"), 42);
        TEST_EQUAL(decnum("-42"), -42);
        TEST_EQUAL(decnum("9223372036854775806"), 9223372036854775806ll); // 2^63-2
        TEST_EQUAL(decnum("9223372036854775807"), 9223372036854775807ll); // 2^63-1
        TEST_EQUAL(decnum("9223372036854775808"), 9223372036854775807ll); // 2^63
        TEST_EQUAL(decnum("9223372036854775809"), 9223372036854775807ll); // 2^63+1
        TEST_EQUAL(decnum("-9223372036854775806"), -9223372036854775806ll); // -(2^63-2)
        TEST_EQUAL(decnum("-9223372036854775807"), -9223372036854775807ll); // -(2^63-1)
        TEST_EQUAL(decnum("-9223372036854775808"), -9223372036854775807ll-1); // -2^63
        TEST_EQUAL(decnum("-9223372036854775809"), -9223372036854775807ll-1); // -(2^63+1)

        TEST_EQUAL(hexnum(""), 0);
        TEST_EQUAL(hexnum("0"), 0);
        TEST_EQUAL(hexnum("42"), 66);
        TEST_EQUAL(hexnum("0042"), 66);
        TEST_EQUAL(hexnum("fffffffffffffffe"), 0xfffffffffffffffeull);
        TEST_EQUAL(hexnum("ffffffffffffffff"), 0xffffffffffffffffull);
        TEST_EQUAL(hexnum("10000000000000000"), 0xffffffffffffffffull);

        TEST_EQUAL(fpnum(""), 0);
        TEST_EQUAL(fpnum("42"), 42);
        TEST_EQUAL(fpnum("-42"), -42);
        TEST_EQUAL(fpnum("1.234e5"), 123400);
        TEST_EQUAL(fpnum("-1.234e5"), -123400);
        TEST_NEAR(fpnum("1.23e-4"), 0.000123);
        TEST_NEAR(fpnum("-1.23e-4"), -0.000123);
        TEST_EQUAL(fpnum("1e9999"), HUGE_VAL);
        TEST_EQUAL(fpnum("-1e9999"), - HUGE_VAL);

        TEST_EQUAL(hexdump(""s), "");
        TEST_EQUAL(hexdump(""s, 5), "");
        TEST_EQUAL(hexdump("Hello world!"s), "48 65 6c 6c 6f 20 77 6f 72 6c 64 21");
        TEST_EQUAL(hexdump("Hello world!"s, 5), "48 65 6c 6c 6f\n20 77 6f 72 6c\n64 21");
        TEST_EQUAL(hexdump("Hello world!"s, 6), "48 65 6c 6c 6f 20\n77 6f 72 6c 64 21");

        sv.clear();                      TEST_EQUAL(join_words(PRI_BOUNDS(sv)), "");
        sv = {"Hello"};                  TEST_EQUAL(join_words(PRI_BOUNDS(sv)), "Hello");
        sv = {"Hello","world"};          TEST_EQUAL(join_words(PRI_BOUNDS(sv)), "Hello world");
        sv = {"Hello","world","again"};  TEST_EQUAL(join_words(PRI_BOUNDS(sv)), "Hello world again");
        sv.clear();                      TEST_EQUAL(join_words(PRI_BOUNDS(sv), ""), "");
        sv = {"Hello"};                  TEST_EQUAL(join_words(PRI_BOUNDS(sv), ""), "Hello");
        sv = {"Hello","world"};          TEST_EQUAL(join_words(PRI_BOUNDS(sv), ""), "Helloworld");
        sv = {"Hello","world","again"};  TEST_EQUAL(join_words(PRI_BOUNDS(sv), ""), "Helloworldagain");
        sv.clear();                      TEST_EQUAL(join_words(PRI_BOUNDS(sv), "<*>"s), "");
        sv = {"Hello"};                  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "<*>"s), "Hello");
        sv = {"Hello","world"};          TEST_EQUAL(join_words(PRI_BOUNDS(sv), "<*>"s), "Hello<*>world");
        sv = {"Hello","world","again"};  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "<*>"s), "Hello<*>world<*>again");

        TRY(split_words("", overwrite(sv)));                         TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "");
        TRY(split_words("Hello", overwrite(sv)));                    TEST_EQUAL(sv.size(), 1);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "Hello");
        TRY(split_words("Hello world", overwrite(sv)));              TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "Hello/world");
        TRY(split_words("\t Hello \t world \t", overwrite(sv)));     TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "Hello/world");
        TRY(split_words("**Hello**world**"s, overwrite(sv), "*"));   TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "Hello/world");
        TRY(split_words("**Hello**world**"s, overwrite(sv), "*"s));  TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "Hello/world");
        TRY(split_words("*****"s, overwrite(sv), "@*"));             TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "");
        TRY(split_words("*****"s, overwrite(sv), "@*"s));            TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join_words(PRI_BOUNDS(sv), "/"), "");

        TRY(s = quote(""s));                            TEST_EQUAL(s, "\"\""s);
        TRY(s = quote("\"\""s));                        TEST_EQUAL(s, "\"\\\"\\\"\""s);
        TRY(s = quote("Hello world"s));                 TEST_EQUAL(s, "\"Hello world\""s);
        TRY(s = quote("\\Hello\\world\\"s));            TEST_EQUAL(s, "\"\\\\Hello\\\\world\\\\\""s);
        TRY(s = quote("\"Hello\" \"world\""s));         TEST_EQUAL(s, "\"\\\"Hello\\\" \\\"world\\\"\""s);
        TRY(s = quote("\t\n\f\r"s));                    TEST_EQUAL(s, "\"\\t\\n\\f\\r\""s);
        TRY(s = quote("\x00\x01\x7f\x80\xff"s));        TEST_EQUAL(s, "\"\\0\\x01\\x7f\\x80\\xff\""s);
        TRY(s = quote("\x00\x01\x7f\x80\xff"s, true));  TEST_EQUAL(s, "\"\\0\\x01\\x7f\x80\xff\""s);

        s = "Hello";

        TEST_EQUAL(to_str(0), "0");
        TEST_EQUAL(to_str(42), "42");
        TEST_EQUAL(to_str(-42), "-42");
        TEST_EQUAL(to_str(123.456), "123.456");
        TEST_EQUAL(to_str(s), "Hello");
        TEST_EQUAL(to_str(s.data()), "Hello");
        TEST_EQUAL(to_str(""s), "");
        TEST_EQUAL(to_str("Hello"s), "Hello");
        TEST_EQUAL(to_str(""), "");
        TEST_EQUAL(to_str("Hello"), "Hello");

        sv.clear();
        iv.clear();
        ism.clear();

        TEST_EQUAL(to_str(iv), "[]");
        TEST_EQUAL(to_str(sv), "[]");
        TEST_EQUAL(to_str(ism), "{}");

        iv = {1,2,3};
        sv = {"hello","world","goodbye"};
        ism = {{1,"hello"},{2,"world"},{3,"goodbye"}};

        TEST_EQUAL(to_str(iv), "[1,2,3]");
        TEST_EQUAL(to_str(sv), "[hello,world,goodbye]");
        TEST_EQUAL(to_str(ism), "{1:hello,2:world,3:goodbye}");

    }

    void check_threads() {

        TEST_COMPARE(Thread::cpu_threads(), >=, 1);
        TRY(Thread::yield());

        Mutex m;
        ConditionVariable cv;
        int n = 0;
        string s;

        {
            MutexLock lock(m);
            TEST(! cv.wait_for(lock, milliseconds(50), [&] { return n > 0; }));
            n = 42;
            TEST(cv.wait_for(lock, milliseconds(50), [&] { return n > 0; }));
        }

        {
            Thread t([&] { s = "Neddie"; });
            TEST_COMPARE(t.get_id(), !=, Thread::current());
            TRY(t.wait());
            TEST(t.poll());
            TEST_EQUAL(s, "Neddie");
        }

        {
            Thread t(0);
            TRY(t.wait());
            TEST(t.poll());
        }

        {
            Thread t([&] { throw std::runtime_error("Hello"); });
            TEST_THROW(t.wait(), std::runtime_error );
            TRY(t.wait());
        }

        {
            Thread t([&] { throw 42; });
            TEST_THROW(t.wait(), int );
            TRY(t.wait());
        }


        {
            auto f = [&] {
                MutexLock lock(m);
                TRY(cv.wait(lock, [&] { return ! s.empty(); }));
                s += "Seagoon";
            };
            Thread t(f);
            TEST(! t.poll());
            s = "Neddie";
            TRY(cv.notify_all());
            TRY(t.wait());
            TEST(t.poll());
            TEST_EQUAL(s, "NeddieSeagoon");
        }

        {
            s.clear();
            auto f1 = [&] {
                MutexLock lock(m);
                TRY(cv.wait(lock));
                s += "Seagoon";
            };
            auto f2 = [&] {
                TRY(sleep_for(0.05));
                MutexLock lock(m);
                s += "Neddie";
                TRY(cv.notify_all());
            };
            Thread t1(f1);
            Thread t2(f2);
            TRY(t1.wait());
            TRY(t2.wait());
            TEST_EQUAL(s, "NeddieSeagoon");
        }

        {
            s.clear();
            auto f1 = [&] {
                MutexLock lock(m);
                TRY(cv.wait(lock, [&] { return ! s.empty(); }));
                s += "Seagoon";
            };
            auto f2 = [&] {
                TRY(cv.notify_one());
                TRY(sleep_for(0.05));
                TRY(cv.notify_one());
                {
                    MutexLock lock(m);
                    s += "Neddie";
                }
                TRY(cv.notify_one());
            };
            Thread t1(f1);
            Thread t2(f2);
            TRY(t1.wait());
            TRY(t2.wait());
            TEST_EQUAL(s, "NeddieSeagoon");
        }

    }

    void check_time_and_date_functions() {

        using IntMsec = duration<int64_t, std::ratio<1, 1000>>;
        using DblSec = duration<double>;

        #if defined(PRI_TARGET_UNIX)
            using IntSec = duration<int64_t>;
            using IntDays = duration<int64_t, std::ratio<86400>>;
        #endif

        hours h;
        minutes m;
        seconds s;
        milliseconds ms;

        TRY(from_seconds(7200.0, h));   TEST_EQUAL(h.count(), 2);
        TRY(from_seconds(7200.0, m));   TEST_EQUAL(m.count(), 120);
        TRY(from_seconds(7200.0, s));   TEST_EQUAL(s.count(), 7200);
        TRY(from_seconds(7200.0, ms));  TEST_EQUAL(ms.count(), 7200000);

        TEST_EQUAL(to_seconds(hours(10)), 36000);
        TEST_EQUAL(to_seconds(minutes(10)), 600);
        TEST_EQUAL(to_seconds(seconds(10)), 10);
        TEST_NEAR(to_seconds(milliseconds(10)), 0.01);

        system_clock::time_point tp;
        system_clock::duration d;
        intmax_t t1, t2;
        u8string str;

        TRY(tp = make_date(1970, 1, 1, 0, 0, 0));
        TRY(t1 = intmax_t(system_clock::to_time_t(tp)));
        TEST_EQUAL(t1, 0);
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5));
        TRY(t1 = intmax_t(system_clock::to_time_t(tp)));
        TEST_EQUAL(t1, 946782245);
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5, local_date));
        TRY(t2 = intmax_t(system_clock::to_time_t(tp)));
        TEST_COMPARE(abs(t2 - t1), <=, 86400);

        TRY(tp = make_date(2000, 1, 2, 3, 4, 5));
        TRY(str = format_date(tp));
        TEST_EQUAL(str, "2000-01-02 03:04:05");
        TRY(str = format_date(tp, 3));
        TEST_EQUAL(str, "2000-01-02 03:04:05.000");
        TRY(from_seconds(0.12345, d));
        TRY(tp += d);
        TRY(str = format_date(tp));
        TEST_EQUAL(str, "2000-01-02 03:04:05");
        TRY(str = format_date(tp, 3));
        TEST_EQUAL(str, "2000-01-02 03:04:05.123");
        TRY(from_seconds(0.44444, d));
        TRY(tp += d);
        TRY(str = format_date(tp));
        TEST_EQUAL(str, "2000-01-02 03:04:05");
        TRY(str = format_date(tp, 3));
        TEST_EQUAL(str, "2000-01-02 03:04:05.568");
        TRY(str = format_date(tp, "%d/%m/%Y %H:%M"));
        TEST_EQUAL(str, "02/01/2000 03:04");
        TRY(str = format_date(tp, ""));
        TEST_EQUAL(str, "");
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5, local_date));
        TRY(str = format_date(tp, 0, local_date));
        TEST_EQUAL(str, "2000-01-02 03:04:05");

        TEST_EQUAL(format_time(DblSec(0)), "0s");
        TEST_EQUAL(format_time(DblSec(0), 3), "0.000s");
        TEST_EQUAL(format_time(DblSec(0.25), 3), "0.250s");
        TEST_EQUAL(format_time(DblSec(0.5), 3), "0.500s");
        TEST_EQUAL(format_time(DblSec(0.75), 3), "0.750s");
        TEST_EQUAL(format_time(DblSec(1), 3), "1.000s");
        TEST_EQUAL(format_time(DblSec(1.25), 3), "1.250s");
        TEST_EQUAL(format_time(DblSec(59.999), 3), "59.999s");
        TEST_EQUAL(format_time(DblSec(60), 3), "1m00.000s");
        TEST_EQUAL(format_time(DblSec(1234), 3), "20m34.000s");
        TEST_EQUAL(format_time(DblSec(12345), 3), "3h25m45.000s");
        TEST_EQUAL(format_time(DblSec(123456), 3), "1d10h17m36.000s");
        TEST_EQUAL(format_time(DblSec(1234567), 3), "14d06h56m07.000s");
        TEST_EQUAL(format_time(DblSec(12345678), 3), "142d21h21m18.000s");
        TEST_EQUAL(format_time(DblSec(123456789), 3), "3y333d03h33m09.000s");
        TEST_EQUAL(format_time(DblSec(1234567890), 3), "39y044d05h31m30.000s");
        TEST_EQUAL(format_time(DblSec(-0.25), 3), "-0.250s");
        TEST_EQUAL(format_time(DblSec(-0.5), 3), "-0.500s");
        TEST_EQUAL(format_time(DblSec(-0.75), 3), "-0.750s");
        TEST_EQUAL(format_time(DblSec(-1), 3), "-1.000s");
        TEST_EQUAL(format_time(DblSec(-1.25), 3), "-1.250s");
        TEST_EQUAL(format_time(DblSec(-59.999), 3), "-59.999s");
        TEST_EQUAL(format_time(DblSec(-60), 3), "-1m00.000s");
        TEST_EQUAL(format_time(DblSec(-1234), 3), "-20m34.000s");
        TEST_EQUAL(format_time(DblSec(-12345), 3), "-3h25m45.000s");
        TEST_EQUAL(format_time(DblSec(-123456), 3), "-1d10h17m36.000s");
        TEST_EQUAL(format_time(DblSec(-1234567), 3), "-14d06h56m07.000s");
        TEST_EQUAL(format_time(DblSec(-12345678), 3), "-142d21h21m18.000s");
        TEST_EQUAL(format_time(DblSec(-123456789), 3), "-3y333d03h33m09.000s");
        TEST_EQUAL(format_time(DblSec(-1234567890), 3), "-39y044d05h31m30.000s");
        TEST_EQUAL(format_time(nanoseconds(1), 10), "0.0000000010s");
        TEST_EQUAL(format_time(microseconds(1), 7), "0.0000010s");
        TEST_EQUAL(format_time(milliseconds(1), 4), "0.0010s");
        TEST_EQUAL(format_time(seconds(1)), "1s");
        TEST_EQUAL(format_time(minutes(1)), "1m00s");
        TEST_EQUAL(format_time(hours(1)), "1h00m00s");
        TEST_EQUAL(format_time(nanoseconds(-1), 10), "-0.0000000010s");
        TEST_EQUAL(format_time(microseconds(-1), 7), "-0.0000010s");
        TEST_EQUAL(format_time(milliseconds(-1), 4), "-0.0010s");
        TEST_EQUAL(format_time(seconds(-1)), "-1s");
        TEST_EQUAL(format_time(minutes(-1)), "-1m00s");
        TEST_EQUAL(format_time(hours(-1)), "-1h00m00s");

        #if defined(PRI_TARGET_UNIX)

            IntMsec ims;
            IntSec is;
            IntDays id;
            DblSec fs;
            timespec ts;
            timeval tv;

            ts = {0, 0};              TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 0);
            ts = {0, 0};              TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 0);
            ts = {0, 0};              TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 0);
            ts = {0, 0};              TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 0);
            ts = {0, 125000000};      TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 0.125);
            ts = {0, 125000000};      TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 0);
            ts = {0, 125000000};      TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 125);
            ts = {0, 125000000};      TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 0);
            ts = {86400, 0};          TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 86400);
            ts = {86400, 0};          TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 1);
            ts = {86400, 0};          TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 86400000);
            ts = {86400, 0};          TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 86400);
            ts = {86400, 125000000};  TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 86400.125);
            ts = {86400, 125000000};  TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 1);
            ts = {86400, 125000000};  TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 86400125);
            ts = {86400, 125000000};  TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 86400);
            tv = {0, 0};              TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 0);
            tv = {0, 0};              TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 0);
            tv = {0, 0};              TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 0);
            tv = {0, 0};              TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 0);
            tv = {0, 125000};         TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 0.125);
            tv = {0, 125000};         TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 0);
            tv = {0, 125000};         TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 125);
            tv = {0, 125000};         TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 0);
            tv = {86400, 0};          TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 86400);
            tv = {86400, 0};          TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 1);
            tv = {86400, 0};          TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 86400000);
            tv = {86400, 0};          TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 86400);
            tv = {86400, 125000};     TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 86400.125);
            tv = {86400, 125000};     TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 1);
            tv = {86400, 125000};     TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 86400125);
            tv = {86400, 125000};     TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 86400);

            fs = DblSec(0);           TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 0);      TEST_EQUAL(ts.tv_nsec, 0);
            id = IntDays(0);          TRY(ts = duration_to_timespec(id));   TEST_EQUAL(ts.tv_sec, 0);      TEST_EQUAL(ts.tv_nsec, 0);
            ims = IntMsec(0);         TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 0);      TEST_EQUAL(ts.tv_nsec, 0);
            is = IntSec(0);           TRY(ts = duration_to_timespec(is));   TEST_EQUAL(ts.tv_sec, 0);      TEST_EQUAL(ts.tv_nsec, 0);
            fs = DblSec(0.125);       TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 0);      TEST_EQUAL(ts.tv_nsec, 125000000);
            ims = IntMsec(125);       TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 0);      TEST_EQUAL(ts.tv_nsec, 125000000);
            fs = DblSec(86400);       TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 86400);  TEST_EQUAL(ts.tv_nsec, 0);
            id = IntDays(1);          TRY(ts = duration_to_timespec(id));   TEST_EQUAL(ts.tv_sec, 86400);  TEST_EQUAL(ts.tv_nsec, 0);
            ims = IntMsec(86400000);  TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 86400);  TEST_EQUAL(ts.tv_nsec, 0);
            is = IntSec(86400);       TRY(ts = duration_to_timespec(is));   TEST_EQUAL(ts.tv_sec, 86400);  TEST_EQUAL(ts.tv_nsec, 0);
            fs = DblSec(86400.125);   TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 86400);  TEST_EQUAL(ts.tv_nsec, 125000000);
            ims = IntMsec(86400125);  TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 86400);  TEST_EQUAL(ts.tv_nsec, 125000000);
            fs = DblSec(0);           TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 0);      TEST_EQUAL(tv.tv_usec, 0);
            id = IntDays(0);          TRY(tv = duration_to_timeval(id));    TEST_EQUAL(tv.tv_sec, 0);      TEST_EQUAL(tv.tv_usec, 0);
            ims = IntMsec(0);         TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 0);      TEST_EQUAL(tv.tv_usec, 0);
            is = IntSec(0);           TRY(tv = duration_to_timeval(is));    TEST_EQUAL(tv.tv_sec, 0);      TEST_EQUAL(tv.tv_usec, 0);
            fs = DblSec(0.125);       TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 0);      TEST_EQUAL(tv.tv_usec, 125000);
            ims = IntMsec(125);       TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 0);      TEST_EQUAL(tv.tv_usec, 125000);
            fs = DblSec(86400);       TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 86400);  TEST_EQUAL(tv.tv_usec, 0);
            id = IntDays(1);          TRY(tv = duration_to_timeval(id));    TEST_EQUAL(tv.tv_sec, 86400);  TEST_EQUAL(tv.tv_usec, 0);
            ims = IntMsec(86400000);  TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 86400);  TEST_EQUAL(tv.tv_usec, 0);
            is = IntSec(86400);       TRY(tv = duration_to_timeval(is));    TEST_EQUAL(tv.tv_sec, 86400);  TEST_EQUAL(tv.tv_usec, 0);
            fs = DblSec(86400.125);   TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 86400);  TEST_EQUAL(tv.tv_usec, 125000);
            ims = IntMsec(86400125);  TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 86400);  TEST_EQUAL(tv.tv_usec, 125000);

        #endif

        #if defined(PRI_TARGET_WINDOWS)

            static constexpr int64_t epoch = 11644473600ll;
            static constexpr int64_t freq = 10000000ll;

            int64_t n;
            FILETIME ft;

            n = epoch * freq;
            ft = {uint32_t(n), uint32_t(n >> 32)};
            TRY(tp = filetime_to_timepoint(ft));
            d = tp - system_clock::from_time_t(0);
            TEST_EQUAL(d.count(), 0);

            n += 86400 * freq;
            ft = {uint32_t(n), uint32_t(n >> 32)};
            TRY(tp = filetime_to_timepoint(ft));
            d = tp - system_clock::from_time_t(0);
            TEST_EQUAL(duration_cast<IntMsec>(d).count(), 86400000);

            tp = system_clock::from_time_t(0);
            TRY(timepoint_to_filetime(tp, ft));
            TEST_EQUAL(ft.dwHighDateTime, 0);
            TEST_EQUAL(ft.dwLowDateTime, 0);

            tp = system_clock::from_time_t(86400);
            TRY(timepoint_to_filetime(tp, ft));
            TEST_EQUAL(ft.dwHighDateTime, 201);
            TEST_EQUAL(ft.dwLowDateTime, 711573504);

        #endif

    }

    void check_type_properties() {

        {  using type = CopyConst<int, string>;              TEST_TYPE(type, string);        }
        {  using type = CopyConst<int, const string>;        TEST_TYPE(type, string);        }
        {  using type = CopyConst<const int, string>;        TEST_TYPE(type, const string);  }
        {  using type = CopyConst<const int, const string>;  TEST_TYPE(type, const string);  }

        u8string s;

        TEST_EQUAL(type_name<void>(), "void");
        TEST_MATCH(type_name<int>(), "^(signed )?int$");
        TEST_MATCH(type_name<string>(), "^std::([^:]+::)*(string|basic_string ?<.+>)$");
        TEST_MATCH(type_name(42), "^(signed )?int$");
        TEST_MATCH(type_name(s), "^std::([^:]+::)*(string|basic_string ?<.+>)$");

    }

    void check_uuid() {

        Uuid u1, u2;
        std::mt19937 rng{uint32_t(time(nullptr))};

        TEST_EQUAL(u1.str(), "00000000-0000-0000-0000-000000000000");
        TEST_EQUAL(to_str(u1), "00000000-0000-0000-0000-000000000000");
        TEST_EQUAL(u1.str(), to_str(u1));
        TEST_EQUAL(u1.as_integer(), 0_u128);

        TRY(u1 = Uuid(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));
        TEST_EQUAL(u1.str(), "01020304-0506-0708-090a-0b0c0d0e0f10");
        TEST_EQUAL(to_str(u1), "01020304-0506-0708-090a-0b0c0d0e0f10");
        TEST_EQUAL(u1.str(), to_str(u1));
        TEST_EQUAL(u1.as_integer(), 0x0102030405060708090a0b0c0d0e0f10_u128);
        for (unsigned i = 0; i < 16; ++i)
            TEST_EQUAL(u1[i], i + 1);
        TRY(u1 = Uuid(0x12345678, 0x9abc, 0xdef0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0));
        TEST_EQUAL(u1.str(), "12345678-9abc-def0-1234-56789abcdef0");

        TRY(u1 = Uuid(0_u128));
        TEST_EQUAL(u1.str(), "00000000-0000-0000-0000-000000000000");
        TRY(u1 = Uuid(0x123456789abcdef0123456789abcdef0_u128));
        TEST_EQUAL(u1.str(), "12345678-9abc-def0-1234-56789abcdef0");

        const uint8_t* bytes = nullptr;
        TRY(u1 = Uuid(bytes));
        TEST_EQUAL(u1.str(), "00000000-0000-0000-0000-000000000000");
        string chars = "abcdefghijklmnop";
        bytes = reinterpret_cast<const uint8_t*>(chars.data());
        TRY(u1 = Uuid(bytes));
        TEST_EQUAL(u1.str(), "61626364-6566-6768-696a-6b6c6d6e6f70");

        TRY(u1 = Uuid("123456789abcdef123456789abcdef12"));
        TEST_EQUAL(u1.str(), "12345678-9abc-def1-2345-6789abcdef12");
        TRY(u1 = Uuid("3456789a-bcde-f123-4567-89abcdef1234"));
        TEST_EQUAL(u1.str(), "3456789a-bcde-f123-4567-89abcdef1234");
        TRY(u1 = Uuid("56,78,9a,bc,de,f1,23,45,67,89,ab,cd,ef,12,34,56"));
        TEST_EQUAL(u1.str(), "56789abc-def1-2345-6789-abcdef123456");
        TRY(u1 = Uuid("{0x78,0x9a,0xbc,0xde,0xf1,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0x12,0x34,0x56,0x78}"));
        TEST_EQUAL(u1.str(), "789abcde-f123-4567-89ab-cdef12345678");
        TRY(u1 = Uuid("{0xbcdef123, 0x4567, 0x89ab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};"));
        TEST_EQUAL(u1.str(), "bcdef123-4567-89ab-cdef-123456789abc");
        TEST_THROW(u1 = Uuid(""), std::invalid_argument);
        TEST_THROW(u1 = Uuid("123456789abcdef123456789abcdefgh"), std::invalid_argument);
        TEST_THROW(u1 = Uuid("123456789abcdef123456789abcdef"), std::invalid_argument);
        TEST_THROW(u1 = Uuid("123456789abcdef123456789abcdef1"), std::invalid_argument);
        TEST_THROW(u1 = Uuid("123456789abcdef123456789abcdef123"), std::invalid_argument);
        TEST_THROW(u1 = Uuid("123456789abcdef123456789abcdef1234"), std::invalid_argument);
        TEST_THROW(u1 = Uuid("123456789-abc-def1-2345-6789abcdef12"), std::invalid_argument);

        for (int i = 0; i < 1000; ++i) {
            TRY(u1 = RandomUuid()(rng));
            TEST_MATCH(u1.str(), "^[[:xdigit:]]{8}-[[:xdigit:]]{4}-4[[:xdigit:]]{3}-[89ab][[:xdigit:]]{3}-[[:xdigit:]]{12}$");
            TRY(u2 = RandomUuid()(rng));
            TEST_MATCH(u2.str(), "^[[:xdigit:]]{8}-[[:xdigit:]]{4}-4[[:xdigit:]]{3}-[89ab][[:xdigit:]]{3}-[[:xdigit:]]{12}$");
            TEST_COMPARE(u1, !=, u2);
        }

    }

    void check_version_number() {

        Version v, w;

        TRY((v = {1,2,3}));
        TEST_EQUAL(v[0], 1);
        TEST_EQUAL(v[1], 2);
        TEST_EQUAL(v[2], 3);
        TEST_EQUAL(v.str(), "1.2.3");

        TRY((w = 2));
        TEST_EQUAL(w[0], 2);
        TEST_EQUAL(w[1], 0);
        TEST_EQUAL(w[2], 0);
        TEST_EQUAL(w.str(), "2.0.0");

        TEST(! (v == w));
        TEST(v != w);
        TEST(v < w);
        TEST(! (v > w));
        TEST(v <= w);
        TEST(! (v >= w));

        TRY(v = Version::from(0));           TEST_EQUAL(v, (Version{0,0,0}));
        TRY(v = Version::from(1));           TEST_EQUAL(v, (Version{0,0,1}));
        TRY(v = Version::from(12));          TEST_EQUAL(v, (Version{0,1,2}));
        TRY(v = Version::from(123));         TEST_EQUAL(v, (Version{1,2,3}));
        TRY(v = Version::from(1234));        TEST_EQUAL(v, (Version{12,3,4}));
        TRY(v = Version::from(0x0, 16));     TEST_EQUAL(v, (Version{0,0,0}));
        TRY(v = Version::from(0x1, 16));     TEST_EQUAL(v, (Version{0,0,1}));
        TRY(v = Version::from(0x12, 16));    TEST_EQUAL(v, (Version{0,1,2}));
        TRY(v = Version::from(0x123, 16));   TEST_EQUAL(v, (Version{1,2,3}));
        TRY(v = Version::from(0x1234, 16));  TEST_EQUAL(v, (Version{18,3,4}));

        TRY(v = Version::parse(""));           TEST_EQUAL(v, (Version{0,0,0}));
        TRY(v = Version::parse("1"));          TEST_EQUAL(v, (Version{1,0,0}));
        TRY(v = Version::parse("1xyz"));       TEST_EQUAL(v, (Version{1,0,0}));
        TRY(v = Version::parse("1.xyz"));      TEST_EQUAL(v, (Version{1,0,0}));
        TRY(v = Version::parse("1.2"));        TEST_EQUAL(v, (Version{1,2,0}));
        TRY(v = Version::parse("1.2xyz"));     TEST_EQUAL(v, (Version{1,2,0}));
        TRY(v = Version::parse("1.2.xyz"));    TEST_EQUAL(v, (Version{1,2,0}));
        TRY(v = Version::parse("1.2.3"));      TEST_EQUAL(v, (Version{1,2,3}));
        TRY(v = Version::parse("1.2.3xyz"));   TEST_EQUAL(v, (Version{1,2,3}));
        TRY(v = Version::parse("1.2.3.xyz"));  TEST_EQUAL(v, (Version{1,2,3}));

        TEST_THROW(Version::parse("-1"), std::invalid_argument);
        TEST_THROW(Version::parse("xyz"), std::invalid_argument);

    }

}

TEST_MODULE(prion, core) {

    check_macros();
    check_arithmetic_literals();
    check_arithmetic_functions();
    check_byte_order();
    check_character_functions();
    check_containers();
    check_exceptions();
    check_flagset();
    check_functional_utilities();
    check_hash_functions();
    check_io_utilities();
    check_keyword_arguments();
    check_range_utilities();
    check_scope_guards();
    check_string_functions();
    check_threads();
    check_time_and_date_functions();
    check_type_properties();
    check_uuid();
    check_version_number();

}
