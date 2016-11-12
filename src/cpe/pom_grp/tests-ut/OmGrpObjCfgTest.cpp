#include "OmGrpObjCfgTest.hpp"

const char * OmGrpObjCfgTest::dump(pom_grp_obj_t obj) {
    cfg_t data = t_cfg_create();

    EXPECT_EQ(0, pom_grp_obj_cfg_dump(data, m_mgr, obj, t_em()));

    return t_cfg_dump(data);
}

