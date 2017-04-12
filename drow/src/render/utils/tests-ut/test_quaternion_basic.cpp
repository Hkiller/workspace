#include "cpe/utils/math_ex.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "render/utils/tests-env/with_render_utils.hpp"

typedef LOKI_TYPELIST_1(
    render::utils::testenv::with_render_utils) QuaternionTestBase;

class QuaternionTest : public testenv::fixture<QuaternionTestBase> {};

TEST_F(QuaternionTest, set_radians) {
    ui_vector_3 ia3 = UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 1.5f);
    ui_quaternion r1;
    ui_quaternion_set_radians(&r1, &ia3);

    ui_vector_3 ra3;
    ui_quaternion_get_radians(&r1, &ra3);
    ui_quaternion r2;
    ui_quaternion_set_radians(&r2, &ra3);

    EXPECT_TRUE(adj_eq(&r1, &r2))
        << "quaternion-l:\n" << dump(&r1)
        << "\nquaternion-r:\n" << dump(&r2);
}

TEST_F(QuaternionTest, set_radians_z) {
    ui_quaternion p1;
    ui_quaternion_set_z_radians(&p1, 1.5f);

    ui_vector_3 a3 = UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 1.5f);
    ui_quaternion p2;
    ui_quaternion_set_radians(&p2, &a3);
    
    EXPECT_TRUE(adj_eq(&p1, &p2))
        << "quaternion-l:\n" << dump(&p1)
        << "\nquaternion-r:\n" << dump(&p2);
}

TEST_F(QuaternionTest, get_radians_z_on_xy) {
    ui_quaternion p1;
    ui_quaternion_set_z_radians(&p1, 1.5f);

    ui_vector_3 r;
    ui_quaternion_adj_vector_3(&p1, &r, &UI_VECTOR_3_POSITIVE_UNIT_X);

    EXPECT_FLOAT_EQ(1.5f, cpe_math_radians(0.0f, 0.0f, r.x, r.y));
}

TEST_F(QuaternionTest, adj_vector_3) {
    ui_quaternion q;
    ui_vector_3 input = UI_VECTOR_3_INITLIZER(4.0, 5.0, 6.0);
    
    ui_quaternion_set_z_radians(&q, 2.4);

    ui_vector_3 adj_q;
    ui_quaternion_adj_vector_3(&q, &adj_q, &input);

    ui_matrix_4x4 rm;
    ui_quaternion_to_rotation_matrix_4x4(&q, &rm);

    ui_vector_3 adj_rm;
    ui_matrix_4x4_adj_vector_3(&rm, &adj_rm, &input);

    EXPECT_EQ(0, ui_vector_3_cmp(&adj_q, &adj_rm));
}

TEST_F(QuaternionTest, flip_x) {
    ui_quaternion p1;
    ui_quaternion_set_z_radians(&p1, - 1.5f);

    ui_quaternion p2;
    ui_quaternion_set_z_radians(&p1, 1.5f);
    p2.y *= -1.0f;
    p2.z *= -1.0f;
    
    EXPECT_TRUE(adj_eq(&p1, &p2))
        << "quaternion-l:\n" << dump(&p1)
        << "\nquaternion-r:\n" << dump(&p2);
}
