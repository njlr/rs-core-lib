#include "rs-core/vector.hpp"
#include "rs-core/float.hpp"
#include "rs-core/string.hpp"
#include "rs-core/unit-test.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace RS;

namespace {

    void check_integer_vector() {

        int x = 0;
        Int3 v1(1), v2, v3;
        const Int3& cv1(v1);
        const Int3& cv2(v2);
        const Int3& cv3(v3);

        TEST_EQUAL(sizeof(Int3), 3 * sizeof(int));

        TEST(! v1.is_null());
        TEST(v2.is_null());

        TEST_EQUAL(to_str(v1), "[1,1,1]");
        TEST_EQUAL(to_str(cv1), "[1,1,1]");

        TEST_EQUAL(v1[0], 1);  TEST_EQUAL(cv1[0], 1);
        TEST_EQUAL(v1[1], 1);  TEST_EQUAL(cv1[1], 1);
        TEST_EQUAL(v1[2], 1);  TEST_EQUAL(cv1[2], 1);

        TRY(v2[0] = 99);
        TRY(v2[1] = 98);
        TRY(v2[2] = 97);
        TEST_EQUAL(to_str(v2), "[99,98,97]");
        TEST_EQUAL(to_str(cv2), "[99,98,97]");

        TEST_EQUAL(v2[0], 99);     TEST_EQUAL(cv2[0], 99);
        TEST_EQUAL(v2[1], 98);     TEST_EQUAL(cv2[1], 98);
        TEST_EQUAL(v2[2], 97);     TEST_EQUAL(cv2[2], 97);
        TEST_EQUAL(v2.at(0), 99);  TEST_EQUAL(cv2.at(0), 99);
        TEST_EQUAL(v2.at(1), 98);  TEST_EQUAL(cv2.at(1), 98);
        TEST_EQUAL(v2.at(2), 97);  TEST_EQUAL(cv2.at(2), 97);
        TEST_EQUAL(v2.x(), 99);    TEST_EQUAL(cv2.x(), 99);
        TEST_EQUAL(v2.y(), 98);    TEST_EQUAL(cv2.y(), 98);
        TEST_EQUAL(v2.z(), 97);    TEST_EQUAL(cv2.z(), 97);

        TEST_THROW(v2.at(3), std::out_of_range);
        TEST_THROW(cv2.at(3), std::out_of_range);

        TRY((v3 = Int3{1,2,3}));
        TEST_EQUAL(to_str(v3), "[1,2,3]");
        TEST_EQUAL(to_str(cv3), "[1,2,3]");

        TRY((v3 = Int3(2,4,6)));
        TEST_EQUAL(to_str(v3), "[2,4,6]");
        TEST_EQUAL(to_str(cv3), "[2,4,6]");

        TRY((v3 = {10,20,30}));
        TEST_EQUAL(to_str(v3), "[10,20,30]");
        TEST_EQUAL(to_str(cv3), "[10,20,30]");

        TEST_EQUAL(v3[0], 10);          TEST_EQUAL(cv3[0], 10);
        TEST_EQUAL(v3[1], 20);          TEST_EQUAL(cv3[1], 20);
        TEST_EQUAL(v3[2], 30);          TEST_EQUAL(cv3[2], 30);
        TEST_EQUAL(v3.begin()[0], 10);  TEST_EQUAL(cv3.begin()[0], 10);
        TEST_EQUAL(v3.begin()[1], 20);  TEST_EQUAL(cv3.begin()[1], 20);
        TEST_EQUAL(v3.begin()[2], 30);  TEST_EQUAL(cv3.begin()[2], 30);

        TEST(v1 == v1);      TEST(! (v1 == v2));  TEST(! (v1 == v3));  TEST(! (v2 == v3));
        TEST(! (v1 != v1));  TEST(v1 != v2);      TEST(v1 != v3);      TEST(v2 != v3);
        TEST(! (v1 < v1));   TEST(v1 < v2);       TEST(v1 < v3);       TEST(! (v2 < v3));
        TEST(! (v1 > v1));   TEST(! (v1 > v2));   TEST(! (v1 > v3));   TEST(v2 > v3);
        TEST(v1 <= v1);      TEST(v1 <= v2);      TEST(v1 <= v3);      TEST(! (v2 <= v3));
        TEST(v1 >= v1);      TEST(! (v1 >= v2));  TEST(! (v1 >= v3));  TEST(v2 >= v3);

        TRY((v1 = {2,3,5}));
        TRY((v2 = {7,11,13}));

        TRY(v3 = + v1);          TEST_EQUAL(to_str(v3), "[2,3,5]");
        TRY(v3 = - v1);          TEST_EQUAL(to_str(v3), "[-2,-3,-5]");
        TRY(v3 = v1 + v2);       TEST_EQUAL(to_str(v3), "[9,14,18]");
        TRY(v3 = v1 - v2);       TEST_EQUAL(to_str(v3), "[-5,-8,-8]");
        TRY(v3 = 17 * v1);       TEST_EQUAL(to_str(v3), "[34,51,85]");
        TRY(v3 = v1 * 19);       TEST_EQUAL(to_str(v3), "[38,57,95]");
        TRY(x = v1 * v2);        TEST_EQUAL(x, 112);
        TRY(v3 = v1 % v2);       TEST_EQUAL(to_str(v3), "[-16,9,1]");
        TRY(v3 = emul(v1, v2));  TEST_EQUAL(to_str(v3), "[14,33,65]");
        TRY(v3 = ediv(v2, v1));  TEST_EQUAL(to_str(v3), "[3,3,2]");

        TEST_EQUAL(v1.sum(), 10);
        TEST_EQUAL(v2.sum(), 31);

        TRY(v3 = Int3::unit(0));  TEST_EQUAL(to_str(v3), "[1,0,0]");
        TRY(v3 = Int3::unit(1));  TEST_EQUAL(to_str(v3), "[0,1,0]");
        TRY(v3 = Int3::unit(2));  TEST_EQUAL(to_str(v3), "[0,0,1]");

        Int2 s2;
        Int3 s3;
        Int4 s4;

        TRY((v1 = {1,2,3}));

        TRY(s2 = v1["xy"]);    TEST_EQUAL(to_str(s2), "[1,2]");
        TRY(s2 = v1["yz"]);    TEST_EQUAL(to_str(s2), "[2,3]");
        TRY(s2 = v1["zx"]);    TEST_EQUAL(to_str(s2), "[3,1]");
        TRY(s2 = v1["xx"]);    TEST_EQUAL(to_str(s2), "[1,1]");
        TRY(s2 = v1["yy"]);    TEST_EQUAL(to_str(s2), "[2,2]");
        TRY(s2 = v1["zz"]);    TEST_EQUAL(to_str(s2), "[3,3]");
        TRY(s3 = v1["xyz"]);   TEST_EQUAL(to_str(s3), "[1,2,3]");
        TRY(s3 = v1["zyx"]);   TEST_EQUAL(to_str(s3), "[3,2,1]");
        TRY(s3 = v1["xxx"]);   TEST_EQUAL(to_str(s3), "[1,1,1]");
        TRY(s3 = v1["yyy"]);   TEST_EQUAL(to_str(s3), "[2,2,2]");
        TRY(s3 = v1["zzz"]);   TEST_EQUAL(to_str(s3), "[3,3,3]");
        TRY(s4 = v1["xyzx"]);  TEST_EQUAL(to_str(s4), "[1,2,3,1]");
        TRY(s4 = v1["yzxy"]);  TEST_EQUAL(to_str(s4), "[2,3,1,2]");
        TRY(s4 = v1["zxyz"]);  TEST_EQUAL(to_str(s4), "[3,1,2,3]");
        TRY(s4 = v1["xxxx"]);  TEST_EQUAL(to_str(s4), "[1,1,1,1]");
        TRY(s4 = v1["yyyy"]);  TEST_EQUAL(to_str(s4), "[2,2,2,2]");
        TRY(s4 = v1["zzzz"]);  TEST_EQUAL(to_str(s4), "[3,3,3,3]");

        Uint8_3 u1, u2, u3;

        TRY((u1 = {8,8,8}));
        TRY((u2 = {0,9,15}));

        TRY(u3 = ~ u2);     TEST_EQUAL(to_str(u3), "[255,246,240]");
        TRY(u3 = u1 & u2);  TEST_EQUAL(to_str(u3), "[0,8,8]");
        TRY(u3 = u1 | u2);  TEST_EQUAL(to_str(u3), "[8,9,15]");
        TRY(u3 = u1 ^ u2);  TEST_EQUAL(to_str(u3), "[8,1,7]");

    }

