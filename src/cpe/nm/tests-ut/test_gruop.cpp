#include "cpe/utils/hash_string.h"
#include "NmTest.hpp"

TEST_F(NmTest, group_basic) {
    nm_node_t group = t_nm_add_group("abc", 128);
    EXPECT_TRUE(group);

    EXPECT_STREQ("abc", nm_node_name(group));
    EXPECT_EQ(nm_node_group, nm_node_category(group));
    EXPECT_EQ((size_t)128, nm_node_capacity(group));

    EXPECT_TRUE(group == t_nm_find("abc"));
}

TEST_F(NmTest, group_free) {
    nm_node_t group = t_nm_add_group("abc", 128);
    EXPECT_TRUE(group);

    nm_node_free(group);

    EXPECT_TRUE(NULL == t_nm_find("abc"));
}

TEST_F(NmTest, group_free_with_members) {
    nm_node_t g1 = t_nm_add_group("g1", 128);
    t_nm_add_group("g2", 128);
    nm_node_t i1 = t_nm_add_instance("i1", 128);

    EXPECT_EQ(0, t_nm_bind("g1", "i1"));

    nm_node_free(g1);

    EXPECT_TRUE(NULL == t_nm_find("g1"));
    EXPECT_TRUE(i1 == t_nm_find("i1"));

    EXPECT_STREQ("", t_nm_node_groups("i1").c_str());
}

TEST_F(NmTest, group_it_multi_group) {
    t_nm_add_group("g1", 128);
    t_nm_add_group("g2", 128);
    t_nm_add_group("i1", 128);

    EXPECT_EQ(0, t_nm_bind("g1", "i1"));
    EXPECT_EQ(0, t_nm_bind("g2", "i1"));

    EXPECT_STREQ("g2:g1:", t_nm_node_groups("i1").c_str());
    EXPECT_STREQ("i1:", t_nm_group_members("g1").c_str());
    EXPECT_STREQ("i1:", t_nm_group_members("g2").c_str());
}

TEST_F(NmTest, group_it_empty) {
    t_nm_add_group("i1", 128);

    EXPECT_STREQ("", t_nm_node_groups("i1").c_str());
}

TEST_F(NmTest, group_bind_to_instance) {
    t_nm_add_instance("i1", 128);
    t_nm_add_instance("i2", 128);

    EXPECT_EQ(-1, t_nm_bind("g1", "i1"));
}

TEST_F(NmTest, group_bind_duplicate) {
    t_nm_add_group("g1", 128);
    t_nm_add_instance("i1", 128);

    EXPECT_EQ(0, t_nm_bind("g1", "i1"));
    EXPECT_EQ(-1, t_nm_bind("g1", "i1"));

    EXPECT_STREQ("g1:", t_nm_node_groups("i1").c_str());
}

TEST_F(NmTest, group_find_member_exist) {
    t_nm_add_group("g1", 128);
    nm_node_t i1 = t_nm_add_instance("i1", 128);

    EXPECT_EQ(0, t_nm_bind("g1", "i1"));
    
    EXPECT_TRUE(i1 == t_nm_group_find("g1", "i1"));
}

TEST_F(NmTest, group_find_member_not_exist) {
    t_nm_add_group("g1", 128);

    EXPECT_TRUE(NULL == t_nm_group_find("g1", "not-exist"));
}

TEST_F(NmTest, group_find_member_not_group) {
    t_nm_add_instance("i1", 128);

    EXPECT_TRUE(NULL == t_nm_group_find("i1", "not-exist"));
}

TEST_F(NmTest, group_member_count_empty) {
    nm_node_t g1 = t_nm_add_group("g1", 128);
    EXPECT_EQ(0, nm_group_member_count(g1));
}

TEST_F(NmTest, group_member_count_not_group) {
    nm_node_t i1 = t_nm_add_instance("i1", 128);
    EXPECT_EQ(-1, nm_group_member_count(i1));
}

TEST_F(NmTest, group_data_to_node) {
    nm_node_t g1 = t_nm_add_group("g1", 128);
    EXPECT_TRUE(g1);

    EXPECT_TRUE(g1 == nm_node_from_data(nm_node_data(g1)));
}
