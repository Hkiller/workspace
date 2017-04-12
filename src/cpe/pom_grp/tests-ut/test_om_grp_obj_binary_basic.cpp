#include "cpe/pom_grp/pom_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjBinaryTest : public OmGrpObjMgrTest {
public:
    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  attributes:\n"
            "    - entry1: { entry-type: binary, capacity: 5 }\n"
            ,
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "</metalib>"
            ) ;

        m_obj = pom_grp_obj_alloc(m_mgr);
        ASSERT_TRUE(m_obj);
    }

    virtual void TearDown() {
        pom_grp_obj_free(m_mgr, m_obj);
        OmGrpObjMgrTest::TearDown();
    }

    pom_grp_obj_t m_obj;
};

TEST_F(OmGrpObjBinaryTest, empty) {
    EXPECT_TRUE(pom_grp_obj_binary(m_mgr, m_obj, "entry1") == NULL);
}

TEST_F(OmGrpObjBinaryTest, capacity) {
    EXPECT_EQ(5, pom_grp_obj_binary_capacity(m_mgr, m_obj, "entry1"));
}

TEST_F(OmGrpObjBinaryTest, check_or_create) {
    EXPECT_TRUE(pom_grp_obj_binary_check_or_create(m_mgr, m_obj, "entry1") != NULL);
}