    void check_floating_vector() {

        double x = 0;
        Double3 v1(1), v2, v3, v4;
        const Double3& cv1(v1);
        const Double3& cv2(v2);
        const Double3& cv3(v3);

        TEST_EQUAL(sizeof(Double3), 3 * sizeof(double));

        TEST(! v1.is_null());
        TEST(v2.is_null());

        TEST_EQUAL(to_str(v1), "[1,1,1]");
        TEST_EQUAL(to_str(cv1), "[1,1,1]");

        TEST_EQUAL(v1[0], 1);  TEST_EQUAL(cv1[0], 1);
        TEST_EQUAL(v1[1], 1);  TEST_EQUAL(cv1[1], 1);
        TEST_EQUAL(v1[2], 1);  TEST_EQUAL(cv1[2], 1);

        TRY(v2[0] = 99);
        TRY(v2[1] = 98);
        TRY(v2[2] = 97);
        TEST_EQUAL(to_str(v2), "[99,98,97]");
        TEST_EQUAL(to_str(cv2), "[99,98,97]");

        TEST_EQUAL(v2[0], 99);     TEST_EQUAL(cv2[0], 99);
        TEST_EQUAL(v2[1], 98);     TEST_EQUAL(cv2[1], 98);
        TEST_EQUAL(v2[2], 97);     TEST_EQUAL(cv2[2], 97);
        TEST_EQUAL(v2.at(0), 99);  TEST_EQUAL(cv2.at(0), 99);
        TEST_EQUAL(v2.at(1), 98);  TEST_EQUAL(cv2.at(1), 98);
        TEST_EQUAL(v2.at(2), 97);  TEST_EQUAL(cv2.at(2), 97);
        TEST_EQUAL(v2.x(), 99);    TEST_EQUAL(cv2.x(), 99);
        TEST_EQUAL(v2.y(), 98);    TEST_EQUAL(cv2.y(), 98);
        TEST_EQUAL(v2.z(), 97);    TEST_EQUAL(cv2.z(), 97);

        TEST_THROW(v2.at(3), std::out_of_range);
        TEST_THROW(cv2.at(3), std::out_of_range);

        TRY((v3 = Double3{1,2,3}));
        TEST_EQUAL(to_str(v3), "[1,2,3]");
        TEST_EQUAL(to_str(cv3), "[1,2,3]");

        TRY((v3 = Double3(2,4,6)));
        TEST_EQUAL(to_str(v3), "[2,4,6]");
        TEST_EQUAL(to_str(cv3), "[2,4,6]");

        TRY((v3 = {10,20,30}));
        TEST_EQUAL(to_str(v3), "[10,20,30]");
        TEST_EQUAL(to_str(cv3), "[10,20,30]");

        TEST_EQUAL(v3[0], 10);          TEST_EQUAL(cv3[0], 10);
        TEST_EQUAL(v3[1], 20);          TEST_EQUAL(cv3[1], 20);
        TEST_EQUAL(v3[2], 30);          TEST_EQUAL(cv3[2], 30);
        TEST_EQUAL(v3.begin()[0], 10);  TEST_EQUAL(cv3.begin()[0], 10);
        TEST_EQUAL(v3.begin()[1], 20);  TEST_EQUAL(cv3.begin()[1], 20);
        TEST_EQUAL(v3.begin()[2], 30);  TEST_EQUAL(cv3.begin()[2], 30);

        TEST(v1 == v1);      TEST(! (v1 == v2));  TEST(! (v1 == v3));  TEST(! (v2 == v3));
        TEST(! (v1 != v1));  TEST(v1 != v2);      TEST(v1 != v3);      TEST(v2 != v3);
        TEST(! (v1 < v1));   TEST(v1 < v2);       TEST(v1 < v3);       TEST(! (v2 < v3));
        TEST(! (v1 > v1));   TEST(! (v1 > v2));   TEST(! (v1 > v3));   TEST(v2 > v3);
        TEST(v1 <= v1);      TEST(v1 <= v2);      TEST(v1 <= v3);      TEST(! (v2 <= v3));
        TEST(v1 >= v1);      TEST(! (v1 >= v2));  TEST(! (v1 >= v3));  TEST(v2 >= v3);

        TRY((v1 = {2,3,5}));
        TRY((v2 = {7,11,13}));

        TRY(v3 = + v1);          TEST_EQUAL(to_str(v3), "[2,3,5]");
        TRY(v3 = - v1);          TEST_EQUAL(to_str(v3), "[-2,-3,-5]");
        TRY(v3 = v1 + v2);       TEST_EQUAL(to_str(v3), "[9,14,18]");
        TRY(v3 = v1 - v2);       TEST_EQUAL(to_str(v3), "[-5,-8,-8]");
        TRY(v3 = 17 * v1);       TEST_EQUAL(to_str(v3), "[34,51,85]");
        TRY(v3 = v1 * 19);       TEST_EQUAL(to_str(v3), "[38,57,95]");
        TRY(v3 = v1 / 23);       TEST_EQUAL(to_str(v3), "[0.0869565,0.130435,0.217391]");
        TRY(x = v1 * v2);        TEST_EQUAL(x, 112);
        TRY(v3 = v1 % v2);       TEST_EQUAL(to_str(v3), "[-16,9,1]");
        TRY(v3 = emul(v1, v2));  TEST_EQUAL(to_str(v3), "[14,33,65]");
        TRY(v3 = ediv(v1, v2));  TEST_EQUAL(to_str(v3), "[0.285714,0.272727,0.384615]");

        TEST_EQUAL(v1.r2(), 38);   TEST_NEAR(v1.r(), 6.164414);
        TEST_EQUAL(v2.r2(), 339);  TEST_NEAR(v2.r(), 18.411953);

        TEST_EQUAL(v1.sum(), 10);
        TEST_EQUAL(v2.sum(), 31);

        TRY(v3 = Double3::unit(0));  TEST_EQUAL(to_str(v3), "[1,0,0]");
        TRY(v3 = Double3::unit(1));  TEST_EQUAL(to_str(v3), "[0,1,0]");
        TRY(v3 = Double3::unit(2));  TEST_EQUAL(to_str(v3), "[0,0,1]");

        TRY(v3 = Double3().dir());  TEST_EQUAL(to_str(v3), "[0,0,0]");
        TRY(v3 = v1.dir());         TEST_EQUAL(to_str(v3), "[0.324443,0.486664,0.811107]");
        TRY(v3 = v2.dir());         TEST_EQUAL(to_str(v3), "[0.380188,0.597438,0.706063]");

        TRY((v1 = {1,2,3}));
        TRY((v2 = {}));          TRY(x = v1.angle(v2));  TEST_EQUAL(x, 0);
        TRY((v2 = {2,4,6}));     TRY(x = v1.angle(v2));  TEST_EQUAL(x, 0);
        TRY((v2 = {-2,-4,-6}));  TRY(x = v1.angle(v2));  TEST_NEAR(x, pi_d);
        TRY((v2 = {3,2,1}));     TRY(x = v1.angle(v2));  TEST_NEAR(x, 0.775193);
        TRY((v2 = {-3,-2,-1}));  TRY(x = v1.angle(v2));  TEST_NEAR(x, 2.366399);

        TRY((v1 = {2,3,5}));
        TRY((v2 = {7,11,13}));
        TRY(v3 = v1.project(v2));
        TRY(v4 = v1.reject(v2));
        TEST_NEAR_RANGE(v3 + v4, v1);
        TEST_NEAR_RANGE(v2 % v3, Double3());
        TEST_NEAR(v2 * v4, 0);
        TEST_NEAR(v3 * v4, 0);

    }

