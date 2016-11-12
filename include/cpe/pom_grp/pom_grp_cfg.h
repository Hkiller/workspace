#ifndef CPE_POM_GRP_CFG_H
#define CPE_POM_GRP_CFG_H
#include "cpe/cfg/cfg_types.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int pom_grp_obj_cfg_dump(cfg_t cfg, pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em);
int pom_grp_obj_cfg_load(pom_grp_obj_t obj, pom_grp_obj_mgr_t mgr, cfg_t cfg, error_monitor_t em);

int pom_grp_obj_cfg_dump_all(cfg_t cfg, pom_grp_obj_mgr_t mgr, error_monitor_t em);
int pom_grp_obj_cfg_load_all(pom_grp_obj_mgr_t mgr, cfg_t cfg, error_monitor_t em);

int pom_grp_obj_cfg_dump_to_stream(write_stream_t stream, pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em);
int pom_grp_obj_cfg_dump_all_to_stream(write_stream_t stream, pom_grp_obj_mgr_t mgr, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
