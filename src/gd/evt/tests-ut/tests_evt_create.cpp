#include "EvtTest.hpp"

class EvtCreateTest : public EvtTest {
    virtual void SetUp() {
        EvtTest::SetUp();
        
        t_evt_mgr_set_metalib(
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "    <struct name='Evt1' version='1'>"
            "	     <entry name='a1' type='int16'/>"
            "	     <entry name='a2' type='int16'/>"
            "    </struct>"
            "</metalib>");
    }
};

TEST_F(EvtCreateTest, create_basic) {
    gd_evt_t evt = createEvt("Evt1", 12, 25);
    ASSERT_TRUE(evt);

    EXPECT_EQ((size_t)25, gd_evt_data_capacity(evt));
    EXPECT_EQ((size_t)12, gd_evt_carry_data_capacity(evt));
}

TEST_F(EvtCreateTest, create_data_capacity_auto_calc) {
    gd_evt_t evt = createEvt("Evt1", 12, -1);
    ASSERT_TRUE(evt);

    EXPECT_EQ((size_t)4, gd_evt_data_capacity(evt));
    EXPECT_EQ((size_t)12, gd_evt_carry_data_capacity(evt));
}

TEST_F(EvtCreateTest, create_data_capacity_too_small) {
    gd_evt_t evt = createEvt("Evt1", 12, 3);
    ASSERT_TRUE(evt == NULL);
}

TEST_F(EvtCreateTest, create_data_capacity_eq) {
    gd_evt_t evt = createEvt("Evt1", 12, 4);
    ASSERT_TRUE(evt);

    EXPECT_EQ((size_t)4, gd_evt_data_capacity(evt));
    EXPECT_EQ((size_t)12, gd_evt_carry_data_capacity(evt));
}

TEST_F(EvtCreateTest, create_data_capacity_bg) {
    gd_evt_t evt = createEvt("Evt1", 12, 5);
    ASSERT_TRUE(evt);

    EXPECT_EQ((size_t)5, gd_evt_data_capacity(evt));
    EXPECT_EQ((size_t)12, gd_evt_carry_data_capacity(evt));
}

TEST_F(EvtCreateTest, create_carry_capacity_zero) {
    gd_evt_t evt = createEvt("Evt1", 0);
    ASSERT_TRUE(evt);

    EXPECT_EQ((size_t)0, gd_evt_carry_data_capacity(evt));
}
