#ifndef GD_APP_TYPES_H
#define GD_APP_TYPES_H
#include "cpe/tl/tl_types.h"
#include "cpe/dp/dp_types.h"
#include "cpe/nm/nm_types.h"
#include "cpe/net/net_types.h"
#include "cpe/vfs/vfs_types.h"
#include "cpe/cfg/cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum gd_app_status {
    gd_app_init,
    gd_app_runing,
    gd_app_shutingdown,
    gd_app_done
} gd_app_status_t;

typedef enum gd_app_flag {
    gd_app_flag_no_auto_load = 1 << 0 /*不自动加载etc下的配置文件 */
    , gd_app_flag_delay_module_unload = 1 << 1 /*模块在run以后不自动卸载 */
} gd_app_flag_t;

typedef struct gd_app_context * gd_app_context_t;
typedef struct gd_app_child_context * gd_app_child_context_t;
typedef struct gd_app_lib * gd_app_lib_t;
typedef struct gd_app_module * gd_app_module_t;
typedef struct gd_app_module_installer * gd_app_module_installer_t;

typedef int (*gd_app_fn_t)(gd_app_context_t ctx, void * user_ctx);

typedef int (*gd_app_module_global_init)(void);
typedef void (*gd_app_module_global_fini)(void);
typedef int (*gd_app_module_app_init)(gd_app_context_t context, gd_app_module_t module, cfg_t cfg);
typedef void (*gd_app_module_app_fini)(gd_app_context_t context, gd_app_module_t module);

typedef int (*gd_app_rsp_init_fun_t)(
    dp_rsp_t rsp, gd_app_context_t context, gd_app_module_t module, cfg_t cfg);

typedef ptr_int_t (*gd_app_tick_fun)(void * ctx, ptr_int_t arg, float delta_s);

typedef struct gd_app_module_def {
    const char * name;
    const char * type;
    const char * lib;
    const char * static_cfg;
    int (*dynamic_cfg)(gd_app_context_t app, cfg_t cfg, struct gd_app_module_def * module_def, void * ctx);
} * gd_app_module_def_t;

#ifdef __cplusplus
}
#endif

#endif
