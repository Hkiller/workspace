#include "cpe/utils/hash_string.h"
#include "NmTest.hpp"

TEST_F(NmTest, manage_add_group) {
    nm_node_t group = t_nm_add_group("g1", 128);
    EXPECT_TRUE(group);

    EXPECT_STREQ("g1:", t_nm_nodes().c_str());
}

TEST_F(NmTest, manage_add_group_duplicate) {
    nm_node_t group = t_nm_add_group("g1", 128);
    EXPECT_TRUE(group);

    EXPECT_FALSE(t_nm_add_group("g1", 128));

    EXPECT_STREQ("g1:", t_nm_nodes().c_str());
}
