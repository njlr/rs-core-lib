#pragma once

#include "prion/core.hpp"
#include <pcre.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Unit testing macros

#define TEST_MAIN int main() { return ::Test::test_main(); }

#define TEST_MODULE(project, module) \
    void test_ ## project ## _ ## module(); \
    void test_ ## project ## _ ## module ## _setup() __attribute__((constructor)); \
    void test_ ## project ## _ ## module ## _setup() \
        { ::Test::register_test(test_ ## project ## _ ## module, # project, # module); } \
    void test_ ## project ## _ ## module()

#define FAIL_POINT "[", __FILE__, ":", __LINE__, "] "

#define TRY_IMPL(expr, desc) \
    do { \
        try { \
            expr; \
        } \
        catch (const std::exception& ex) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, desc, " threw exception: ", ex.what()); \
        } \
        catch (...) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, desc, " threw exception of unknown type"); \
        } \
    } while (false)

#define TEST_IMPL(status, expr, desc) \
    do { \
        status = true; \
        TRY_IMPL(status = bool(expr), desc); \
    } while (false)

#define FAIL(msg) \
    do { \
        ::Test::record_failure(); \
        ::Test::print_fail(FAIL_POINT, (msg)); \
    } while (false)

#define MESSAGE(msg) \
    do { \
        ::Test::print_fail("*** [", __FILE__, ":", __LINE__, "] ", (msg)); \
    } while (false)

#define REQUIRE(expr) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, (expr), # expr); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr " is false"); \
            return; \
        } \
    } while (false)

#define TEST(expr) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, (expr), # expr); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr " is false"); \
        } \
    } while (false)

#define TEST_COMPARE(lhs, op, rhs) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, (lhs) op (rhs), # lhs " " # op " " # rhs); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # lhs " " # op " " # rhs " failed: lhs = ", lhs, ", rhs = ", rhs); \
        } \
    } while (false)

#define TEST_EQUAL(lhs, rhs) \
    TEST_COMPARE((lhs), ==, (rhs))

#define TEST_EQUAL_RANGE(lhs, rhs) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, ::Test::range_equal((lhs), (rhs)), # lhs " == " # rhs); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # lhs " != " # rhs ": lhs = ", ::Test::format_range(lhs), \
                ", rhs = ", ::Test::format_range(rhs)); \
        } \
    } while (false)

#define TEST_MATCH(str, pattern) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, ::Test::regex_match(pattern, str), \
            "regex match(" # str ", " # pattern ")"); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, "regex match(" # str ", " # pattern ") failed: lhs = ", str); \
        } \
    } while (false)

#define TEST_NEAR_EPSILON(lhs, rhs, tolerance) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, fabs(double(lhs) - double(rhs)) <= double(tolerance), \
            # lhs " approx == " # rhs); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # lhs " and " # rhs " not approximately equal: lhs = ", \
                (lhs), ", rhs = ", (rhs), ", tolerance = ", (tolerance)); \
        } \
    } while (false)

#define TEST_NEAR_EPSILON_RANGE(lhs, rhs, tolerance) \
    do { \
        bool local_test_status = false; \
        TEST_IMPL(local_test_status, ::Test::range_near((lhs), (rhs), double(tolerance)), \
            # lhs " approx == " # rhs); \
        if (! local_test_status) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # lhs " and " # rhs " not approximately equal: lhs = ", \
                ::Test::format_range(lhs), ", rhs = ", ::Test::format_range(rhs)); \
        } \
    } while (false)

#define TEST_NEAR(lhs, rhs) \
    TEST_NEAR_EPSILON((lhs), (rhs), 1e-6)

#define TEST_NEAR_RANGE(lhs, rhs) \
    TEST_NEAR_EPSILON_RANGE((lhs), (rhs), 1e-6)

#define TEST_THROW(expr, expect) \
    do { \
        try { \
            expr; \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " failed to throw exception: expected ", # expect); \
        } \
        catch (const expect&) {} \
        catch (const std::exception& ex) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " threw wrong exception: expected ", # expect, \
                ", got ", ex.what()); \
        } \
        catch (...) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " threw wrong exception: expected ", # expect, \
                ", got unknown exception"); \
        } \
    } while (false)

#define TEST_THROW_EQUAL(expr, expect, message) \
    do { \
        try { \
            expr; \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " failed to throw exception: expected ", # expect); \
        } \
        catch (const expect& exception) { \
            TEST_EQUAL(std::string(exception.what()), message); \
        } \
        catch (const std::exception& ex) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " threw wrong exception: expected ", # expect, \
                ", got ", ex.what()); \
        } \
        catch (...) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " threw wrong exception: expected ", # expect, \
                ", got unknown exception"); \
        } \
    } while (false)

