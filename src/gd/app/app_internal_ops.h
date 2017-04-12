#ifndef GD_APP_INTERNAL_OPS_H
#define GD_APP_INTERNAL_OPS_H
#include "app_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*tick operations*/
void gd_app_tick_chain_free(gd_app_context_t context);

/*tl operations*/
void gd_app_tl_free(struct gd_app_tl * app_tl);

/*lib operations*/
struct gd_app_lib *
gd_app_lib_open_for_app(
    const char * libName, gd_app_context_t context, error_monitor_t em);

void gd_app_lib_close_for_app(
    struct gd_app_lib * lib, gd_app_context_t context, error_monitor_t em);

struct gd_app_lib *
gd_app_lib_open_for_module(
    const char * libName, struct gd_app_module_type * module, error_monitor_t em);

void gd_app_lib_close_for_module(
    struct gd_app_lib * lib, struct gd_app_module_type * module, error_monitor_t em);

void * gd_app_lib_sym(struct gd_app_lib * lib, const char * symName, error_monitor_t em);

/*module operations*/
int gd_app_module_create(gd_app_context_t context, const char * module_name, cfg_t cfg);
    
int gd_app_modules_load(gd_app_context_t context);
void gd_app_modules_unload(gd_app_context_t context);

/*module type operations*/
struct gd_app_module_type *
gd_app_module_type_create_from_lib(
    const char * type,
    const char * libName,
    error_monitor_t em);

void gd_app_module_type_free(struct gd_app_module_type * module, error_monitor_t em);

struct gd_app_module_type *
gd_app_module_type_find(const char * moduleName);

/*module data operations*/
nm_node_t
gd_app_module_data_load(
    gd_app_context_t context,
    const char * moduleName);

void gd_app_module_data_free(
    gd_app_context_t context,
    const char * moduleName);

/*child app operations*/
int gd_app_load_childs(gd_app_context_t context);
void gd_app_child_context_cancel_all(gd_app_context_t context);
void gd_app_child_context_wait_all(gd_app_context_t context);
void gd_app_child_context_free(struct gd_app_child_context * child_context);
void gd_app_child_context_free_all(gd_app_context_t context);

/*gd_app_em operations*/
uint32_t gd_app_em_hash(const struct gd_app_em * require);
int gd_app_em_cmp(const struct gd_app_em * l, const struct gd_app_em * r);
void gd_app_em_free_all(gd_app_context_t context);

void gd_app_ins_set(gd_app_context_t context);

#ifdef GD_APP_MULTI_THREAD
int gd_app_ms_global_init(void);
void gd_app_ms_global_fini(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

