#ifndef CPE_POM_GRP_TESTENV_OMGRP_H
#define CPE_POM_GRP_TESTENV_OMGRP_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../pom_grp_meta.h"
#include "../pom_grp_obj.h"
#include "../pom_grp_obj_mgr.h"

namespace cpe { namespace pom_grp { namespace testenv {

class with_pom_grp : public ::testenv::env<> {
public:
    with_pom_grp();

    const char * t_pom_grp_meta_dump(pom_grp_meta_t meta);

    pom_grp_meta_t t_pom_grp_meta_create_by_cfg(
        const char * om_meta,
        const char * metalib, 
        uint16_t page_size = 256);

    pom_grp_meta_t t_pom_grp_meta_create_by_cfg(
        const char * om_meta,
        LPDRMETALIB metalib,
        uint16_t page_size = 256);

    pom_grp_obj_mgr_t t_pom_grp_obj_mgr_create_by_cfg(
        const char * om_meta,
        const char * metalib, 
        size_t capacity = 2048, uint16_t page_size = 256);

    pom_grp_obj_mgr_t t_pom_grp_obj_mgr_create_by_cfg(
        const char * om_meta,
        LPDRMETALIB metalib, 
        size_t capacity = 2048, uint16_t page_size = 256);

    pom_grp_meta_t t_pom_grp_meta_create_by_meta(
        const char * metalib, 
        const char * metaname,
        uint16_t page_size = 256);

    pom_grp_meta_t t_pom_grp_meta_create_by_meta(
        LPDRMETA meta, uint16_t page_size = 256);

    pom_grp_obj_mgr_t t_pom_grp_obj_mgr_create_by_meta(
        const char * metalib,
        const char * metaname, 
        size_t capacity = 2048, uint16_t page_size = 256);

    pom_grp_obj_mgr_t t_pom_grp_obj_mgr_create_by_meta(
        LPDRMETA meta, size_t capacity = 2048, uint16_t page_size = 256);


    pom_grp_obj_t t_pom_grp_obj_create(
        pom_grp_obj_mgr_t obj_mgr,
        const char * data);

    pom_grp_obj_t t_pom_grp_obj_create(
        pom_grp_obj_mgr_t obj_mgr,
        cfg_t data);

    int t_pom_grp_obj_load(
        pom_grp_obj_mgr_t obj_mgr,
        pom_grp_obj_t obj,
        const char * data);

    int t_pom_grp_obj_load(
        pom_grp_obj_mgr_t obj_mgr,
        pom_grp_obj_t obj,
        cfg_t data);

    const char * t_pom_grp_obj_dump(
        pom_grp_obj_mgr_t obj_mgr,
        pom_grp_obj_t obj);
};

}}}

#endif
