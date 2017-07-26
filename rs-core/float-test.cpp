#include "rs-core/float.hpp"
#include "rs-core/unit-test.hpp"

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

}

TEST_MODULE(core, float) {

    check_arithmetic_constants();
    check_arithmetic_functions();
    check_arithmetic_literals();

}
