#ifndef GD_APP_CONTEXT_H
#define GD_APP_CONTEXT_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "app_basic.h"

#ifdef __cplusplus
extern "C" {
#endif

gd_app_context_t
gd_app_context_create_main(
    mem_allocrator_t alloc, size_t capacity,
    int argc, char ** argv);

void gd_app_context_free(gd_app_context_t context);

void gd_app_set_main(gd_app_context_t context, gd_app_fn_t fn_main, gd_app_fn_t fn_stop, void * fn_ctx);

void gd_app_ins_set(gd_app_context_t context);

mem_allocrator_t gd_app_alloc(gd_app_context_t context);
void gd_app_set_alloc(gd_app_context_t context, mem_allocrator_t alloc);

/*arg operations*/
int gd_app_argc(gd_app_context_t context);
char ** gd_app_argv(gd_app_context_t context);
const char * gd_app_arg_find(gd_app_context_t context, const char * arg_name);
const char * gd_app_arg_find_ex(gd_app_context_t context, const char * arg_name, int * from_pos);
int gd_app_arg_is_set(gd_app_context_t context, const char * arg_name);
int gd_app_add_arg(gd_app_context_t context, char * arg);

/*basic suupport operations*/
error_monitor_t gd_app_print_em(gd_app_context_t context);
void gd_app_set_em(gd_app_context_t context, error_monitor_t em);
error_monitor_t gd_app_named_em(gd_app_context_t context, const char * name);
int gd_app_set_named_em(gd_app_context_t context, const char * name, error_monitor_t em);
void gd_app_remove_named_em(gd_app_context_t context, const char * name);

/*config operations*/
int gd_app_cfg_reload(gd_app_context_t context);
cfg_t gd_app_cfg(gd_app_context_t context);

/*extern module operations*/
tl_manage_t gd_app_tl_mgr(gd_app_context_t context);
dp_mgr_t gd_app_dp_mgr(gd_app_context_t context);
nm_mgr_t gd_app_nm_mgr(gd_app_context_t context);
net_mgr_t gd_app_net_mgr(gd_app_context_t context);
vfs_mgr_t gd_app_vfs_mgr(gd_app_context_t context);

mem_buffer_t gd_app_tmp_buffer(gd_app_context_t context);
mem_buffer_t gd_app_tmp_buffer_at(gd_app_context_t context, uint8_t idx);
    
/*app global infos*/
int gd_app_set_root(gd_app_context_t context, const char * root);
const char * gd_app_root(gd_app_context_t context);

void * gd_app_user_data(gd_app_context_t context);

void gd_app_set_state(gd_app_context_t context, gd_app_status_t state);
gd_app_status_t gd_app_state(gd_app_context_t context);

/*app tick function*/
int gd_app_tick_add(gd_app_context_t context, gd_app_tick_fun tick, void * tick_ctx, ptr_int_t tick_arg);
int gd_app_tick_remove(gd_app_context_t context, gd_app_tick_fun tick, void * tick_ctx);
int gd_app_tick(gd_app_context_t context, float delta_s);

/*app falgs functions*/
uint32_t gd_app_flags(gd_app_context_t context);
void gd_app_flags_set(gd_app_context_t context, uint32_t flag);
void gd_app_flag_enable(gd_app_context_t context, gd_app_flag_t flag);
void gd_app_flag_disable(gd_app_context_t context, gd_app_flag_t flag);
int gd_app_flag_is_enable(gd_app_context_t context, gd_app_flag_t flag);
 
/*the main*/
int gd_app_run(gd_app_context_t context);
int gd_app_notify_stop(gd_app_context_t context);

int gd_app_begin(gd_app_context_t context);
void gd_app_end(gd_app_context_t context);

int gd_app_debug(gd_app_context_t context);
void gd_app_set_debug(gd_app_context_t context, int level);

void gd_set_default_library(void * handler);
void gd_stop_on_signal(int sig);

/*tree*/    
gd_app_context_t gd_app_context_from(gd_app_context_t app);
nm_node_t gd_app_context_find_node(gd_app_context_t app, cpe_hash_string_t name);
nm_node_t gd_app_context_find_node_nc(gd_app_context_t app, const char * name);
    
#ifdef __cplusplus
}
#endif

#endif