    template <typename T, size_t N, MatrixLayout L>
    Matrix<T, N, L> fuzz(Matrix<T, N, L>& m) noexcept {
        T temp[N * N];
        std::copy(m.begin(), m.end(), temp);
        for (auto& x: temp)
            if (std::abs(x) < 1e-6)
                x = 0;
        return Matrix<T, N, L>::from_array(temp);
    }

    template <typename T, size_t N, MatrixLayout L>
    U8string matrix_str(const Matrix<T, N, L>& m) {
        std::ostringstream out;
        out << m;
        return out.str();
    }

    void check_matrix() {

        Double3 v1, v2;
        Double3x3r rm1(2, 1), rm2, rm3;
        Double3x3c cm1(2, 1), cm2, cm3;
        std::vector<double> v;

        TEST_EQUAL(sizeof(Double3x3r), 9 * sizeof(double));
        TEST_EQUAL(sizeof(Double3x3c), 9 * sizeof(double));

        TEST_EQUAL(matrix_str(rm1),
            "[2,1,1;\n"
            " 1,2,1;\n"
            " 1,1,2]\n"
        );
        TEST_EQUAL(matrix_str(cm1),
            "[2,1,1;\n"
            " 1,2,1;\n"
            " 1,1,2]\n"
        );

        TRY((rm1 = Double3x3r{1,2,3,4,5,6,7,8,9}));
        TRY((cm1 = Double3x3c{1,2,3,4,5,6,7,8,9}));
        TEST_EQUAL(to_str(rm1), "[1,2,3,4,5,6,7,8,9]");
        TEST_EQUAL(to_str(cm1), "[1,2,3,4,5,6,7,8,9]");

        TRY((rm1 = {2,3,5,7,11,13,17,19,23}));
        TRY((cm1 = {2,3,5,7,11,13,17,19,23}));
        TEST_EQUAL(to_str(rm1), "[2,3,5,7,11,13,17,19,23]");
        TEST_EQUAL(to_str(cm1), "[2,3,5,7,11,13,17,19,23]");

        TEST_EQUAL(matrix_str(rm1),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );
        TEST_EQUAL(matrix_str(cm1),
            "[2,7,17;\n"
            " 3,11,19;\n"
            " 5,13,23]\n"
        );

        TRY(rm2 = Double3x3r(cm1));
        TEST_EQUAL(matrix_str(rm2),
            "[2,7,17;\n"
            " 3,11,19;\n"
            " 5,13,23]\n"
        );
        TRY(cm2 = Double3x3c(rm1));
        TEST_EQUAL(matrix_str(cm2),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );

        TRY(rm2 = cm1);
        TEST_EQUAL(matrix_str(rm2),
            "[2,7,17;\n"
            " 3,11,19;\n"
            " 5,13,23]\n"
        );
        TRY(cm2 = rm1);
        TEST_EQUAL(matrix_str(cm2),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );

        TEST_EQUAL(rm1(0, 0), 2);
        TEST_EQUAL(rm1(0, 1), 3);
        TEST_EQUAL(rm1(0, 2), 5);
        TEST_EQUAL(rm1(1, 0), 7);
        TEST_EQUAL(rm1(1, 1), 11);
        TEST_EQUAL(rm1(1, 2), 13);
        TEST_EQUAL(rm1(2, 0), 17);
        TEST_EQUAL(rm1(2, 1), 19);
        TEST_EQUAL(rm1(2, 2), 23);
        TEST_EQUAL(cm1(0, 0), 2);
        TEST_EQUAL(cm1(0, 1), 7);
        TEST_EQUAL(cm1(0, 2), 17);
        TEST_EQUAL(cm1(1, 0), 3);
        TEST_EQUAL(cm1(1, 1), 11);
        TEST_EQUAL(cm1(1, 2), 19);
        TEST_EQUAL(cm1(2, 0), 5);
        TEST_EQUAL(cm1(2, 1), 13);
        TEST_EQUAL(cm1(2, 2), 23);

        TEST_EQUAL(rm1.at(0, 0), 2);
        TEST_EQUAL(rm1.at(0, 1), 3);
        TEST_EQUAL(rm1.at(0, 2), 5);
        TEST_EQUAL(rm1.at(1, 0), 7);
        TEST_EQUAL(rm1.at(1, 1), 11);
        TEST_EQUAL(rm1.at(1, 2), 13);
        TEST_EQUAL(rm1.at(2, 0), 17);
        TEST_EQUAL(rm1.at(2, 1), 19);
        TEST_EQUAL(rm1.at(2, 2), 23);
        TEST_EQUAL(cm1.at(0, 0), 2);
        TEST_EQUAL(cm1.at(0, 1), 7);
        TEST_EQUAL(cm1.at(0, 2), 17);
        TEST_EQUAL(cm1.at(1, 0), 3);
        TEST_EQUAL(cm1.at(1, 1), 11);
        TEST_EQUAL(cm1.at(1, 2), 19);
        TEST_EQUAL(cm1.at(2, 0), 5);
        TEST_EQUAL(cm1.at(2, 1), 13);
        TEST_EQUAL(cm1.at(2, 2), 23);

        TEST_THROW(rm1.at(0, 3), std::out_of_range);
        TEST_THROW(rm1.at(3, 0), std::out_of_range);
        TEST_THROW(rm1.at(3, 3), std::out_of_range);
        TEST_THROW(cm1.at(0, 3), std::out_of_range);
        TEST_THROW(cm1.at(3, 0), std::out_of_range);
        TEST_THROW(cm1.at(3, 3), std::out_of_range);

        TRY((rm2 = {29,31,37,41,43,47,53,59,61}));
        TRY(cm1 = rm1);
        TRY(cm2 = rm2);

        TEST_EQUAL(matrix_str(rm1),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );
        TEST_EQUAL(matrix_str(cm1),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );
        TEST_EQUAL(matrix_str(rm2),
            "[29,31,37;\n"
            " 41,43,47;\n"
            " 53,59,61]\n"
        );
        TEST_EQUAL(matrix_str(cm2),
            "[29,31,37;\n"
            " 41,43,47;\n"
            " 53,59,61]\n"
        );

        TEST(rm1 == rm1);
        TEST(rm1 != rm2);
        TEST(rm1 == cm1);
        TEST(rm1 != cm2);
        TEST(cm1 == rm1);
        TEST(cm1 != rm2);
        TEST(cm1 == cm1);
        TEST(cm1 != cm2);

        TRY(rm3 = rm1 * 10);
        TEST_EQUAL(matrix_str(rm3),
            "[20,30,50;\n"
            " 70,110,130;\n"
            " 170,190,230]\n"
        );
        TRY(cm3 = cm1 * 10);
        TEST_EQUAL(matrix_str(cm3),
            "[20,30,50;\n"
            " 70,110,130;\n"
            " 170,190,230]\n"
        );

        TRY(rm3 = 10 * rm1);
        TEST_EQUAL(matrix_str(rm3),
            "[20,30,50;\n"
            " 70,110,130;\n"
            " 170,190,230]\n"
        );
        TRY(cm3 = 10 * cm1);
        TEST_EQUAL(matrix_str(cm3),
            "[20,30,50;\n"
            " 70,110,130;\n"
            " 170,190,230]\n"
        );

        TRY(rm3 = rm1 / 10);
        TEST_EQUAL(matrix_str(rm3),
            "[0.2,0.3,0.5;\n"
            " 0.7,1.1,1.3;\n"
            " 1.7,1.9,2.3]\n"
        );
        TRY(cm3 = cm1 / 10);
        TEST_EQUAL(matrix_str(cm3),
            "[0.2,0.3,0.5;\n"
            " 0.7,1.1,1.3;\n"
            " 1.7,1.9,2.3]\n"
        );

        TRY(rm3 = rm1 + rm2);
        TEST_EQUAL(matrix_str(rm3),
            "[31,34,42;\n"
            " 48,54,60;\n"
            " 70,78,84]\n"
        );
        TRY(cm3 = cm1 + cm2);
        TEST_EQUAL(matrix_str(cm3),
            "[31,34,42;\n"
            " 48,54,60;\n"
            " 70,78,84]\n"
        );
        TRY(rm3 = rm1 + cm2);
        TEST_EQUAL(matrix_str(rm3),
            "[31,34,42;\n"
            " 48,54,60;\n"
            " 70,78,84]\n"
        );
        TRY(cm3 = cm1 + rm2);
        TEST_EQUAL(matrix_str(cm3),
            "[31,34,42;\n"
            " 48,54,60;\n"
            " 70,78,84]\n"
        );

        TRY(rm3 = rm1 - rm2);
        TEST_EQUAL(matrix_str(rm3),
            "[-27,-28,-32;\n"
            " -34,-32,-34;\n"
            " -36,-40,-38]\n"
        );
        TRY(cm3 = cm1 - cm2);
        TEST_EQUAL(matrix_str(cm3),
            "[-27,-28,-32;\n"
            " -34,-32,-34;\n"
            " -36,-40,-38]\n"
        );
        TRY(rm3 = rm1 - cm2);
        TEST_EQUAL(matrix_str(rm3),
            "[-27,-28,-32;\n"
            " -34,-32,-34;\n"
            " -36,-40,-38]\n"
        );
        TRY(cm3 = cm1 - rm2);
        TEST_EQUAL(matrix_str(cm3),
            "[-27,-28,-32;\n"
            " -34,-32,-34;\n"
            " -36,-40,-38]\n"
        );

        TRY(rm3 = rm1 * rm2);
        TEST_EQUAL(matrix_str(rm3),
            "[446,486,520;\n"
            " 1343,1457,1569;\n"
            " 2491,2701,2925]\n"
        );
        TRY(cm3 = cm1 * cm2);
        TEST_EQUAL(matrix_str(cm3),
            "[446,486,520;\n"
            " 1343,1457,1569;\n"
            " 2491,2701,2925]\n"
        );
        TRY(rm3 = rm1 * cm2);
        TEST_EQUAL(matrix_str(rm3),
            "[446,486,520;\n"
            " 1343,1457,1569;\n"
            " 2491,2701,2925]\n"
        );
        TRY(cm3 = cm1 * rm2);
        TEST_EQUAL(matrix_str(cm3),
            "[446,486,520;\n"
            " 1343,1457,1569;\n"
            " 2491,2701,2925]\n"
        );

        TRY((v1 = {67,71,73}));

        TRY(v2 = rm1 * v1);  TEST_EQUAL(to_str(v2), "[712,2199,4167]");
        TRY(v2 = v1 * rm1);  TEST_EQUAL(to_str(v2), "[1872,2369,2937]");
        TRY(v2 = cm1 * v1);  TEST_EQUAL(to_str(v2), "[712,2199,4167]");
        TRY(v2 = v1 * cm1);  TEST_EQUAL(to_str(v2), "[1872,2369,2937]");

        TRY(rm3 = Double3x3r::identity());
        TEST_EQUAL(matrix_str(rm3),
            "[1,0,0;\n"
            " 0,1,0;\n"
            " 0,0,1]\n"
        );
        TRY(cm3 = Double3x3r::identity());
        TEST_EQUAL(matrix_str(cm3),
            "[1,0,0;\n"
            " 0,1,0;\n"
            " 0,0,1]\n"
        );

        TRY(rm3 = rm1);
        TRY(v1 = rm3.column(0));  TEST_EQUAL(to_str(v1), "[2,7,17]");
        TRY(v1 = rm3.column(1));  TEST_EQUAL(to_str(v1), "[3,11,19]");
        TRY(v1 = rm3.column(2));  TEST_EQUAL(to_str(v1), "[5,13,23]");
        TRY(v1 = rm3.row(0));     TEST_EQUAL(to_str(v1), "[2,3,5]");
        TRY(v1 = rm3.row(1));     TEST_EQUAL(to_str(v1), "[7,11,13]");
        TRY(v1 = rm3.row(2));     TEST_EQUAL(to_str(v1), "[17,19,23]");

        TRY(cm3 = cm1);
        TRY(v1 = cm3.column(0));  TEST_EQUAL(to_str(v1), "[2,7,17]");
        TRY(v1 = cm3.column(1));  TEST_EQUAL(to_str(v1), "[3,11,19]");
        TRY(v1 = cm3.column(2));  TEST_EQUAL(to_str(v1), "[5,13,23]");
        TRY(v1 = cm3.row(0));     TEST_EQUAL(to_str(v1), "[2,3,5]");
        TRY(v1 = cm3.row(1));     TEST_EQUAL(to_str(v1), "[7,11,13]");
        TRY(v1 = cm3.row(2));     TEST_EQUAL(to_str(v1), "[17,19,23]");

        TRY(rm3 = rm1.swap_rows(0, 1));
        TEST_EQUAL(matrix_str(rm3),
            "[7,11,13;\n"
            " 2,3,5;\n"
            " 17,19,23]\n"
        );
        TRY(rm3 = rm3.swap_rows(1, 2));
        TEST_EQUAL(matrix_str(rm3),
            "[7,11,13;\n"
            " 17,19,23;\n"
            " 2,3,5]\n"
        );
        TRY(rm3 = rm3.swap_columns(0, 1));
        TEST_EQUAL(matrix_str(rm3),
            "[11,7,13;\n"
            " 19,17,23;\n"
            " 3,2,5]\n"
        );
        TRY(rm3 = rm3.swap_columns(1, 2));
        TEST_EQUAL(matrix_str(rm3),
            "[11,13,7;\n"
            " 19,23,17;\n"
            " 3,5,2]\n"
        );

        TRY(cm3 = rm1.swap_rows(0, 1));
        TEST_EQUAL(matrix_str(cm3),
            "[7,11,13;\n"
            " 2,3,5;\n"
            " 17,19,23]\n"
        );
        TRY(cm3 = cm3.swap_rows(1, 2));
        TEST_EQUAL(matrix_str(cm3),
            "[7,11,13;\n"
            " 17,19,23;\n"
            " 2,3,5]\n"
        );
        TRY(cm3 = cm3.swap_columns(0, 1));
        TEST_EQUAL(matrix_str(cm3),
            "[11,7,13;\n"
            " 19,17,23;\n"
            " 3,2,5]\n"
        );
        TRY(cm3 = cm3.swap_columns(1, 2));
        TEST_EQUAL(matrix_str(cm3),
            "[11,13,7;\n"
            " 19,23,17;\n"
            " 3,5,2]\n"
        );

        TRY(rm3 = rm1.transpose());
        TEST_EQUAL(matrix_str(rm3),
            "[2,7,17;\n"
            " 3,11,19;\n"
            " 5,13,23]\n"
        );
        TRY(rm3 = rm3.transpose());
        TEST_EQUAL(matrix_str(rm3),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );
        TRY(cm3 = cm1.transpose());
        TEST_EQUAL(matrix_str(cm3),
            "[2,7,17;\n"
            " 3,11,19;\n"
            " 5,13,23]\n"
        );
        TRY(cm3 = cm3.transpose());
        TEST_EQUAL(matrix_str(cm3),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );

        v = {1,2,3,4,5,6,7,8,9};
        TRY(rm1 = Double3x3r::from_array(v.data()));
        TEST_EQUAL(matrix_str(rm1),
            "[1,2,3;\n"
            " 4,5,6;\n"
            " 7,8,9]\n"
        );
        TRY(cm1 = Double3x3c::from_array(v.data()));
        TEST_EQUAL(matrix_str(cm1),
            "[1,4,7;\n"
            " 2,5,8;\n"
            " 3,6,9]\n"
        );

        TRY(rm1 = Double3x3r::from_columns(1,2,3,4,5,6,7,8,9));
        TEST_EQUAL(matrix_str(rm1),
            "[1,4,7;\n"
            " 2,5,8;\n"
            " 3,6,9]\n"
        );
        TRY(rm1 = Double3x3r::from_rows(1,2,3,4,5,6,7,8,9));
        TEST_EQUAL(matrix_str(rm1),
            "[1,2,3;\n"
            " 4,5,6;\n"
            " 7,8,9]\n"
        );
        TRY(cm1 = Double3x3r::from_columns(1,2,3,4,5,6,7,8,9));
        TEST_EQUAL(matrix_str(cm1),
            "[1,4,7;\n"
            " 2,5,8;\n"
            " 3,6,9]\n"
        );
        TRY(cm1 = Double3x3r::from_rows(1,2,3,4,5,6,7,8,9));
        TEST_EQUAL(matrix_str(cm1),
            "[1,2,3;\n"
            " 4,5,6;\n"
            " 7,8,9]\n"
        );

    }

