#include "gd/dr_dm/dr_dm_data.h"
#include "ManageTest.hpp"

class ManageBasicTest : public ManageTest {
public:
#pragma pack(push,1)
    struct S {
        int64_t id;
        int32_t attr1;
        char attr2[4];
    };
#pragma pack(pop)

    virtual void SetUp() {
        ManageTest::SetUp();

        setMetaLib(
            "<metalib tagsetversion='1' name='LobbyServer' version='1' >\n"
            "    <struct name='S' version='1'>\n"
            "        <entry name='id' type='uint64'/>\n"
            "        <entry name='attr1' type='int32'/>\n"
            "        <entry name='attr2' type='string' size='4'/>\n"
            "    </struct>\n"
            "</metalib>\n"
            );

        setMeta("S");

        EXPECT_EQ(0, dr_dm_manage_set_id_attr(m_manage, "id"));
    }
};

TEST_F(ManageBasicTest, create_basic) {
    S input = { 0, 3, "ab"};

    const char * duplicate_index;
    dr_dm_data_t data = dr_dm_data_create(m_manage, &input, sizeof(input), &duplicate_index);
    EXPECT_TRUE(duplicate_index == NULL);

    S * output = (S*) dr_dm_data_data(data);

    EXPECT_EQ(0, (int)output->id);
    EXPECT_EQ(3, (int)output->attr1);
    EXPECT_STREQ("ab", output->attr2);
}

TEST_F(ManageBasicTest, create_multi_node) {
    S input1 = { 1, 3, "ab"};
    EXPECT_TRUE(dr_dm_data_create(m_manage, &input1, sizeof(input1), NULL));

    S input2 = { 2, 4, "cd"};
    EXPECT_TRUE(dr_dm_data_create(m_manage, &input2, sizeof(input2), NULL));
}

TEST_F(ManageBasicTest, create_id_duplicate) {
    S input1 = { 1, 3, "ab"};
    dr_dm_data_create(m_manage, &input1, sizeof(input1), NULL);

    S input2 = { 1, 4, "cd"};
    const char * duplicate_index;
    dr_dm_data_t data = dr_dm_data_create(m_manage, &input2, sizeof(input2), &duplicate_index);
    EXPECT_TRUE(data == NULL);
    EXPECT_STREQ("id", duplicate_index);
}

TEST_F(ManageBasicTest, create_with_index) {
    t_em_set_print();
    EXPECT_EQ(0, dr_dm_manage_create_index(m_manage, "attr1", 1));

    S input1 = { 1, 3, "ab"};
    EXPECT_TRUE(dr_dm_data_create(m_manage, &input1, sizeof(input1), NULL));

    S input2 = { 2, 4, "cd"};
    EXPECT_TRUE(dr_dm_data_create(m_manage, &input2, sizeof(input2), NULL));
}

TEST_F(ManageBasicTest, find_by_id_basic) {
    S input = { 0, 3, "ab"};

    dr_dm_data_create(m_manage, &input, sizeof(input), NULL);

    dr_dm_data_t data = dr_dm_data_find_by_id(m_manage, 0);
    ASSERT_TRUE(data);

    S * output = (S*) dr_dm_data_data(data);
    EXPECT_EQ(0, (int)output->id);
}

TEST_F(ManageBasicTest, find_by_index_basic) {
    t_em_set_print();
    EXPECT_EQ(0, dr_dm_manage_create_index(m_manage, "attr1", 1));

    S input1 = { 1, 3, "ab"};
    EXPECT_TRUE(dr_dm_data_create(m_manage, &input1, sizeof(input1), NULL));

    S input2 = { 2, 4, "cd"};
    EXPECT_TRUE(dr_dm_data_create(m_manage, &input2, sizeof(input2), NULL));

    dr_dm_data_t data = dr_dm_data_find_by_index_int32(m_manage, "attr1", 3);
    ASSERT_TRUE(data);
    S * output = (S*) dr_dm_data_data(data);
    EXPECT_EQ(1, (int)output->id);
}

