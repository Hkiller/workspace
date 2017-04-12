#ifndef DROW_PLUGIN_PACKAGE_MANIP_TYPES_H
#define DROW_PLUGIN_PACKAGE_MANIP_TYPES_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/xcalc/xcalc_types.h"
#include "render/model_ed/ui_ed_types.h"
#include "plugin/package/plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_package_manip * plugin_package_manip_t;
typedef struct plugin_package_manip_res_collector * plugin_package_manip_res_collector_t;
typedef struct plugin_package_manip_src_convertor * plugin_package_manip_src_convertor_t;
    
#ifdef __cplusplus
}
#endif

#endif
