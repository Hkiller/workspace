#include "cpe/pom_grp/pom_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjBitarryyTest : public OmGrpObjMgrTest {
public:
    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  attributes:\n"
            "    - entry1: { entry-type: ba, bit-capacity: 30, byte-per-page=2 }\n"
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

    uint16_t ba_bit(void) {
        return pom_grp_obj_ba_bit_count(m_mgr, m_obj, "entry1");
    }

    void ba_set_all(cpe_ba_value_t value) {
        EXPECT_EQ(0, pom_grp_obj_ba_set_all(m_mgr, m_obj, "entry1", value));
    }

    void ba_set(uint16_t pos, cpe_ba_value_t value) {
        EXPECT_EQ(0, pom_grp_obj_ba_set(m_mgr, m_obj, "entry1", pos, value));
    }

    cpe_ba_value_t ba_get(uint16_t pos) {
        return pom_grp_obj_ba_get(m_mgr, m_obj, "entry1", pos);
    }

    pom_grp_obj_t m_obj;
};

TEST_F(OmGrpObjBitarryyTest, basic) {
    EXPECT_EQ(30, pom_grp_obj_ba_bit_capacity(m_mgr, m_obj, "entry1"));
    EXPECT_EQ(4, pom_grp_obj_ba_byte_capacity(m_mgr, m_obj, "entry1"));
}

TEST_F(OmGrpObjBitarryyTest, get_empty) {
    EXPECT_EQ(cpe_ba_false, ba_get(12));
}

TEST_F(OmGrpObjBitarryyTest, get_overflow) {
    EXPECT_EQ(cpe_ba_false, ba_get(120));
}

TEST_F(OmGrpObjBitarryyTest, set_first) {
    ba_set(0, cpe_ba_true);

    EXPECT_EQ(cpe_ba_true, ba_get(0));
}

