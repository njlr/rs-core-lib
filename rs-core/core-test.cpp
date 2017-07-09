#include "rs-core/core.hpp"
#include "rs-core/unit-test.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <ratio>
#include <set>
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

#ifdef _XOPEN_SOURCE
    #include <sys/time.h>
#endif

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace RS;
using namespace RS::Literals;
using namespace std::chrono;
using namespace std::literals;

TEST_MAIN;

// These tests are a bit incestuous since the unit test module itself uses
// some of the facilities in the core module. The shared functions are mostly
// simple enough that it should be clear where the error lies if anything goes
// wrong, and in any case the alternative of duplicating large parts of the
// core module in the unit test module would not be an improvement.

namespace {

    U8string f1(int n) { return U8string(n, '*'); }
    U8string f1(U8string s) { return '[' + s + ']'; }
    U8string f2() { return "Hello"; }
    U8string f2(U8string s) { return '[' + s + ']'; }
    U8string f2(int x, int y) { return dec(x * y); }

    RS_ENUM(FooEnum, int16_t, 1, alpha, bravo, charlie)
    RS_ENUM_CLASS(BarEnum, int32_t, 1, alpha, bravo, charlie)

    enum class ZapEnum: uint32_t {
        alpha = 1,
        bravo = 2,
        charlie = 4,
    };

    RS_BITMASK_OPERATORS(ZapEnum)

    void check_preprocessor_macros() {

        int n = int(std::time(nullptr));
        RS_ASSUME(n > 0);
        if (RS_LIKELY(n > 1000))
            ++n;
        if (RS_UNLIKELY(n < 1000))
            ++n;
        switch (n - n) {
            case 0:
                break;
            default:
                RS_NOTREACHED;
                break;
        }

        auto assert1 = [&] { RS_ASSERT(n == 42); };
        auto assert2 = [&] { RS_ASSERT(n == 6 * 9); };
        n = 42;
        TRY(assert1());
        TEST_THROW_MATCH(assert2(), std::logic_error, "\\bcore-test\\.cpp:\\d+\\b.+\\bn == 6 \\* 9\\b");

        std::vector<int> iv = {1, 2, 3, 4, 5}, iv2 = {2, 3, 5, 7, 11};
        std::vector<U8string> result, sv = {"Neddie", "Eccles", "Bluebottle"};

        TRY(std::transform(iv.begin(), iv.end(), overwrite(result), RS_OVERLOAD(f1)));
        TEST_EQUAL_RANGE(result, (std::vector<U8string>{"*", "**", "***", "****", "*****"}));
        TRY(std::transform(sv.begin(), sv.end(), overwrite(result), RS_OVERLOAD(f1)));
        TEST_EQUAL_RANGE(result, (std::vector<U8string>{"[Neddie]", "[Eccles]", "[Bluebottle]"}));

        TRY(std::generate_n(overwrite(result), 3, RS_OVERLOAD(f2)));
        TEST_EQUAL_RANGE(result, (std::vector<U8string>{"Hello", "Hello", "Hello"}));
        TRY(std::transform(sv.begin(), sv.end(), overwrite(result), RS_OVERLOAD(f2)));
        TEST_EQUAL_RANGE(result, (std::vector<U8string>{"[Neddie]", "[Eccles]", "[Bluebottle]"}));
        auto of2 = RS_OVERLOAD(f2);
        auto out = overwrite(result);
        for (size_t i = 0; i < iv.size() && i < iv2.size(); ++i)
            TRY(*out++ = of2(iv[i], iv2[i]));
        TEST_EQUAL_RANGE(result, (std::vector<U8string>{"2", "6", "15", "28", "55"}));

        std::vector<FooEnum> vf;
        std::vector<BarEnum> vb;

        TEST_TYPE(std::underlying_type_t<FooEnum>, int16_t);
        TEST_TYPE(std::underlying_type_t<BarEnum>, int32_t);
        TEST_TYPE(std::underlying_type_t<ZapEnum>, uint32_t);

        TEST_EQUAL(sizeof(FooEnum), 2);
        TEST_EQUAL(sizeof(BarEnum), 4);
        TEST_EQUAL(sizeof(ZapEnum), 4);

        TEST_EQUAL(int(alpha), 1);
        TEST_EQUAL(int(bravo), 2);
        TEST_EQUAL(int(charlie), 3);

        TEST_EQUAL(to_str(alpha), "alpha");
        TEST_EQUAL(to_str(bravo), "bravo");
        TEST_EQUAL(to_str(charlie), "charlie");
        TEST_EQUAL(to_str(FooEnum(0)), "0");
        TEST_EQUAL(to_str(FooEnum(4)), "4");
        TEST_EQUAL(to_str(FooEnum(99)), "99");
        TEST_EQUAL(to_str(nullptr), "null");

        TEST(! enum_is_valid(FooEnum(0)));
        TEST(enum_is_valid(FooEnum(1)));
        TEST(enum_is_valid(FooEnum(2)));
        TEST(enum_is_valid(FooEnum(3)));
        TEST(! enum_is_valid(FooEnum(4)));

        TRY(vf = enum_values<FooEnum>());
        TEST_EQUAL(vf.size(), 3);
        TEST_EQUAL(to_str(vf), "[alpha,bravo,charlie]");

        TEST_EQUAL(int(BarEnum::alpha), 1);
        TEST_EQUAL(int(BarEnum::bravo), 2);
        TEST_EQUAL(int(BarEnum::charlie), 3);

        TEST_EQUAL(to_str(BarEnum::alpha), "BarEnum::alpha");
        TEST_EQUAL(to_str(BarEnum::bravo), "BarEnum::bravo");
        TEST_EQUAL(to_str(BarEnum::charlie), "BarEnum::charlie");
        TEST_EQUAL(to_str(BarEnum(0)), "0");
        TEST_EQUAL(to_str(BarEnum(4)), "4");
        TEST_EQUAL(to_str(BarEnum(99)), "99");

        TEST(! enum_is_valid(BarEnum(0)));
        TEST(enum_is_valid(BarEnum(1)));
        TEST(enum_is_valid(BarEnum(2)));
        TEST(enum_is_valid(BarEnum(3)));
        TEST(! enum_is_valid(BarEnum(4)));

        TRY(vb = enum_values<BarEnum>());
        TEST_EQUAL(vb.size(), 3);
        TEST_EQUAL(to_str(vb), "[BarEnum::alpha,BarEnum::bravo,BarEnum::charlie]");

        ZapEnum x = ZapEnum::alpha, y = ZapEnum::bravo, z = ZapEnum(0);

        TEST(x);    TEST(bool(x));
        TEST(y);    TEST(bool(y));
        TEST(! z);  TEST(! bool(z));

        TRY(z = ~ x);             TEST_EQUAL(uint32_t(z), 0xfffffffe);
        TRY(z = ~ y);             TEST_EQUAL(uint32_t(z), 0xfffffffd);
        TRY(z = ~ ZapEnum(0));        TEST_EQUAL(uint32_t(z), 0xffffffff);
        TRY(z = x & y);           TEST_EQUAL(uint32_t(z), 0);
        TRY(z = x | y);           TEST_EQUAL(uint32_t(z), 3);
        TRY(z = x ^ y);           TEST_EQUAL(uint32_t(z), 3);
        TRY(z = x); TRY(z &= y);  TEST_EQUAL(uint32_t(z), 0);
        TRY(z = x); TRY(z |= y);  TEST_EQUAL(uint32_t(z), 3);
        TRY(z = x); TRY(z ^= y);  TEST_EQUAL(uint32_t(z), 3);

        TEST(ZapEnum::alpha == ZapEnum::alpha);   TEST(ZapEnum::alpha < ZapEnum::bravo);    TEST(ZapEnum::alpha < ZapEnum::charlie);
        TEST(ZapEnum::bravo > ZapEnum::alpha);    TEST(ZapEnum::bravo == ZapEnum::bravo);   TEST(ZapEnum::bravo < ZapEnum::charlie);
        TEST(ZapEnum::charlie > ZapEnum::alpha);  TEST(ZapEnum::charlie > ZapEnum::bravo);  TEST(ZapEnum::charlie == ZapEnum::charlie);

    }

