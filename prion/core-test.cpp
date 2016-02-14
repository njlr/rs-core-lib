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
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(PRI_TARGET_UNIX)
    #include <sys/time.h>
#endif

#if defined(PRI_TARGET_WIN32)
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

    void check_preprocessor_macros() {

        int n = 42;
        auto assert1 = [n] { PRI_ASSERT(n == 42); };
        auto assert2 = [n] { PRI_ASSERT(n == 6 * 9); };

        TRY(assert1());
        TEST_THROW_MATCH(assert2(), std::logic_error, "\\bcore-test\\.cpp:\\d+\\b.+\\bn == 6 \\* 9\\b");

        vector<int> iv = {1, 2, 3, 4, 5}, iv2 = {2, 3, 5, 7, 11};
        vector<u8string> result, sv = {"Neddie", "Eccles", "Bluebottle"};

        TRY(std::transform(iv.begin(), iv.end(), overwrite(result), PRI_OVERLOAD(f1)));
        TEST_EQUAL_RANGE(result, (vector<u8string>{"*", "**", "***", "****", "*****"}));
        TRY(std::transform(sv.begin(), sv.end(), overwrite(result), PRI_OVERLOAD(f1)));
        TEST_EQUAL_RANGE(result, (vector<u8string>{"[Neddie]", "[Eccles]", "[Bluebottle]"}));

        TRY(std::generate_n(overwrite(result), 3, PRI_OVERLOAD(f2)));
        TEST_EQUAL_RANGE(result, (vector<u8string>{"Hello", "Hello", "Hello"}));
        TRY(std::transform(sv.begin(), sv.end(), overwrite(result), PRI_OVERLOAD(f2)));
        TEST_EQUAL_RANGE(result, (vector<u8string>{"[Neddie]", "[Eccles]", "[Bluebottle]"}));
        auto of2 = PRI_OVERLOAD(f2);
        auto out = overwrite(result);
        for (size_t i = 0; i < iv.size() && i < iv2.size(); ++i)
            TRY(*out++ = of2(iv[i], iv2[i]));
        TEST_EQUAL_RANGE(result, (vector<u8string>{"2", "6", "15", "28", "55"}));

        auto c1 = PRI_CHAR('A', char);      TEST_TYPE_OF(c1, char);      TEST_EQUAL(c1, 'A');
        auto c2 = PRI_CHAR('A', char16_t);  TEST_TYPE_OF(c2, char16_t);  TEST_EQUAL(c2, u'A');
        auto c3 = PRI_CHAR('A', char32_t);  TEST_TYPE_OF(c3, char32_t);  TEST_EQUAL(c3, U'A');
        auto c4 = PRI_CHAR('A', wchar_t);   TEST_TYPE_OF(c4, wchar_t);   TEST_EQUAL(c4, L'A');

        auto cp1 = PRI_CSTR("Hello", char);      TEST_TYPE_OF(cp1, const char*);      TEST_EQUAL(cp1, "Hello"s);
        auto cp2 = PRI_CSTR("Hello", char16_t);  TEST_TYPE_OF(cp2, const char16_t*);  TEST_EQUAL(cp2, u"Hello"s);
        auto cp3 = PRI_CSTR("Hello", char32_t);  TEST_TYPE_OF(cp3, const char32_t*);  TEST_EQUAL(cp3, U"Hello"s);
        auto cp4 = PRI_CSTR("Hello", wchar_t);   TEST_TYPE_OF(cp4, const wchar_t*);   TEST_EQUAL(cp4, L"Hello"s);

    }

    class TopTail {
    public:
        TopTail(): sp(nullptr), ch() {}
        TopTail(u8string& s, char c): sp(&s), ch(c) { s += '+'; s += c; }
        ~TopTail() { term(); }
        TopTail(TopTail&& t): sp(t.sp), ch(t.ch) { t.sp = nullptr; }
        TopTail& operator=(TopTail&& t) { if (&t != this) { term(); sp = t.sp; ch = t.ch; t.sp = nullptr; } return *this; }
    private:
        u8string* sp;
        char ch;
        TopTail(const TopTail&) = delete;
        TopTail& operator=(const TopTail&) = delete;
        void term() { if (sp) { *sp += '-'; *sp += ch; sp = nullptr; } }
    };

    void check_containers() {

        using buf = SimpleBuffer<int32_t>;

        buf b1, b2;
        shared_ptr<buf> bp;
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

        Stacklike<TopTail> st;
        u8string s;

        TEST(st.empty());
        TRY(st.push(TopTail(s, 'a')));
        TEST_EQUAL(st.size(), 1);
        TEST_EQUAL(s, "+a");
        TRY(st.push(TopTail(s, 'b')));
        TEST_EQUAL(st.size(), 2);
        TEST_EQUAL(s, "+a+b");
        TRY(st.push(TopTail(s, 'c')));
        TEST_EQUAL(st.size(), 3);
        TEST_EQUAL(s, "+a+b+c");
        TRY(st.clear());
        TEST(st.empty());
        TEST_EQUAL(s, "+a+b+c-c-b-a");

    }

    void check_exceptions() {

        #if defined(PRI_TARGET_WIN32)

            u8string s;

            TRY(s = windows_category().message(ERROR_INVALID_FUNCTION));
            TEST_EQUAL(s, "Incorrect function.");
            try {
                throw std::system_error(ERROR_INVALID_FUNCTION, windows_category(), "SomeFunction()");
            }
            catch (const std::exception& ex) {
                TEST_MATCH(string(ex.what()), "^SomeFunction\\(\\).+Incorrect function\\.$");
            }

        #endif

    }

    void check_metaprogramming_and_type_traits() {

        TEST_TYPE(BinaryType<char>, uint8_t);
        TEST_TYPE(BinaryType<float>, uint32_t);
        TEST_TYPE(BinaryType<double>, uint64_t);

        {  using type = CopyConst<int, string>;              TEST_TYPE(type, string);        }
        {  using type = CopyConst<int, const string>;        TEST_TYPE(type, string);        }
        {  using type = CopyConst<const int, string>;        TEST_TYPE(type, const string);  }
        {  using type = CopyConst<const int, const string>;  TEST_TYPE(type, const string);  }

        TEST_TYPE(SignedInteger<8>, int8_t);
        TEST_TYPE(SignedInteger<16>, int16_t);
        TEST_TYPE(SignedInteger<32>, int32_t);
        TEST_TYPE(SignedInteger<64>, int64_t);
        TEST_TYPE(SignedInteger<128>, int128_t);
        TEST_TYPE(UnsignedInteger<8>, uint8_t);
        TEST_TYPE(UnsignedInteger<16>, uint16_t);
        TEST_TYPE(UnsignedInteger<32>, uint32_t);
        TEST_TYPE(UnsignedInteger<64>, uint64_t);
        TEST_TYPE(UnsignedInteger<128>, uint128_t);

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

        shared_ptr<Base> b1 = make_shared<Derived1>();
        shared_ptr<Base> b2 = make_shared<Derived2>();

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

        u8string s;

        TEST_EQUAL(type_name<void>(), "void");
        TEST_MATCH(type_name<int>(), "^(signed )?int$");
        TEST_MATCH(type_name<string>(), "^std::([^:]+::)*(string|basic_string ?<.+>)$");
        TEST_MATCH(type_name(42), "^(signed )?int$");
        TEST_MATCH(type_name(s), "^std::([^:]+::)*(string|basic_string ?<.+>)$");

    }

    void check_uuid() {

        Uuid u, u2;

        TEST_EQUAL(u.str(), "00000000-0000-0000-0000-000000000000");
        TEST_EQUAL(to_str(u), "00000000-0000-0000-0000-000000000000");
        TEST_EQUAL(u.str(), to_str(u));
        TEST_EQUAL(u.as_integer(), 0_u128);

        TRY(u = Uuid(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));
        TEST_EQUAL(u.str(), "01020304-0506-0708-090a-0b0c0d0e0f10");
        TEST_EQUAL(to_str(u), "01020304-0506-0708-090a-0b0c0d0e0f10");
        TEST_EQUAL(u.str(), to_str(u));
        TEST_EQUAL(u.as_integer(), 0x0102030405060708090a0b0c0d0e0f10_u128);
        for (unsigned i = 0; i < 16; ++i)
            TEST_EQUAL(u[i], i + 1);
        TRY(u = Uuid(0x12345678, 0x9abc, 0xdef0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0));
        TEST_EQUAL(u.str(), "12345678-9abc-def0-1234-56789abcdef0");

        TRY(u = Uuid(0_u128));
        TEST_EQUAL(u.str(), "00000000-0000-0000-0000-000000000000");
        TRY(u = Uuid(0x123456789abcdef0123456789abcdef0_u128));
        TEST_EQUAL(u.str(), "12345678-9abc-def0-1234-56789abcdef0");

        const uint8_t* bytes = nullptr;
        TRY(u = Uuid(bytes));
        TEST_EQUAL(u.str(), "00000000-0000-0000-0000-000000000000");
        string chars = "abcdefghijklmnop";
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

        std::unordered_set<Uuid> us;

        for (uint128_t i = 0; i < 1000; ++i)
            TRY(us.insert(Uuid(i)));
        TEST_EQUAL(us.size(), 1000);
        for (uint128_t i = 0; i < 1000; ++i)
            TRY(us.insert(Uuid(i)));
        TEST_EQUAL(us.size(), 1000);

        std::mt19937 rng{uint32_t(time(nullptr))};

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

        TEST_EQUAL(42_k, 42000ull);
        TEST_EQUAL(42_M, 42000000ull);
        TEST_EQUAL(42_G, 42000000000ull);
        TEST_EQUAL(42_T, 42000000000000ull);
        TEST_EQUAL(42_kb, 43008ull);
        TEST_EQUAL(42_MB, 44040192ull);
        TEST_EQUAL(42_GB, 45097156608ull);
        TEST_EQUAL(42_TB, 46179488366592ull);

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

    void check_generic_algorithms() {

        std::string s1, s2;
        int c;

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

        s1 = "";               s2 = "";               TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, 0);
        s1 = "";               s2 = "hello";          TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, -1);
        s1 = "hello";          s2 = "";               TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, 1);
        s1 = "hello";          s2 = "world";          TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, -1);
        s1 = "hello";          s2 = "hello";          TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, 0);
        s1 = "hello";          s2 = "goodbye";        TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, 1);
        s1 = "hello";          s2 = "hello world";    TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, -1);
        s1 = "hello world";    s2 = "hello";          TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, 1);
        s1 = "hello goodbye";  s2 = "hello world";    TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, -1);
        s1 = "hello world";    s2 = "hello goodbye";  TRY(c = compare_3way(s1, s2));                    TEST_EQUAL(c, 1);
        s1 = "";               s2 = "";               TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, 0);
        s1 = "";               s2 = "hello";          TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, -1);
        s1 = "hello";          s2 = "";               TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, 1);
        s1 = "hello";          s2 = "world";          TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, 1);
        s1 = "hello";          s2 = "hello";          TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, 0);
        s1 = "hello";          s2 = "goodbye";        TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, -1);
        s1 = "hello";          s2 = "hello world";    TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, -1);
        s1 = "hello world";    s2 = "hello";          TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, 1);
        s1 = "hello goodbye";  s2 = "hello world";    TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, 1);
        s1 = "hello world";    s2 = "hello goodbye";  TRY(c = compare_3way(s1, s2, std::greater<>()));  TEST_EQUAL(c, -1);

        const auto same_case = [] (char a, char b) { return ascii_isupper(a) == ascii_isupper(b); };

        s1 = "abcabcabc";        TRY(con_remove(s1, 'c'));                        TEST_EQUAL(s1, "ababab");
        s1 = "abc123abc123";     TRY(con_remove_if(s1, ascii_isalpha));           TEST_EQUAL(s1, "123123");
        s1 = "abc123abc123";     TRY(con_remove_if_not(s1, ascii_isalpha));       TEST_EQUAL(s1, "abcabc");
        s1 = "abbcccddddeeeee";  TRY(con_unique(s1));                             TEST_EQUAL(s1, "abcde");
        s1 = "ABCabcABCabc";     TRY(con_unique(s1, same_case));                  TEST_EQUAL(s1, "AaAa");
        s1 = "abcdeabcdabcaba";  TRY(con_sort_unique(s1));                        TEST_EQUAL(s1, "abcde");
        s1 = "abcdeabcdabcaba";  TRY(con_sort_unique(s1, std::greater<char>()));  TEST_EQUAL(s1, "edcba");

        auto f = [&] { s1 += "Hello"; };

        s1.clear();  TRY(do_n(0, f));  TEST_EQUAL(s1, "");
        s1.clear();  TRY(do_n(1, f));  TEST_EQUAL(s1, "Hello");
        s1.clear();  TRY(do_n(2, f));  TEST_EQUAL(s1, "HelloHello");
        s1.clear();  TRY(do_n(3, f));  TEST_EQUAL(s1, "HelloHelloHello");

    }

    void check_integer_sequences() {

        vector<int> v;
        u8string s;

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

        string s1, s2;
        int x = 0;

        s1 = "";             s2 = "";             TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, 0);
        s1 = "hello";        s2 = "";             TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, 1);
        s1 = "";             s2 = "world";        TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, -1);
        s1 = "hello";        s2 = "world";        TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, -1);
        s1 = "hello world";  s2 = "hello";        TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, 1);
        s1 = "hello";        s2 = "hello world";  TRY(x = mem_compare(s1.data(), s1.size(), s2.data(), s2.size()));  TEST_EQUAL(x, -1);

        s1 = "hello";
        s2 = "world";
        TRY(mem_swap(&s1[0], &s2[0], 5));
        TEST_EQUAL(s1, "world");
        TEST_EQUAL(s2, "hello");

    }

    void check_secure_memory_algorithms() {

        // Only testing the memory manipulation aspect of these

        std::mt19937_64 rng(42);
        uint64_t x = 0, y = 0, bx = 0, by = 0;
        int d = 0, e = 0;

        for (int i = 0; i < 10000; ++i) {
            x = rng();
            if (rng() < (1ull << 62))
                y = x;
            else
                y = rng();
            e = x < y ? -1 : x == y ? 0 : 1;
            bx = big_endian(x);
            by = big_endian(y);
            TRY(d = secure_compare(&bx, &by, 8));
            TEST_EQUAL(d, e);
        }

        x = 0x123456789abcdef0ull;
        y = 0;
        TRY(secure_move(&y, &x, 8));
        TEST_EQUAL(x, 0);
        TEST_EQUAL(y, 0x123456789abcdef0ull);

        x = 0x123456789abcdef0ull;
        TRY(secure_zero(&x, 8));
        TEST_EQUAL(x, 0);

    }

    void check_range_traits() {

        char array[] {'H','e','l','l','o'};

        TEST_TYPE(RangeIterator<string>, string::iterator);
        TEST_TYPE(RangeIterator<const string>, string::const_iterator);
        TEST_TYPE(RangeIterator<vector<int>>, vector<int>::iterator);
        TEST_TYPE(RangeIterator<const vector<int>>, vector<int>::const_iterator);
        TEST_TYPE(RangeIterator<Irange<int*>>, int*);
        TEST_TYPE(RangeIterator<Irange<const int*>>, const int*);

        TEST_TYPE(RangeValue<string>, char);
        TEST_TYPE(RangeValue<vector<int>>, int);
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
        string s1, s2;

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
                    TEST_COMPARE(cd, <=, std::min(abs(a), abs(b)));
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
                        TEST_COMPARE(cm, >=, std::max(abs(a), abs(b)));
                        TEST_EQUAL(cm % a, 0);
                        TEST_EQUAL(cm % b, 0);
                    }
                }
                TEST_EQUAL(abs(a * b), cd * cm);
            }
        }

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

    void check_integer_arithmetic_functions() {

        int8_t s8 = -42;
        uint8_t u8 = 42;
        int16_t s16 = -42;
        uint16_t u16 = 42;
        int32_t s32 = -42;
        uint32_t u32 = 42;
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

    }

    void check_bitwise_operations() {

        TEST_EQUAL(binary_size(0), 0);
        TEST_EQUAL(binary_size(1), 1);
        TEST_EQUAL(binary_size(0x8000), 16);
        TEST_EQUAL(binary_size(0x8888), 16);
        TEST_EQUAL(binary_size(0xffff), 16);
        TEST_EQUAL(binary_size(0x8000000000000000ull), 64);
        TEST_EQUAL(binary_size(0xffffffffffffffffull), 64);

        TEST_EQUAL(bits_set(0), 0);
        TEST_EQUAL(bits_set(1), 1);
        TEST_EQUAL(bits_set(0x8000), 1);
        TEST_EQUAL(bits_set(0x8888), 4);
        TEST_EQUAL(bits_set(0xffff), 16);
        TEST_EQUAL(bits_set(0x8000000000000000ull), 1);
        TEST_EQUAL(bits_set(0xffffffffffffffffull), 64);

        TEST_EQUAL(letter_to_mask('A'), 1);
        TEST_EQUAL(letter_to_mask('B'), 2);
        TEST_EQUAL(letter_to_mask('C'), 4);
        TEST_EQUAL(letter_to_mask('Z'), 0x2000000ull);
        TEST_EQUAL(letter_to_mask('a'), 0x4000000ull);
        TEST_EQUAL(letter_to_mask('b'), 0x8000000ull);
        TEST_EQUAL(letter_to_mask('c'), 0x10000000ull);
        TEST_EQUAL(letter_to_mask('z'), 0x8000000000000ull);

        uint16_t u = 0x1234;

        TEST_EQUAL(rotl(u, 0), 0x1234);
        TEST_EQUAL(rotl(u, 4), 0x2341);
        TEST_EQUAL(rotl(u, 8), 0x3412);
        TEST_EQUAL(rotl(u, 12), 0x4123);
        TEST_EQUAL(rotr(u, 0), 0x1234);
        TEST_EQUAL(rotr(u, 4), 0x4123);
        TEST_EQUAL(rotr(u, 8), 0x3412);
        TEST_EQUAL(rotr(u, 12), 0x2341);

        uint32_t v = 0x12345678;

        TEST_EQUAL(rotl(v, 0), 0x12345678);
        TEST_EQUAL(rotl(v, 4), 0x23456781);
        TEST_EQUAL(rotl(v, 8), 0x34567812);
        TEST_EQUAL(rotl(v, 12), 0x45678123);
        TEST_EQUAL(rotl(v, 16), 0x56781234);
        TEST_EQUAL(rotl(v, 20), 0x67812345);
        TEST_EQUAL(rotl(v, 24), 0x78123456);
        TEST_EQUAL(rotl(v, 28), 0x81234567);
        TEST_EQUAL(rotr(v, 0), 0x12345678);
        TEST_EQUAL(rotr(v, 4), 0x81234567);
        TEST_EQUAL(rotr(v, 8), 0x78123456);
        TEST_EQUAL(rotr(v, 12), 0x67812345);
        TEST_EQUAL(rotr(v, 16), 0x56781234);
        TEST_EQUAL(rotr(v, 20), 0x45678123);
        TEST_EQUAL(rotr(v, 24), 0x34567812);
        TEST_EQUAL(rotr(v, 28), 0x23456781);

    }

    void check_byte_order_functions() {

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
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 0.50), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 1.00), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 1.50), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 2.00), 5.0);
        TEST_EQUAL(interpolate(1.0, 5.0, 1.0, 5.0, 2.50), 5.0);

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

    }

    u8string fa0() { return "hello"; }
    u8string fa1(char c) { return {c}; }
    u8string fa2(size_t n, char c) { return u8string(n, c); }

    u8string (*fb0)() = &fa0;
    u8string (*fb1)(char c) = &fa1;
    u8string (*fb2)(size_t n, char c) = &fa2;

    struct FC0 { u8string operator()() { return "hello"; } };
    struct FC1 { u8string operator()(char c) { return {c}; } };
    struct FC2 { u8string operator()(size_t n, char c) { return u8string(n, c); } };

    struct FD0 { u8string operator()() const { return "hello"; } };
    struct FD1 { u8string operator()(char c) const { return {c}; } };
    struct FD2 { u8string operator()(size_t n, char c) const { return u8string(n, c); } };

    FC0 fc0;
    FC1 fc1;
    FC2 fc2;

    const FD0 fd0 = {};
    const FD1 fd1 = {};
    const FD2 fd2 = {};

    auto fe0 = [] () -> u8string { return "hello"; };
    auto fe1 = [] (char c) -> u8string { return {c}; };
    auto fe2 = [] (size_t n, char c) -> u8string { return u8string(n, c); };

    using tuple0 = tuple<>;
    using tuple1 = tuple<char>;
    using tuple2 = tuple<size_t, char>;

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

        TEST_TYPE(ResultType<FTA0>, u8string);  TEST_TYPE(ResultType<FTA1>, u8string);  TEST_TYPE(ResultType<FTA2>, u8string);
        TEST_TYPE(ResultType<FTB0>, u8string);  TEST_TYPE(ResultType<FTB1>, u8string);  TEST_TYPE(ResultType<FTB2>, u8string);
        TEST_TYPE(ResultType<FTC0>, u8string);  TEST_TYPE(ResultType<FTC1>, u8string);  TEST_TYPE(ResultType<FTC2>, u8string);
        TEST_TYPE(ResultType<FTD0>, u8string);  TEST_TYPE(ResultType<FTD1>, u8string);  TEST_TYPE(ResultType<FTD2>, u8string);
        TEST_TYPE(ResultType<FTE0>, u8string);  TEST_TYPE(ResultType<FTE1>, u8string);  TEST_TYPE(ResultType<FTE2>, u8string);

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
        shared_ptr<Counted> csp;
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

        TRY(n2 = identity(n1));              TEST_EQUAL(n2, 42);
        TRY(n2 = identity(100));             TEST_EQUAL(n2, 100);
        TRY(s2 = identity(s1));              TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity(sr));              TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity(csr));             TEST_EQUAL(s2, "Hello world");
        TRY(s2 = identity("Goodbye"));       TEST_EQUAL(s2, "Goodbye");
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

        vector<string> sv {"hello", "world", "goodbye"};

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
        string s;

        {
            n = 1;
            Resource<int> r;
            TRY((r = {2, [&] (int i) { n = i; }}));
            TEST_EQUAL(r.get(), 2);
            TEST_EQUAL(n, 1);
        }
        TEST_EQUAL(n, 2);

        {
            n = 1;
            Resource<int> r;
            TRY((r = {2, [&] (int i) { n = i; }}));
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
            throw std::runtime_error("Hello");
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
            throw std::runtime_error("Hello");
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

        {
            s.clear();
            ScopedTransaction t;
            TRY(t.call([&] { s += 'a'; }, [&] { s += 'z'; }));
            TRY(t.call([&] { s += 'b'; }, [&] { s += 'y'; }));
            TRY(t.call([&] { s += 'c'; }, [&] { s += 'x'; }));
            TEST_EQUAL(s, "abc");
        }
        TEST_EQUAL(s, "abcxyz");

        {
            s.clear();
            ScopedTransaction t;
            TRY(t.call([&] { s += 'a'; }, [&] { s += 'z'; }));
            TRY(t.call([&] { s += 'b'; }, [&] { s += 'y'; }));
            TRY(t.call([&] { s += 'c'; }, [&] { s += 'x'; }));
            TEST_EQUAL(s, "abc");
            TRY(t.rollback());
            TEST_EQUAL(s, "abcxyz");
        }
        TEST_EQUAL(s, "abcxyz");

        {
            s.clear();
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
        explicit TempFile(const u8string& file): f(file) {}
        ~TempFile() { remove(f.data()); }
    private:
        u8string f;
        TempFile(const TempFile&) = delete;
        TempFile(TempFile&&) = delete;
        TempFile& operator=(const TempFile&) = delete;
        TempFile& operator=(TempFile&&) = delete;
    };

    void check_file_io_operations() {

        string s, readme = "README.md", testfile = "__test__", nofile = "__no_such_file__";
        TempFile tempfile(testfile);

        TEST(load_file(readme, s));
        TEST_EQUAL(s.substr(0, 18), "# Prion Library #\n");
        TEST(load_file(readme, s, 100));
        TEST_EQUAL(s.size(), 100);
        TEST_EQUAL(s.substr(0, 18), "# Prion Library #\n");
        TEST(load_file(readme, s, 10));
        TEST_EQUAL(s.size(), 10);
        TEST_EQUAL(s.substr(0, 18), "# Prion Li");

        TEST(save_file(testfile, "Hello world\n"s));
        TEST(load_file(testfile, s));
        TEST_EQUAL(s, "Hello world\n");
        TEST(save_file(testfile, "Goodbye\n"s, true));
        TEST(load_file(testfile, s));
        TEST_EQUAL(s, "Hello world\nGoodbye\n");
        TEST(! load_file(nofile, s));

        #if defined(PRI_TARGET_WINDOWS)

            wstring wreadme = L"README.md", wtestfile = L"__test__", wnofile = L"__no_such_file__";

            TEST(load_file(wreadme, s));
            TEST_EQUAL(s.substr(0, 18), "# Prion Library #\n");
            TEST(load_file(wreadme, s, 100));
            TEST_EQUAL(s.size(), 100);
            TEST_EQUAL(s.substr(0, 18), "# Prion Library #\n");
            TEST(load_file(wreadme, s, 10));
            TEST_EQUAL(s.size(), 10);
            TEST_EQUAL(s.substr(0, 18), "# Prion Li");

            TEST(save_file(wtestfile, "Hello world\n"s));
            TEST(load_file(wtestfile, s));
            TEST_EQUAL(s, "Hello world\n");
            TEST(save_file(wtestfile, "Goodbye\n"s, true));
            TEST(load_file(wtestfile, s));
            TEST_EQUAL(s, "Hello world\nGoodbye\n");
            TEST(! load_file(wnofile, s));

        #endif

    }

    void check_terminal_io_operations() {

        u8string s;

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

        string s;
        vector<string> sv;

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

        TRY(split("", overwrite(sv)));                         TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");
        TRY(split("Hello", overwrite(sv)));                    TEST_EQUAL(sv.size(), 1);  TEST_EQUAL(join(sv, "/"), "Hello");
        TRY(split("Hello world", overwrite(sv)));              TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("\t Hello \t world \t", overwrite(sv)));     TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("**Hello**world**"s, overwrite(sv), "*"));   TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("**Hello**world**"s, overwrite(sv), "*"s));  TEST_EQUAL(sv.size(), 2);  TEST_EQUAL(join(sv, "/"), "Hello/world");
        TRY(split("*****"s, overwrite(sv), "@*"));             TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");
        TRY(split("*****"s, overwrite(sv), "@*"s));            TEST_EQUAL(sv.size(), 0);  TEST_EQUAL(join(sv, "/"), "");

        TRY(s = quote(""s));                            TEST_EQUAL(s, "\"\""s);
        TRY(s = quote("\"\""s));                        TEST_EQUAL(s, "\"\\\"\\\"\""s);
        TRY(s = quote("Hello world"s));                 TEST_EQUAL(s, "\"Hello world\""s);
        TRY(s = quote("\\Hello\\world\\"s));            TEST_EQUAL(s, "\"\\\\Hello\\\\world\\\\\""s);
        TRY(s = quote("\"Hello\" \"world\""s));         TEST_EQUAL(s, "\"\\\"Hello\\\" \\\"world\\\"\""s);
        TRY(s = quote("\t\n\f\r"s));                    TEST_EQUAL(s, "\"\\t\\n\\f\\r\""s);
        TRY(s = quote("\x00\x01\x7f\x80\xff"s));        TEST_EQUAL(s, "\"\\0\\x01\\x7f\\x80\\xff\""s);
        TRY(s = quote("\x00\x01\x7f\x80\xff"s, true));  TEST_EQUAL(s, "\"\\0\\x01\\x7f\x80\xff\""s);

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

    }

    void check_string_formatting_and_parsing_functions() {

        string s;
        vector<string> sv;
        vector<int> iv;
        std::map<int, string> ism;

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

        TEST_EQUAL(si_to_f("0"), 0);
        TEST_EQUAL(si_to_f("42"), 42);
        TEST_EQUAL(si_to_f("-42"), -42);
        TEST_EQUAL(si_to_f("1234.5"), 1234.5);
        TEST_EQUAL(si_to_f("-1234.5"), -1234.5);
        TEST_EQUAL(si_to_f("12.34k"), 12.34e3);
        TEST_EQUAL(si_to_f("12.34K"), 12.34e3);
        TEST_EQUAL(si_to_f("12.34 k"), 12.34e3);
        TEST_EQUAL(si_to_f("12.34 K"), 12.34e3);
        TEST_EQUAL(si_to_f("12.34 M"), 12.34e6);
        TEST_EQUAL(si_to_f("12.34 G"), 12.34e9);
        TEST_EQUAL(si_to_f("12.34 T"), 12.34e12);
        TEST_EQUAL(si_to_f("12.34 P"), 12.34e15);
        TEST_EQUAL(si_to_f("12.34 E"), 12.34e18);
        TEST_EQUAL(si_to_f("12.34 Z"), 12.34e21);
        TEST_EQUAL(si_to_f("12.34 Y"), 12.34e24);
        TEST_THROW(si_to_f("1e9999"), std::range_error);
        TEST_THROW(si_to_f(""), std::invalid_argument);
        TEST_THROW(si_to_f("k999"), std::invalid_argument);
        TEST_THROW(si_to_f("999x"), std::invalid_argument);

        TEST_EQUAL(si_to_i("0"), 0);
        TEST_EQUAL(si_to_i("42"), 42);
        TEST_EQUAL(si_to_i("-42"), -42);
        TEST_EQUAL(si_to_i("12.34k"), 12340);
        TEST_EQUAL(si_to_i("12.34K"), 12340);
        TEST_EQUAL(si_to_i("12.34 k"), 12340);
        TEST_EQUAL(si_to_i("12.34 K"), 12340);
        TEST_EQUAL(si_to_i("12.34 M"), 12340000ll);
        TEST_EQUAL(si_to_i("12.34 G"), 12340000000ll);
        TEST_EQUAL(si_to_i("12.34 T"), 12340000000000ll);
        TEST_EQUAL(si_to_i("12.34 P"), 12340000000000000ll);
        TEST_THROW(si_to_i("12.34 E"), std::range_error);
        TEST_THROW(si_to_i("12.34 Z"), std::range_error);
        TEST_THROW(si_to_i("12.34 Y"), std::range_error);
        TEST_THROW(si_to_i("1e9999"), std::range_error);
        TEST_THROW(si_to_i(""), std::invalid_argument);
        TEST_THROW(si_to_i("k999"), std::invalid_argument);
        TEST_THROW(si_to_i("999x"), std::invalid_argument);

        TEST_EQUAL(to_si(0), "0.00");
        TEST_EQUAL(to_si(42), "42.0");
        TEST_EQUAL(to_si(-42), "-42.0");
        TEST_EQUAL(to_si(1.234e-2), "0.0123");
        TEST_EQUAL(to_si(1.234e-1), "0.123");
        TEST_EQUAL(to_si(1.234e0), "1.23");
        TEST_EQUAL(to_si(1.234e1), "12.3");
        TEST_EQUAL(to_si(1.234e2), "123");
        TEST_EQUAL(to_si(1.234e3), "1.23k");
        TEST_EQUAL(to_si(1.234e4), "12.3k");
        TEST_EQUAL(to_si(1.234e5), "123k");
        TEST_EQUAL(to_si(1.234e6), "1.23M");
        TEST_EQUAL(to_si(1.234e7), "12.3M");
        TEST_EQUAL(to_si(1.234e8), "123M");
        TEST_EQUAL(to_si(1.234e9), "1.23G");
        TEST_EQUAL(to_si(1.234e10), "12.3G");
        TEST_EQUAL(to_si(1.234e11), "123G");
        TEST_EQUAL(to_si(1.234e12), "1.23T");
        TEST_EQUAL(to_si(1.234e13), "12.3T");
        TEST_EQUAL(to_si(1.234e14), "123T");
        TEST_EQUAL(to_si(1.234e15), "1.23P");
        TEST_EQUAL(to_si(1.234e16), "12.3P");
        TEST_EQUAL(to_si(1.234e17), "123P");
        TEST_EQUAL(to_si(1.234e18), "1.23E");
        TEST_EQUAL(to_si(1.234e19), "12.3E");
        TEST_EQUAL(to_si(1.234e20), "123E");
        TEST_EQUAL(to_si(1.234e21), "1.23Z");
        TEST_EQUAL(to_si(1.234e22), "12.3Z");
        TEST_EQUAL(to_si(1.234e23), "123Z");
        TEST_EQUAL(to_si(1.234e24), "1.23Y");
        TEST_EQUAL(to_si(1.234e25), "12.3Y");
        TEST_EQUAL(to_si(1.234e26), "123Y");
        TEST_EQUAL(to_si(12345678), "12.3M");
        TEST_EQUAL(to_si(12345678, 6), "12.3457M");
        TEST_EQUAL(to_si(12345678, 3, " "), "12.3 M");

        TEST_EQUAL(hexdump(""s), "");
        TEST_EQUAL(hexdump(""s, 5), "");
        TEST_EQUAL(hexdump("Hello world!"s), "48 65 6c 6c 6f 20 77 6f 72 6c 64 21");
        TEST_EQUAL(hexdump("Hello world!"s, 5), "48 65 6c 6c 6f\n20 77 6f 72 6c\n64 21");
        TEST_EQUAL(hexdump("Hello world!"s, 6), "48 65 6c 6c 6f 20\n77 6f 72 6c 64 21");

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
        TEST_EQUAL(to_str('X'), "X");

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

    void check_html_xml_tags() {

        const u8string expected =
            "<h1>Header</h1>\n"
            "<br/>\n"
            "<ul>\n"
            "<li>alpha</li>\n"
            "<li>bravo</li>\n"
            "<li>charlie</li>\n"
            "</ul>\n";

        {
            std::ostringstream out;
            {
                Tag h1;
                TRY(h1 = Tag("<h1>\n", out));
                out << "Header";
            }
            Tag br;
            TRY(br = Tag("<br/>\n", out));
            {
                Tag ul;
                TRY(ul = Tag("<ul>\n\n", out));
                for (auto item: {"alpha", "bravo", "charlie"}) {
                    Tag li;
                    TRY(li = Tag("<li>\n", out));
                    out << item;
                }
            }
            u8string s = out.str();
            TEST_EQUAL(s, expected);
        }

        {
            std::ostringstream out;
            {
                Tag h1("<h1>\n", out);
                out << "Header";
            }
            Tag br("<br/>\n", out);
            {
                Tag ul("<ul>\n\n", out);
                for (auto item: {"alpha", "bravo", "charlie"}) {
                    Tag li("<li>\n", out);
                    out << item;
                }
            }
            u8string s = out.str();
            TEST_EQUAL(s, expected);
        }

    }

    void check_thread_class() {

        TEST_COMPARE(Thread::cpu_threads(), >=, 1);
        TRY(Thread::yield());

        {
            Thread t(0);
            TRY(t.wait());
            TEST(t.poll());
        }

        {
            string s;
            Thread t([&] { s = "Neddie"; });
            TEST_COMPARE(t.get_id(), !=, Thread::current());
            TRY(t.wait());
            TEST(t.poll());
            TEST_EQUAL(s, "Neddie");
        }

        {
            Thread t([&] { throw std::runtime_error("Hello"); });
            TEST_THROW_MATCH(t.wait(), std::runtime_error, "Hello");
            TRY(t.wait());
        }

    }

    void check_synchronisation_objects() {

        {
            Mutex m;
            ConditionVariable cv;
            int n = 0;
            MutexLock lock(m);
            TEST(! cv.wait_for(lock, milliseconds(50), [&] { return n > 0; }));
            n = 42;
            TEST(cv.wait_for(lock, milliseconds(50), [&] { return n > 0; }));
        }

        {
            Mutex m;
            ConditionVariable cv;
            string s;
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
            Mutex m;
            ConditionVariable cv;
            string s;
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
            Mutex m;
            ConditionVariable cv;
            string s;
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
        TRY(from_seconds(7200.0, ms));  TEST_EQUAL(ms.count(), 7200000);

        TEST_EQUAL(to_seconds(hours(10)), 36000);
        TEST_EQUAL(to_seconds(minutes(10)), 600);
        TEST_EQUAL(to_seconds(seconds(10)), 10);
        TEST_NEAR(to_seconds(milliseconds(10)), 0.01);

        TRY(tp = make_date(1970, 1, 1, 0, 0, 0));
        TRY(t1 = int64_t(system_clock::to_time_t(tp)));
        TEST_EQUAL(t1, 0);
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5));
        TRY(t1 = int64_t(system_clock::to_time_t(tp)));
        TEST_EQUAL(t1, 946782245);
        TRY(tp = make_date(2000, 1, 2, 3, 4, 5, local_date));
        TRY(t2 = int64_t(system_clock::to_time_t(tp)));
        TEST_COMPARE(abs(t2 - t1), <=, 86400);

    }

    void check_time_and_date_formatting() {

        using DblSec = duration<double>;

        system_clock::time_point tp;
        system_clock::duration d;
        u8string str;

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

    }

    void check_system_specific_time_and_date_conversions() {

        using IntMsec = duration<int64_t, std::ratio<1, 1000>>;

        #if defined(PRI_TARGET_UNIX)

            using IntSec = duration<int64_t>;
            using IntDays = duration<int64_t, std::ratio<86400>>;
            using DblSec = duration<double>;

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

        #if defined(PRI_TARGET_WIN32)

            static constexpr int64_t epoch = 11644473600ll;
            static constexpr int64_t freq = 10000000ll;

            int64_t n;
            FILETIME ft;
            system_clock::time_point tp;
            system_clock::duration d;

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
            TRY(ft = timepoint_to_filetime(tp));
            TEST_EQUAL(ft.dwHighDateTime, 0);
            TEST_EQUAL(ft.dwLowDateTime, 0);

            tp = system_clock::from_time_t(86400);
            TRY(ft = timepoint_to_filetime(tp));
            TEST_EQUAL(ft.dwHighDateTime, 201);
            TEST_EQUAL(ft.dwLowDateTime, 711573504);

        #endif

    }

}

TEST_MODULE(prion, core) {

    check_preprocessor_macros();
    check_containers();
    check_exceptions();
    check_metaprogramming_and_type_traits();
    check_type_related_functions();
    check_uuid();
    check_version_number();
    check_arithmetic_literals();
    check_generic_algorithms();
    check_integer_sequences();
    check_memory_algorithms();
    check_secure_memory_algorithms();
    check_range_traits();
    check_range_types();
    check_generic_arithmetic_functions();
    check_integer_arithmetic_functions();
    check_bitwise_operations();
    check_byte_order_functions();
    check_floating_point_arithmetic_functions();
    check_function_traits();
    check_function_operations();
    check_generic_function_objects();
    check_hash_functions();
    check_keyword_arguments();
    check_scope_guards();
    check_file_io_operations();
    check_terminal_io_operations();
    check_character_functions();
    check_general_string_functions();
    check_string_formatting_and_parsing_functions();
    check_html_xml_tags();
    check_thread_class();
    check_synchronisation_objects();
    check_general_time_and_date_operations();
    check_time_and_date_formatting();
    check_system_specific_time_and_date_conversions();

}
