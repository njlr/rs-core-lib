#include "rs-core/random.hpp"
#include "rs-core/unit-test.hpp"

using namespace RS;

namespace {

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
        for (size_t k = 0; k <= pop_size; ++k) {
            double count = 0, sum = 0, sum2 = 0;
            for (size_t i = 0; i < sample_iterations; ++i) {
                TRY(sample = random_sample(rng, pop, k));
                TEST_EQUAL(sample.size(), k);
                TRY(con_sort_unique(sample));
                TEST_EQUAL(sample.size(), k);
                count += sample.size();
                for (auto x: sample) {
                    sum += x;
                    sum2 += x * x;
                }
            }
            if (k >= 1) {
                double mean = sum / count;
                TEST_NEAR_EPSILON(mean, expect_mean, 4);
                if (k >= 2) {
                    double sd = sqrt((sum2 - count * mean * mean) / (count - 1));
                    TEST_NEAR_EPSILON(sd, expect_sd, 2);
                }
            }
        }
        TEST_THROW(random_sample(rng, pop, 101), std::length_error);

    }

    void check_other_random_functions() {

        static constexpr size_t n = 10000;

        std::mt19937 rng(42);
        uint32_t u32 = 0;
        uint64_t u64 = 0;
        double x = 0, sum = 0, sum2 = 0, mean = 0, sd = 0;

        for (size_t i = 0; i < n; ++i) {
            TRY(random_bytes(rng, &u32, 4));
            x = ldexp(u32, -32);
            sum += x;
            sum2 += x * x;
        }
        mean = sum / n;
        sd = sqrt(sum2 / n - mean * mean);
        TEST_NEAR_EPSILON(mean, 0.5, 0.05);
        TEST_NEAR_EPSILON(sd, 0.288675, 0.05);

        sum = sum2 = 0;
        for (size_t i = 0; i < n; ++i) {
            TRY(random_bytes(rng, &u64, 8));
            x = ldexp(u64, -64);
            sum += x;
            sum2 += x * x;
        }
        mean = sum / n;
        sd = sqrt(sum2 / n - mean * mean);
        TEST_NEAR_EPSILON(mean, 0.5, 0.05);
        TEST_NEAR_EPSILON(sd, 0.288675, 0.05);

        U8string s;

        TRY(shuffle(rng, s));
        TEST_EQUAL(s, "");

        for (int i = 0; i < 1000; ++i) {
            s = "abcdefghij";
            TRY(shuffle(rng, s));
            TEST_COMPARE(s, >, "abcdefghij");
            TRY(std::sort(s.begin(), s.end()));
            TEST_EQUAL(s, "abcdefghij");
        }

    }

}

TEST_MODULE(core, random) {

    check_simple_random_generators();
    check_other_random_functions();

}
