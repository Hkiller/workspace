#include "MgrTest.hpp" 

TEST_F(MgrTest, class_object_it_basic) {
    pom_class_t the_class = pom_mgr_get_class(m_omm, addClass("class1", 20));
    ASSERT_TRUE(the_class);

    pom_oid_t oid = obj_alloc(pom_class_name_hs(the_class));
    EXPECT_TRUE(oid != POM_INVALID_OID);

    pom_obj_it it;
    pom_class_objects(the_class, &it);

    EXPECT_TRUE(pom_obj_it_next(&it) == pom_obj_get(m_omm, oid, t_em()));
    EXPECT_TRUE(pom_obj_it_next(&it) == NULL);
}

TEST_F(MgrTest, class_object_it_multi) {
    pom_class_t the_class = pom_mgr_get_class(m_omm, addClass("class1", 20));
    ASSERT_TRUE(the_class);

    ::std::set<pom_oid_t> oids;

    for(int i = 0; i < 1000; ++i) {
        pom_oid_t oid = obj_alloc(pom_class_name_hs(the_class));
        EXPECT_TRUE(oid != POM_INVALID_OID);
        EXPECT_TRUE(oids.insert(oid).second);
    }

    pom_obj_it it;
    pom_class_objects(the_class, &it);

    while(void * obj = pom_obj_it_next(&it) ) {
        pom_oid_t oid = pom_obj_id_from_addr(m_omm, obj, t_em());

        ::std::set<pom_oid_t>::iterator pos = oids.find(oid);
        ASSERT_TRUE(pos != oids.end());
        oids.erase(pos);
    }

    ASSERT_TRUE(oids.empty());
}
