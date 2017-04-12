#include "cpe/utils/math_ex.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "render/utils/tests-env/with_render_utils.hpp"

typedef LOKI_TYPELIST_1(
    render::utils::testenv::with_render_utils) TransformUseTestBase;

class TransformUseTest : public testenv::fixture<TransformUseTestBase> {
public:
    TransformUseTest();

    bool check_adj(ui_vector_3_t check, ui_vector_3_t input);

    ui_transform m_t;
};

TEST_F(TransformUseTest, adj_parent_no_r) {
    ui_transform t = UI_TRANSFORM_IDENTITY;
    ui_vector_3 t_t = UI_VECTOR_3_INITLIZER(3.0, 4.0, 5.0);
    ui_vector_3 t_s = UI_VECTOR_3_INITLIZER(0.1, 0.2, 0.3);
    ui_quaternion t_q = UI_QUATERNION_IDENTITY;
    
    ui_transform_set_pos_3(&t, &t_t);
    ui_quaternion_set_z_radians(&t_q, 3.4f);
    ui_transform_set_quation_scale(&t, &t_q, &t_s);

    ui_vector_3 input = UI_VECTOR_3_INITLIZER(4.0, 5.0, 6.0);

    ui_vector_3 adj_t;
    ui_transform_adj_vector_3(&t, &adj_t, &input);

    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(&t);

    ui_vector_3 adj_m;
    ui_matrix_4x4_adj_vector_3(m, &adj_m, &input);

    EXPECT_EQ(0, ui_vector_3_cmp(&adj_t, &adj_m));
}

TEST_F(TransformUseTest, identity) {
    ui_vector_3 i = UI_VECTOR_3_INITLIZER(1.0f, 0.0f, 0.0f);
    ui_vector_3 expect = UI_VECTOR_3_INITLIZER(1.0f, 0.0f, 0.0f);
    EXPECT_TRUE(check_adj(&expect, &i));
}

TEST_F(TransformUseTest, only_trans) {
    ui_vector_3 pos = UI_VECTOR_3_INITLIZER(1.0f, 2.0f, 3.0f);
    ui_transform_set_pos_3(&m_t, &pos);
    
    ui_vector_3 i = UI_VECTOR_3_INITLIZER(1.0f, 2.0f, 3.0f);
    ui_vector_3 expect = UI_VECTOR_3_INITLIZER(2.0f, 4.0f, 6.0f);

    EXPECT_TRUE(check_adj(&expect, &i));
}

TEST_F(TransformUseTest, parent_flip_child_rotate) {
    ui_vector_3 i = UI_VECTOR_3_INITLIZER(1.0f, 1.0f, 0.0f);
    
    ui_quaternion q;
    ui_quaternion_set_z_radians(&q, 1.2f);
    ui_transform_set_quation(&m_t, &q);
        
    ui_transform p = UI_TRANSFORM_IDENTITY;
    ui_vector_3 f = UI_VECTOR_3_INITLIZER(1.0f, -1.0f, 1.0f);
    ui_transform_set_scale(&p, &f);
    ui_transform_adj_by_parent(&m_t, &p);

    ui_vector_3 expect = UI_VECTOR_3_INITLIZER(-0.569681, -1.294397, 0.000000);
    EXPECT_TRUE(check_adj(&expect, &i));
}

TEST_F(TransformUseTest, flip_x) {
    ui_vector_3 i = UI_VECTOR_3_INITLIZER(1.0f, 1.0f, 1.0f);
    ui_vector_3 expect = UI_VECTOR_3_INITLIZER(-1.0f, 1.0f, 1.0f);
    ui_vector_3 f = UI_VECTOR_3_INITLIZER(-1.0f, 1.0f, 1.0f);
    ui_transform_set_scale(&m_t, &f);

    EXPECT_TRUE(check_adj(&expect, &i));
}

TEST_F(TransformUseTest, child_flip_rotate) {
    ui_vector_3 i = UI_VECTOR_3_INITLIZER(1.0f, 1.0f, 0.0f);
    
    ui_quaternion q;
    ui_quaternion_set_z_radians(&q, 1.2f);

    ui_vector_3 s = UI_VECTOR_3_INITLIZER(1.0f, -1.0f, 1.0f);
    ui_transform_set_quation_scale(&m_t, &q, &s);

    ui_vector_3 expect = UI_VECTOR_3_INITLIZER(1.294397, 0.569681, 0.000000);
    EXPECT_TRUE(check_adj(&expect, &i));
}

TransformUseTest::TransformUseTest() {
    m_t = UI_TRANSFORM_IDENTITY;
}

bool TransformUseTest::check_adj(ui_vector_3_t check, ui_vector_3_t input) {
    ui_vector_3 r;

    ui_transform_adj_vector_3(&m_t, &r, input);
    
    if (ui_vector_3_cmp(check, &r) != 0) {
        printf("expect: %s\n", dump(check));
        printf("result: %s\n", dump(&r));
        printf("input: %s\n", dump(input));

        return false;
    }
    else {
        return true;
    }
}
