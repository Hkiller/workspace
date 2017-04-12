#include <stdexcept>
#include "NmTest.hpp"

class ObjectTest : public NmTest {
};

TEST_F(ObjectTest, create_basic) {
    TestObject * o = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o);

    EXPECT_STREQ("object1", o->name());
    EXPECT_EQ(nm_node_instance, o->category());
}

TEST_F(ObjectTest, create_duplicate) {
    TestObject * o1 = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o1);

    EXPECT_THROW(
        new(t_nm(), "object1") TestObject(*this, 1)
        , ::std::runtime_error);
}

class ExceptionCreateObject : public ObjectTest::TestObject {
public:
    ExceptionCreateObject(ObjectTest & t) : TestObject(t, 0) {
        throw "exception";
    }
};

TEST_F(ObjectTest, create_exception) {
    EXPECT_THROW(
        new(t_nm(), "object1") ExceptionCreateObject(*this),
        const char *);

    EXPECT_EQ(1, _destoryCount);
}

TEST_F(ObjectTest, destory_by_delete) {
    TestObject * o = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o);

    delete o;

    EXPECT_EQ(1, _destoryCount);
}

TEST_F(ObjectTest, destory_by_node) {
    TestObject * o = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o);

    nm_node_t node = nm_mgr_find_node(t_nm(), o->name_hs());
    ASSERT_TRUE(node);
    nm_node_free(node);

    EXPECT_EQ(1, _destoryCount);
}

TEST_F(ObjectTest, groups_empty) {
    TestObject * o = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o);

    Cpe::Nm::ObjectIterator it = o->groups();
    EXPECT_TRUE(NULL == it.next());
}

TEST_F(ObjectTest, const_groups_empty) {
    TestObject const * o = new(t_nm(), "object1") TestObject(*this, 1);
    ASSERT_TRUE(o);

    Cpe::Nm::ConstObjectIterator it = o->groups();
    EXPECT_TRUE(NULL == it.next());
}