#define TEST_THROW_MATCH(expr, expect, pattern) \
    do { \
        try { \
            expr; \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " failed to throw exception: expected ", # expect); \
        } \
        catch (const expect& exception) { \
            TEST_MATCH(exception.what(), pattern); \
        } \
        catch (const std::exception& ex) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " threw wrong exception: expected ", # expect, \
                ", got ", ex.what()); \
        } \
        catch (...) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, # expr, " threw wrong exception: expected ", # expect, \
                ", got unknown exception"); \
        } \
    } while (false)

#define TEST_TYPE(type1, type2) \
    do { \
        using local_test_type1 = std::decay_t<type1>; \
        using local_test_type2 = std::decay_t<type2>; \
        if (! std::is_same<local_test_type1, local_test_type2>::value) { \
            ::Test::record_failure(); \
            ::Test::print_fail(FAIL_POINT, "Type mismatch: ", # type1, " != ", # type2, \
                "; type 1 = ", Prion::type_name(typeid(local_test_type1)), ", ", \
                "type 2 = ", Prion::type_name(typeid(local_test_type2))); \
        } \
    } while (false)

#define TEST_TYPE_OF(object, type) \
    TEST_TYPE(decltype(object), type)

#define TEST_TYPES_OF(object1, object2) \
    TEST_TYPE(decltype(object1), decltype(object2))