    void check_matrix_inversion() {

        double x = 0;

        Double2x2r r2, s2, t2;
        Double3x3r r3, s3, t3;
        Double4x4r r4, s4, t4;
        Double2x2c c2, d2, e2;
        Double3x3c c3, d3, e3;
        Double4x4c c4, d4, e4;

        TRY((r2 = {2,3,5,7}));
        TEST_EQUAL(matrix_str(r2),
            "[2,3;\n"
            " 5,7]\n"
        );
        TRY(c2 = r2);
        TEST_EQUAL(matrix_str(c2),
            "[2,3;\n"
            " 5,7]\n"
        );
        TRY((r3 = {2,3,5,7,11,13,17,19,23}));
        TEST_EQUAL(matrix_str(r3),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );
        TRY(c3 = r3);
        TEST_EQUAL(matrix_str(c3),
            "[2,3,5;\n"
            " 7,11,13;\n"
            " 17,19,23]\n"
        );
        TRY((r4 = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53}));
        TEST_EQUAL(matrix_str(r4),
            "[2,3,5,7;\n"
            " 11,13,17,19;\n"
            " 23,29,31,37;\n"
            " 41,43,47,53]\n"
        );
        TRY(c4 = r4);
        TEST_EQUAL(matrix_str(c4),
            "[2,3,5,7;\n"
            " 11,13,17,19;\n"
            " 23,29,31,37;\n"
            " 41,43,47,53]\n"
        );

