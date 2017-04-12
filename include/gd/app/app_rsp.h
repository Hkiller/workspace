#ifndef GD_APP_RSP_H
#define GD_APP_RSP_H
#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int gd_app_rsp_load(gd_app_context_t context, gd_app_module_t module, cfg_t moduleCfg);

#ifdef __cplusplus
}
#endif

#endif
