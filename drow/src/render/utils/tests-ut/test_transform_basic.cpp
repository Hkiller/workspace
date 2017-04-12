#include "cpe/utils/tests-env/test-fixture.hpp"
#include "render/utils/tests-env/with_render_utils.hpp"

typedef LOKI_TYPELIST_1(
    render::utils::testenv::with_render_utils) TransformBasicTestBase;

class TransformBasicTest : public testenv::fixture<TransformBasicTestBase> {};

TEST_F(TransformBasicTest, identity) {
    ui_transform p = UI_TRANSFORM_IDENTITY;
    ui_transform c = UI_TRANSFORM_IDENTITY;

    ui_transform_adj_by_parent(&c, &p);

    EXPECT_EQ(0, ui_transform_cmp(&c, &UI_TRANSFORM_IDENTITY));
}
