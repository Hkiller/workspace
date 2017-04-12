#include "2DTest.hpp"

TEST_F(P2DTest, single_in_screen) {
    EXPECT_EQ(
        -120.0,
        t_s_entity_calc_double("angle-flip-x(-60)"));
}