    void check_endian_integers() {

        uint8_t a8;
        uint16_t a16;
        uint32_t a32;
        uint64_t a64;

        constexpr uint8_t c8 = 0x12u;
        constexpr uint16_t c16 = 0x1234u;
        constexpr uint32_t c32 = 0x12345678ul;
        constexpr uint64_t c64 = 0x123456789abcdef0ull;

        a8 = RS_Detail::swap_ends(c8);    TEST_EQUAL(a8, 0x12u);
        a16 = RS_Detail::swap_ends(c16);  TEST_EQUAL(a16, 0x3412u);
        a32 = RS_Detail::swap_ends(c32);  TEST_EQUAL(a32, 0x78563412ul);
        a64 = RS_Detail::swap_ends(c64);  TEST_EQUAL(a64, 0xf0debc9a78563412ull);

        a8 = RS_Detail::order_bytes<big_endian>(c8);       TEST_EQUAL(a8, 0x12u);
        a16 = RS_Detail::order_bytes<big_endian>(c16);     TEST_EQUAL(a16, 0x3412u);
        a32 = RS_Detail::order_bytes<big_endian>(c32);     TEST_EQUAL(a32, 0x78563412ul);
        a64 = RS_Detail::order_bytes<big_endian>(c64);     TEST_EQUAL(a64, 0xf0debc9a78563412ull);
        a8 = RS_Detail::order_bytes<little_endian>(c8);    TEST_EQUAL(a8, 0x12u);
        a16 = RS_Detail::order_bytes<little_endian>(c16);  TEST_EQUAL(a16, 0x1234u);
        a32 = RS_Detail::order_bytes<little_endian>(c32);  TEST_EQUAL(a32, 0x12345678ul);
        a64 = RS_Detail::order_bytes<little_endian>(c64);  TEST_EQUAL(a64, 0x123456789abcdef0ull);

        constexpr BigEndian<uint8_t> cbe8 = c8;
        constexpr BigEndian<uint16_t> cbe16 = c16;
        constexpr BigEndian<uint32_t> cbe32 = c32;
        constexpr BigEndian<uint64_t> cbe64 = c64;
        constexpr LittleEndian<uint8_t> cle8 = c8;
        constexpr LittleEndian<uint16_t> cle16 = c16;
        constexpr LittleEndian<uint32_t> cle32 = c32;
        constexpr LittleEndian<uint64_t> cle64 = c64;

        TEST_EQUAL(cbe8, 0x12u);                   TEST_EQUAL(cbe8.rep(), 0x12u);
        TEST_EQUAL(cbe16, 0x1234u);                TEST_EQUAL(cbe16.rep(), 0x3412u);
        TEST_EQUAL(cbe32, 0x12345678ul);           TEST_EQUAL(cbe32.rep(), 0x78563412ul);
        TEST_EQUAL(cbe64, 0x123456789abcdef0ull);  TEST_EQUAL(cbe64.rep(), 0xf0debc9a78563412ull);
        TEST_EQUAL(cle8, 0x12u);                   TEST_EQUAL(cle8.rep(), 0x12u);
        TEST_EQUAL(cle16, 0x1234u);                TEST_EQUAL(cle16.rep(), 0x1234u);
        TEST_EQUAL(cle32, 0x12345678ul);           TEST_EQUAL(cle32.rep(), 0x12345678ul);
        TEST_EQUAL(cle64, 0x123456789abcdef0ull);  TEST_EQUAL(cle64.rep(), 0x123456789abcdef0ull);

        BigEndian<uint8_t> be8;
        BigEndian<uint16_t> be16;
        BigEndian<uint32_t> be32;
        BigEndian<uint64_t> be64;
        LittleEndian<uint8_t> le8;
        LittleEndian<uint16_t> le16;
        LittleEndian<uint32_t> le32;
        LittleEndian<uint64_t> le64;

        be8.rep() = c8;    TEST_EQUAL(be8, 0x12u);
        be16.rep() = c16;  TEST_EQUAL(be16, 0x3412u);
        be32.rep() = c32;  TEST_EQUAL(be32, 0x78563412ul);
        be64.rep() = c64;  TEST_EQUAL(be64, 0xf0debc9a78563412ull);
        le8.rep() = c8;    TEST_EQUAL(le8, 0x12u);
        le16.rep() = c16;  TEST_EQUAL(le16, 0x1234u);
        le32.rep() = c32;  TEST_EQUAL(le32, 0x12345678ul);
        le64.rep() = c64;  TEST_EQUAL(le64, 0x123456789abcdef0ull);

        size_t h1 = 0, h2 = 0, h3 = 0;

        for (unsigned i = 0; i <= 1000000; i += 100) {
            TRY(h1 = std::hash<uint8_t>()(uint8_t(i)));
            TRY(h2 = std::hash<BigEndian<uint8_t>>()(BigEndian<uint8_t>(i)));
            TRY(h3 = std::hash<LittleEndian<uint8_t>>()(LittleEndian<uint8_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<uint16_t>()(uint16_t(i)));
            TRY(h2 = std::hash<BigEndian<uint16_t>>()(BigEndian<uint16_t>(i)));
            TRY(h3 = std::hash<LittleEndian<uint16_t>>()(LittleEndian<uint16_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<uint32_t>()(uint32_t(i)));
            TRY(h2 = std::hash<BigEndian<uint32_t>>()(BigEndian<uint32_t>(i)));
            TRY(h3 = std::hash<LittleEndian<uint32_t>>()(LittleEndian<uint32_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<uint64_t>()(uint64_t(i)));
            TRY(h2 = std::hash<BigEndian<uint64_t>>()(BigEndian<uint64_t>(i)));
            TRY(h3 = std::hash<LittleEndian<uint64_t>>()(LittleEndian<uint64_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<int8_t>()(int8_t(i)));
            TRY(h2 = std::hash<BigEndian<int8_t>>()(BigEndian<int8_t>(i)));
            TRY(h3 = std::hash<LittleEndian<int8_t>>()(LittleEndian<int8_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<int16_t>()(int16_t(i)));
            TRY(h2 = std::hash<BigEndian<int16_t>>()(BigEndian<int16_t>(i)));
            TRY(h3 = std::hash<LittleEndian<int16_t>>()(LittleEndian<int16_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<int32_t>()(int32_t(i)));
            TRY(h2 = std::hash<BigEndian<int32_t>>()(BigEndian<int32_t>(i)));
            TRY(h3 = std::hash<LittleEndian<int32_t>>()(LittleEndian<int32_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
            TRY(h1 = std::hash<int64_t>()(int64_t(i)));
            TRY(h2 = std::hash<BigEndian<int64_t>>()(BigEndian<int64_t>(i)));
            TRY(h3 = std::hash<LittleEndian<int64_t>>()(LittleEndian<int64_t>(i)));
            TEST_EQUAL(h1, h2);
            TEST_EQUAL(h1, h3);
        }

        BigEndian<long> be = 20, be1 = 10, be2 = 20, be3 = 30;
        LittleEndian<long> le = 20, le1 = 10, le2 = 20, le3 = 30;

        TEST_COMPARE(be, !=, be1);  TEST_COMPARE(be, >, be1);   TEST_COMPARE(be, >=, be1);
        TEST_COMPARE(be, ==, be2);  TEST_COMPARE(be, <=, be2);  TEST_COMPARE(be, >=, be2);
        TEST_COMPARE(be, !=, be3);  TEST_COMPARE(be, <, be3);   TEST_COMPARE(be, <=, be3);
        TEST_COMPARE(be, !=, 10);  TEST_COMPARE(be, >, 10);   TEST_COMPARE(be, >=, 10);
        TEST_COMPARE(be, ==, 20);  TEST_COMPARE(be, <=, 20);  TEST_COMPARE(be, >=, 20);
        TEST_COMPARE(be, !=, 30);  TEST_COMPARE(be, <, 30);   TEST_COMPARE(be, <=, 30);
        TEST_COMPARE(10, !=, be);  TEST_COMPARE(10, <, be);   TEST_COMPARE(10, <=, be);
        TEST_COMPARE(20, ==, be);  TEST_COMPARE(20, >=, be);  TEST_COMPARE(20, <=, be);
        TEST_COMPARE(30, !=, be);  TEST_COMPARE(30, >, be);   TEST_COMPARE(30, >=, be);

        TEST_COMPARE(le, !=, le1);  TEST_COMPARE(le, >, le1);   TEST_COMPARE(le, >=, le1);
        TEST_COMPARE(le, ==, le2);  TEST_COMPARE(le, <=, le2);  TEST_COMPARE(le, >=, le2);
        TEST_COMPARE(le, !=, le3);  TEST_COMPARE(le, <, le3);   TEST_COMPARE(le, <=, le3);
        TEST_COMPARE(le, !=, 10);  TEST_COMPARE(le, >, 10);   TEST_COMPARE(le, >=, 10);
        TEST_COMPARE(le, ==, 20);  TEST_COMPARE(le, <=, 20);  TEST_COMPARE(le, >=, 20);
        TEST_COMPARE(le, !=, 30);  TEST_COMPARE(le, <, 30);   TEST_COMPARE(le, <=, 30);
        TEST_COMPARE(10, !=, le);  TEST_COMPARE(10, <, le);   TEST_COMPARE(10, <=, le);
        TEST_COMPARE(20, ==, le);  TEST_COMPARE(20, >=, le);  TEST_COMPARE(20, <=, le);
        TEST_COMPARE(30, !=, le);  TEST_COMPARE(30, >, le);   TEST_COMPARE(30, >=, le);

        TEST_COMPARE(be, !=, le1);  TEST_COMPARE(be, >, le1);   TEST_COMPARE(be, >=, le1);
        TEST_COMPARE(be, ==, le2);  TEST_COMPARE(be, <=, le2);  TEST_COMPARE(be, >=, le2);
        TEST_COMPARE(be, !=, le3);  TEST_COMPARE(be, <, le3);   TEST_COMPARE(be, <=, le3);
        TEST_COMPARE(le, !=, be1);  TEST_COMPARE(le, >, be1);   TEST_COMPARE(le, >=, be1);
        TEST_COMPARE(le, ==, be2);  TEST_COMPARE(le, <=, be2);  TEST_COMPARE(le, >=, be2);
        TEST_COMPARE(le, !=, be3);  TEST_COMPARE(le, <, be3);   TEST_COMPARE(le, <=, be3);

    }

    class TopTail {
    public:
        TopTail(): sp(nullptr), ch() {}
        TopTail(U8string& s, char c): sp(&s), ch(c) { s += '+'; s += c; }
        ~TopTail() { term(); }
        TopTail(TopTail&& t): sp(t.sp), ch(t.ch) { t.sp = nullptr; }
        TopTail& operator=(TopTail&& t) { if (&t != this) { term(); sp = t.sp; ch = t.ch; t.sp = nullptr; } return *this; }
        char get() const noexcept { return ch; }
    private:
        U8string* sp;
        char ch;
        TopTail(const TopTail&) = delete;
        TopTail& operator=(const TopTail&) = delete;
        void term() { if (sp) { *sp += '-'; *sp += ch; sp = nullptr; } }
    };

    void check_containers() {

        Blob b;

        TEST(b.empty());
        TEST_EQUAL(b.size(), 0);
        TEST_EQUAL(b.hex(), "");
        TEST_EQUAL(b.str(), "");
        TEST_EQUAL(std::string(b.chars().begin(), b.chars().end()), "");

        TRY(b = Blob(nullptr, 1));
        TEST(b.empty());
        TEST_EQUAL(b.size(), 0);

        TRY(b = Blob(5, 'a'));
        TEST(! b.empty());
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.hex(), "61 61 61 61 61");
        TEST_EQUAL(b.str(), "aaaaa");
        TEST_EQUAL(std::string(b.chars().begin(), b.chars().end()), "aaaaa");

        TRY(b.fill('z'));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.hex(), "7a 7a 7a 7a 7a");
        TEST_EQUAL(b.str(), "zzzzz");

        TRY(b.clear());
        TEST(b.empty());
        TEST_EQUAL(b.size(), 0);
        TEST_EQUAL(b.hex(), "");
        TEST_EQUAL(b.str(), "");

        TRY(b.reset(10, 'a'));
        TEST(! b.empty());
        TEST_EQUAL(b.size(), 10);
        TEST_EQUAL(b.hex(), "61 61 61 61 61 61 61 61 61 61");
        TEST_EQUAL(b.str(), "aaaaaaaaaa");

        TRY(b.copy("hello", 5));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.hex(), "68 65 6c 6c 6f");
        TEST_EQUAL(b.str(), "hello");

        U8string s, h = "hello", w = "world";

        TRY(b.clear());
        TRY(b.reset(&h[0], 5, [&] (void*) { s += "a"; }));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.str(), "hello");
        TEST_EQUAL(s, "");
        TRY(b = Blob(&w[0], 5, [&] (void*) { s += "b"; }));
        TEST_EQUAL(b.size(), 5);
        TEST_EQUAL(b.str(), "world");
        TEST_EQUAL(s, "a");
        TRY(b.clear());
        TEST_EQUAL(s, "ab");

        Stack<TopTail> st;

        s.clear();
        TEST(st.empty());
        TRY(st.push(TopTail(s, 'a')));
        TEST_EQUAL(st.size(), 1);
        TEST_EQUAL(st.top().get(), 'a');
        TEST_EQUAL(s, "+a");
        TRY(st.push(TopTail(s, 'b')));
        TEST_EQUAL(st.size(), 2);
        TEST_EQUAL(st.top().get(), 'b');
        TEST_EQUAL(s, "+a+b");
        TRY(st.push(TopTail(s, 'c')));
        TEST_EQUAL(st.size(), 3);
        TEST_EQUAL(st.top().get(), 'c');
        TEST_EQUAL(s, "+a+b+c");
        TRY(st.clear());
        TEST(st.empty());
        TEST_EQUAL(s, "+a+b+c-c-b-a");

    }

    void check_exceptions() {

        #ifdef _WIN32

            U8string s;

            TRY(s = windows_category().message(ERROR_INVALID_FUNCTION));
            TEST_EQUAL(s, "Incorrect function.");
            try {
                throw std::system_error(ERROR_INVALID_FUNCTION, windows_category(), "SomeFunction()");
            }
            catch (const std::exception& ex) {
                TEST_MATCH(std::string(ex.what()), "^SomeFunction\\(\\).+Incorrect function\\.$");
            }

        #endif

    }

    void check_metaprogramming_and_type_traits() {

        TEST_TYPE(BinaryType<char>, uint8_t);
        TEST_TYPE(BinaryType<float>, uint32_t);
        TEST_TYPE(BinaryType<double>, uint64_t);

        {  using type = CopyConst<int, std::string>;              TEST_TYPE(type, std::string);        }
        {  using type = CopyConst<int, const std::string>;        TEST_TYPE(type, std::string);        }
        {  using type = CopyConst<const int, std::string>;        TEST_TYPE(type, const std::string);  }
        {  using type = CopyConst<const int, const std::string>;  TEST_TYPE(type, const std::string);  }

        TEST_TYPE(SignedInteger<8>, int8_t);
        TEST_TYPE(SignedInteger<16>, int16_t);
        TEST_TYPE(SignedInteger<32>, int32_t);
        TEST_TYPE(SignedInteger<64>, int64_t);
        TEST_TYPE(UnsignedInteger<8>, uint8_t);
        TEST_TYPE(UnsignedInteger<16>, uint16_t);
        TEST_TYPE(UnsignedInteger<32>, uint32_t);
        TEST_TYPE(UnsignedInteger<64>, uint64_t);

    }

    struct Nnptest {
        int num;
        static int count;
        static U8string log;
        Nnptest(): num(0) { ++count; log += "+nil;"; }
        explicit Nnptest(int n): num(n) { ++count; log += "+" + dec(num) + ";"; }
        ~Nnptest() { --count; log += "-" + dec(num) + ";"; }
        Nnptest(const Nnptest&) = delete;
        Nnptest(Nnptest&&) = delete;
        Nnptest& operator=(const Nnptest&) = delete;
        Nnptest& operator=(Nnptest&&) = delete;
    };

    int Nnptest::count = 0;
    U8string Nnptest::log;

    void check_smart_pointers() {

        {

            Nnptr<Nnptest> p(new Nnptest);
            TEST_EQUAL(p->num, 0);
            TEST(p.unique());
            TEST_EQUAL(Nnptest::count, 1);
            TEST_EQUAL(Nnptest::log, "+nil;");

            Nnptr<Nnptest> q = p;
            TEST_EQUAL(p->num, 0);
            TEST_EQUAL(q->num, 0);
            TEST(! p.unique());
            TEST(! q.unique());
            TEST_EQUAL(Nnptest::count, 1);
            TEST_EQUAL(Nnptest::log, "+nil;");
            TEST(p == q);

            Nnptest* fp = nullptr;
            TEST_THROW(p.reset(fp), NullPointer);
            TEST_EQUAL(p->num, 0);
            TEST_EQUAL(q->num, 0);

            TRY(p.reset(new Nnptest(1)));
            TEST_EQUAL(p->num, 1);
            TEST_EQUAL(q->num, 0);
            TEST(p.unique());
            TEST(q.unique());
            TEST_EQUAL(Nnptest::count, 2);
            TEST_EQUAL(Nnptest::log, "+nil;+1;");
            TEST(p != q);

            TRY(q = make_nnptr<Nnptest>(2));
            TEST_EQUAL(p->num, 1);
            TEST_EQUAL(q->num, 2);
            TEST(p.unique());
            TEST(q.unique());
            TEST_EQUAL(Nnptest::count, 2);
            TEST_EQUAL(Nnptest::log, "+nil;+1;+2;-0;");
            TEST(p != q);

            auto u = std::make_unique<Nnptest>(3);
            TEST_EQUAL(Nnptest::count, 3);
            TEST_EQUAL(Nnptest::log, "+nil;+1;+2;-0;+3;");

            TRY(p = std::move(u));
            TEST_EQUAL(p->num, 3);
            TEST_EQUAL(q->num, 2);
            TEST(! u);
            TEST_EQUAL(Nnptest::count, 2);
            TEST_EQUAL(Nnptest::log, "+nil;+1;+2;-0;+3;-1;");

        }

        TEST_EQUAL(Nnptest::count, 0);
        TEST_EQUAL(Nnptest::log, "+nil;+1;+2;-0;+3;-1;-2;-3;");

        int i = 42;
        U8string s = "Hello";

        auto ni = nnptr(i);
        TEST_TYPE_OF(ni, Nnptr<int>);
        TEST(ni);
        if (ni) TEST_EQUAL(*ni, 42);

        auto si = shptr(i);
        TEST_TYPE_OF(si, std::shared_ptr<int>);
        TEST(si);
        if (si) TEST_EQUAL(*si, 42);

        auto ui = unptr(i);
        TEST_TYPE_OF(ui, std::unique_ptr<int>);
        TEST(ui);
        if (ui) TEST_EQUAL(*ui, 42);

        auto ns = nnptr(s);
        TEST_TYPE_OF(ns, Nnptr<std::string>);
        TEST(ns);
        if (ns) TEST_EQUAL(*ns, "Hello");

        auto ss = shptr(s);
        TEST_TYPE_OF(ss, std::shared_ptr<std::string>);
        TEST(ss);
        if (ss) TEST_EQUAL(*ss, "Hello");

        auto us = unptr(s);
        TEST_TYPE_OF(us, std::unique_ptr<std::string>);
        TEST(us);
        if (us) TEST_EQUAL(*us, "Hello");

    }

    struct Adapt {
        Movable<U8string> s;
        Movable<int> x;
        Movable<int, 10> y;
        int z = 20;
    };

    void check_type_adapters() {

        Adapt a, b;

        TEST_EQUAL(a.s.value, "");
        TEST_EQUAL(a.x.value, 0);
        TEST_EQUAL(a.y.value, 10);
        TEST_EQUAL(a.z, 20);

        TRY((b = {"Hello"s, 100, 200, 300}));

        TEST_EQUAL(b.s.value, "Hello");
        TEST_EQUAL(b.x.value, 100);
        TEST_EQUAL(b.y.value, 200);
        TEST_EQUAL(b.z, 300);

        TRY(a = std::move(b));

        TEST_EQUAL(a.s.value, "Hello");
        TEST_EQUAL(a.x.value, 100);
        TEST_EQUAL(a.y.value, 200);
        TEST_EQUAL(a.z, 300);

        TEST_EQUAL(b.s.value, "");
        TEST_EQUAL(b.x.value, 0);
        TEST_EQUAL(b.y.value, 10);
        TEST_EQUAL(b.z, 300);

    }

    class Base {
    public:
        virtual ~Base() noexcept {}
        virtual int get() const = 0;
    };

    class Derived1: public Base {
    public:
        virtual int get() const { return 1; }
    };

    class Derived2: public Base {
    public:
        virtual int get() const { return 2; }
    };

    void check_type_related_functions() {

        std::shared_ptr<Base> b1 = std::make_shared<Derived1>();
        std::shared_ptr<Base> b2 = std::make_shared<Derived2>();

        TEST(is<Derived1>(b1));
        TEST(! is<Derived1>(b2));
        TEST(! is<Derived2>(b1));
        TEST(is<Derived2>(b2));

        TEST(is<Derived1>(*b1));
        TEST(! is<Derived1>(*b2));
        TEST(! is<Derived2>(*b1));
        TEST(is<Derived2>(*b2));

        TEST_EQUAL(as<Derived1>(b1).get(), 1);
        TEST_THROW(as<Derived2>(b1).get(), std::bad_cast);
        TEST_THROW(as<Derived1>(b2).get(), std::bad_cast);
        TEST_EQUAL(as<Derived2>(b2).get(), 2);

        TEST_EQUAL(as<Derived1>(*b1).get(), 1);
        TEST_THROW(as<Derived2>(*b1).get(), std::bad_cast);
        TEST_THROW(as<Derived1>(*b2).get(), std::bad_cast);
        TEST_EQUAL(as<Derived2>(*b2).get(), 2);

        int16_t i;
        uint16_t u;

        i = 42;      TRY(u = binary_cast<uint16_t>(i));  TEST_EQUAL(u, 42);
        i = -32767;  TRY(u = binary_cast<uint16_t>(i));  TEST_EQUAL(u, 32769);

        U8string s;

        const std::type_info& v_info = typeid(void);
        const std::type_info& i_info = typeid(int);
        const std::type_info& s_info = typeid(std::string);
        auto v_index = std::type_index(typeid(void));
        auto i_index = std::type_index(typeid(int));
        auto s_index = std::type_index(typeid(std::string));

        TEST_EQUAL(type_name(v_info), "void");
        TEST_MATCH(type_name(i_info), "^(signed )?int$");
        TEST_MATCH(type_name(s_info), "^std::([^:]+::)*(string|basic_string ?<.+>)$");
        TEST_EQUAL(type_name(v_index), "void");
        TEST_MATCH(type_name(i_index), "^(signed )?int$");
        TEST_MATCH(type_name(s_index), "^std::([^:]+::)*(string|basic_string ?<.+>)$");
        TEST_EQUAL(type_name<void>(), "void");
        TEST_MATCH(type_name<int>(), "^(signed )?int$");
        TEST_MATCH(type_name<std::string>(), "^std::([^:]+::)*(string|basic_string ?<.+>)$");
        TEST_MATCH(type_name(42), "^(signed )?int$");
        TEST_MATCH(type_name(s), "^std::([^:]+::)*(string|basic_string ?<.+>)$");

        Derived1 d;
        Base& b(d);
        const std::type_info& d_info = typeid(d);
        const std::type_info& b_info = typeid(b);
        auto d_index = std::type_index(typeid(d));
        auto b_index = std::type_index(typeid(b));

        TEST_MATCH(type_name(d), "::Derived1$");
        TEST_MATCH(type_name(b), "::Derived1$");
        TEST_MATCH(type_name(d_info), "::Derived1$");
        TEST_MATCH(type_name(b_info), "::Derived1$");
        TEST_MATCH(type_name(d_index), "::Derived1$");
        TEST_MATCH(type_name(b_index), "::Derived1$");

    }

    void check_uuid() {

        Uuid u, u2;

        TEST_EQUAL(u.str(), "00000000-0000-0000-0000-000000000000");
        TEST_EQUAL(to_str(u), "00000000-0000-0000-0000-000000000000");
        TEST_EQUAL(u.str(), to_str(u));

        TRY(u = Uuid(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));
        TEST_EQUAL(u.str(), "01020304-0506-0708-090a-0b0c0d0e0f10");
        TEST_EQUAL(to_str(u), "01020304-0506-0708-090a-0b0c0d0e0f10");
        TEST_EQUAL(u.str(), to_str(u));
        for (unsigned i = 0; i < 16; ++i)
            TEST_EQUAL(u[i], i + 1);
        TRY(u = Uuid(0x12345678, 0x9abc, 0xdef0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0));
        TEST_EQUAL(u.str(), "12345678-9abc-def0-1234-56789abcdef0");

        const uint8_t* bytes = nullptr;
        TRY(u = Uuid(bytes));
        TEST_EQUAL(u.str(), "00000000-0000-0000-0000-000000000000");
        std::string chars = "abcdefghijklmnop";
        bytes = reinterpret_cast<const uint8_t*>(chars.data());
        TRY(u = Uuid(bytes));
        TEST_EQUAL(u.str(), "61626364-6566-6768-696a-6b6c6d6e6f70");

        TRY(u = Uuid("123456789abcdef123456789abcdef12"));
        TEST_EQUAL(u.str(), "12345678-9abc-def1-2345-6789abcdef12");
        TRY(u = Uuid("3456789a-bcde-f123-4567-89abcdef1234"));
        TEST_EQUAL(u.str(), "3456789a-bcde-f123-4567-89abcdef1234");
        TRY(u = Uuid("56,78,9a,bc,de,f1,23,45,67,89,ab,cd,ef,12,34,56"));
        TEST_EQUAL(u.str(), "56789abc-def1-2345-6789-abcdef123456");
        TRY(u = Uuid("{0x78,0x9a,0xbc,0xde,0xf1,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0x12,0x34,0x56,0x78}"));
        TEST_EQUAL(u.str(), "789abcde-f123-4567-89ab-cdef12345678");
        TRY(u = Uuid("{0xbcdef123, 0x4567, 0x89ab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};"));
        TEST_EQUAL(u.str(), "bcdef123-4567-89ab-cdef-123456789abc");
        TEST_THROW(u = Uuid(""), std::invalid_argument);
        TEST_THROW(u = Uuid("123456789abcdef123456789abcdefgh"), std::invalid_argument);
        TEST_THROW(u = Uuid("123456789abcdef123456789abcdef"), std::invalid_argument);
        TEST_THROW(u = Uuid("123456789abcdef123456789abcdef1"), std::invalid_argument);
        TEST_THROW(u = Uuid("123456789abcdef123456789abcdef123"), std::invalid_argument);
        TEST_THROW(u = Uuid("123456789abcdef123456789abcdef1234"), std::invalid_argument);
        TEST_THROW(u = Uuid("123456789-abc-def1-2345-6789abcdef12"), std::invalid_argument);

        std::mt19937 rng(42);

        for (int i = 0; i < 1000; ++i) {
            TRY(u = RandomUuid()(rng));
            TEST_MATCH(u.str(), "^[[:xdigit:]]{8}-[[:xdigit:]]{4}-4[[:xdigit:]]{3}-[89ab][[:xdigit:]]{3}-[[:xdigit:]]{12}$");
            TRY(u2 = RandomUuid()(rng));
            TEST_MATCH(u2.str(), "^[[:xdigit:]]{8}-[[:xdigit:]]{4}-4[[:xdigit:]]{3}-[89ab][[:xdigit:]]{3}-[[:xdigit:]]{12}$");
            TEST_COMPARE(u, !=, u2);
        }

    }

    void check_version_number() {

        Version v;

        TEST_EQUAL(v[0], 0);
        TEST_EQUAL(v[1], 0);
        TEST_EQUAL(v[2], 0);
        TEST_EQUAL(v.major(), 0);
        TEST_EQUAL(v.minor(), 0);
        TEST_EQUAL(v.patch(), 0);
        TEST_EQUAL(v.suffix(), "");
        TEST_EQUAL(v.str(), "0.0");
        TEST_EQUAL(v.str(0), "");
        TEST_EQUAL(v.str(1), "0");
        TEST_EQUAL(v.str(2), "0.0");
        TEST_EQUAL(v.str(3), "0.0.0");
        TEST_EQUAL(v.str(4), "0.0.0.0");
        TEST_EQUAL(v.to32(), 0);

        TRY((v = {42}));
        TEST_EQUAL(v[0], 42);
        TEST_EQUAL(v[1], 0);
        TEST_EQUAL(v[2], 0);
        TEST_EQUAL(v.major(), 42);
        TEST_EQUAL(v.minor(), 0);
        TEST_EQUAL(v.patch(), 0);
        TEST_EQUAL(v.suffix(), "");
        TEST_EQUAL(v.str(), "42.0");
        TEST_EQUAL(v.to32(), 0x2a000000);

        TRY((v = {42,"beta"}));
        TEST_EQUAL(v[0], 42);
        TEST_EQUAL(v[1], 0);
        TEST_EQUAL(v[2], 0);
        TEST_EQUAL(v.major(), 42);
        TEST_EQUAL(v.minor(), 0);
        TEST_EQUAL(v.patch(), 0);
        TEST_EQUAL(v.suffix(), "beta");
        TEST_EQUAL(v.str(), "42.0beta");
        TEST_EQUAL(v.to32(), 0x2a000000);

        TRY((v = {1,2,3}));
        TEST_EQUAL(v[0], 1);
        TEST_EQUAL(v[1], 2);
        TEST_EQUAL(v[2], 3);
        TEST_EQUAL(v.major(), 1);
        TEST_EQUAL(v.minor(), 2);
        TEST_EQUAL(v.patch(), 3);
        TEST_EQUAL(v.suffix(), "");
        TEST_EQUAL(v.str(), "1.2.3");
        TEST_EQUAL(v.str(0), "1.2.3");
        TEST_EQUAL(v.str(1), "1.2.3");
        TEST_EQUAL(v.str(2), "1.2.3");
        TEST_EQUAL(v.str(3), "1.2.3");
        TEST_EQUAL(v.str(4), "1.2.3.0");
        TEST_EQUAL(v.to32(), 0x01020300);

        TRY((v = {1,2,3,"beta"}));
        TEST_EQUAL(v[0], 1);
        TEST_EQUAL(v[1], 2);
        TEST_EQUAL(v[2], 3);
        TEST_EQUAL(v.major(), 1);
        TEST_EQUAL(v.minor(), 2);
        TEST_EQUAL(v.patch(), 3);
        TEST_EQUAL(v.suffix(), "beta");
        TEST_EQUAL(v.str(), "1.2.3beta");
        TEST_EQUAL(v.str(0), "1.2.3beta");
        TEST_EQUAL(v.str(1), "1.2.3beta");
        TEST_EQUAL(v.str(2), "1.2.3beta");
        TEST_EQUAL(v.str(3), "1.2.3beta");
        TEST_EQUAL(v.str(4), "1.2.3.0beta");
        TEST_EQUAL(v.to32(), 0x01020300);

        TRY(v = Version::from32(0x12345678));
        TEST_EQUAL(v[0], 18);
        TEST_EQUAL(v[1], 52);
        TEST_EQUAL(v[2], 86);
        TEST_EQUAL(v[3], 120);
        TEST_EQUAL(v.major(), 18);
        TEST_EQUAL(v.minor(), 52);
        TEST_EQUAL(v.patch(), 86);
        TEST_EQUAL(v.suffix(), "");
        TEST_EQUAL(v.str(), "18.52.86.120");
        TEST_EQUAL(v.to32(), 0x12345678);

        TRY(v = Version(""));
        TEST_EQUAL(v[0], 0);
        TEST_EQUAL(v[1], 0);
        TEST_EQUAL(v[2], 0);
        TEST_EQUAL(v.major(), 0);
        TEST_EQUAL(v.minor(), 0);
        TEST_EQUAL(v.patch(), 0);
        TEST_EQUAL(v.suffix(), "");
        TEST_EQUAL(v.str(), "0.0");
        TEST_EQUAL(v.to32(), 0);

        TRY(v = Version("1.2.3"));
        TEST_EQUAL(v[0], 1);
        TEST_EQUAL(v[1], 2);
        TEST_EQUAL(v[2], 3);
        TEST_EQUAL(v.major(), 1);
        TEST_EQUAL(v.minor(), 2);
        TEST_EQUAL(v.patch(), 3);
        TEST_EQUAL(v.suffix(), "");
        TEST_EQUAL(v.str(), "1.2.3");
        TEST_EQUAL(v.to32(), 0x01020300);

        TRY(v = Version("1.2.3beta"));
        TEST_EQUAL(v[0], 1);
        TEST_EQUAL(v[1], 2);
        TEST_EQUAL(v[2], 3);
        TEST_EQUAL(v.major(), 1);
        TEST_EQUAL(v.minor(), 2);
        TEST_EQUAL(v.patch(), 3);
        TEST_EQUAL(v.suffix(), "beta");
        TEST_EQUAL(v.str(), "1.2.3beta");
        TEST_EQUAL(v.to32(), 0x01020300);

        TRY(v = Version("beta"));
        TEST_EQUAL(v[0], 0);
        TEST_EQUAL(v[1], 0);
        TEST_EQUAL(v[2], 0);
        TEST_EQUAL(v.major(), 0);
        TEST_EQUAL(v.minor(), 0);
        TEST_EQUAL(v.patch(), 0);
        TEST_EQUAL(v.suffix(), "beta");
        TEST_EQUAL(v.str(), "0.0beta");
        TEST_EQUAL(v.to32(), 0);

    }

    void check_arithmetic_constants() {

        TEST_TYPE_OF(pi_f, float);
        TEST_TYPE_OF(pi_d, double);
        TEST_TYPE_OF(pi_ld, long double);
        TEST_TYPE_OF(pi_c<float>, float);

        TEST_NEAR(pi_f, 3.141593);
        TEST_NEAR(pi_d, 3.141593);
        TEST_NEAR(pi_ld, 3.141593);
        TEST_NEAR(pi_c<float>, 3.141593);

    }

    void check_arithmetic_literals() {

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Woverflow"

        auto a = 123_s8;
        auto b = 123_u8;
        auto c = 123_s16;
        auto d = 123_u16;
        auto e = 123_s32;
        auto f = 123_u32;
        auto g = 123_s64;
        auto h = 123_u64;
        auto i = 123_wc;
        auto j = 123_c16;
        auto k = 123_c32;

        TEST_TYPE_OF(a, int8_t);
        TEST_TYPE_OF(b, uint8_t);
        TEST_TYPE_OF(c, int16_t);
        TEST_TYPE_OF(d, uint16_t);
        TEST_TYPE_OF(e, int32_t);
        TEST_TYPE_OF(f, uint32_t);
        TEST_TYPE_OF(g, int64_t);
        TEST_TYPE_OF(h, uint64_t);
        TEST_TYPE_OF(i, wchar_t);
        TEST_TYPE_OF(j, char16_t);
        TEST_TYPE_OF(k, char32_t);

        TEST_EQUAL(a, int8_t(123));
        TEST_EQUAL(b, uint8_t(123));
        TEST_EQUAL(c, int16_t(123));
        TEST_EQUAL(d, uint16_t(123));
        TEST_EQUAL(e, int32_t(123));
        TEST_EQUAL(f, uint32_t(123));
        TEST_EQUAL(g, int64_t(123));
        TEST_EQUAL(h, uint64_t(123));
        TEST_EQUAL(i, wchar_t(123));
        TEST_EQUAL(j, char16_t(123));
        TEST_EQUAL(k, char32_t(123));

        ptrdiff_t t = 0;
        size_t z = 0;

        TRY(t = 0_t);
        TEST_EQUAL(t, 0_t);
        TEST_EQUAL(dec(t), "0");
        TEST_EQUAL(hex(t, 1), "0");
        TRY(t = 12345_t);
        TEST_EQUAL(t, 12345_t);
        TEST_EQUAL(dec(t), "12345");
        TRY(t = 0x0_t);
        TEST_EQUAL(dec(t), "0");
        TRY(t = 0x12345_t);
        TEST_EQUAL(hex(t, 1), "12345");
        TEST_EQUAL(dec(t), "74565");

        TRY(z = 0_z);
        TEST_EQUAL(z, 0_z);
        TEST_EQUAL(dec(z), "0");
        TEST_EQUAL(hex(z, 1), "0");
        TRY(z = 12345_z);
        TEST_EQUAL(z, 12345_z);
        TEST_EQUAL(dec(z), "12345");
        TRY(z = 0x0_z);
        TEST_EQUAL(dec(z), "0");
        TRY(z = 0x12345_z);
        TEST_EQUAL(hex(z, 1), "12345");
        TEST_EQUAL(dec(z), "74565");

        if (sizeof(size_t) == 8) {

            TRY(t = 0_t);
            TEST_EQUAL(hex(t), "0000000000000000");
            TRY(t = 9'223'372'036'854'775'807_t); // 2^63-1
            TEST_EQUAL(t, 9'223'372'036'854'775'807_t);
            TEST_EQUAL(dec(t), "9223372036854775807");
            TRY(t = -9'223'372'036'854'775'807_t); // -(2^63-1)
            TEST_EQUAL(t, -9'223'372'036'854'775'807_t);
            TEST_EQUAL(dec(t), "-9223372036854775807");
            TRY(t = -9'223'372'036'854'775'808_t); // -2^63
            TEST_EQUAL(t, -9'223'372'036'854'775'808_t);
            TEST_EQUAL(dec(t), "-9223372036854775808");
            TRY(t = 0x7fffffffffffffff_t);
            TEST_EQUAL(hex(t), "7fffffffffffffff");
            TEST_EQUAL(dec(t), "9223372036854775807");

            TRY(z = 0_z);
            TEST_EQUAL(hex(z), "0000000000000000");
            TRY(z = 18'446'744'073'709'551'615_z); // 2^64-1
            TEST_EQUAL(z, 18'446'744'073'709'551'615_z);
            TEST_EQUAL(dec(z), "18446744073709551615");
            TRY(z = 0xffffffffffffffff_z);
            TEST_EQUAL(hex(z), "ffffffffffffffff");
            TEST_EQUAL(dec(z), "18446744073709551615");

        } else if (sizeof(size_t) == 4) {

            TRY(t = 0_t);
            TEST_EQUAL(hex(t), "00000000");
            TRY(t = 2'147'483'647_t); // 2^31-1
            TEST_EQUAL(t, 2'147'483'647_t);
            TEST_EQUAL(dec(t), "2147483647");
            TRY(t = -2'147'483'647_t); // -(2^31-1)
            TEST_EQUAL(t, -2'147'483'647_t);
            TEST_EQUAL(dec(t), "-2147483647");
            TRY(t = -2'147'483'648_t); // -2^31
            TEST_EQUAL(t, -2'147'483'648_t);
            TEST_EQUAL(dec(t), "-2147483648");
            TRY(t = 0x7fffffff_t);
            TEST_EQUAL(hex(t), "7fffffff");
            TEST_EQUAL(dec(t), "2147483647");

            TRY(z = 0_z);
            TEST_EQUAL(hex(z), "00000000");
            TRY(z = 4'294'967'295_z); // 2^32-1
            TEST_EQUAL(z, 4'294'967'295_z);
            TEST_EQUAL(dec(z), "4294967295");
            TRY(z = 0xffffffff_z);
            TEST_EQUAL(hex(z), "ffffffff");
            TEST_EQUAL(dec(z), "4294967295");

        }

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

        #pragma GCC diagnostic pop

    }

    void check_generic_algorithms() {

        U8string s1, s2;
        std::vector<int> v1, v2, v3, v4;
        int n;

        s1 = "Hello";
        TRY(std::copy(s1.begin(), s1.end(), append(s2)));
        TEST_EQUAL(s2, "Hello");
        s1 = " world";
        TRY(std::copy(s1.begin(), s1.end(), append(s2)));
        TEST_EQUAL(s2, "Hello world");
        s1 = "Goodbye";
        TRY(std::copy(s1.begin(), s1.end(), overwrite(s2)));
        TEST_EQUAL(s2, "Goodbye");

        std::set<int> set1, set2;
        set1 = {1, 2, 3};
        TRY(std::copy(set1.begin(), set1.end(), append(set2)));
        TEST_EQUAL(to_str(set2), "[1,2,3]");
        set1 = {4, 5, 6};
        TRY(std::copy(set1.begin(), set1.end(), append(set2)));
        TEST_EQUAL(to_str(set2), "[1,2,3,4,5,6]");
        set1 = {1, 2, 3};
        TRY(std::copy(set1.begin(), set1.end(), append(set2)));
        TEST_EQUAL(to_str(set2), "[1,2,3,4,5,6]");
        set1 = {7, 8, 9};
        TRY(std::copy(set1.begin(), set1.end(), overwrite(set2)));
        TEST_EQUAL(to_str(set2), "[7,8,9]");

        s1 = "";               s2 = "";               TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, 0);
        s1 = "";               s2 = "hello";          TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, -1);
        s1 = "hello";          s2 = "";               TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, 1);
        s1 = "hello";          s2 = "world";          TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, -1);
        s1 = "hello";          s2 = "hello";          TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, 0);
        s1 = "hello";          s2 = "goodbye";        TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, 1);
        s1 = "hello";          s2 = "hello world";    TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, -1);
        s1 = "hello world";    s2 = "hello";          TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, 1);
        s1 = "hello goodbye";  s2 = "hello world";    TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, -1);
        s1 = "hello world";    s2 = "hello goodbye";  TRY(n = compare_3way(s1, s2));                    TEST_EQUAL(n, 1);
        s1 = "";               s2 = "";               TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, 0);
        s1 = "";               s2 = "hello";          TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, -1);
        s1 = "hello";          s2 = "";               TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, 1);
        s1 = "hello";          s2 = "world";          TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, 1);
        s1 = "hello";          s2 = "hello";          TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, 0);
        s1 = "hello";          s2 = "goodbye";        TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, -1);
        s1 = "hello";          s2 = "hello world";    TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, -1);
        s1 = "hello world";    s2 = "hello";          TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, 1);
        s1 = "hello goodbye";  s2 = "hello world";    TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, 1);
        s1 = "hello world";    s2 = "hello goodbye";  TRY(n = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(n, -1);

        const auto same_case = [] (char a, char b) { return ascii_isupper(a) == ascii_isupper(b); };

        s1 = "abcabcabc";        TRY(con_remove(s1, 'c'));                        TEST_EQUAL(s1, "ababab");
        s1 = "abc123abc123";     TRY(con_remove_if(s1, ascii_isalpha));           TEST_EQUAL(s1, "123123");
        s1 = "abc123abc123";     TRY(con_remove_if_not(s1, ascii_isalpha));       TEST_EQUAL(s1, "abcabc");
        s1 = "abbcccddddeeeee";  TRY(con_unique(s1));                             TEST_EQUAL(s1, "abcde");
        s1 = "ABCabcABCabc";     TRY(con_unique(s1, same_case));                  TEST_EQUAL(s1, "AaAa");
        s1 = "abcdeabcdabcaba";  TRY(con_sort_unique(s1));                        TEST_EQUAL(s1, "abcde");
        s1 = "abcdeabcdabcaba";  TRY(con_sort_unique(s1, std::greater<char>()));  TEST_EQUAL(s1, "edcba");

        v2 = {1, 2, 3};
        v3 = {4, 5, 6};
        v4 = {7, 8, 9};

        TRY(v1 = concatenate(v2));          TEST_EQUAL(to_str(v1), "[1,2,3]");
        TRY(v1 = concatenate(v2, v3));      TEST_EQUAL(to_str(v1), "[1,2,3,4,5,6]");
        TRY(v1 = concatenate(v2, v3, v4));  TEST_EQUAL(to_str(v1), "[1,2,3,4,5,6,7,8,9]");

        auto f1 = [&] { s1 += "Hello"; };
        auto f2 = [&] (size_t i) { s1 += dec(i) + ";"; };

        s1.clear();  TRY(do_n(0, f1));   TEST_EQUAL(s1, "");
        s1.clear();  TRY(do_n(1, f1));   TEST_EQUAL(s1, "Hello");
        s1.clear();  TRY(do_n(2, f1));   TEST_EQUAL(s1, "HelloHello");
        s1.clear();  TRY(do_n(3, f1));   TEST_EQUAL(s1, "HelloHelloHello");
        s1.clear();  TRY(for_n(0, f2));  TEST_EQUAL(s1, "");
        s1.clear();  TRY(for_n(1, f2));  TEST_EQUAL(s1, "0;");
        s1.clear();  TRY(for_n(2, f2));  TEST_EQUAL(s1, "0;1;");
        s1.clear();  TRY(for_n(3, f2));  TEST_EQUAL(s1, "0;1;2;");

        std::map<int, U8string> m = {
            {1, "alpha"},
            {2, "bravo"},
            {3, "charlie"},
        };
        U8string s;

        TRY(s = find_in_map(m, 1));          TEST_EQUAL(s, "alpha");
        TRY(s = find_in_map(m, 2));          TEST_EQUAL(s, "bravo");
        TRY(s = find_in_map(m, 3));          TEST_EQUAL(s, "charlie");
        TRY(s = find_in_map(m, 4));          TEST_EQUAL(s, "");
        TRY(s = find_in_map(m, 1, "none"));  TEST_EQUAL(s, "alpha");
        TRY(s = find_in_map(m, 2, "none"));  TEST_EQUAL(s, "bravo");
        TRY(s = find_in_map(m, 3, "none"));  TEST_EQUAL(s, "charlie");
        TRY(s = find_in_map(m, 4, "none"));  TEST_EQUAL(s, "none");

        std::set<int> is1, is2;
        TEST(! sets_intersect(is1, is2));
        is1 = {1,2,3,4,5};
        TEST(! sets_intersect(is1, is2));
        is2 = {3,6,9};
        TEST(sets_intersect(is1, is2));
        is2 = {10,20};
        TEST(! sets_intersect(is1, is2));

        std::vector<int> iv0, iv5 = {1,2,3,4,5};
        const std::vector<int>& civ5(iv5);
        double d, da5[] = {10,20,30,40,50};

        TRY(n = sum_of(iv0));       TEST_EQUAL(n, 0);
        TRY(n = sum_of(iv5));       TEST_EQUAL(n, 15);
        TRY(n = sum_of(civ5));      TEST_EQUAL(n, 15);
        TRY(n = product_of(iv0));   TEST_EQUAL(n, 1);
        TRY(n = product_of(iv5));   TEST_EQUAL(n, 120);
        TRY(n = product_of(civ5));  TEST_EQUAL(n, 120);
        TRY(d = sum_of(da5));       TEST_EQUAL(d, 150);
        TRY(d = product_of(da5));   TEST_EQUAL(d, 1.2e7);

    }

    void check_integer_sequences() {

        std::vector<int> v;
        U8string s;

        TRY(con_overwrite(iseq(5), v));            TRY(s = to_str(v));  TEST_EQUAL(s, "[0,1,2,3,4,5]");
        TRY(con_overwrite(iseq(-5), v));           TRY(s = to_str(v));  TEST_EQUAL(s, "[0,-1,-2,-3,-4,-5]");
        TRY(con_overwrite(iseq(1, 5), v));         TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(iseq(-1, -5), v));       TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4,-5]");
        TRY(con_overwrite(iseq(1, -1, 1), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(iseq(1, 0, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(iseq(1, 1, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1]");
        TRY(con_overwrite(iseq(1, 2, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2]");
        TRY(con_overwrite(iseq(1, 3, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3]");
        TRY(con_overwrite(iseq(1, 4, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(iseq(1, 5, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(iseq(-1, 1, -1), v));    TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(iseq(-1, 0, -1), v));    TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(iseq(-1, -1, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1]");
        TRY(con_overwrite(iseq(-1, -2, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2]");
        TRY(con_overwrite(iseq(-1, -3, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3]");
        TRY(con_overwrite(iseq(-1, -4, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4]");
        TRY(con_overwrite(iseq(-1, -5, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4,-5]");
        TRY(con_overwrite(iseq(1, 10, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10]");
        TRY(con_overwrite(iseq(1, 11, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10]");
        TRY(con_overwrite(iseq(1, 12, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10]");
        TRY(con_overwrite(iseq(1, 13, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10,13]");
        TRY(con_overwrite(iseq(-1, -10, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10]");
        TRY(con_overwrite(iseq(-1, -11, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10]");
        TRY(con_overwrite(iseq(-1, -12, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10]");
        TRY(con_overwrite(iseq(-1, -13, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10,-13]");

        TRY(con_overwrite(xseq(5), v));            TRY(s = to_str(v));  TEST_EQUAL(s, "[0,1,2,3,4]");
        TRY(con_overwrite(xseq(-5), v));           TRY(s = to_str(v));  TEST_EQUAL(s, "[0,-1,-2,-3,-4]");
        TRY(con_overwrite(xseq(1, 5), v));         TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(xseq(-1, -5), v));       TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4]");
        TRY(con_overwrite(xseq(1, -1, 1), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(xseq(1, 0, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(xseq(1, 1, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(xseq(1, 2, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1]");
        TRY(con_overwrite(xseq(1, 3, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2]");
        TRY(con_overwrite(xseq(1, 4, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3]");
        TRY(con_overwrite(xseq(1, 5, 1), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(xseq(-1, 1, -1), v));    TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(xseq(-1, 0, -1), v));    TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(xseq(-1, -1, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[]");
        TRY(con_overwrite(xseq(-1, -2, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1]");
        TRY(con_overwrite(xseq(-1, -3, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2]");
        TRY(con_overwrite(xseq(-1, -4, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3]");
        TRY(con_overwrite(xseq(-1, -5, -1), v));   TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4]");
        TRY(con_overwrite(xseq(1, 10, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7]");
        TRY(con_overwrite(xseq(1, 11, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10]");
        TRY(con_overwrite(xseq(1, 12, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10]");
        TRY(con_overwrite(xseq(1, 13, 3), v));     TRY(s = to_str(v));  TEST_EQUAL(s, "[1,4,7,10]");
        TRY(con_overwrite(xseq(-1, -10, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7]");
        TRY(con_overwrite(xseq(-1, -11, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10]");
        TRY(con_overwrite(xseq(-1, -12, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10]");
        TRY(con_overwrite(xseq(-1, -13, -3), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-4,-7,-10]");

        TRY(con_overwrite(iseq(int8_t(1), int8_t(5)), v));          TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(iseq(int8_t(-1), int8_t(-5)), v));        TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4,-5]");
        TRY(con_overwrite(xseq(int8_t(1), int8_t(5)), v));          TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(xseq(int8_t(-1), int8_t(-5)), v));        TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4]");
        TRY(con_overwrite(iseq(uint8_t(1), uint8_t(5)), v));        TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(xseq(uint8_t(1), uint8_t(5)), v));        TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(iseq(int16_t(1), int16_t(5)), v));        TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(iseq(int16_t(-1), int16_t(-5)), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4,-5]");
        TRY(con_overwrite(xseq(int16_t(1), int16_t(5)), v));        TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(xseq(int16_t(-1), int16_t(-5)), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4]");
        TRY(con_overwrite(iseq(uint16_t(1), uint16_t(5)), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(xseq(uint16_t(1), uint16_t(5)), v));      TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(iseq(ptrdiff_t(1), ptrdiff_t(5)), v));    TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(iseq(ptrdiff_t(-1), ptrdiff_t(-5)), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4,-5]");
        TRY(con_overwrite(xseq(ptrdiff_t(1), ptrdiff_t(5)), v));    TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");
        TRY(con_overwrite(xseq(ptrdiff_t(-1), ptrdiff_t(-5)), v));  TRY(s = to_str(v));  TEST_EQUAL(s, "[-1,-2,-3,-4]");
        TRY(con_overwrite(iseq(size_t(1), size_t(5)), v));          TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4,5]");
        TRY(con_overwrite(xseq(size_t(1), size_t(5)), v));          TRY(s = to_str(v));  TEST_EQUAL(s, "[1,2,3,4]");

    }

    void check_memory_algorithms() {

        std::string s1, s2;
        int x = 0;
        size_t z = 0;

        s1 = "";             s2 = "";             TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, 0);
        s1 = "hello";        s2 = "";             TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, 1);
        s1 = "";             s2 = "world";        TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, -1);
        s1 = "hello";        s2 = "world";        TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, -1);
        s1 = "hello world";  s2 = "hello";        TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, 1);
        s1 = "hello";        s2 = "hello world";  TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, -1);

        TRY(x = mem_compare(nullptr, 42, s2.data(), s2.size()));  TEST_EQUAL(x, -1);
        TRY(x = mem_compare(s1.data(), s1.size(), nullptr, 42));  TEST_EQUAL(x, 1);
        TRY(x = mem_compare(nullptr, 42, nullptr, 42));           TEST_EQUAL(x, 0);

        s1 = "";             s2 = "";               TRY(z = mem_match(s1.data(), s2.data(), 0));   TEST_EQUAL(z, 0);
        s1 = "hello";        s2 = "hello";          TRY(z = mem_match(s1.data(), s2.data(), 0));   TEST_EQUAL(z, 0);
        s1 = "hello";        s2 = "hello";          TRY(z = mem_match(s1.data(), s2.data(), 5));   TEST_EQUAL(z, 5);
        s1 = "hello world";  s2 = "hello world";    TRY(z = mem_match(s1.data(), s2.data(), 11));  TEST_EQUAL(z, 11);
        s1 = "hello world";  s2 = "hello goodbye";  TRY(z = mem_match(s1.data(), s2.data(), 11));  TEST_EQUAL(z, 6);
        s1 = "hello world";  s2 = "jello world";    TRY(z = mem_match(s1.data(), s2.data(), 11));  TEST_EQUAL(z, 0);

        s1 = "hello";
        s2 = "world";
        TRY(mem_swap(&s1[0], &s2[0], 5));
        TEST_EQUAL(s1, "world");
        TEST_EQUAL(s2, "hello");

    }

    void check_range_traits() {

        char array[] {'H','e','l','l','o'};

        TEST_TYPE(RangeIterator<std::string>, std::string::iterator);
        TEST_TYPE(RangeIterator<const std::string>, std::string::const_iterator);
        TEST_TYPE(RangeIterator<std::vector<int>>, std::vector<int>::iterator);
        TEST_TYPE(RangeIterator<const std::vector<int>>, std::vector<int>::const_iterator);
        TEST_TYPE(RangeIterator<Irange<int*>>, int*);
        TEST_TYPE(RangeIterator<Irange<const int*>>, const int*);

        TEST_TYPE(RangeValue<std::string>, char);
        TEST_TYPE(RangeValue<std::vector<int>>, int);
        TEST_TYPE(RangeValue<Irange<int*>>, int);
        TEST_TYPE(RangeValue<Irange<const int*>>, int);

        TEST_EQUAL(array_count(""), 1);
        TEST_EQUAL(array_count("Hello"), 6);
        TEST_EQUAL(array_count(array), 5);
        TEST_EQUAL(range_count(""s), 0);
        TEST_EQUAL(range_count("Hello"s), 5);
        TEST_EQUAL(range_count(""), 1);
        TEST_EQUAL(range_count("Hello"), 6);
        TEST(range_empty(""s));
        TEST(! range_empty("Hello"s));

    }

    void check_range_types() {

        char array[] {'H','e','l','l','o'};
        const char* cp = array;
        std::string s1, s2;

        auto ar = array_range(array, sizeof(array));
        TEST_EQUAL(ar.size(), 5);
        TRY(std::copy(ar.begin(), ar.end(), overwrite(s1)));
        TEST_EQUAL(s1, "Hello");

        auto car = array_range(cp, sizeof(array));
        TEST_EQUAL(car.size(), 5);
        TRY(std::copy(car.begin(), car.end(), overwrite(s1)));
        TEST_EQUAL(s1, "Hello");

    }

    void check_generic_arithmetic_functions() {

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

        int sx, sy, sq, sr;

        sx = -6;  sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -5;  sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -4;  sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -3;  sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -2;  sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -1;  sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 0;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 1;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 2;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 3;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 4;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 5;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 6;   sy = 3;   TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -6;  sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -5;  sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -4;  sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -3;  sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -2;  sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -1;  sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 0;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 1;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 2;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 3;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 4;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 5;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 2);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 6;   sy = -3;  TRY(sq = quo(sx, sy));  TRY(sr = rem(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 0);  TEST_EQUAL(sy * sq + sr, sx);

        sx = -6;  sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -5;  sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -4;  sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -3;  sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -2;  sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -1;  sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 0;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 1;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 2;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 3;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 4;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 5;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 6;   sy = 3;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -6;  sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -5;  sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -4;  sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -3;  sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -2;  sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -1;  sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 0;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 1;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 2;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 3;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 4;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 5;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 6;   sy = -3;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);

        sx = -8;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -7;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -6;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -5;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -4;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -3;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -2;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -1;  sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 0;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 1;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 2;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 3;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 4;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 5;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 6;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 7;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 8;   sy = 4;   TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -8;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -7;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -6;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 2);   TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -5;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = -4;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -3;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -2;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 1);   TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = -1;  sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 0;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 1;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 2;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, 0);   TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 3;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 4;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 5;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 1);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 6;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -1);  TEST_EQUAL(sr, 2);   TEST_EQUAL(sy * sq + sr, sx);
        sx = 7;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, -1);  TEST_EQUAL(sy * sq + sr, sx);
        sx = 8;   sy = -4;  TRY(sq = symmetric_quotient(sx, sy));  TRY(sr = symmetric_remainder(sx, sy));  TEST_EQUAL(sq, -2);  TEST_EQUAL(sr, 0);   TEST_EQUAL(sy * sq + sr, sx);

        unsigned ux, uy, uq, ur;

        ux = 0;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 0);  TEST_EQUAL(ur, 0);  TEST_EQUAL(uy * uq + ur, ux);
        ux = 1;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 0);  TEST_EQUAL(ur, 1);  TEST_EQUAL(uy * uq + ur, ux);
        ux = 2;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 0);  TEST_EQUAL(ur, 2);  TEST_EQUAL(uy * uq + ur, ux);
        ux = 3;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 1);  TEST_EQUAL(ur, 0);  TEST_EQUAL(uy * uq + ur, ux);
        ux = 4;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 1);  TEST_EQUAL(ur, 1);  TEST_EQUAL(uy * uq + ur, ux);
        ux = 5;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 1);  TEST_EQUAL(ur, 2);  TEST_EQUAL(uy * uq + ur, ux);
        ux = 6;  uy = 3;  TRY(uq = quo(ux, uy));  TRY(ur = rem(ux, uy));  TEST_EQUAL(uq, 2);  TEST_EQUAL(ur, 0);  TEST_EQUAL(uy * uq + ur, ux);

        double fx, fy, fq, fr;

        fx = -6;  fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -5;  fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -4;  fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -3;  fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -2;  fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -1;  fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 0;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 1;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 2;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 3;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 4;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 5;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 6;   fy = 3;   TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -6;  fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -5;  fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -4;  fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -3;  fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -2;  fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -1;  fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 0;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 1;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 2;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 3;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 4;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 5;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 2);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 6;   fy = -3;  TRY(fq = quo(fx, fy));  TRY(fr = rem(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 0);  TEST_EQUAL(fy * fq + fr, fx);

        fx = -8;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -7;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -6;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -5;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -4;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -3;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -2;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -1;  fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 0;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 1;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 2;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 3;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 4;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 5;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 6;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 7;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 8;   fy = 4;   TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -8;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -7;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -6;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 2);   TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -5;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = -4;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -3;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -2;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 1);   TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = -1;  fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 0;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 1;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 2;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, 0);   TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 3;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 4;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 5;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 1);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 6;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -1);  TEST_EQUAL(fr, 2);   TEST_EQUAL(fy * fq + fr, fx);
        fx = 7;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, -1);  TEST_EQUAL(fy * fq + fr, fx);
        fx = 8;   fy = -4;  TRY(fq = symmetric_quotient(fx, fy));  TRY(fr = symmetric_remainder(fx, fy));  TEST_EQUAL(fq, -2);  TEST_EQUAL(fr, 0);   TEST_EQUAL(fy * fq + fr, fx);

        TEST_EQUAL(shift_left(42, 0), 42);           TEST_EQUAL(shift_right(42, 0), 42);
        TEST_EQUAL(shift_left(42, 1), 84);           TEST_EQUAL(shift_right(42, 1), 21);
        TEST_EQUAL(shift_left(42, 2), 168);          TEST_EQUAL(shift_right(42, 2), 10);
        TEST_EQUAL(shift_left(42, 3), 336);          TEST_EQUAL(shift_right(42, 3), 5);
        TEST_EQUAL(shift_left(42, 4), 672);          TEST_EQUAL(shift_right(42, 4), 2);
        TEST_EQUAL(shift_left(42, 5), 1344);         TEST_EQUAL(shift_right(42, 5), 1);
        TEST_EQUAL(shift_left(42, 6), 2688);         TEST_EQUAL(shift_right(42, 6), 0);
        TEST_EQUAL(shift_left(42, 7), 5376);         TEST_EQUAL(shift_right(42, 7), 0);
        TEST_EQUAL(shift_left(42, -1), 21);          TEST_EQUAL(shift_right(42, -1), 84);
        TEST_EQUAL(shift_left(42, -2), 10);          TEST_EQUAL(shift_right(42, -2), 168);
        TEST_EQUAL(shift_left(42, -3), 5);           TEST_EQUAL(shift_right(42, -3), 336);
        TEST_EQUAL(shift_left(42, -4), 2);           TEST_EQUAL(shift_right(42, -4), 672);
        TEST_EQUAL(shift_left(42, -5), 1);           TEST_EQUAL(shift_right(42, -5), 1344);
        TEST_EQUAL(shift_left(42, -6), 0);           TEST_EQUAL(shift_right(42, -6), 2688);
        TEST_EQUAL(shift_left(42, -7), 0);           TEST_EQUAL(shift_right(42, -7), 5376);
        TEST_EQUAL(shift_left(-42, 0), -42);         TEST_EQUAL(shift_right(-42, 0), -42);
        TEST_EQUAL(shift_left(-42, 1), -84);         TEST_EQUAL(shift_right(-42, 1), -21);
        TEST_EQUAL(shift_left(-42, 2), -168);        TEST_EQUAL(shift_right(-42, 2), -10);
        TEST_EQUAL(shift_left(-42, 3), -336);        TEST_EQUAL(shift_right(-42, 3), -5);
        TEST_EQUAL(shift_left(-42, 4), -672);        TEST_EQUAL(shift_right(-42, 4), -2);
        TEST_EQUAL(shift_left(-42, 5), -1344);       TEST_EQUAL(shift_right(-42, 5), -1);
        TEST_EQUAL(shift_left(-42, 6), -2688);       TEST_EQUAL(shift_right(-42, 6), 0);
        TEST_EQUAL(shift_left(-42, 7), -5376);       TEST_EQUAL(shift_right(-42, 7), 0);
        TEST_EQUAL(shift_left(-42, -1), -21);        TEST_EQUAL(shift_right(-42, -1), -84);
        TEST_EQUAL(shift_left(-42, -2), -10);        TEST_EQUAL(shift_right(-42, -2), -168);
        TEST_EQUAL(shift_left(-42, -3), -5);         TEST_EQUAL(shift_right(-42, -3), -336);
        TEST_EQUAL(shift_left(-42, -4), -2);         TEST_EQUAL(shift_right(-42, -4), -672);
        TEST_EQUAL(shift_left(-42, -5), -1);         TEST_EQUAL(shift_right(-42, -5), -1344);
        TEST_EQUAL(shift_left(-42, -6), 0);          TEST_EQUAL(shift_right(-42, -6), -2688);
        TEST_EQUAL(shift_left(-42, -7), 0);          TEST_EQUAL(shift_right(-42, -7), -5376);
        TEST_EQUAL(shift_left(42.0, 0), 42.0);       TEST_EQUAL(shift_right(42.0, 0), 42.0);
        TEST_EQUAL(shift_left(42.0, 1), 84.0);       TEST_EQUAL(shift_right(42.0, 1), 21.0);
        TEST_EQUAL(shift_left(42.0, 2), 168.0);      TEST_EQUAL(shift_right(42.0, 2), 10.5);
        TEST_EQUAL(shift_left(42.0, 3), 336.0);      TEST_EQUAL(shift_right(42.0, 3), 5.25);
        TEST_EQUAL(shift_left(42.0, 4), 672.0);      TEST_EQUAL(shift_right(42.0, 4), 2.625);
        TEST_EQUAL(shift_left(42.0, 5), 1344.0);     TEST_EQUAL(shift_right(42.0, 5), 1.3125);
        TEST_EQUAL(shift_left(42.0, 6), 2688.0);     TEST_EQUAL(shift_right(42.0, 6), 0.65625);
        TEST_EQUAL(shift_left(42.0, 7), 5376.0);     TEST_EQUAL(shift_right(42.0, 7), 0.328125);
        TEST_EQUAL(shift_left(42.0, -1), 21.0);      TEST_EQUAL(shift_right(42.0, -1), 84.0);
        TEST_EQUAL(shift_left(42.0, -2), 10.5);      TEST_EQUAL(shift_right(42.0, -2), 168.0);
        TEST_EQUAL(shift_left(42.0, -3), 5.25);      TEST_EQUAL(shift_right(42.0, -3), 336.0);
        TEST_EQUAL(shift_left(42.0, -4), 2.625);     TEST_EQUAL(shift_right(42.0, -4), 672.0);
        TEST_EQUAL(shift_left(42.0, -5), 1.3125);    TEST_EQUAL(shift_right(42.0, -5), 1344.0);
        TEST_EQUAL(shift_left(42.0, -6), 0.65625);   TEST_EQUAL(shift_right(42.0, -6), 2688.0);
        TEST_EQUAL(shift_left(42.0, -7), 0.328125);  TEST_EQUAL(shift_right(42.0, -7), 5376.0);

    }

    void check_integer_arithmetic_functions() {

        int8_t s8 = -42;
        uint8_t u8 = 42;
        int16_t s16 = -42;
        uint16_t u16 = 42;
        int32_t s32 = -42;
        uint32_t u32 = 42;
        int64_t s64 = -42;
        uint64_t u64 = 42;

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
        TEST_EQUAL(as_unsigned(s32), 4'294'967'254ul);
        TEST_EQUAL(as_unsigned(u32), 42);
        TEST_EQUAL(as_signed(s64), -42);
        TEST_EQUAL(as_signed(u64), 42);
        TEST_EQUAL(as_unsigned(s64), 18'446'744'073'709'551'574ull);
        TEST_EQUAL(as_unsigned(u64), 42);

        TEST_EQUAL(binomial(-1, -1), 0);
        TEST_EQUAL(binomial(-1, 0), 0);
        TEST_EQUAL(binomial(-1, 1), 0);
        TEST_EQUAL(binomial(0, -1), 0);
        TEST_EQUAL(binomial(0, 0), 1);
        TEST_EQUAL(binomial(0, 1), 0);
        TEST_EQUAL(binomial(1, -1), 0);
        TEST_EQUAL(binomial(1, 0), 1);
        TEST_EQUAL(binomial(1, 1), 1);
        TEST_EQUAL(binomial(1, 2), 0);
        TEST_EQUAL(binomial(2, -1), 0);
        TEST_EQUAL(binomial(2, 0), 1);
        TEST_EQUAL(binomial(2, 1), 2);
        TEST_EQUAL(binomial(2, 2), 1);
        TEST_EQUAL(binomial(2, 3), 0);
        TEST_EQUAL(binomial(3, -1), 0);
        TEST_EQUAL(binomial(3, 0), 1);
        TEST_EQUAL(binomial(3, 1), 3);
        TEST_EQUAL(binomial(3, 2), 3);
        TEST_EQUAL(binomial(3, 3), 1);
        TEST_EQUAL(binomial(3, 4), 0);
        TEST_EQUAL(binomial(4, -1), 0);
        TEST_EQUAL(binomial(4, 0), 1);
        TEST_EQUAL(binomial(4, 1), 4);
        TEST_EQUAL(binomial(4, 2), 6);
        TEST_EQUAL(binomial(4, 3), 4);
        TEST_EQUAL(binomial(4, 4), 1);
        TEST_EQUAL(binomial(4, 5), 0);
        TEST_EQUAL(binomial(5, -1), 0);
        TEST_EQUAL(binomial(5, 0), 1);
        TEST_EQUAL(binomial(5, 1), 5);
        TEST_EQUAL(binomial(5, 2), 10);
        TEST_EQUAL(binomial(5, 3), 10);
        TEST_EQUAL(binomial(5, 4), 5);
        TEST_EQUAL(binomial(5, 5), 1);
        TEST_EQUAL(binomial(5, 6), 0);

        TEST_EQUAL(xbinomial(-1, -1), 0);
        TEST_EQUAL(xbinomial(-1, 0), 0);
        TEST_EQUAL(xbinomial(-1, 1), 0);
        TEST_EQUAL(xbinomial(0, -1), 0);
        TEST_EQUAL(xbinomial(0, 0), 1);
        TEST_EQUAL(xbinomial(0, 1), 0);
        TEST_EQUAL(xbinomial(1, -1), 0);
        TEST_EQUAL(xbinomial(1, 0), 1);
        TEST_EQUAL(xbinomial(1, 1), 1);
        TEST_EQUAL(xbinomial(1, 2), 0);
        TEST_EQUAL(xbinomial(2, -1), 0);
        TEST_EQUAL(xbinomial(2, 0), 1);
        TEST_EQUAL(xbinomial(2, 1), 2);
        TEST_EQUAL(xbinomial(2, 2), 1);
        TEST_EQUAL(xbinomial(2, 3), 0);
        TEST_EQUAL(xbinomial(3, -1), 0);
        TEST_EQUAL(xbinomial(3, 0), 1);
        TEST_EQUAL(xbinomial(3, 1), 3);
        TEST_EQUAL(xbinomial(3, 2), 3);
        TEST_EQUAL(xbinomial(3, 3), 1);
        TEST_EQUAL(xbinomial(3, 4), 0);
        TEST_EQUAL(xbinomial(4, -1), 0);
        TEST_EQUAL(xbinomial(4, 0), 1);
        TEST_EQUAL(xbinomial(4, 1), 4);
        TEST_EQUAL(xbinomial(4, 2), 6);
        TEST_EQUAL(xbinomial(4, 3), 4);
        TEST_EQUAL(xbinomial(4, 4), 1);
        TEST_EQUAL(xbinomial(4, 5), 0);
        TEST_EQUAL(xbinomial(5, -1), 0);
        TEST_EQUAL(xbinomial(5, 0), 1);
        TEST_EQUAL(xbinomial(5, 1), 5);
        TEST_EQUAL(xbinomial(5, 2), 10);
        TEST_EQUAL(xbinomial(5, 3), 10);
        TEST_EQUAL(xbinomial(5, 4), 5);
        TEST_EQUAL(xbinomial(5, 5), 1);
        TEST_EQUAL(xbinomial(5, 6), 0);

        int cd, cm, max = 100;

        for (int a = - max; a <= max; ++a) {
            for (int b = - max; b <= max; ++b) {
                TRY(cd = gcd(a, b));
                TRY(cm = lcm(a, b));
                if (a == 0 && b == 0)
                    TEST_EQUAL(cd, 0);
                else
                    TEST_COMPARE(cd, >, 0);
                if (a == 0 || b == 0) {
                    TEST_EQUAL(cd, std::abs(a + b));
                    TEST_EQUAL(cm, 0);
                } else  {
                    TEST_COMPARE(cm, >, 0);
                    TEST_COMPARE(cd, <=, std::min(std::abs(a), std::abs(b)));
                    if (a % b == 0) {
                        TEST_EQUAL(cd, std::abs(b));
                        TEST_EQUAL(cm, std::abs(a));
                    } else if (b % a == 0) {
                        TEST_EQUAL(cd, std::abs(a));
                        TEST_EQUAL(cm, std::abs(b));
                    } else {
                        TEST_EQUAL(a % cd, 0);
                        TEST_EQUAL(b % cd, 0);
                        for (int x = cd + 1, xmax = std::min(a, b) / 2; x <= xmax; ++x)
                            TEST(a % x != 0 || b % x != 0);
                        TEST_COMPARE(cm, >=, std::max(std::abs(a), std::abs(b)));
                        TEST_EQUAL(cm % a, 0);
                        TEST_EQUAL(cm % b, 0);
                    }
                }
                TEST_EQUAL(std::abs(a * b), cd * cm);
            }
        }

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

    }

    void check_bitwise_operations() {

        TEST_EQUAL(ibits(0), 0);
        TEST_EQUAL(ibits(1), 1);
        TEST_EQUAL(ibits(2), 1);
        TEST_EQUAL(ibits(3), 2);
        TEST_EQUAL(ibits(4), 1);
        TEST_EQUAL(ibits(5), 2);
        TEST_EQUAL(ibits(6), 2);
        TEST_EQUAL(ibits(7), 3);
        TEST_EQUAL(ibits(8), 1);
        TEST_EQUAL(ibits(9), 2);
        TEST_EQUAL(ibits(10), 2);
        TEST_EQUAL(ibits(0x7fff), 15);
        TEST_EQUAL(ibits(0x8000), 1);
        TEST_EQUAL(ibits(0x8888), 4);
        TEST_EQUAL(ibits(0xffff), 16);
        TEST_EQUAL(ibits(0x7fffffffffffffffull), 63);
        TEST_EQUAL(ibits(0x8000000000000000ull), 1);
        TEST_EQUAL(ibits(0xffffffffffffffffull), 64);

        TEST_EQUAL(ifloor2(0), 0);
        TEST_EQUAL(ifloor2(1), 1);
        TEST_EQUAL(ifloor2(2), 2);
        TEST_EQUAL(ifloor2(3), 2);
        TEST_EQUAL(ifloor2(4), 4);
        TEST_EQUAL(ifloor2(5), 4);
        TEST_EQUAL(ifloor2(6), 4);
        TEST_EQUAL(ifloor2(7), 4);
        TEST_EQUAL(ifloor2(8), 8);
        TEST_EQUAL(ifloor2(9), 8);
        TEST_EQUAL(ifloor2(10), 8);
        TEST_EQUAL(ifloor2(0x7ffful), 0x4000ul);
        TEST_EQUAL(ifloor2(0x8000ul), 0x8000ul);
        TEST_EQUAL(ifloor2(0x8001ul), 0x8000ul);
        TEST_EQUAL(ifloor2(0xfffful), 0x8000ul);

        TEST_EQUAL(iceil2(0), 0);
        TEST_EQUAL(iceil2(1), 1);
        TEST_EQUAL(iceil2(2), 2);
        TEST_EQUAL(iceil2(3), 4);
        TEST_EQUAL(iceil2(4), 4);
        TEST_EQUAL(iceil2(5), 8);
        TEST_EQUAL(iceil2(6), 8);
        TEST_EQUAL(iceil2(7), 8);
        TEST_EQUAL(iceil2(8), 8);
        TEST_EQUAL(iceil2(9), 16);
        TEST_EQUAL(iceil2(10), 16);
        TEST_EQUAL(iceil2(0x7ffful), 0x8000ul);
        TEST_EQUAL(iceil2(0x8000ul), 0x8000ul);
        TEST_EQUAL(iceil2(0x8001ul), 0x10000ul);
        TEST_EQUAL(iceil2(0xfffful), 0x10000ul);

        TEST(! ispow2(0));
        TEST(ispow2(1));
        TEST(ispow2(2));
        TEST(! ispow2(3));
        TEST(ispow2(4));
        TEST(! ispow2(5));
        TEST(! ispow2(6));
        TEST(! ispow2(7));
        TEST(ispow2(8));
        TEST(! ispow2(9));
        TEST(! ispow2(10));
        TEST(! ispow2(0x7fff));
        TEST(ispow2(0x8000));
        TEST(! ispow2(0x8001));
        TEST(! ispow2(0xffff));
        TEST(! ispow2(0x7fffffffffffffffull));
        TEST(ispow2(0x8000000000000000ull));
        TEST(! ispow2(0x8000000000000001ull));
        TEST(! ispow2(0xffffffffffffffffull));

        TEST_EQUAL(ilog2p1(0), 0);
        TEST_EQUAL(ilog2p1(1), 1);
        TEST_EQUAL(ilog2p1(2), 2);
        TEST_EQUAL(ilog2p1(3), 2);
        TEST_EQUAL(ilog2p1(4), 3);
        TEST_EQUAL(ilog2p1(5), 3);
        TEST_EQUAL(ilog2p1(6), 3);
        TEST_EQUAL(ilog2p1(7), 3);
        TEST_EQUAL(ilog2p1(8), 4);
        TEST_EQUAL(ilog2p1(9), 4);
        TEST_EQUAL(ilog2p1(10), 4);
        TEST_EQUAL(ilog2p1(0x7fff), 15);
        TEST_EQUAL(ilog2p1(0x8000), 16);
        TEST_EQUAL(ilog2p1(0x8888), 16);
        TEST_EQUAL(ilog2p1(0xffff), 16);
        TEST_EQUAL(ilog2p1(0x7fffffffffffffffull), 63);
        TEST_EQUAL(ilog2p1(0x8000000000000000ull), 64);
        TEST_EQUAL(ilog2p1(0xffffffffffffffffull), 64);

        TEST_EQUAL(letter_to_mask('A'), 1);
        TEST_EQUAL(letter_to_mask('B'), 2);
        TEST_EQUAL(letter_to_mask('C'), 4);
        TEST_EQUAL(letter_to_mask('Z'), 0x2000000ull);
        TEST_EQUAL(letter_to_mask('a'), 0x4000000ull);
        TEST_EQUAL(letter_to_mask('b'), 0x8000000ull);
        TEST_EQUAL(letter_to_mask('c'), 0x10000000ull);
        TEST_EQUAL(letter_to_mask('z'), 0x8000000000000ull);
        TEST_EQUAL(letter_to_mask('~'), 0);

        uint16_t u = 0x1234;

        TEST_EQUAL(rotl(u, 0), 0x1234);    TEST_EQUAL(rotr(u, 0), 0x1234);
        TEST_EQUAL(rotl(u, 4), 0x2341);    TEST_EQUAL(rotr(u, 4), 0x4123);
        TEST_EQUAL(rotl(u, 8), 0x3412);    TEST_EQUAL(rotr(u, 8), 0x3412);
        TEST_EQUAL(rotl(u, 12), 0x4123);   TEST_EQUAL(rotr(u, 12), 0x2341);
        TEST_EQUAL(rotl(u, 16), 0x1234);   TEST_EQUAL(rotr(u, 16), 0x1234);
        TEST_EQUAL(rotl(u, 20), 0x2341);   TEST_EQUAL(rotr(u, 20), 0x4123);
        TEST_EQUAL(rotl(u, 24), 0x3412);   TEST_EQUAL(rotr(u, 24), 0x3412);
        TEST_EQUAL(rotl(u, 28), 0x4123);   TEST_EQUAL(rotr(u, 28), 0x2341);
        TEST_EQUAL(rotl(u, 32), 0x1234);   TEST_EQUAL(rotr(u, 32), 0x1234);
        TEST_EQUAL(rotl(u, -4), 0x4123);   TEST_EQUAL(rotr(u, -4), 0x2341);
        TEST_EQUAL(rotl(u, -8), 0x3412);   TEST_EQUAL(rotr(u, -8), 0x3412);
        TEST_EQUAL(rotl(u, -12), 0x2341);  TEST_EQUAL(rotr(u, -12), 0x4123);
        TEST_EQUAL(rotl(u, -16), 0x1234);  TEST_EQUAL(rotr(u, -16), 0x1234);
        TEST_EQUAL(rotl(u, -20), 0x4123);  TEST_EQUAL(rotr(u, -20), 0x2341);
        TEST_EQUAL(rotl(u, -24), 0x3412);  TEST_EQUAL(rotr(u, -24), 0x3412);
        TEST_EQUAL(rotl(u, -28), 0x2341);  TEST_EQUAL(rotr(u, -28), 0x4123);
        TEST_EQUAL(rotl(u, -32), 0x1234);  TEST_EQUAL(rotr(u, -32), 0x1234);

        uint32_t v = 0x12345678;

        TEST_EQUAL(rotl(v, 0), 0x12345678);    TEST_EQUAL(rotr(v, 0), 0x12345678);
        TEST_EQUAL(rotl(v, 4), 0x23456781);    TEST_EQUAL(rotr(v, 4), 0x81234567);
        TEST_EQUAL(rotl(v, 8), 0x34567812);    TEST_EQUAL(rotr(v, 8), 0x78123456);
        TEST_EQUAL(rotl(v, 12), 0x45678123);   TEST_EQUAL(rotr(v, 12), 0x67812345);
        TEST_EQUAL(rotl(v, 16), 0x56781234);   TEST_EQUAL(rotr(v, 16), 0x56781234);
        TEST_EQUAL(rotl(v, 20), 0x67812345);   TEST_EQUAL(rotr(v, 20), 0x45678123);
        TEST_EQUAL(rotl(v, 24), 0x78123456);   TEST_EQUAL(rotr(v, 24), 0x34567812);
        TEST_EQUAL(rotl(v, 28), 0x81234567);   TEST_EQUAL(rotr(v, 28), 0x23456781);
        TEST_EQUAL(rotl(v, 32), 0x12345678);   TEST_EQUAL(rotr(v, 32), 0x12345678);
        TEST_EQUAL(rotl(v, 36), 0x23456781);   TEST_EQUAL(rotr(v, 36), 0x81234567);
        TEST_EQUAL(rotl(v, 40), 0x34567812);   TEST_EQUAL(rotr(v, 40), 0x78123456);
        TEST_EQUAL(rotl(v, 44), 0x45678123);   TEST_EQUAL(rotr(v, 44), 0x67812345);
        TEST_EQUAL(rotl(v, 48), 0x56781234);   TEST_EQUAL(rotr(v, 48), 0x56781234);
        TEST_EQUAL(rotl(v, 52), 0x67812345);   TEST_EQUAL(rotr(v, 52), 0x45678123);
        TEST_EQUAL(rotl(v, 56), 0x78123456);   TEST_EQUAL(rotr(v, 56), 0x34567812);
        TEST_EQUAL(rotl(v, 60), 0x81234567);   TEST_EQUAL(rotr(v, 60), 0x23456781);
        TEST_EQUAL(rotl(v, 64), 0x12345678);   TEST_EQUAL(rotr(v, 64), 0x12345678);
        TEST_EQUAL(rotl(v, -4), 0x81234567);   TEST_EQUAL(rotr(v, -4), 0x23456781);
        TEST_EQUAL(rotl(v, -8), 0x78123456);   TEST_EQUAL(rotr(v, -8), 0x34567812);
        TEST_EQUAL(rotl(v, -12), 0x67812345);  TEST_EQUAL(rotr(v, -12), 0x45678123);
        TEST_EQUAL(rotl(v, -16), 0x56781234);  TEST_EQUAL(rotr(v, -16), 0x56781234);
        TEST_EQUAL(rotl(v, -20), 0x45678123);  TEST_EQUAL(rotr(v, -20), 0x67812345);
        TEST_EQUAL(rotl(v, -24), 0x34567812);  TEST_EQUAL(rotr(v, -24), 0x78123456);
        TEST_EQUAL(rotl(v, -28), 0x23456781);  TEST_EQUAL(rotr(v, -28), 0x81234567);
        TEST_EQUAL(rotl(v, -32), 0x12345678);  TEST_EQUAL(rotr(v, -32), 0x12345678);
        TEST_EQUAL(rotl(v, -36), 0x81234567);  TEST_EQUAL(rotr(v, -36), 0x23456781);
        TEST_EQUAL(rotl(v, -40), 0x78123456);  TEST_EQUAL(rotr(v, -40), 0x34567812);
        TEST_EQUAL(rotl(v, -44), 0x67812345);  TEST_EQUAL(rotr(v, -44), 0x45678123);
        TEST_EQUAL(rotl(v, -48), 0x56781234);  TEST_EQUAL(rotr(v, -48), 0x56781234);
        TEST_EQUAL(rotl(v, -52), 0x45678123);  TEST_EQUAL(rotr(v, -52), 0x67812345);
        TEST_EQUAL(rotl(v, -56), 0x34567812);  TEST_EQUAL(rotr(v, -56), 0x78123456);
        TEST_EQUAL(rotl(v, -60), 0x23456781);  TEST_EQUAL(rotr(v, -60), 0x81234567);
        TEST_EQUAL(rotl(v, -64), 0x12345678);  TEST_EQUAL(rotr(v, -64), 0x12345678);

    }

    void check_floating_point_arithmetic_functions() {

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
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 0.5), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 1.0), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 1.5), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 10.0, 0.5), 7.5);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 10.0, 1.0), 7.5);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 10.0, 1.5), 7.5);

        TEST_EQUAL(iceil<int>(42), 42);          TEST_EQUAL(ifloor<int>(42), 42);          TEST_EQUAL(iround<int>(42), 42);
        TEST_EQUAL(iceil<double>(42), 42.0);     TEST_EQUAL(ifloor<double>(42), 42.0);     TEST_EQUAL(iround<double>(42), 42.0);
        TEST_EQUAL(iceil<double>(42.0), 42.0);   TEST_EQUAL(ifloor<double>(42.0), 42.0);   TEST_EQUAL(iround<double>(42.0), 42.0);
        TEST_EQUAL(iceil<double>(42.42), 43.0);  TEST_EQUAL(ifloor<double>(42.42), 42.0);  TEST_EQUAL(iround<double>(42.42), 42.0);
        TEST_EQUAL(iceil<float>(42.0), 42.0f);   TEST_EQUAL(ifloor<float>(42.0), 42.0f);   TEST_EQUAL(iround<float>(42.0), 42.0f);
        TEST_EQUAL(iceil<float>(42.42), 43.0f);  TEST_EQUAL(ifloor<float>(42.42), 42.0f);  TEST_EQUAL(iround<float>(42.42), 42.0f);
        TEST_EQUAL(iceil<int>(0.0), 0);          TEST_EQUAL(ifloor<int>(0.0), 0);          TEST_EQUAL(iround<int>(0.0), 0);
        TEST_EQUAL(iceil<int>(0.25), 1);         TEST_EQUAL(ifloor<int>(0.25), 0);         TEST_EQUAL(iround<int>(0.25), 0);
        TEST_EQUAL(iceil<int>(0.5), 1);          TEST_EQUAL(ifloor<int>(0.5), 0);          TEST_EQUAL(iround<int>(0.5), 1);
        TEST_EQUAL(iceil<int>(0.75), 1);         TEST_EQUAL(ifloor<int>(0.75), 0);         TEST_EQUAL(iround<int>(0.75), 1);
        TEST_EQUAL(iceil<int>(1), 1);            TEST_EQUAL(ifloor<int>(1), 1);            TEST_EQUAL(iround<int>(1), 1);
        TEST_EQUAL(iceil<int>(-0.25), 0);        TEST_EQUAL(ifloor<int>(-0.25), -1);       TEST_EQUAL(iround<int>(-0.25), 0);
        TEST_EQUAL(iceil<int>(-0.5), 0);         TEST_EQUAL(ifloor<int>(-0.5), -1);        TEST_EQUAL(iround<int>(-0.5), 0);
        TEST_EQUAL(iceil<int>(-0.75), 0);        TEST_EQUAL(ifloor<int>(-0.75), -1);       TEST_EQUAL(iround<int>(-0.75), -1);
        TEST_EQUAL(iceil<int>(-1), -1);          TEST_EQUAL(ifloor<int>(-1), -1);          TEST_EQUAL(iround<int>(-1), -1);
        TEST_EQUAL(iceil<double>(0.0), 0.0);     TEST_EQUAL(ifloor<double>(0.0), 0.0);     TEST_EQUAL(iround<double>(0.0), 0.0);
        TEST_EQUAL(iceil<double>(0.25), 1.0);    TEST_EQUAL(ifloor<double>(0.25), 0.0);    TEST_EQUAL(iround<double>(0.25), 0.0);
        TEST_EQUAL(iceil<double>(0.5), 1.0);     TEST_EQUAL(ifloor<double>(0.5), 0.0);     TEST_EQUAL(iround<double>(0.5), 1.0);
        TEST_EQUAL(iceil<double>(0.75), 1.0);    TEST_EQUAL(ifloor<double>(0.75), 0.0);    TEST_EQUAL(iround<double>(0.75), 1.0);
        TEST_EQUAL(iceil<double>(1), 1.0);       TEST_EQUAL(ifloor<double>(1), 1.0);       TEST_EQUAL(iround<double>(1), 1.0);
        TEST_EQUAL(iceil<double>(-0.25), 0.0);   TEST_EQUAL(ifloor<double>(-0.25), -1.0);  TEST_EQUAL(iround<double>(-0.25), 0.0);
        TEST_EQUAL(iceil<double>(-0.5), 0.0);    TEST_EQUAL(ifloor<double>(-0.5), -1.0);   TEST_EQUAL(iround<double>(-0.5), 0.0);
        TEST_EQUAL(iceil<double>(-0.75), 0.0);   TEST_EQUAL(ifloor<double>(-0.75), -1.0);  TEST_EQUAL(iround<double>(-0.75), -1.0);
        TEST_EQUAL(iceil<double>(-1), -1.0);     TEST_EQUAL(ifloor<double>(-1), -1.0);     TEST_EQUAL(iround<double>(-1), -1.0);

    }

    U8string fa0() { return "hello"; }
    U8string fa1(char c) { return {c}; }
    U8string fa2(size_t n, char c) { return U8string(n, c); }

    U8string (*fb0)() = &fa0;
    U8string (*fb1)(char c) = &fa1;
    U8string (*fb2)(size_t n, char c) = &fa2;

    struct FC0 { U8string operator()() { return "hello"; } };
    struct FC1 { U8string operator()(char c) { return {c}; } };
    struct FC2 { U8string operator()(size_t n, char c) { return U8string(n, c); } };

    struct FD0 { U8string operator()() const { return "hello"; } };
    struct FD1 { U8string operator()(char c) const { return {c}; } };
    struct FD2 { U8string operator()(size_t n, char c) const { return U8string(n, c); } };

    FC0 fc0;
    FC1 fc1;
    FC2 fc2;

    const FD0 fd0 = {};
    const FD1 fd1 = {};
    const FD2 fd2 = {};

    auto fe0 = [] () -> U8string { return "hello"; };
    auto fe1 = [] (char c) -> U8string { return {c}; };
    auto fe2 = [] (size_t n, char c) -> U8string { return U8string(n, c); };

    using tuple0 = std::tuple<>;
    using tuple1 = std::tuple<char>;
    using tuple2 = std::tuple<size_t, char>;

    void check_function_traits() {

        TEST_EQUAL(fa0(), "hello");  TEST_EQUAL(fa1('a'), "a");  TEST_EQUAL(fa2(5, 'z'), "zzzzz");
        TEST_EQUAL(fb0(), "hello");  TEST_EQUAL(fb1('a'), "a");  TEST_EQUAL(fb2(5, 'z'), "zzzzz");
        TEST_EQUAL(fc0(), "hello");  TEST_EQUAL(fc1('a'), "a");  TEST_EQUAL(fc2(5, 'z'), "zzzzz");
        TEST_EQUAL(fd0(), "hello");  TEST_EQUAL(fd1('a'), "a");  TEST_EQUAL(fd2(5, 'z'), "zzzzz");
        TEST_EQUAL(fe0(), "hello");  TEST_EQUAL(fe1('a'), "a");  TEST_EQUAL(fe2(5, 'z'), "zzzzz");

        using FTA0 = decltype(fa0);  using FTA1 = decltype(fa1);  using FTA2 = decltype(fa2);
        using FTB0 = decltype(fb0);  using FTB1 = decltype(fb1);  using FTB2 = decltype(fb2);
        using FTC0 = decltype(fc0);  using FTC1 = decltype(fc1);  using FTC2 = decltype(fc2);
        using FTD0 = decltype(fd0);  using FTD1 = decltype(fd1);  using FTD2 = decltype(fd2);
        using FTE0 = decltype(fe0);  using FTE1 = decltype(fe1);  using FTE2 = decltype(fe2);

        TEST_EQUAL(Arity<FTA0>::value, 0);  TEST_EQUAL(Arity<FTA1>::value, 1);  TEST_EQUAL(Arity<FTA2>::value, 2);
        TEST_EQUAL(Arity<FTB0>::value, 0);  TEST_EQUAL(Arity<FTB1>::value, 1);  TEST_EQUAL(Arity<FTB2>::value, 2);
        TEST_EQUAL(Arity<FTC0>::value, 0);  TEST_EQUAL(Arity<FTC1>::value, 1);  TEST_EQUAL(Arity<FTC2>::value, 2);
        TEST_EQUAL(Arity<FTD0>::value, 0);  TEST_EQUAL(Arity<FTD1>::value, 1);  TEST_EQUAL(Arity<FTD2>::value, 2);
        TEST_EQUAL(Arity<FTE0>::value, 0);  TEST_EQUAL(Arity<FTE1>::value, 1);  TEST_EQUAL(Arity<FTE2>::value, 2);

        TEST_TYPE(ArgumentTuple<FTA0>, tuple0);  TEST_TYPE(ArgumentTuple<FTA1>, tuple1);  TEST_TYPE(ArgumentTuple<FTA2>, tuple2);
        TEST_TYPE(ArgumentTuple<FTB0>, tuple0);  TEST_TYPE(ArgumentTuple<FTB1>, tuple1);  TEST_TYPE(ArgumentTuple<FTB2>, tuple2);
        TEST_TYPE(ArgumentTuple<FTC0>, tuple0);  TEST_TYPE(ArgumentTuple<FTC1>, tuple1);  TEST_TYPE(ArgumentTuple<FTC2>, tuple2);
        TEST_TYPE(ArgumentTuple<FTD0>, tuple0);  TEST_TYPE(ArgumentTuple<FTD1>, tuple1);  TEST_TYPE(ArgumentTuple<FTD2>, tuple2);
        TEST_TYPE(ArgumentTuple<FTE0>, tuple0);  TEST_TYPE(ArgumentTuple<FTE1>, tuple1);  TEST_TYPE(ArgumentTuple<FTE2>, tuple2);

        { using T = ArgumentType<FTA1, 0>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTA2, 0>; TEST_TYPE(T, size_t); }
        { using T = ArgumentType<FTA2, 1>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTB1, 0>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTB2, 0>; TEST_TYPE(T, size_t); }
        { using T = ArgumentType<FTB2, 1>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTC1, 0>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTC2, 0>; TEST_TYPE(T, size_t); }
        { using T = ArgumentType<FTC2, 1>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTD1, 0>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTD2, 0>; TEST_TYPE(T, size_t); }
        { using T = ArgumentType<FTD2, 1>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTE1, 0>; TEST_TYPE(T, char); }
        { using T = ArgumentType<FTE2, 0>; TEST_TYPE(T, size_t); }
        { using T = ArgumentType<FTE2, 1>; TEST_TYPE(T, char); }

        TEST_TYPE(ReturnType<FTA0>, U8string);  TEST_TYPE(ReturnType<FTA1>, U8string);  TEST_TYPE(ReturnType<FTA2>, U8string);
        TEST_TYPE(ReturnType<FTB0>, U8string);  TEST_TYPE(ReturnType<FTB1>, U8string);  TEST_TYPE(ReturnType<FTB2>, U8string);
        TEST_TYPE(ReturnType<FTC0>, U8string);  TEST_TYPE(ReturnType<FTC1>, U8string);  TEST_TYPE(ReturnType<FTC2>, U8string);
        TEST_TYPE(ReturnType<FTD0>, U8string);  TEST_TYPE(ReturnType<FTD1>, U8string);  TEST_TYPE(ReturnType<FTD2>, U8string);
        TEST_TYPE(ReturnType<FTE0>, U8string);  TEST_TYPE(ReturnType<FTE1>, U8string);  TEST_TYPE(ReturnType<FTE2>, U8string);

    }

    void check_function_operations() {

        auto t0 = tuple0{};
        auto t1 = tuple1{'a'};
        auto t2 = tuple2{5, 'z'};

        TEST_EQUAL(tuple_invoke(fa0, t0), "hello");  TEST_EQUAL(tuple_invoke(fa1, t1), "a");  TEST_EQUAL(tuple_invoke(fa2, t2), "zzzzz");
        TEST_EQUAL(tuple_invoke(fb0, t0), "hello");  TEST_EQUAL(tuple_invoke(fb1, t1), "a");  TEST_EQUAL(tuple_invoke(fb2, t2), "zzzzz");
        TEST_EQUAL(tuple_invoke(fc0, t0), "hello");  TEST_EQUAL(tuple_invoke(fc1, t1), "a");  TEST_EQUAL(tuple_invoke(fc2, t2), "zzzzz");
        TEST_EQUAL(tuple_invoke(fd0, t0), "hello");  TEST_EQUAL(tuple_invoke(fd1, t1), "a");  TEST_EQUAL(tuple_invoke(fd2, t2), "zzzzz");
        TEST_EQUAL(tuple_invoke(fe0, t0), "hello");  TEST_EQUAL(tuple_invoke(fe1, t1), "a");  TEST_EQUAL(tuple_invoke(fe2, t2), "zzzzz");

        auto lf1 = [] (int x) { return x * x; };
        auto sf1 = stdfun(lf1);
        TEST_TYPE_OF(sf1, std::function<int(int)>);
        TEST_EQUAL(sf1(5), 25);

        int z = 0;
        auto lf2 = [&] (int x, int y) { z = x * y; };
        auto sf2 = stdfun(lf2);
        TEST_TYPE_OF(sf2, std::function<void(int, int)>);
        TRY(sf2(6, 7));
        TEST_EQUAL(z, 42);

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

    void check_generic_function_objects() {

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
        U8string s1 = "Hello world", s2;
        U8string& sr(s1);
        const U8string& csr(s1);

        TRY(n2 = identity(n1));              TEST_EQUAL(n2, 42);
        TRY(n2 = identity(100));             TEST_EQUAL(n2, 100);
        TRY(s2 = identity(s1));              TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity(sr));              TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity(csr));             TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity("Goodbye"));       TEST_EQUAL(s2, "Goodbye");
        TRY(s2 = identity("Hello again"s));  TEST_EQUAL(s2, "Hello again");

    }

    void check_hash_functions() {

        uint32_t u = 0;
        size_t h1 = 0, h2 = 0, h3 = 0, h4 = 0, h5 = 0;
        std::string s0, s1 = "Hello world";

        TRY(u = Djb2a());                      TEST_EQUAL(u, 0x00001505);
        TRY(u = Djb2a()(s0));                  TEST_EQUAL(u, 0x00001505);
        TRY(u = Djb2a()(s1));                  TEST_EQUAL(u, 0x33c13465);
        TRY(u = djb2a(s0.data(), s0.size()));  TEST_EQUAL(u, 0x00001505);
        TRY(u = djb2a(s1.data(), s1.size()));  TEST_EQUAL(u, 0x33c13465);

        TRY(hash_combine(h1, 42));
        TRY(hash_combine(h2, 42, s1));
        TRY(hash_combine(h3, 86, 99, s1));
        TEST_COMPARE(h1, !=, 0);
        TEST_COMPARE(h2, !=, 0);
        TEST_COMPARE(h3, !=, 0);
        TEST_COMPARE(h1, !=, h2);
        TEST_COMPARE(h1, !=, h3);
        TEST_COMPARE(h2, !=, h3);

        std::vector<std::string> sv {"hello", "world", "goodbye"};

        for (auto& s: sv)
            TRY(hash_combine(h4, s));
        TRY(h5 = hash_range(sv));
        TEST_EQUAL(h4, h5);

    }

    constexpr Kwarg<int> kw_alpha;
    constexpr Kwarg<bool> kw_bravo;
    constexpr Kwarg<std::string> kw_charlie;

    struct Kwtest {
        int a = 0;
        bool b = false;
        std::string c;
        template <typename... Args> int fun(const Args&... args) {
            auto i = int(kwget(kw_alpha, a, args...));
            auto j = int(kwget(kw_bravo, b, args...));
            auto k = int(kwget(kw_charlie, c, args...));
            return (i << 2) + (j << 1) + k;
        }
    };

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

    void check_scope_guards() {

        int n;
        std::string s;

        {
            n = 1;
            Resource<int> r;
            TRY((r = {2, [&] (int i) { n = i; }}));
            TEST(bool(r));
            TEST_EQUAL(r.get(), 2);
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 2);

        {
            n = 1;
            Resource<int> r;
            TRY((r = {2, [&] (int i) { n = i; }}));
            TEST(bool(r));
            TEST_EQUAL(r.get(), 2);
            TEST_EQUAL(n, 1);
            TRY(r = {});
            TEST_EQUAL(n, 2);
        }
        TEST_EQUAL(n, 2);

        {
            n = 1;
            Resource<int> r;
            TRY((r = {2, [&] (int i) { n = i; }}));
            TEST(bool(r));
            TEST_EQUAL(r.get(), 2);
            TEST_EQUAL(n, 1);
            TRY(r.release());
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 1);

        {
            n = 1;
            Resource<int> r;
            TRY((r = {2, [&] (int i) { n = i; }}));
            TEST(bool(r));
            TEST_EQUAL(r.get(), 2);
            TEST_EQUAL(n, 1);
            TRY(r.get() = 3);
            TEST_EQUAL(r.get(), 3);
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 3);

        {
            n = 1;
            ScopeExit x([&] { n = 2; });
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 2);

        try {
            n = 1;
            ScopeExit x([&] { n = 2; });
            TEST_EQUAL(n, 1);
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(n, 2);

        {
            n = 1;
            ScopeSuccess x([&] { n = 2; });
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 2);

        try {
            n = 1;
            ScopeSuccess x([&] { n = 2; });
            TEST_EQUAL(n, 1);
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(n, 1);

        {
            n = 1;
            ScopeFailure x([&] { n = 2; });
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 1);

        try {
            n = 1;
            ScopeFailure x([&] { n = 2; });
            TEST_EQUAL(n, 1);
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(n, 2);

        {
            ScopeExit x(nullptr);
        }

        try {
            ScopeExit x(nullptr);
            throw std::runtime_error("fail");
        }
        catch (...) {}

        {
            ScopeSuccess x(nullptr);
        }

        try {
            ScopeSuccess x(nullptr);
            throw std::runtime_error("fail");
        }
        catch (...) {}

        {
            ScopeFailure x(nullptr);
        }

        try {
            ScopeFailure x(nullptr);
            throw std::runtime_error("fail");
        }
        catch (...) {}

        s = "hello";
        {
            SizeGuard g(s);
            s += " world";
            TEST_EQUAL(s, "hello world");
        }
        TEST_EQUAL(s, "hello world");

        s = "hello";
        try {
            SizeGuard g(s);
            s += " world";
            TEST_EQUAL(s, "hello world");
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(s, "hello");

        s = "hello";
        {
            SizeGuard g(s);
            s += " world";
            TEST_EQUAL(s, "hello world");
            TRY(g.release());
        }
        TEST_EQUAL(s, "hello world");

        s = "hello";
        try {
            SizeGuard g(s);
            s += " world";
            TEST_EQUAL(s, "hello world");
            TRY(g.release());
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(s, "hello world");

        s = "hello";
        {
            ValueGuard g(s);
            s = "goodbye";
            TEST_EQUAL(s, "goodbye");
        }
        TEST_EQUAL(s, "goodbye");

        s = "hello";
        try {
            ValueGuard g(s);
            s = "goodbye";
            TEST_EQUAL(s, "goodbye");
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(s, "hello");

        s = "hello";
        {
            ValueGuard g(s);
            s = "goodbye";
            TEST_EQUAL(s, "goodbye");
            TRY(g.release());
        }
        TEST_EQUAL(s, "goodbye");

        s = "hello";
        try {
            ValueGuard g(s);
            s = "goodbye";
            TEST_EQUAL(s, "goodbye");
            TRY(g.release());
            throw std::runtime_error("fail");
        }
        catch (...) {}
        TEST_EQUAL(s, "goodbye");

        s.clear();
        {
            ScopedTransaction t;
            TRY(t.call([&] { s += 'a'; }, [&] { s += 'z'; }));
            TRY(t.call([&] { s += 'b'; }, [&] { s += 'y'; }));
            TRY(t.call([&] { s += 'c'; }, [&] { s += 'x'; }));
            TEST_EQUAL(s, "abc");
        }
        TEST_EQUAL(s, "abcxyz");

        s.clear();
        {
            ScopedTransaction t;
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
            ScopedTransaction t;
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

    class TempFile {
    public:
        explicit TempFile(const U8string& file): f(file) {}
        ~TempFile() { remove(f.data()); }
    private:
        U8string f;
        TempFile(const TempFile&) = delete;
        TempFile(TempFile&&) = delete;
        TempFile& operator=(const TempFile&) = delete;
        TempFile& operator=(TempFile&&) = delete;
    };

    void check_file_io_operations() {

        std::string s, readme = "README.md", testfile = "__test__", nofile = "__no_such_file__";
        TempFile tempfile(testfile);

        TEST(load_file(readme, s));
        TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
        TEST(load_file(readme, s, 100));
        TEST_EQUAL(s.size(), 100);
        TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
        TEST(load_file(readme, s, 10));
        TEST_EQUAL(s.size(), 10);
        TEST_EQUAL(s.substr(0, 20), "# RS Core ");

        TEST(save_file(testfile, "Hello world\n"s));
        TEST(load_file(testfile, s));
        TEST_EQUAL(s, "Hello world\n");
        TEST(save_file(testfile, "Goodbye\n"s, true));
        TEST(load_file(testfile, s));
        TEST_EQUAL(s, "Hello world\nGoodbye\n");
        TEST(! load_file(nofile, s));

        #ifndef _XOPEN_SOURCE

            std::wstring wreadme = L"README.md", wtestfile = L"__test__", wnofile = L"__no_such_file__";

            TEST(load_file(wreadme, s));
            TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
            TEST(load_file(wreadme, s, 100));
            TEST_EQUAL(s.size(), 100);
            TEST_EQUAL(s.substr(0, 20), "# RS Core Library #\n");
            TEST(load_file(wreadme, s, 10));
            TEST_EQUAL(s.size(), 10);
            TEST_EQUAL(s.substr(0, 20), "# RS Core ");

            TEST(save_file(wtestfile, "Hello world\n"s));
            TEST(load_file(wtestfile, s));
            TEST_EQUAL(s, "Hello world\n");
            TEST(save_file(wtestfile, "Goodbye\n"s, true));
            TEST(load_file(wtestfile, s));
            TEST_EQUAL(s, "Hello world\nGoodbye\n");
            TEST(! load_file(wnofile, s));

        #endif

    }

    void check_process_io_operations() {

        U8string s;

        #ifdef _XOPEN_SOURCE
            TRY(s = run_command("ls"));
        #else
            TRY(s = run_command("dir /b"));
        #endif

        TEST(! s.empty());
        std::set<U8string> ss;
        TRY(split(s, append(ss), "\r\n"));
        TEST(! ss.empty());
        TEST(ss.count("Makefile"));
        TEST(ss.count("build"));

    }

    void check_terminal_io_operations() {

        U8string s;

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

    #define CHECK_RANDOM_GENERATOR(g, xmin, xmax, xmean, xsd) \
        do { \
            using T = std::decay_t<decltype(g())>; \
            double epsilon, lo = inf, hi = - inf, sum = 0, sum2 = 0; \
            if (xmin > - inf && xmax < inf) \
                epsilon = (xmax - xmin) / sqrt(double(iterations)); \
            else \
                epsilon = 2 * xsd / sqrt(double(iterations)); \
            T t = {}; \
            for (size_t i = 0; i < iterations; ++i) { \
                TRY(t = g()); \
                auto x = double(t); \
                lo = std::min(lo, x); \
                hi = std::max(hi, x); \
                sum += x; \
                sum2 += x * x; \
            } \
            if (std::is_floating_point<T>::value) { \
                TEST_COMPARE(lo, >, xmin); \
                TEST_COMPARE(hi, <, xmax); \
            } else if (xmax - xmin < 0.1 * iterations) { \
                TEST_EQUAL(lo, xmin); \
                TEST_EQUAL(hi, xmax); \
            } else { \
                TEST_COMPARE(lo, >=, xmin); \
                TEST_COMPARE(hi, <=, xmax); \
            } \
            double m = sum / iterations; \
            double s = sqrt(sum2 / iterations - m * m); \
            TEST_NEAR_EPSILON(m, xmean, epsilon); \
            TEST_NEAR_EPSILON(s, xsd, epsilon); \
        } while (false)

    void check_simple_random_generators() {

        constexpr size_t iterations = 100000;
        constexpr double inf = std::numeric_limits<double>::infinity();

        std::mt19937 rng(42);

        auto rbq = [&] { return random_bool(rng); };
        CHECK_RANDOM_GENERATOR(rbq, 0, 1, 0.5, 0.5);
        auto rb2 = [&] { return random_bool(rng, 0.25); };
        CHECK_RANDOM_GENERATOR(rb2, 0, 1, 0.25, 0.433013);
        auto rb3 = [&] { return random_bool(rng, 3, 4); };
        CHECK_RANDOM_GENERATOR(rb3, 0, 1, 0.75, 0.433013);

        auto ri1 = [&] { return random_integer(rng, 100); };
        CHECK_RANDOM_GENERATOR(ri1, 0, 99, 49.5, 28.8661);
        auto ri2 = [&] { return random_integer(rng, 101, 200); };
        CHECK_RANDOM_GENERATOR(ri2, 101, 200, 150.5, 28.8661);
        auto ri3 = [&] { return random_integer(rng, int64_t(0), int64_t(1e18)); };
        CHECK_RANDOM_GENERATOR(ri3, 0, 1e18, 5e17, 2.88661e17);
        auto ri4 = [&] { return random_integer(rng, uint64_t(0), uint64_t(-1)); };
        CHECK_RANDOM_GENERATOR(ri4, 0, 18446744073709551615.0, 9223372036854775807.5, 5325116328314171700.524384);

        auto rd1 = [&] { return random_dice<int>(rng); };
        CHECK_RANDOM_GENERATOR(rd1, 1, 6, 3.5, 1.70783);
        auto rd2 = [&] { return random_dice(rng, 3); };
        CHECK_RANDOM_GENERATOR(rd2, 3, 18, 10.5, 2.95804);
        auto rd3 = [&] { return random_dice(rng, 3, 10); };
        CHECK_RANDOM_GENERATOR(rd3, 3, 30, 16.5, 4.97494);

        auto rf1 = [&] { return random_float<double>(rng); };
        CHECK_RANDOM_GENERATOR(rf1, 0, 1, 0.5, 0.288661);
        auto rf2 = [&] { return random_float(rng, -100.0, 100.0); };
        CHECK_RANDOM_GENERATOR(rf2, -100, 100, 0, 57.7350);

        auto rn1 = [&] { return random_normal<double>(rng); };
        CHECK_RANDOM_GENERATOR(rn1, - inf, inf, 0, 1);
        auto rn2 = [&] { return random_normal(rng, 10.0, 5.0); };
        CHECK_RANDOM_GENERATOR(rn2, - inf, inf, 10, 5);

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10};
        auto rc1 = [&] { return random_choice(rng, v); };
        CHECK_RANDOM_GENERATOR(rc1, 1, 10, 5.5, 2.88661);
        auto rc2 = [&] { return random_choice(rng, {1,2,3,4,5,6,7,8,9,10}); };
        CHECK_RANDOM_GENERATOR(rc2, 1, 10, 5.5, 2.88661);

        static constexpr size_t pop_size = 100;
        static constexpr size_t sample_iterations = 100;
        const double expect_mean = double(pop_size + 1) / 2;
        const double expect_sd = sqrt(double(pop_size * pop_size - 1) / 12);
        std::vector<int> pop(pop_size), sample;
        std::iota(pop.begin(), pop.end(), 1);
        for (size_t n = 0; n <= pop_size; ++n) {
            double count = 0, sum = 0, sum2 = 0;
            for (size_t i = 0; i < sample_iterations; ++i) {
                TRY(sample = random_sample(rng, pop, n));
                TEST_EQUAL(sample.size(), n);
                TRY(con_sort_unique(sample));
                TEST_EQUAL(sample.size(), n);
                count += sample.size();
                for (auto x: sample) {
                    sum += x;
                    sum2 += x * x;
                }
            }
            if (n > 0) {
                double mean = sum / count;
                TEST_NEAR_EPSILON(mean, expect_mean, 4);
                if (n > 0) {
                    double sd = sqrt((sum2 - count * mean * mean) / (count - 1));
                    TEST_NEAR_EPSILON(sd, expect_sd, 2);
                }
            }
        }
        TEST_THROW(random_sample(rng, pop, 101), std::length_error);

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

    void check_general_string_functions() {

        std::string s;
        std::wstring ws;
        std::vector<std::string> sv;

        const char* p0 = nullptr;
        const char* p1 = "";
        const char* p2 = "Hello";
        const char16_t* q0 = nullptr;
        const char16_t* q1 = u"";
        const char16_t* q2 = u"Hello";
        const char32_t* r0 = nullptr;
        const char32_t* r1 = U"";
        const char32_t* r2 = U"Hello";

        TEST(ascii_icase_equal("", ""));                          TEST(! ascii_icase_less("", ""));
        TEST(! ascii_icase_equal("", "hello"));                   TEST(ascii_icase_less("", "hello"));
        TEST(! ascii_icase_equal("hello", ""));                   TEST(! ascii_icase_less("hello", ""));
        TEST(ascii_icase_equal("hello", "hello"));                TEST(! ascii_icase_less("hello", "hello"));
        TEST(ascii_icase_equal("hello", "HELLO"));                TEST(! ascii_icase_less("hello", "HELLO"));
        TEST(ascii_icase_equal("HELLO", "hello"));                TEST(! ascii_icase_less("HELLO", "hello"));
        TEST(! ascii_icase_equal("hello", "hello world"));        TEST(ascii_icase_less("hello", "hello world"));
        TEST(! ascii_icase_equal("hello", "HELLO WORLD"));        TEST(ascii_icase_less("hello", "HELLO WORLD"));
        TEST(! ascii_icase_equal("HELLO", "hello world"));        TEST(ascii_icase_less("HELLO", "hello world"));
        TEST(! ascii_icase_equal("hello world", "hello"));        TEST(! ascii_icase_less("hello world", "hello"));
        TEST(! ascii_icase_equal("hello world", "HELLO"));        TEST(! ascii_icase_less("hello world", "HELLO"));
        TEST(! ascii_icase_equal("HELLO WORLD", "hello"));        TEST(! ascii_icase_less("HELLO WORLD", "hello"));
        TEST(! ascii_icase_equal("hello there", "hello world"));  TEST(ascii_icase_less("hello there", "hello world"));
        TEST(! ascii_icase_equal("hello there", "HELLO WORLD"));  TEST(ascii_icase_less("hello there", "HELLO WORLD"));
        TEST(! ascii_icase_equal("HELLO THERE", "hello world"));  TEST(ascii_icase_less("HELLO THERE", "hello world"));
        TEST(! ascii_icase_equal("hello world", "hello there"));  TEST(! ascii_icase_less("hello world", "hello there"));
        TEST(! ascii_icase_equal("hello world", "HELLO THERE"));  TEST(! ascii_icase_less("hello world", "HELLO THERE"));
        TEST(! ascii_icase_equal("HELLO WORLD", "hello there"));  TEST(! ascii_icase_less("HELLO WORLD", "hello there"));

        TEST_EQUAL(ascii_lowercase("Hello World"s), "hello world");
        TEST_EQUAL(ascii_uppercase("Hello World"s), "HELLO WORLD");
        TEST_EQUAL(ascii_titlecase("hello world"s), "Hello World");
        TEST_EQUAL(ascii_titlecase("HELLO WORLD"s), "Hello World");

        TEST_EQUAL(ascii_sentencecase(""), "");
        TEST_EQUAL(ascii_sentencecase("hello world"), "Hello world");
        TEST_EQUAL(ascii_sentencecase("hello world. goodbye. hello again."), "Hello world. Goodbye. Hello again.");
        TEST_EQUAL(ascii_sentencecase("hello world\ngoodbye\nhello again"), "Hello world\ngoodbye\nhello again");
        TEST_EQUAL(ascii_sentencecase("hello world\n\ngoodbye\n\nhello again"), "Hello world\n\nGoodbye\n\nHello again");
        TEST_EQUAL(ascii_sentencecase("hello world\r\n\r\ngoodbye\r\n\r\nhello again"), "Hello world\r\n\r\nGoodbye\r\n\r\nHello again");

        TEST_EQUAL(cstr(p0), ""s);
        TEST_EQUAL(cstr(p1), ""s);
        TEST_EQUAL(cstr(p2), "Hello"s);
        TEST_EQUAL(cstr(p0, 0), ""s);
        TEST_EQUAL(cstr(p1, 0), ""s);
        TEST_EQUAL(cstr(p2, 0), ""s);
        TEST_EQUAL(cstr(p0, 5), "\0\0\0\0\0"s);
        TEST_EQUAL(cstr(p2, 5), "Hello"s);
        TEST_EQUAL(cstr(q0), u""s);
        TEST_EQUAL(cstr(q1), u""s);
        TEST_EQUAL(cstr(q2), u"Hello"s);
        TEST_EQUAL(cstr(q0, 0), u""s);
        TEST_EQUAL(cstr(q1, 0), u""s);
        TEST_EQUAL(cstr(q2, 0), u""s);
        TEST_EQUAL(cstr(q0, 5), u"\0\0\0\0\0"s);
        TEST_EQUAL(cstr(q2, 5), u"Hello"s);
        TEST_EQUAL(cstr(r0), U""s);
        TEST_EQUAL(cstr(r1), U""s);
        TEST_EQUAL(cstr(r2), U"Hello"s);
        TEST_EQUAL(cstr(r0, 0), U""s);
        TEST_EQUAL(cstr(r1, 0), U""s);
        TEST_EQUAL(cstr(r2, 0), U""s);
        TEST_EQUAL(cstr(r0, 5), U"\0\0\0\0\0"s);
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

        sv.clear();                      TEST_EQUAL(join(sv), "");
        sv = {"Hello"};                  TEST_EQUAL(join(sv), "Hello");
        sv = {"Hello","world"};          TEST_EQUAL(join(sv), "Helloworld");
        sv = {"Hello","world","again"};  TEST_EQUAL(join(sv), "Helloworldagain");
        sv.clear();                      TEST_EQUAL(join(sv, " "), "");
        sv = {"Hello"};                  TEST_EQUAL(join(sv, " "), "Hello");
        sv = {"Hello","world"};          TEST_EQUAL(join(sv, " "), "Hello world");
        sv = {"Hello","world","again"};  TEST_EQUAL(join(sv, " "), "Hello world again");
        sv.clear();                      TEST_EQUAL(join(sv, "<*>"s), "");
        sv = {"Hello"};                  TEST_EQUAL(join(sv, "<*>"s), "Hello");
        sv = {"Hello","world"};          TEST_EQUAL(join(sv, "<*>"s), "Hello<*>world");
        sv = {"Hello","world","again"};  TEST_EQUAL(join(sv, "<*>"s), "Hello<*>world<*>again");
        sv.clear();                      TEST_EQUAL(join(sv, "\n", true), "");
        sv = {"Hello"};                  TEST_EQUAL(join(sv, "\n", true), "Hello\n");
        sv = {"Hello","world"};          TEST_EQUAL(join(sv, "\n", true), "Hello\nworld\n");
        sv = {"Hello","world","again"};  TEST_EQUAL(join(sv, "\n", true), "Hello\nworld\nagain\n");

        TRY(s = linearize(""));                                  TEST_EQUAL(s, "");
        TRY(s = linearize("\r\n\r\n"));                          TEST_EQUAL(s, "");
        TRY(s = linearize("Hello world."));                      TEST_EQUAL(s, "Hello world.");
        TRY(s = linearize("\r\nHello world.\r\nGoodbye.\r\n"));  TEST_EQUAL(s, "Hello world. Goodbye.");

        s = ""s;                TRY(null_term(s));   TEST_EQUAL(s, "");
        s = "Hello world"s;     TRY(null_term(s));   TEST_EQUAL(s, "Hello world");
        s = "Hello\0world"s;    TRY(null_term(s));   TEST_EQUAL(s, "Hello");
        ws = L""s;              TRY(null_term(ws));  TEST_EQUAL(ws, L"");
        ws = L"Hello world"s;   TRY(null_term(ws));  TEST_EQUAL(ws, L"Hello world");
        ws = L"Hello\0world"s;  TRY(null_term(ws));  TEST_EQUAL(ws, L"Hello");

        TRY(s = quote(""s));                      TEST_EQUAL(s, "\"\""s);
        TRY(s = quote("\"\""s));                  TEST_EQUAL(s, "\"\\\"\\\"\""s);
        TRY(s = quote("Hello world"s));           TEST_EQUAL(s, "\"Hello world\""s);
        TRY(s = quote("\\Hello\\world\\"s));      TEST_EQUAL(s, "\"\\\\Hello\\\\world\\\\\""s);
        TRY(s = quote("\"Hello\" \"world\""s));   TEST_EQUAL(s, "\"\\\"Hello\\\" \\\"world\\\"\""s);
        TRY(s = quote("\t\n\f\r"s));              TEST_EQUAL(s, "\"\\t\\n\\f\\r\""s);
        TRY(s = quote(u8"åß∂ƒ"s));                TEST_EQUAL(s, u8"\"åß∂ƒ\""s);
        TRY(s = quote("\x00\x01\x7f\x80\xff"s));  TEST_EQUAL(s, "\"\\0\\x01\\x7f\\x80\\xff\""s);

        TRY(s = bquote(""s));                      TEST_EQUAL(s, "\"\""s);
        TRY(s = bquote("\"\""s));                  TEST_EQUAL(s, "\"\\\"\\\"\""s);
        TRY(s = bquote("Hello world"s));           TEST_EQUAL(s, "\"Hello world\""s);
        TRY(s = bquote("\\Hello\\world\\"s));      TEST_EQUAL(s, "\"\\\\Hello\\\\world\\\\\""s);
        TRY(s = bquote("\"Hello\" \"world\""s));   TEST_EQUAL(s, "\"\\\"Hello\\\" \\\"world\\\"\""s);
        TRY(s = bquote("\t\n\f\r"s));              TEST_EQUAL(s, "\"\\t\\n\\f\\r\""s);
        TRY(s = bquote(u8"åß∂ƒ"s));                TEST_EQUAL(s, "\"\\xc3\\xa5\\xc3\\x9f\\xe2\\x88\\x82\\xc6\\x92\""s);
        TRY(s = bquote("\x00\x01\x7f\x80\xff"s));  TEST_EQUAL(s, "\"\\0\\x01\\x7f\\x80\\xff\""s);

        TRY(split("", overwrite(sv)));                         TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");
        TRY(split("Hello", overwrite(sv)));                    TEST_EQUAL(sv.size(), 1);  TEST_EQUAL(join(sv, "/"), "Hello");
        TRY(split("Hello world", overwrite(sv)));              TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("\t Hello \t world \t", overwrite(sv)));     TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("**Hello**world**"s, overwrite(sv), "*"));   TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("**Hello**world**"s, overwrite(sv), "*"s));  TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("*****"s, overwrite(sv), "@*"));             TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");
        TRY(split("*****"s, overwrite(sv), "@*"s));            TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");

        TRY(sv = splitv(""));                         TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");
        TRY(sv = splitv("Hello"));                    TEST_EQUAL(sv.size(), 1);  TEST_EQUAL(join(sv, "/"), "Hello");
        TRY(sv = splitv("Hello world"));              TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(sv = splitv("\t Hello \t world \t"));     TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(sv = splitv("**Hello**world**"s, "*"));   TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(sv = splitv("**Hello**world**"s, "*"s));  TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(sv = splitv("*****"s, "@*"));             TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");
        TRY(sv = splitv("*****"s, "@*"s));            TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");

        TEST(starts_with("", ""));
        TEST(starts_with("Hello world", ""));
        TEST(starts_with("Hello world", "Hello"));
        TEST(! starts_with("Hello world", "hello"));
        TEST(! starts_with("Hello world", "world"));
        TEST(! starts_with("Hello", "Hello world"));

        TEST(ends_with("", ""));
        TEST(ends_with("Hello world", ""));
        TEST(ends_with("Hello world", "world"));
        TEST(! ends_with("Hello world", "World"));
        TEST(! ends_with("Hello world", "Hello"));
        TEST(! ends_with("world", "Hello world"));

        TEST(string_is_ascii(""));
        TEST(string_is_ascii("Hello world"));
        TEST(! string_is_ascii("Hello world\xff"));
        TEST(! string_is_ascii(u8"åß∂"));

        TEST_EQUAL(trim(""), "");
        TEST_EQUAL(trim("Hello"), "Hello");
        TEST_EQUAL(trim("Hello world"), "Hello world");
        TEST_EQUAL(trim("", ""), "");
        TEST_EQUAL(trim("Hello", ""), "Hello");
        TEST_EQUAL(trim("<<<>>>", "<>"), "");
        TEST_EQUAL(trim("<<<Hello>>>", "<>"), "Hello");
        TEST_EQUAL(trim("<<<Hello>>> <<<world>>>", "<>"), "Hello>>> <<<world");

        TEST_EQUAL(trim_left(""), "");
        TEST_EQUAL(trim_left("Hello"), "Hello");
        TEST_EQUAL(trim_left("Hello world"), "Hello world");
        TEST_EQUAL(trim_left("", ""), "");
        TEST_EQUAL(trim_left("Hello", ""), "Hello");
        TEST_EQUAL(trim_left("<<<>>>", "<>"), "");
        TEST_EQUAL(trim_left("<<<Hello>>>", "<>"), "Hello>>>");
        TEST_EQUAL(trim_left("<<<Hello>>> <<<world>>>", "<>"), "Hello>>> <<<world>>>");

        TEST_EQUAL(trim_right(""), "");
        TEST_EQUAL(trim_right("Hello"), "Hello");
        TEST_EQUAL(trim_right("Hello world"), "Hello world");
        TEST_EQUAL(trim_right("", ""), "");
        TEST_EQUAL(trim_right("Hello", ""), "Hello");
        TEST_EQUAL(trim_right("<<<>>>", "<>"), "");
        TEST_EQUAL(trim_right("<<<Hello>>>", "<>"), "<<<Hello");
        TEST_EQUAL(trim_right("<<<Hello>>> <<<world>>>", "<>"), "<<<Hello>>> <<<world");

        TEST_EQUAL(unqualify(""), "");
        TEST_EQUAL(unqualify("alpha"), "alpha");
        TEST_EQUAL(unqualify("alpha.bravo"), "bravo");
        TEST_EQUAL(unqualify("alpha.bravo.charlie"), "charlie");
        TEST_EQUAL(unqualify("alpha::bravo"), "bravo");
        TEST_EQUAL(unqualify("alpha::bravo::charlie"), "charlie");
        TEST_EQUAL(unqualify("alpha-bravo"), "alpha-bravo");
        TEST_EQUAL(unqualify("alpha-bravo-charlie"), "alpha-bravo-charlie");

    }

    void check_string_formatting_and_parsing_functions() {

        std::string s;
        std::u16string s16;
        std::u32string s32;
        std::wstring ws;
        std::vector<bool> bv;
        std::vector<int> iv;
        std::vector<std::string> sv;
        std::map<int, std::string> ism;
        std::atomic<int> ai;

        TEST_EQUAL(bin(0, 1), "0");
        TEST_EQUAL(bin(0, 10), "0000000000");
        TEST_EQUAL(bin(42, 1), "101010");
        TEST_EQUAL(bin(42, 5), "101010");
        TEST_EQUAL(bin(42, 10), "0000101010");
        TEST_EQUAL(bin(0xabcdef, 20), "101010111100110111101111");
        TEST_EQUAL(bin(0xabcdef, 24), "101010111100110111101111");
        TEST_EQUAL(bin(0xabcdef, 28), "0000101010111100110111101111");
        TEST_EQUAL(bin(int8_t(0)), "00000000");
        TEST_EQUAL(bin(int16_t(0)), "0000000000000000");
        TEST_EQUAL(bin(int32_t(0)), "00000000000000000000000000000000");
        TEST_EQUAL(bin(int8_t(42)), "00101010");
        TEST_EQUAL(bin(int16_t(42)), "0000000000101010");
        TEST_EQUAL(bin(int32_t(42)), "00000000000000000000000000101010");
        TEST_EQUAL(bin(int8_t(-42)), "-00101010");
        TEST_EQUAL(bin(int16_t(-42)), "-0000000000101010");
        TEST_EQUAL(bin(int32_t(-42)), "-00000000000000000000000000101010");
        TEST_EQUAL(bin(uint32_t(0xabcdef)), "00000000101010111100110111101111");
        TEST_EQUAL(bin(uint64_t(0x123456789abcdefull)), "0000000100100011010001010110011110001001101010111100110111101111");

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

        TEST_EQUAL(binnum(""), 0);
        TEST_EQUAL(binnum("0"), 0);
        TEST_EQUAL(binnum("101010"), 42);
        TEST_EQUAL(binnum("000000101010"), 42);
        TEST_EQUAL(binnum("1111111111111111111111111111111111111111111111111111111111111110"), 0xfffffffffffffffeull);
        TEST_EQUAL(binnum("1111111111111111111111111111111111111111111111111111111111111111"), 0xffffffffffffffffull);
        TEST_EQUAL(binnum("10000000000000000000000000000000000000000000000000000000000000000"), 0xffffffffffffffffull);

        TEST_EQUAL(decnum(""), 0);
        TEST_EQUAL(decnum("0"), 0);
        TEST_EQUAL(decnum("42"), 42);
        TEST_EQUAL(decnum("0042"), 42);
        TEST_EQUAL(decnum("+42"), 42);
        TEST_EQUAL(decnum("-42"), -42);
        TEST_EQUAL(decnum("9223372036854775806"), 9'223'372'036'854'775'806ll); // 2^63-2
        TEST_EQUAL(decnum("9223372036854775807"), 9'223'372'036'854'775'807ll); // 2^63-1
        TEST_EQUAL(decnum("9223372036854775808"), 9'223'372'036'854'775'807ll); // 2^63
        TEST_EQUAL(decnum("9223372036854775809"), 9'223'372'036'854'775'807ll); // 2^63+1
        TEST_EQUAL(decnum("-9223372036854775806"), -9'223'372'036'854'775'806ll); // -(2^63-2)
        TEST_EQUAL(decnum("-9223372036854775807"), -9'223'372'036'854'775'807ll); // -(2^63-1)
        TEST_EQUAL(decnum("-9223372036854775808"), -9'223'372'036'854'775'807ll-1); // -2^63
        TEST_EQUAL(decnum("-9223372036854775809"), -9'223'372'036'854'775'807ll-1); // -(2^63+1)

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

        TEST_EQUAL(fp_format(0), "0");
        TEST_EQUAL(fp_format(0, 'e', 3), "0.000e0");
        TEST_EQUAL(fp_format(42, 'e', 3), "4.200e1");
        TEST_EQUAL(fp_format(-42, 'e', 3), "-4.200e1");
        TEST_EQUAL(fp_format(1.23456e8, 'e', 3), "1.235e8");
        TEST_EQUAL(fp_format(1.23456e-6, 'e', 3), "1.235e-6");
        TEST_EQUAL(fp_format(0, 'f', 3), "0.000");
        TEST_EQUAL(fp_format(42, 'f', 3), "42.000");
        TEST_EQUAL(fp_format(-42, 'f', 3), "-42.000");
        TEST_EQUAL(fp_format(1.23456e8, 'f', 3), "123456000.000");
        TEST_EQUAL(fp_format(1.23456e-6, 'f', 3), "0.000");
        TEST_EQUAL(fp_format(0, 'g', 3), "0");
        TEST_EQUAL(fp_format(42, 'g', 3), "42");
        TEST_EQUAL(fp_format(-42, 'g', 3), "-42");
        TEST_EQUAL(fp_format(1.23456e8, 'g', 3), "1.23e8");
        TEST_EQUAL(fp_format(1.23456e-6, 'g', 3), "1.23e-6");
        TEST_EQUAL(fp_format(0, 'z', 3), "0.00");
        TEST_EQUAL(fp_format(42, 'z', 3), "42.0");
        TEST_EQUAL(fp_format(-42, 'z', 3), "-42.0");
        TEST_EQUAL(fp_format(1.23456e8, 'z', 3), "1.23e8");
        TEST_EQUAL(fp_format(1.23456e-6, 'z', 3), "1.23e-6");
        TEST_EQUAL(fp_format(0.123, 'z', 3), "0.123");
        TEST_EQUAL(fp_format(1.23, 'z', 3), "1.23");
        TEST_EQUAL(fp_format(12.3, 'z', 3), "12.3");
        TEST_EQUAL(fp_format(123, 'z', 3), "123");

        TEST_EQUAL(si_to_int("0"), 0);
        TEST_EQUAL(si_to_int("42"), 42);
        TEST_EQUAL(si_to_int("-42"), -42);
        TEST_EQUAL(si_to_int("0k"), 0);
        TEST_EQUAL(si_to_int("123k"), 123'000);
        TEST_EQUAL(si_to_int("123K"), 123'000);
        TEST_EQUAL(si_to_int("123 k"), 123'000);
        TEST_EQUAL(si_to_int("123 K"), 123'000);
        TEST_EQUAL(si_to_int("123 M"), 123'000'000ll);
        TEST_EQUAL(si_to_int("123 G"), 123'000'000'000ll);
        TEST_EQUAL(si_to_int("123 T"), 123'000'000'000'000ll);
        TEST_EQUAL(si_to_int("123 P"), 123'000'000'000'000'000ll);
        TEST_THROW(si_to_int("123 E"), std::range_error);
        TEST_THROW(si_to_int("123 Z"), std::range_error);
        TEST_THROW(si_to_int("123 Y"), std::range_error);
        TEST_EQUAL(si_to_int("-123k"), -123'000);
        TEST_EQUAL(si_to_int("-123K"), -123'000);
        TEST_EQUAL(si_to_int("-123 k"), -123'000);
        TEST_EQUAL(si_to_int("-123 K"), -123'000);
        TEST_THROW(si_to_int(""), std::invalid_argument);
        TEST_THROW(si_to_int("k9"), std::invalid_argument);

        TEST_EQUAL(si_to_float("0"), 0);
        TEST_EQUAL(si_to_float("42"), 42);
        TEST_EQUAL(si_to_float("-42"), -42);
        TEST_EQUAL(si_to_float("1234.5"), 1234.5);
        TEST_EQUAL(si_to_float("-1234.5"), -1234.5);
        TEST_EQUAL(si_to_float("0k"), 0);
        TEST_EQUAL(si_to_float("12.34k"), 12.34e3);
        TEST_EQUAL(si_to_float("12.34K"), 12.34e3);
        TEST_EQUAL(si_to_float("12.34 k"), 12.34e3);
        TEST_EQUAL(si_to_float("12.34 K"), 12.34e3);
        TEST_EQUAL(si_to_float("12.34 M"), 12.34e6);
        TEST_EQUAL(si_to_float("12.34 G"), 12.34e9);
        TEST_EQUAL(si_to_float("12.34 T"), 12.34e12);
        TEST_EQUAL(si_to_float("12.34 P"), 12.34e15);
        TEST_EQUAL(si_to_float("12.34 E"), 12.34e18);
        TEST_EQUAL(si_to_float("12.34 Z"), 12.34e21);
        TEST_EQUAL(si_to_float("12.34 Y"), 12.34e24);
        TEST_NEAR_EPSILON(si_to_float("12.34 m"), 12.34e-3, 1e-9);
        TEST_NEAR_EPSILON(si_to_float("12.34 u"), 12.34e-6, 1e-12);
        TEST_NEAR_EPSILON(si_to_float("12.34 n"), 12.34e-9, 1e-15);
        TEST_NEAR_EPSILON(si_to_float("12.34 p"), 12.34e-12, 1e-18);
        TEST_NEAR_EPSILON(si_to_float("12.34 f"), 12.34e-15, 1e-21);
        TEST_NEAR_EPSILON(si_to_float("12.34 a"), 12.34e-18, 1e-24);
        TEST_NEAR_EPSILON(si_to_float("12.34 z"), 12.34e-21, 1e-27);
        TEST_NEAR_EPSILON(si_to_float("12.34 y"), 12.34e-24, 1e-30);
        TEST_EQUAL(si_to_float("-12.34k"), -12.34e3);
        TEST_EQUAL(si_to_float("-12.34K"), -12.34e3);
        TEST_EQUAL(si_to_float("-12.34 k"), -12.34e3);
        TEST_EQUAL(si_to_float("-12.34 K"), -12.34e3);
        TEST_THROW(si_to_float("1e999999"), std::range_error);
        TEST_THROW(si_to_float(""), std::invalid_argument);
        TEST_THROW(si_to_float("k9"), std::invalid_argument);

        TEST_EQUAL(hexdump(""s), "");
        TEST_EQUAL(hexdump(""s, 5), "");
        TEST_EQUAL(hexdump("Hello world!"s), "48 65 6c 6c 6f 20 77 6f 72 6c 64 21");
        TEST_EQUAL(hexdump("Hello world!"s, 5), "48 65 6c 6c 6f\n20 77 6f 72 6c\n64 21\n");
        TEST_EQUAL(hexdump("Hello world!"s, 6), "48 65 6c 6c 6f 20\n77 6f 72 6c 64 21\n");

        TEST_EQUAL(tf(true), "true");
        TEST_EQUAL(tf(false), "false");
        TEST_EQUAL(yn(true), "yes");
        TEST_EQUAL(yn(false), "no");

        s = "Hello";
        s16 = u"Hello";
        s32 = U"Hello";
        ws = L"Hello";
        ai = 42;

        TEST_EQUAL(to_str(0), "0");
        TEST_EQUAL(to_str(42), "42");
        TEST_EQUAL(to_str(-42), "-42");
        TEST_EQUAL(to_str(123.456), "123.456");
        TEST_EQUAL(to_str(s), "Hello");
        TEST_EQUAL(to_str(s.data()), "Hello");
        TEST_EQUAL(to_str(""s), "");
        TEST_EQUAL(to_str("Hello"s), "Hello");
        TEST_EQUAL(to_str('X'), "X");
        TEST_EQUAL(to_str(true), "true");
        TEST_EQUAL(to_str(false), "false");
        TEST_EQUAL(to_str(std::make_pair(10,20)), "{10,20}");
        TEST_EQUAL(to_str(std::make_pair("hello"s,"world"s)), "{hello,world}");
        TEST_EQUAL(to_str(ai), "42");

        bv.clear();
        iv.clear();
        sv.clear();
        ism.clear();

        TEST_EQUAL(to_str(bv), "[]");
        TEST_EQUAL(to_str(iv), "[]");
        TEST_EQUAL(to_str(sv), "[]");
        TEST_EQUAL(to_str(ism), "{}");

        bv = {true,false};
        iv = {1,2,3};
        sv = {"hello","world","goodbye"};
        ism = {{1,"hello"},{2,"world"},{3,"goodbye"}};

        TEST_EQUAL(to_str(bv), "[true,false]");
        TEST_EQUAL(to_str(iv), "[1,2,3]");
        TEST_EQUAL(to_str(sv), "[hello,world,goodbye]");
        TEST_EQUAL(to_str(ism), "{1:hello,2:world,3:goodbye}");

        TRY(s = fmt(""));                               TEST_EQUAL(s, "");
        TRY(s = fmt("Hello world"));                    TEST_EQUAL(s, "Hello world");
        TRY(s = fmt("Hello $1"));                       TEST_EQUAL(s, "Hello ");
        TRY(s = fmt("Hello $1", "world"s));             TEST_EQUAL(s, "Hello world");
        TRY(s = fmt("Hello $1", 42));                   TEST_EQUAL(s, "Hello 42");
        TRY(s = fmt("($1) ($2) ($3)", 10, 20, 30));     TEST_EQUAL(s, "(10) (20) (30)");
        TRY(s = fmt("${1} ${2} ${3}", 10, 20, 30));     TEST_EQUAL(s, "10 20 30");
        TRY(s = fmt("$3,$3,$2,$2,$1,$1", 10, 20, 30));  TEST_EQUAL(s, "30,30,20,20,10,10");
        TRY(s = fmt("Hello $1 $$ ${}", 42));            TEST_EQUAL(s, "Hello 42 $ {}");

    }

    void check_html_xml_tags() {

        const std::string expected =
            "<h1>Header</h1>\n"
            "<br/>\n"
            "<ul>\n"
            "<li><code>alpha</code></li>\n"
            "<li><code>bravo</code></li>\n"
            "<li><code>charlie</code></li>\n"
            "</ul>\n";

        {
            std::ostringstream out;
            {
                Tag h1;
                TRY(h1 = Tag(out, "<h1>\n"));
                out << "Header";
            }
            Tag br;
            TRY(br = Tag(out, "<br/>\n"));
            {
                Tag ul;
                TRY(ul = Tag(out, "<ul>\n\n"));
                for (auto item: {"alpha", "bravo", "charlie"}) {
                    Tag li, code;
                    TRY(li = Tag(out, "<li>\n"));
                    TRY(code = Tag(out, "<code>"));
                    out << item;
                }
            }
            std::string s = out.str();
            TEST_EQUAL(s, expected);
        }

        {
            std::ostringstream out;
            {
                Tag h1(out, "h1\n");
                out << "Header";
            }
            Tag br(out, "br/\n");
            {
                Tag ul(out, "ul\n\n");
                for (auto item: {"alpha", "bravo", "charlie"}) {
                    Tag li(out, "li\n");
                    Tag code(out, "code");
                    out << item;
                }
            }
            std::string s = out.str();
            TEST_EQUAL(s, expected);
        }

        {
            std::ostringstream out;
            tagged(out, "h1\n", "Header");
            Tag br(out, "br/\n");
            {
                Tag ul(out, "ul\n\n");
                for (auto item: {"alpha", "bravo", "charlie"})
                    tagged(out, "li\n", "code", item);
            }
            std::string s = out.str();
            TEST_EQUAL(s, expected);
        }

    }

    void check_unicode_functions() {

        // UTF-32    UTF-16     UTF-8
        // 0000004d  004d       4d
        // 00000430  0430       d0 b0
        // 00004e8c  4e8c       e4 ba 8c
        // 00010302  d800 df02  f0 90 8c 82
        // 0010fffd  dbff dffd  f4 8f bf bd

        U8string s8, t8 = "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82\xf4\x8f\xbf\xbd";
        std::u16string s16, t16 = {0x4d,0x430,0x4e8c,0xd800,0xdf02,0xdbff,0xdffd};
        std::u32string s32, t32 = {0x4d,0x430,0x4e8c,0x10302,0x10fffd};
        std::wstring sw, tw =
            #if WCHAR_MAX > 0xffff
                {0x4d,0x430,0x4e8c,0x10302,0x10fffd};
            #else
                {0x4d,0x430,0x4e8c,0xd800,0xdf02,0xdbff,0xdffd};
            #endif
        size_t n = 0;

        TRY(s8 = uconv<U8string>(""s));          TEST_EQUAL(s8, ""s);
        TRY(s8 = uconv<U8string>(u""s));         TEST_EQUAL(s8, ""s);
        TRY(s8 = uconv<U8string>(U""s));         TEST_EQUAL(s8, ""s);
        TRY(s8 = uconv<U8string>(L""s));         TEST_EQUAL(s8, ""s);
        TRY(s16 = uconv<std::u16string>(""s));   TEST_EQUAL(s16, u""s);
        TRY(s16 = uconv<std::u16string>(u""s));  TEST_EQUAL(s16, u""s);
        TRY(s16 = uconv<std::u16string>(U""s));  TEST_EQUAL(s16, u""s);
        TRY(s16 = uconv<std::u16string>(L""s));  TEST_EQUAL(s16, u""s);
        TRY(s32 = uconv<std::u32string>(""s));   TEST_EQUAL(s32, U""s);
        TRY(s32 = uconv<std::u32string>(u""s));  TEST_EQUAL(s32, U""s);
        TRY(s32 = uconv<std::u32string>(U""s));  TEST_EQUAL(s32, U""s);
        TRY(s32 = uconv<std::u32string>(L""s));  TEST_EQUAL(s32, U""s);
        TRY(sw = uconv<std::wstring>(""s));      TEST_EQUAL(sw, L""s);
        TRY(sw = uconv<std::wstring>(u""s));     TEST_EQUAL(sw, L""s);
        TRY(sw = uconv<std::wstring>(U""s));     TEST_EQUAL(sw, L""s);
        TRY(sw = uconv<std::wstring>(L""s));     TEST_EQUAL(sw, L""s);
        TRY(s8 = uconv<U8string>(t8));           TEST_EQUAL(s8, t8);
        TRY(s8 = uconv<U8string>(t16));          TEST_EQUAL(s8, t8);
        TRY(s8 = uconv<U8string>(t32));          TEST_EQUAL(s8, t8);
        TRY(s8 = uconv<U8string>(tw));           TEST_EQUAL(s8, t8);
        TRY(s16 = uconv<std::u16string>(t8));    TEST_EQUAL(s16, t16);
        TRY(s16 = uconv<std::u16string>(t16));   TEST_EQUAL(s16, t16);
        TRY(s16 = uconv<std::u16string>(t32));   TEST_EQUAL(s16, t16);
        TRY(s16 = uconv<std::u16string>(tw));    TEST_EQUAL(s16, t16);
        TRY(s32 = uconv<std::u32string>(t8));    TEST_EQUAL(s32, t32);
        TRY(s32 = uconv<std::u32string>(t16));   TEST_EQUAL(s32, t32);
        TRY(s32 = uconv<std::u32string>(t32));   TEST_EQUAL(s32, t32);
        TRY(s32 = uconv<std::u32string>(tw));    TEST_EQUAL(s32, t32);
        TRY(sw = uconv<std::wstring>(t8));       TEST_EQUAL(sw, tw);
        TRY(sw = uconv<std::wstring>(t16));      TEST_EQUAL(sw, tw);
        TRY(sw = uconv<std::wstring>(t32));      TEST_EQUAL(sw, tw);
        TRY(sw = uconv<std::wstring>(tw));       TEST_EQUAL(sw, tw);

        TEST(uvalid(""s, n));                       TEST_EQUAL(n, 0);
        TEST(uvalid(u""s, n));                      TEST_EQUAL(n, 0);
        TEST(uvalid(U""s, n));                      TEST_EQUAL(n, 0);
        TEST(uvalid(L""s, n));                      TEST_EQUAL(n, 0);
        TEST(uvalid(t8, n));                        TEST_EQUAL(n, t8.size());
        TEST(uvalid(t16, n));                       TEST_EQUAL(n, t16.size());
        TEST(uvalid(t32, n));                       TEST_EQUAL(n, t32.size());
        TEST(uvalid(tw, n));                        TEST_EQUAL(n, tw.size());
        TEST(! uvalid(t8 + '\xff', n));             TEST_EQUAL(n, t8.size());
        TEST(! uvalid(t16 + char16_t(0xd800), n));  TEST_EQUAL(n, t16.size());
        TEST(! uvalid(t32 + char32_t(0xd800), n));  TEST_EQUAL(n, t32.size());
        TEST(! uvalid(tw + wchar_t(0xd800), n));    TEST_EQUAL(n, tw.size());

    }

    void check_thread_class() {

        TEST_COMPARE(Thread::cpu_threads(), >=, 1);
        TRY(Thread::yield());

        {
            Thread t;
            TRY(t.wait());
            TEST(t.poll());
        }

        {
            Thread t(nullptr);
            TRY(t.wait());
            TEST(t.poll());
        }

        {
            std::string s;
            Thread t;
            TRY(t = Thread([&] { s = "Neddie"; }));
            TEST_COMPARE(t.get_id(), !=, Thread::current());
            TRY(t.wait());
            TEST(t.poll());
            TEST_EQUAL(s, "Neddie");
        }

        {
            Thread t;
            TRY(t = Thread([&] { throw std::runtime_error("Hello"); }));
            TEST_THROW_MATCH(t.wait(), std::runtime_error, "Hello");
            TRY(t.wait());
        }

    }

    void check_synchronisation_objects() {

        {
            Mutex m;
            ConditionVariable cv;
            int n = 0;
            MutexLock lock;
            TRY(lock = make_lock(m));
            TEST(! cv.wait_for(lock, milliseconds(50), [&] { return n > 0; }));
            n = 42;
            TEST(cv.wait_for(lock, milliseconds(50), [&] { return n > 0; }));
        }

        {
            Mutex m;
            ConditionVariable cv;
            std::string s;
            auto f = [&] {
                MutexLock lock;
                TRY(lock = make_lock(m));
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
            Mutex m;
            ConditionVariable cv;
            std::string s;
            auto f1 = [&] {
                MutexLock lock;
                TRY(lock = make_lock(m));
                TRY(cv.wait(lock));
                s += "Seagoon";
            };
            auto f2 = [&] {
                TRY(sleep_for(0.05));
                MutexLock lock;
                TRY(lock = make_lock(m));
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
            Mutex m;
            ConditionVariable cv;
            std::string s;
            auto f1 = [&] {
                MutexLock lock;
                TRY(lock = make_lock(m));
                TRY(cv.wait(lock, [&] { return ! s.empty(); }));
                s += "Seagoon";
            };
            auto f2 = [&] {
                TRY(cv.notify_one());
                TRY(sleep_for(0.05));
                TRY(cv.notify_one());
                {
                    MutexLock lock;
                    TRY(lock = make_lock(m));
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

    void check_time_and_date_types() {

        auto ds = Dseconds(1.25);
        auto ms = duration_cast<milliseconds>(ds);
        TEST_EQUAL(ms.count(), 1250);

        auto dd = Ddays(7);
        auto s = duration_cast<seconds>(dd);
        TEST_EQUAL(s.count(), 604800);

        auto dy = Dyears(100);
        s = duration_cast<seconds>(dy);
        TEST_EQUAL(s.count(), 3155760000);

        TEST(ReliableClock::is_steady);
        TEST_COMPARE(ReliableClock::now().time_since_epoch().count(), >, 0);

    }

    void check_general_time_and_date_operations() {

        hours h;
        minutes m;
        seconds s;
        milliseconds ms;
        system_clock::time_point tp;
        int64_t t1, t2;

        TRY(from_seconds(7200.0, h));   TEST_EQUAL(h.count(), 2);
        TRY(from_seconds(7200.0, m));   TEST_EQUAL(m.count(), 120);
        TRY(from_seconds(7200.0, s));   TEST_EQUAL(s.count(), 7200);
        TRY(from_seconds(7200.0, ms));  TEST_EQUAL(ms.count(), 7'200'000);

        TEST_EQUAL(to_seconds(hours(10)), 36000);
        TEST_EQUAL(to_seconds(minutes(10)), 600);
        TEST_EQUAL(to_seconds(seconds(10)), 10);
        TEST_NEAR(to_seconds(milliseconds(10)), 0.01);

        TRY(tp = make_date(1970, 1, 1, 0, 0, 0));
        TRY(t1 = int64_t(system_clock::to_time_t(tp)));
        TEST_EQUAL(t1, 0);
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5));
        TRY(t1 = int64_t(system_clock::to_time_t(tp)));
        TEST_EQUAL(t1, 946'782'245);
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5, Zone::local));
        TRY(t2 = int64_t(system_clock::to_time_t(tp)));
        TEST_COMPARE(std::abs(t2 - t1), <=, 86400);

    }

    void check_time_and_date_formatting() {

        system_clock::time_point tp;
        system_clock::duration d;
        U8string str;

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
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5, Zone::local));
        TRY(str = format_date(tp, 0, Zone::local));
        TEST_EQUAL(str, "2000-01-02 03:04:05");

        TEST_EQUAL(format_time(Dseconds(0)), "0s");
        TEST_EQUAL(format_time(Dseconds(0), 3), "0.000s");
        TEST_EQUAL(format_time(Dseconds(0.25), 3), "0.250s");
        TEST_EQUAL(format_time(Dseconds(0.5), 3), "0.500s");
        TEST_EQUAL(format_time(Dseconds(0.75), 3), "0.750s");
        TEST_EQUAL(format_time(Dseconds(1), 3), "1.000s");
        TEST_EQUAL(format_time(Dseconds(1.25), 3), "1.250s");
        TEST_EQUAL(format_time(Dseconds(59.999), 3), "59.999s");
        TEST_EQUAL(format_time(Dseconds(60), 3), "1m00.000s");
        TEST_EQUAL(format_time(Dseconds(1234), 3), "20m34.000s");
        TEST_EQUAL(format_time(Dseconds(12345), 3), "3h25m45.000s");
        TEST_EQUAL(format_time(Dseconds(123456), 3), "1d10h17m36.000s");
        TEST_EQUAL(format_time(Dseconds(1234567), 3), "14d06h56m07.000s");
        TEST_EQUAL(format_time(Dseconds(12345678), 3), "142d21h21m18.000s");
        TEST_EQUAL(format_time(Dseconds(123456789), 3), "1428d21h33m09.000s");
        TEST_EQUAL(format_time(Dseconds(-0.25), 3), "-0.250s");
        TEST_EQUAL(format_time(Dseconds(-0.5), 3), "-0.500s");
        TEST_EQUAL(format_time(Dseconds(-0.75), 3), "-0.750s");
        TEST_EQUAL(format_time(Dseconds(-1), 3), "-1.000s");
        TEST_EQUAL(format_time(Dseconds(-1.25), 3), "-1.250s");
        TEST_EQUAL(format_time(Dseconds(-59.999), 3), "-59.999s");
        TEST_EQUAL(format_time(Dseconds(-60), 3), "-1m00.000s");
        TEST_EQUAL(format_time(Dseconds(-1234), 3), "-20m34.000s");
        TEST_EQUAL(format_time(Dseconds(-12345), 3), "-3h25m45.000s");
        TEST_EQUAL(format_time(Dseconds(-123456), 3), "-1d10h17m36.000s");
        TEST_EQUAL(format_time(Dseconds(-1234567), 3), "-14d06h56m07.000s");
        TEST_EQUAL(format_time(Dseconds(-12345678), 3), "-142d21h21m18.000s");
        TEST_EQUAL(format_time(Dseconds(-123456789), 3), "-1428d21h33m09.000s");
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

    }

    void check_system_specific_time_and_date_conversions() {

        using IntMsec = duration<int64_t, std::ratio<1, 1000>>;
        using IntSec = duration<int64_t>;
        using IntDays = duration<int64_t, std::ratio<86400>>;

        IntMsec ims;
        IntSec is;
        IntDays id;
        Dseconds fs;
        timespec ts;
        timeval tv;

        ts = {0, 0};                 TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 0);
        ts = {0, 0};                 TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 0);
        ts = {0, 0};                 TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 0);
        ts = {0, 0};                 TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 0);
        ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 0.125);
        ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 0);
        ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 125);
        ts = {0, 125'000'000};       TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 0);
        ts = {86'400, 0};            TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 86'400);
        ts = {86'400, 0};            TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 1);
        ts = {86'400, 0};            TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 86'400'000);
        ts = {86'400, 0};            TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 86'400);
        ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, fs));   TEST_EQUAL(fs.count(), 86'400.125);
        ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, id));   TEST_EQUAL(id.count(), 1);
        ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, ims));  TEST_EQUAL(ims.count(), 86'400'125);
        ts = {86'400, 125'000'000};  TRY(timespec_to_duration(ts, is));   TEST_EQUAL(is.count(), 86'400);
        tv = {0, 0};                 TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 0);
        tv = {0, 0};                 TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 0);
        tv = {0, 0};                 TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 0);
        tv = {0, 0};                 TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 0);
        tv = {0, 125'000};           TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 0.125);
        tv = {0, 125'000};           TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 0);
        tv = {0, 125'000};           TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 125);
        tv = {0, 125'000};           TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 0);
        tv = {86'400, 0};            TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 86'400);
        tv = {86'400, 0};            TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 1);
        tv = {86'400, 0};            TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 86'400'000);
        tv = {86'400, 0};            TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 86'400);
        tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, fs));    TEST_EQUAL(fs.count(), 86'400.125);
        tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, id));    TEST_EQUAL(id.count(), 1);
        tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, ims));   TEST_EQUAL(ims.count(), 86'400'125);
        tv = {86'400, 125'000};      TRY(timeval_to_duration(tv, is));    TEST_EQUAL(is.count(), 86'400);

        fs = Dseconds(0);           TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
        id = IntDays(0);            TRY(ts = duration_to_timespec(id));   TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
        ims = IntMsec(0);           TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
        is = IntSec(0);             TRY(ts = duration_to_timespec(is));   TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 0);
        fs = Dseconds(0.125);       TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 125'000'000);
        ims = IntMsec(125);         TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 0);       TEST_EQUAL(ts.tv_nsec, 125'000'000);
        fs = Dseconds(86'400);      TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
        id = IntDays(1);            TRY(ts = duration_to_timespec(id));   TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
        ims = IntMsec(86'400'000);  TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
        is = IntSec(86'400);        TRY(ts = duration_to_timespec(is));   TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 0);
        fs = Dseconds(86'400.125);  TRY(ts = duration_to_timespec(fs));   TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 125'000'000);
        ims = IntMsec(86'400'125);  TRY(ts = duration_to_timespec(ims));  TEST_EQUAL(ts.tv_sec, 86'400);  TEST_EQUAL(ts.tv_nsec, 125'000'000);
        fs = Dseconds(0);           TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
        id = IntDays(0);            TRY(tv = duration_to_timeval(id));    TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
        ims = IntMsec(0);           TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
        is = IntSec(0);             TRY(tv = duration_to_timeval(is));    TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 0);
        fs = Dseconds(0.125);       TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 125'000);
        ims = IntMsec(125);         TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 0);       TEST_EQUAL(tv.tv_usec, 125'000);
        fs = Dseconds(86'400);      TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
        id = IntDays(1);            TRY(tv = duration_to_timeval(id));    TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
        ims = IntMsec(86'400'000);  TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
        is = IntSec(86'400);        TRY(tv = duration_to_timeval(is));    TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 0);
        fs = Dseconds(86'400.125);  TRY(tv = duration_to_timeval(fs));    TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 125'000);
        ims = IntMsec(86'400'125);  TRY(tv = duration_to_timeval(ims));   TEST_EQUAL(tv.tv_sec, 86'400);  TEST_EQUAL(tv.tv_usec, 125'000);

        #ifdef _WIN32

            static constexpr int64_t epoch = 11'644'473'600ll;
            static constexpr int64_t freq = 10'000'000ll;

            int64_t n;
            FILETIME ft;
            system_clock::time_point tp;
            system_clock::duration d;

            n = epoch * freq;
            ft = {uint32_t(n), uint32_t(n >> 32)};
            TRY(tp = filetime_to_timepoint(ft));
            d = tp - system_clock::from_time_t(0);
            TEST_EQUAL(d.count(), 0);

            n += 86'400 * freq;
            ft = {uint32_t(n), uint32_t(n >> 32)};
            TRY(tp = filetime_to_timepoint(ft));
            d = tp - system_clock::from_time_t(0);
            TEST_EQUAL(duration_cast<IntMsec>(d).count(), 86'400'000);

            tp = system_clock::from_time_t(0);
            TRY(ft = timepoint_to_filetime(tp));
            TEST_EQUAL(ft.dwHighDateTime, 0);
            TEST_EQUAL(ft.dwLowDateTime, 0);

            tp = system_clock::from_time_t(86'400);
            TRY(ft = timepoint_to_filetime(tp));
            TEST_EQUAL(ft.dwHighDateTime, 201);
            TEST_EQUAL(ft.dwLowDateTime, 711'573'504);

        #endif

    }

}

TEST_MODULE(core, core) {

    check_preprocessor_macros();
    check_endian_integers();
    check_containers();
    check_exceptions();
    check_metaprogramming_and_type_traits();
    check_smart_pointers();
    check_type_adapters();
    check_type_related_functions();
    check_uuid();
    check_version_number();
    check_arithmetic_constants();
    check_arithmetic_literals();
    check_generic_algorithms();
    check_integer_sequences();
    check_memory_algorithms();
    check_range_traits();
    check_range_types();
    check_generic_arithmetic_functions();
    check_integer_arithmetic_functions();
    check_bitwise_operations();
    check_floating_point_arithmetic_functions();
    check_function_traits();
    check_function_operations();
    check_generic_function_objects();
    check_hash_functions();
    check_keyword_arguments();
    check_scope_guards();
    check_file_io_operations();
    check_process_io_operations();
    check_terminal_io_operations();
    check_simple_random_generators();
    check_character_functions();
    check_general_string_functions();
    check_string_formatting_and_parsing_functions();
    check_html_xml_tags();
    check_unicode_functions();
    check_thread_class();
    check_synchronisation_objects();
    check_time_and_date_types();
    check_general_time_and_date_operations();
    check_time_and_date_formatting();
    check_system_specific_time_and_date_conversions();

}
