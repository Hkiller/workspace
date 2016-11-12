#include "cpe/utils/hash_string.h"
#include "NmTest.hpp"

TEST_F(NmTest, instance_basic) {
    nm_node_t instance = t_nm_add_instance("abc", 128);
    EXPECT_TRUE(instance);

    EXPECT_STREQ("abc", nm_node_name(instance));
    EXPECT_EQ(nm_node_instance, nm_node_category(instance));
    EXPECT_EQ((size_t)128, nm_node_capacity(instance));

    EXPECT_TRUE(instance == t_nm_find("abc"));
}

TEST_F(NmTest, instance_free) {
    nm_node_t instance = t_nm_add_instance("abc", 128);
    EXPECT_TRUE(instance);

    nm_node_free(instance);

    EXPECT_TRUE(NULL == t_nm_find("abc"));
}

TEST_F(NmTest, instance_free_with_troups) {
    t_nm_add_group("g1", 128);
    t_nm_add_group("g2", 128);
    t_nm_add_instance("i1", 128);

    EXPECT_EQ(0, t_nm_bind("g1", "i1"));
    EXPECT_EQ(0, t_nm_bind("g2", "i1"));

    EXPECT_STREQ("i1:", t_nm_group_members("g1").c_str());
    EXPECT_STREQ("i1:", t_nm_group_members("g2").c_str());

    nm_node_free(t_nm_find("i1"));

    EXPECT_STREQ("", t_nm_group_members("g1").c_str());
    EXPECT_STREQ("", t_nm_group_members("g2").c_str());
}

TEST_F(NmTest, instance_it_multi_group) {
    t_nm_add_group("g1", 128);
    t_nm_add_group("g2", 128);
    t_nm_add_instance("i1", 128);

    EXPECT_EQ(0, t_nm_bind("g1", "i1"));
    EXPECT_EQ(0, t_nm_bind("g2", "i1"));

    EXPECT_STREQ("g2:g1:", t_nm_node_groups("i1").c_str());
}

TEST_F(NmTest, instance_it_empty) {
    t_nm_add_instance("i1", 128);

    EXPECT_STREQ("", t_nm_node_groups("i1").c_str());
}

TEST_F(NmTest, instance_data_to_node) {
    nm_node_t instance = t_nm_add_instance("abc", 128);
    EXPECT_TRUE(instance);

    EXPECT_TRUE(instance == nm_node_from_data(nm_node_data(instance)));
}