#define TRY(expr) \
    TRY_IMPL((expr), # expr)

// Implementation wrapper

struct Test {

    struct compare_modules {
        static bool is_core(const std::string& s) noexcept {
            return s.size() > 5 && memcmp(s.data() + s.size() - 5, "/core", 5) == 0;
        }
        bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
            bool lcore = is_core(lhs), rcore = is_core(rhs);
            if (lcore && ! rcore)
                return true;
            else if (rcore && ! lcore)
                return false;
            else
                return lhs < rhs;
        }
    };

    using test_function = void (*)();
    using test_map = std::map<std::string, test_function, compare_modules>;

    static constexpr const char* x_reset = "\x1b[0m";
    static constexpr const char* x_head = "\x1b[38;5;220m";
    static constexpr const char* x_decor = "\x1b[38;5;244m";
    static constexpr const char* x_info = "\x1b[38;5;228m";
    static constexpr const char* x_good = "\x1b[38;5;83m";
    static constexpr const char* x_fail = "\x1b[38;5;208m";

    static std::atomic<size_t>& test_failures() noexcept {
        static std::atomic<size_t> fails;
        return fails;
    }

    static test_map& test_functions() noexcept {
        static test_map funcs;
        return funcs;
    }

    static void record_failure() {
        ++test_failures();
    }

    static void register_test(test_function f, const std::string& project, const std::string& module) {
        auto key = project + '/' + module;
        std::replace(key.begin(), key.end(), '_', '-');
        test_functions()[key] = f;
    }

    static void print_build(std::ostringstream&) {}

    template <typename T, typename... Args>
    static void print_build(std::ostringstream& out, const T& t, const Args&... args) {
        out << preformat(t);
        print_build(out, args...);
    }

    static void print_out(const std::string& str) {
        static Prion::Mutex mtx;
        Prion::MutexLock lock(mtx);
        std::cout << str << std::endl;
    }

    template <typename... Args>
    static void print(const Args&... args) {
        std::ostringstream out;
        print_build(out, args...);
        print_out(out.str());
    }

    template <typename... Args>
    static void print_fail(const Args&... args) {
        std::ostringstream out;
        out << "\x1b[38;5;208m*** ";
        print_build(out, args...);
        out << "\x1b[0m";
        print_out(out.str());
    }

    template <typename R1, typename R2>
    static bool range_equal(const R1& r1, const R2& r2) {
        auto i = std::begin(r1), e1 = std::end(r1);
        auto j = std::begin(r2), e2 = std::end(r2);
        for (; i != e1 && j != e2 && *i == *j; ++i, ++j) {}
        return i == e1 && j == e2;
    }

    template <typename R1, typename R2>
    static bool range_near(const R1& r1, const R2& r2, double tolerance) {
        auto i = std::begin(r1), e1 = std::end(r1);
        auto j = std::begin(r2), e2 = std::end(r2);
        for (; i != e1 && j != e2 && fabs(double(*i) - double(*j)) <= tolerance; ++i, ++j) {}
        return i == e1 && j == e2;
    }

    static bool regex_match(const std::string& pattern, const std::string& str) {
        static constexpr int options = PCRE_DOTALL | PCRE_NEWLINE_LF | PCRE_UTF8;
        const char* estr = nullptr;
        int epos = 0;
        std::shared_ptr<pcre> p(pcre_compile(pattern.data(), options, &estr, &epos, nullptr), pcre_free);
        if (! p)
            throw std::invalid_argument("Invalid regex: " + preformat(pattern) + "\n" + estr);
        int caps = 0;
        pcre_fullinfo(p.get(), nullptr, PCRE_INFO_CAPTURECOUNT, &caps);
        std::vector<int> ofs(3 * (caps + 1));
        auto rc = pcre_exec(p.get(), nullptr, str.data(), int(str.size()),
            0, 0, ofs.data(), int(ofs.size()));
        return rc > 0;
    }

    template <typename C>
    static std::string preformat_string(const std::basic_string<C>& t) {
        using utype = std::make_unsigned_t<C>;
        std::string result;
        for (C c: t) {
            auto uc = utype(c);
            switch (uc) {
                case '\0': result += "\\0"; break;
                case '\t': result += "\\t"; break;
                case '\n': result += "\\n"; break;
                case '\f': result += "\\f"; break;
                case '\r': result += "\\r"; break;
                case '\\': result += "\\\\"; break;
                default:
                    if (uc >= 0x20 && uc <= 0x7e) {
                        result += char(uc);
                    } else if (uc <= 0xff) {
                        result += "\\x";
                        result += Prion::hex(uc, 2);
                    } else {
                        result += "\\x{";
                        result += Prion::hex(uc);
                        result += "}";
                    }
                    break;
            }
        }
        return result;
    }

    template <typename T> static T preformat(const T& t) { return t; }
    template <typename T> static const void* preformat(T* t) noexcept { return t; }
    static unsigned preformat(unsigned char t) noexcept { return t; }
    static int preformat(signed char t) noexcept { return t; }
    static std::string preformat(Prion::int128_t t) noexcept { return Prion::dec(t); }
    static std::string preformat(Prion::uint128_t t) noexcept { return Prion::dec(t); }
    static std::string preformat(char16_t t) noexcept { return "U+" + Prion::hex(t, 4); }
    static std::string preformat(char32_t t) noexcept { return "U+" + Prion::hex(t, 4); }
    static std::string preformat(wchar_t t) noexcept { return "U+" + Prion::hex(std::make_unsigned_t<wchar_t>(t), 4); }
    static std::string preformat(const std::string& t) { return preformat_string(t); }
    static std::string preformat(const std::u16string& t) { return preformat_string(t); }
    static std::string preformat(const std::u32string& t) { return preformat_string(t); }
    static std::string preformat(const std::wstring& t) { return preformat_string(t); }
    static std::string preformat(const char* t) { return t ? preformat(std::string(t)) : "<null>"; }
    static std::string preformat(const char16_t* t) { return t ? preformat(std::u16string(t)) : "<null>"; }
    static std::string preformat(const char32_t* t) { return t ? preformat(std::u32string(t)) : "<null>"; }
    static std::string preformat(const wchar_t* t) { return t ? preformat(std::wstring(t)) : "<null>"; }
    template <typename T> static T preformat(const std::atomic<T>& t) { return t; }

    template <typename R, typename I = decltype(std::begin(std::declval<R>())),
        typename V = typename std::iterator_traits<I>::value_type>
    struct FormatRange {
        std::string operator()(const R& r) const {
            std::ostringstream out;
            out << '[';
            for (auto& x: r)
                out << preformat(x) << ',';
            std::string s = out.str();
            if (s.size() > 1)
                s.pop_back();
            s += ']';
            return s;
        }
    };

    template <typename R, typename I, typename K, typename V>
    struct FormatRange<R, I, std::pair<K, V>> {
        std::string operator()(const R& r) const {
            std::ostringstream out;
            out << '{';
            for (auto& x: r)
                out << preformat(x.first) << ':' << preformat(x.second) << ',';
            std::string s = out.str();
            if (s.size() > 1)
                s.pop_back();
            s += '}';
            return s;
        }
    };

    template <typename R>
    static std::string format_range(const R& r) {
        return FormatRange<R>()(r);
    }

    static int test_main() {
        using namespace std::chrono;
        const char* unit_ptr = getenv("UNIT");
        std::string unit_pattern = unit_ptr ? unit_ptr : ".*";
        std::cout << std::endl;
        try {
            size_t test_count = 0;
            std::string rule(20, '=');
            std::cout << x_head << "Running test modules" << x_reset << std::endl;
            std::cout << x_decor << rule << x_reset << std::endl;
            auto start = system_clock::now();
            for (auto&& test: test_functions()) {
                if (regex_match(unit_pattern, test.first)) {
                    std::cout << x_info << "Testing " << test.first << x_reset << std::endl;
                    test.second();
                    ++test_count;
                }
            }
            auto seconds = duration_cast<duration<double>>(system_clock::now() - start).count();
            std::cout << x_decor << rule << x_reset << std::endl;
            if (! test_count)
                std::cout << x_fail << "No tests were run" << x_reset << std::endl;
            else if (test_failures() > 0)
                std::cout << x_fail << "*** Test failures: " << test_failures() << x_reset << std::endl;
            else
                std::cout << x_good << "OK: all tests passed" << x_reset << std::endl;
            std::cout << x_info << "Time: " << seconds << " s" << x_reset << std::endl;
        }
        catch (const std::exception& ex) {
            std::cout << x_fail << "*** " << ex.what() << x_reset << std::endl;
        }
        std::cout << std::endl;
        return test_failures() > 0;
    }

};
