#include "MgrTest.hpp" 

TEST_F(MgrTest, object_alloc_basic) {
    CPE_HS_DEF_VAR(className, "class1");
    addClass("class1", 20);

    t_em_set_print();

    pom_oid_t oid = obj_alloc(className);
    EXPECT_TRUE(oid != POM_INVALID_OID);

    void * objData = pom_obj_get(m_omm, oid, t_em());
    ASSERT_TRUE(objData);

    pom_class_t cls = pom_obj_class(m_omm, oid, t_em());
    ASSERT_TRUE(cls);

    EXPECT_STREQ("class1", pom_class_name(cls));
}

TEST_F(MgrTest, object_alloc_no_class) {
    CPE_HS_DEF_VAR(className, "not-exist-class");

    EXPECT_EQ(POM_INVALID_OID, obj_alloc(className));

    EXPECT_TRUE(t_em_have_errno(pom_class_not_exist));
}

TEST_F(MgrTest, object_size) {
    CPE_HS_DEF_VAR(className, "class1");
    addClass("class1", 20);

    pom_oid_t oid1 = obj_alloc(className);
    EXPECT_TRUE(oid1 != POM_INVALID_OID);
    void * objData1 = pom_obj_get(m_omm, oid1, t_em());
    ASSERT_TRUE(objData1);

    pom_oid_t oid2 = obj_alloc(className);
    EXPECT_TRUE(oid2 != POM_INVALID_OID);

    void * objData2 = pom_obj_get(m_omm, oid2, t_em());
    ASSERT_TRUE(objData2);

    EXPECT_EQ(1, (int)(oid2 - oid1));
    EXPECT_EQ(20, (int)((char*)objData2 - (char*)objData1));
}

TEST_F(MgrTest, object_free) {
    CPE_HS_DEF_VAR(className, "class1");
    addClass("class1", 20);

    pom_oid_t oid = obj_alloc(className);
    EXPECT_TRUE(oid != POM_INVALID_OID);

    void * objData = pom_obj_get(m_omm, oid, t_em());
    ASSERT_TRUE(objData);

    pom_obj_free(m_omm, oid, t_em());

    EXPECT_TRUE(NULL == pom_obj_get(m_omm, oid, t_em()));

    EXPECT_EQ(oid, obj_alloc(className));
}

TEST_F(MgrTest, object_from_addr_basic) {
    CPE_HS_DEF_VAR(className, "class1");
    addClass("class1", 20);

    pom_oid_t oid = obj_alloc(className);
    EXPECT_TRUE(oid != POM_INVALID_OID);

    void * objData = pom_obj_get(m_omm, oid, t_em());
    ASSERT_TRUE(objData);

    EXPECT_EQ(oid, pom_obj_id_from_addr(m_omm, objData, t_em()));
}
