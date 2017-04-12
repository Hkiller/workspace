#include <stdexcept>
#include "NmTest.hpp"

class GroupTest : public NmTest {
};

TEST_F(GroupTest, create_basic) {
    TestGroup * o = new(t_nm(), "object1") TestGroup(*this, 1);
    ASSERT_TRUE(o);

    EXPECT_STREQ("object1", o->name());
    EXPECT_EQ(nm_node_group, o->category());
}

TEST_F(GroupTest, create_duplicate) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    ASSERT_TRUE(g1);

    EXPECT_THROW(
        new(t_nm(), "g1") TestGroup(*this, 1)
        , ::std::runtime_error);
}

class ExceptionCreateGroup : public GroupTest::TestGroup {
public:
    ExceptionCreateGroup(GroupTest & t) : TestGroup(t, 0) {
        throw "exception";
    }
};

TEST_F(GroupTest, create_exception) {
    EXPECT_THROW(
        new(t_nm(), "object1") ExceptionCreateGroup(*this),
        const char *);

    EXPECT_EQ(1, _destoryCount);
}

TEST_F(GroupTest, destory_by_delete) {
    TestGroup * o = new(t_nm(), "object1") TestGroup(*this, 1);
    ASSERT_TRUE(o);

    delete o;

    EXPECT_EQ(1, _destoryCount);
}

TEST_F(GroupTest, destory_by_node) {
    TestGroup * o = new(t_nm(), "object1") TestGroup(*this, 1);
    ASSERT_TRUE(o);

    nm_node_t node = nm_mgr_find_node(t_nm(), o->name_hs());
    ASSERT_TRUE(node);
    nm_node_free(node);

    EXPECT_EQ(1, _destoryCount);
}

TEST_F(GroupTest, addMember_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    ASSERT_TRUE(g1);

    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    ASSERT_TRUE(i1);
    TestObject * i2 = new(t_nm(), "o2") TestObject(*this, 1);
    ASSERT_TRUE(i2);

    g1->addMember(*i1);
    g1->addMember(*i2);

    EXPECT_EQ(2, g1->memberCount());

    Cpe::Nm::ObjectIterator it = g1->members();
    EXPECT_TRUE(i2 == it.next());
    EXPECT_TRUE(i1 == it.next());
    EXPECT_TRUE(NULL == it.next());
}

TEST_F(GroupTest, groups_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    ASSERT_TRUE(g1);

    TestGroup * g2 = new(t_nm(), "g2") TestGroup(*this, 1);
    ASSERT_TRUE(g2);

    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    ASSERT_TRUE(i1);

    g1->addMember(*i1);
    g2->addMember(*i1);

    Cpe::Nm::ObjectIterator it = i1->groups();
    EXPECT_TRUE(g2 == it.next());
    EXPECT_TRUE(g1 == it.next());
    EXPECT_TRUE(NULL == it.next());
}

TEST_F(GroupTest, const_groups_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    ASSERT_TRUE(g1);

    TestGroup * g2 = new(t_nm(), "g2") TestGroup(*this, 1);
    ASSERT_TRUE(g2);

    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    ASSERT_TRUE(i1);

    g1->addMember(*i1);
    g2->addMember(*i1);

    Cpe::Nm::ConstObjectIterator it = const_cast<TestObject const *>(i1)->groups();
    EXPECT_TRUE(g2 == it.next());
    EXPECT_TRUE(g1 == it.next());
    EXPECT_TRUE(NULL == it.next());
}

TEST_F(GroupTest, destoryMembers_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    g1->addMember(*i1);

    g1->destoryMembers();

    ASSERT_TRUE(NULL == g1->findMemberNc("o1"));
}

TEST_F(GroupTest, findMember_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    g1->addMember(*i1);

    CPE_HS_DEF_VAR(name1, "o1");

    ASSERT_TRUE(i1 == g1->findMember(name1));
}

TEST_F(GroupTest, findMember_not_exist) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);

    CPE_HS_DEF_VAR(name1, "not-exist");

    ASSERT_TRUE(NULL == g1->findMember(name1));
}

TEST_F(GroupTest, member_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    g1->addMember(*i1);

    CPE_HS_DEF_VAR(name1, "o1");

    ASSERT_TRUE(i1 == &g1->member(name1));
}

TEST_F(GroupTest, member_not_exist) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);

    CPE_HS_DEF_VAR(name1, "not-exist");

    ASSERT_THROW(g1->member(name1), ::std::exception);
}

TEST_F(GroupTest, findMemberNc_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    g1->addMember(*i1);

    ASSERT_TRUE(i1 == g1->findMemberNc("o1"));
}

TEST_F(GroupTest, findMemberNc_not_exist) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);

    ASSERT_TRUE(NULL == g1->findMemberNc("not-exist"));
}

TEST_F(GroupTest, memberNc_basic) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);
    TestObject * i1 = new(t_nm(), "o1") TestObject(*this, 1);
    g1->addMember(*i1);

    ASSERT_TRUE(i1 == &g1->memberNc("o1"));
}

TEST_F(GroupTest, memberNc_not_exist) {
    TestGroup * g1 = new(t_nm(), "g1") TestGroup(*this, 1);

    ASSERT_THROW(g1->memberNc("not-exist"), ::std::exception);
}

