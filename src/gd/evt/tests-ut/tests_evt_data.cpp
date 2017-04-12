#include "EvtTest.hpp"

TEST_F(EvtTest, data_set_from_string_basic) {
    t_evt_mgr_set_metalib(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='Evt1' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>");

    gd_evt_t evt = createEvt("Evt1");
    ASSERT_TRUE(evt);

    EXPECT_STREQ("Evt1", gd_evt_type(evt));

    EXPECT_EQ(
        0,
        gd_evt_set_from_string(evt, "a1", "3", t_em()));

    int32_t data;
    EXPECT_EQ(
        0,
        gd_evt_try_get_int32(&data, evt, "a1", t_em()));
    EXPECT_EQ(3, data);
}

TEST_F(EvtTest, data_set_from_int32_basic) {
    t_evt_mgr_set_metalib(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='Evt1' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>");

    gd_evt_t evt = createEvt("Evt1");
    ASSERT_TRUE(evt);

    EXPECT_STREQ("Evt1", gd_evt_type(evt));

    EXPECT_EQ(
        0,
        gd_evt_set_from_int32(evt, "a1", 3, t_em()));

    int32_t data;
    EXPECT_EQ(
        0,
        gd_evt_try_get_int32(&data, evt, "a1", t_em()));
    EXPECT_EQ(3, data);
}
