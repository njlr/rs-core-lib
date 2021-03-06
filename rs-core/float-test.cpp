#include "rs-core/float.hpp"
#include "rs-core/unit-test.hpp"
#include "rs-core/vector.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>
#include <vector>

using namespace RS;
using namespace RS::Literals;

namespace {

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

    void check_arithmetic_functions() {

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

    void check_arithmetic_literals() {

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

    void check_line_integral() {

        double integral = 0;

        TRY((integral = line_integral(0.0, 10.0, 10, [] (auto x) { return x * x; })));
        TEST_EQUAL(integral, 335);

    }

    void check_volume_integral() {

        double integral = 0;

        TRY((integral = volume_integral(Double2(0, 0), Double2(5, 5), 5, [] (auto x) { return x[0] * x[1]; })));
        TEST_EQUAL(integral, 156.25);

    }

    template <typename T, typename RNG>
    void precision_sum_test(size_t copies, size_t cycles, T highval, RNG& rng) {
        std::vector<T> values;
        values.reserve(4 * copies);
        for (size_t i = 0; i < copies; ++i) {
            values.push_back(1);
            values.push_back(highval);
            values.push_back(1);
            values.push_back(- highval);
        }
        for (size_t i = 0; i < cycles; ++i) {
            T sum = 0;
            std::shuffle(values.begin(), values.end(), rng);
            TRY(sum = precision_sum(values));
            TEST_EQUAL(sum, static_cast<T>(2 * copies));
        }
    }

    void check_precision_sum() {

        std::mt19937 rng(42);
        precision_sum_test<float>(1000, 100, 1e20f, rng);
        precision_sum_test<double>(1000, 100, 1e100, rng);
        precision_sum_test<long double>(1000, 100, 1e100l, rng);

    }

    void check_root_finding_parameters() {

        using namespace RS_Detail;

        float fe = default_epsilon<float>();
        double de = default_epsilon<double>();
        long double lde = default_epsilon<long double>();

        TEST_NEAR_EPSILON(fe, 1e-4, 1e-8);
        TEST_NEAR_EPSILON(de, 1e-8, 1e-16);
        TEST_NEAR_EPSILON(lde, 1e-8, 1e-16);

    }

    struct F {
        double k = 1;
        double operator()(double x) const { return cos(k * x) - pow(k * x, 3); };
    };

    struct DF {
        double k = 1;
        double operator()(double x) const { return - k * (3 * pow(k * x, 2) + sin(k * x)); };
    };

    constexpr double root = 0.8654740331;

    void check_bisection() {

        Bisection<double> algo;
        double x = 0;

        TRY(x = algo(F{1}, 0, 1));
        TEST_NEAR(x, root);

        for (int i = 1; i <= 1024; i *= 2) {
            double k = i;
            TRY(x = algo(F{k}, 0.5 / i, 1.0 / i));
            // logx("BI k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR(x, root / i);
            TEST_COMPARE(algo.count, <, 50);
        }

        for (int i = 1; i <= 1024; i *= 2) {
            double k = 1.0 / i;
            TRY(x = algo(F{k}, 0.5, i));
            // logx("BI 1/k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR_EPSILON(x, root * i, 1e-6 * i);
            TEST_COMPARE(algo.count, <, 50);
        }

    }

    void check_false_position() {

        FalsePosition<double> algo;
        double x = 0;

        TRY(x = algo(F{1}, 0, 1));
        TEST_NEAR(x, root);

        for (int i = 1; i <= 1024; i *= 2) {
            double k = i;
            TRY(x = algo(F{k}, 0.5 / i, 1.0 / i));
            // logx("FP k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR(x, root / i);
            TEST_COMPARE(algo.count, <, 50);
        }

        for (int i = 1; i <= 1024; i *= 2) {
            double k = 1.0 / i;
            TRY(x = algo(F{k}, 0.5, i));
            // logx("FP 1/k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR_EPSILON(x, root * i, 1e-6 * i);
            TEST_COMPARE(algo.count, <, 50);
        }

    }

    void check_newton_raphson() {

        NewtonRaphson<double> algo;
        double x = 0;

        TRY(x = algo(F{1}, DF{1}, 1));
        TEST_NEAR(x, root);

        for (int i = 1; i <= 1024; i *= 2) {
            double k = i;
            TRY(x = algo(F{k}, DF{k}, 1));
            // logx("NR k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR(x, root / i);
            TEST_COMPARE(algo.count, <, 50);
        }

        for (int i = 1; i <= 1024; i *= 2) {
            double k = 1.0 / i;
            TRY(x = algo(F{k}, DF{k}, 1));
            // logx("NR 1/k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR_EPSILON(x, root * i, 1e-6 * i);
            TEST_COMPARE(algo.count, <, 50);
        }

    }

    void check_pseudo_newton_raphson() {

        PseudoNewtonRaphson<double> algo;
        double x = 0;

        TRY(x = algo(F{1}, 1));
        TEST_NEAR(x, root);

        for (int i = 1; i <= 1024; i *= 2) {
            double k = i;
            TRY(x = algo(F{k}, 1));
            // logx("PNR k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR(x, root / i);
            TEST_COMPARE(algo.count, <, 50);
        }

        for (int i = 1; i <= 1024; i *= 2) {
            double k = 1.0 / i;
            TRY(x = algo(F{k}, 1));
            // logx("PNR 1/k", i, "x", x, "e", algo.error, "c", algo.count);
            TEST_NEAR_EPSILON(x, root * i, 1e-6 * i);
            TEST_COMPARE(algo.count, <, 50);
        }

    }

}

TEST_MODULE(core, float) {

    check_arithmetic_constants();
    check_arithmetic_functions();
    check_arithmetic_literals();
    check_line_integral();
    check_volume_integral();
    check_precision_sum();
    check_root_finding_parameters();
    check_bisection();
    check_false_position();
    check_newton_raphson();
    check_pseudo_newton_raphson();

}
