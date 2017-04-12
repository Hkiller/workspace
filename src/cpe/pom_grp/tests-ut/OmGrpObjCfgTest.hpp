#ifndef CPE_OM_GRP_TEST_OBJCFG_H
#define CPE_OM_GRP_TEST_OBJCFG_H
#include "cpe/pom_grp/pom_grp_cfg.h"
#include "OmGrpObjMgrTest.hpp"

class OmGrpObjCfgTest : public OmGrpObjMgrTest {
public:
    const char * dump(pom_grp_obj_t obj);
};

#endif
