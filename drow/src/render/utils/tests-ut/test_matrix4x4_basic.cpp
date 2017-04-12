#include "cpe/utils/math_ex.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "render/utils/tests-env/with_render_utils.hpp"

typedef LOKI_TYPELIST_1(
    render::utils::testenv::with_render_utils) Matrix4x4TestBase;

class Matrix4x4Test : public testenv::fixture<Matrix4x4TestBase> {};

TEST_F(Matrix4x4Test, invers_basic) {
    ui_matrix_4x4 a =
        UI_MATRIX_4X4_INITLIZER(
            1, 0, 0, 0,
            1, 2, 0, 0,
            2, 1, 3, 0,
            1, 2, 1, 4);

    ui_matrix_4x4 a_invers;
    ui_matrix_4x4_invers(&a_invers, &a);

    ui_matrix_4x4 r;
    ui_matrix_4x4_cross_product(&r, &a, &a_invers);

    EXPECT_TRUE(ui_matrix_4x4_cmp(&r, &UI_MATRIX_4X4_IDENTITY) == 0);
}
