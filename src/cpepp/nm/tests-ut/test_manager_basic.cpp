#include "NmTest.hpp"

class ManagerTest : public NmTest {
};

TEST_F(ManagerTest, objects_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    TestObject * o2 = new(t_nm(), "object2") TestObject(*this, 1);
    ASSERT_TRUE(o2);

    Cpe::Nm::ObjectIterator objs = mgr().objects();
    ASSERT_TRUE(o1 == objs.next());
    ASSERT_TRUE(o2 == objs.next());
    ASSERT_TRUE(NULL == objs.next());
}

TEST_F(ManagerTest, objects_empty) {
    Cpe::Nm::ObjectIterator objs = mgr().objects();
    ASSERT_TRUE(NULL == objs.next());
}

TEST_F(ManagerTest, findObject_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    CPE_HS_DEF_VAR(name1, "object1");
    ASSERT_TRUE(o1 == mgr().findObject(name1));
}

TEST_F(ManagerTest, findObject_not_exist) {
    CPE_HS_DEF_VAR(name1, "not-exist-object");
    ASSERT_TRUE(NULL == mgr().findObject(name1));
}

TEST_F(ManagerTest, object_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    CPE_HS_DEF_VAR(name1, "object1");
    ASSERT_TRUE(o1 == &mgr().object(name1));
}

TEST_F(ManagerTest, object_not_exist) {
    CPE_HS_DEF_VAR(name1, "not-exist-object");
    ASSERT_THROW(
        mgr().object(name1),
        std::exception);
}

TEST_F(ManagerTest, findObject_const_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    CPE_HS_DEF_VAR(name1, "object1");
    ASSERT_TRUE(o1 == mgr_const().findObject(name1));
}

TEST_F(ManagerTest, findObject_const_not_exist) {
    CPE_HS_DEF_VAR(name1, "not-exist-object");
    ASSERT_TRUE(NULL == mgr_const().findObject(name1));
}

TEST_F(ManagerTest, object_const_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    CPE_HS_DEF_VAR(name1, "object1");
    ASSERT_TRUE(o1 == &mgr_const().object(name1));
}

TEST_F(ManagerTest, object_const_not_exist) {
    CPE_HS_DEF_VAR(name1, "not-exist-object");
    ASSERT_THROW(mgr_const().object(name1), ::std::exception);
}


TEST_F(ManagerTest, findObjectNc_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    ASSERT_TRUE(o1 == mgr().findObjectNc("object1"));
}

TEST_F(ManagerTest, findObjectNc_not_exist) {
    ASSERT_TRUE(NULL == mgr().findObjectNc("not-exist-object"));
}

TEST_F(ManagerTest, objectNc_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    ASSERT_TRUE(o1 == &mgr().objectNc("object1"));
}

TEST_F(ManagerTest, objectNc_not_exist) {
    ASSERT_THROW(
        mgr().objectNc("not-exist-object"),
        std::exception);
}

TEST_F(ManagerTest, findObjectNc_const_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    ASSERT_TRUE(o1 == mgr_const().findObjectNc("object1"));
}

TEST_F(ManagerTest, findObjectNc_const_not_exist) {
    ASSERT_TRUE(NULL == mgr_const().findObjectNc("not-exist-object"));
}

TEST_F(ManagerTest, objectNc_const_basic) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    ASSERT_TRUE(o1 == &mgr_const().objectNc("object1"));
}

TEST_F(ManagerTest, objectNc_const_not_exist) {
    ASSERT_THROW(mgr_const().objectNc("not-exist-object"), ::std::exception);
}

TEST_F(ManagerTest, removeObject_basic) {
    new(t_nm(), "object1") TestObject(*this, 1);

    CPE_HS_DEF_VAR(name1, "object1");

    EXPECT_EQ(
        true,
        mgr().removeObject(name1));
}

TEST_F(ManagerTest, removeObject_not_exist) {
    CPE_HS_DEF_VAR(name1, "not-exist-object");

    EXPECT_EQ(
        false,
        mgr().removeObject(name1));
}