        TRY(x = r2.det());  TEST_EQUAL(x, -1);
        TRY(x = c2.det());  TEST_EQUAL(x, -1);
        TRY(x = r3.det());  TEST_EQUAL(x, -78);
        TRY(x = c3.det());  TEST_EQUAL(x, -78);
        TRY(x = r4.det());  TEST_EQUAL(x, 880);
        TRY(x = c4.det());  TEST_EQUAL(x, 880);

        U8string id_str_2 = matrix_str(Double2x2r::identity());
        U8string id_str_3 = matrix_str(Double3x3r::identity());
        U8string id_str_4 = matrix_str(Double4x4r::identity());
        U8string inv_str_2 =
            "[-7,3;\n"
            " 5,-2]\n";
        U8string inv_str_3 =
            "[-0.0769231,-0.333333,0.205128;\n"
            " -0.769231,0.5,-0.115385;\n"
            " 0.692308,-0.166667,-0.0128205]\n";
        U8string inv_str_4 =
            "[0.272727,-0.218182,-0.2,0.181818;\n"
            " -0.454545,-0.0363636,0.3,-0.136364;\n"
            " -0.590909,0.697727,-0.1,-0.102273;\n"
            " 0.681818,-0.420455,0,0.0795455]\n";

        TRY(s2 = r2.inverse());  TEST_EQUAL(matrix_str(s2), inv_str_2);  TRY(t2 = r2 * s2);  TRY(t2 = fuzz(t2));  TEST_EQUAL(matrix_str(t2), id_str_2);
        TRY(d2 = c2.inverse());  TEST_EQUAL(matrix_str(d2), inv_str_2);  TRY(e2 = c2 * d2);  TRY(e2 = fuzz(e2));  TEST_EQUAL(matrix_str(e2), id_str_2);
        TRY(s3 = r3.inverse());  TEST_EQUAL(matrix_str(s3), inv_str_3);  TRY(t3 = r3 * s3);  TRY(t3 = fuzz(t3));  TEST_EQUAL(matrix_str(t3), id_str_3);
        TRY(d3 = c3.inverse());  TEST_EQUAL(matrix_str(d3), inv_str_3);  TRY(e3 = c3 * d3);  TRY(e3 = fuzz(e3));  TEST_EQUAL(matrix_str(e3), id_str_3);
        TRY(s4 = r4.inverse());  TEST_EQUAL(matrix_str(s4), inv_str_4);  TRY(t4 = r4 * s4);  TRY(t4 = fuzz(t4));  TEST_EQUAL(matrix_str(t4), id_str_4);
        TRY(d4 = c4.inverse());  TEST_EQUAL(matrix_str(d4), inv_str_4);  TRY(e4 = c4 * d4);  TRY(e4 = fuzz(e4));  TEST_EQUAL(matrix_str(e4), id_str_4);

    }

    template <typename T>
    void make_approx(Quaternion<T>& q) noexcept {
        using std::floor;
        for (auto& x: q)
            x = x + T(1) - T(1);
    }

    void check_quaternion() {

        Qdouble q, r, s;

        TEST_EQUAL(to_str(q), "[0,0,0,0]");
        TRY(q = 42);
        TEST_EQUAL(to_str(q), "[42,0,0,0]");
        TRY((q = {10,20,30,40}));
        TEST_EQUAL(to_str(q), "[10,20,30,40]");
        TEST_EQUAL(q.a(), 10);
        TEST_EQUAL(q.b(), 20);
        TEST_EQUAL(q.c(), 30);
        TEST_EQUAL(q.d(), 40);
        TEST_EQUAL(q[0], 10);
        TEST_EQUAL(q[1], 20);
        TEST_EQUAL(q[2], 30);
        TEST_EQUAL(q[3], 40);

        TEST_EQUAL(q.s_part(), 10);
        TEST_EQUAL(to_str(q.v_part()), "[20,30,40]");
        TEST_EQUAL(q.norm2(), 3000);
        TEST_NEAR(q.norm(), 54.772256);
        TEST_EQUAL(to_str(q.versor()), "[0.182574,0.365148,0.547723,0.730297]");
        TEST_EQUAL(to_str(q.recip()), "[0.00333333,-0.00666667,-0.01,-0.0133333]");

        TRY(q *= 2);
        TEST_EQUAL(to_str(q), "[20,40,60,80]");
        TRY(q /= 4);
        TEST_EQUAL(to_str(q), "[5,10,15,20]");

        TRY((q = {10,20,30,40}));
        TRY((r = {100,200,300,400}));
        TRY(s = q + r);
        TEST_EQUAL(to_str(s), "[110,220,330,440]");
        TRY(s = q - r);
        TEST_EQUAL(to_str(s), "[-90,-180,-270,-360]");

        TRY((q = {2,3,5,7}));
        TRY((r = {11,13,17,19}));
        TRY(s = q * r);
        TEST_EQUAL(to_str(s), "[-235,35,123,101]");

        TRY((q = {10,20,30,40}));
        TRY(r = q * q.recip());
        TRY(make_approx(r));
        TEST_EQUAL(to_str(r), "[1,0,0,0]");
        TRY(r = q.recip() * q);
        TRY(make_approx(r));
        TEST_EQUAL(to_str(r), "[1,0,0,0]");

    }

}

TEST_MODULE(core, vector) {

    check_integer_vector();
    check_floating_vector();
    check_matrix();
    check_matrix_inversion();
    check_quaternion();

}
