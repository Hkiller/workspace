#ifndef UI_SPRITE_CFG_LOADER_H
#define UI_SPRITE_CFG_LOADER_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/cfg/cfg_types.h"
#include "gd/app/app_types.h"
#include "ui_sprite_cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ui_sprite_world_res_t (*ui_sprite_cfg_load_resource_fun_t)(void * ctx, ui_sprite_world_t world, cfg_t cfg);
typedef int (*ui_sprite_cfg_load_comp_fun_t)(void * ctx, ui_sprite_component_t comp, cfg_t cfg);
typedef ui_sprite_fsm_action_t (*ui_sprite_cfg_load_action_fun_t)(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

ui_sprite_cfg_loader_t
ui_sprite_cfg_loader_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em);
void ui_sprite_cfg_loader_free(ui_sprite_cfg_loader_t loader);

ui_sprite_cfg_loader_t ui_sprite_cfg_loader_find(gd_app_context_t app, cpe_hash_string_t name);
ui_sprite_cfg_loader_t ui_sprite_cfg_loader_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_sprite_cfg_loader_app(ui_sprite_cfg_loader_t loader);
const char * ui_sprite_cfg_loader_name(ui_sprite_cfg_loader_t loader);
int ui_sprite_cfg_loader_init_default_loaders(ui_sprite_cfg_loader_t loader);

/*world*/
int ui_sprite_cfg_loader_load_world_from_path(
    ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * src_path);

int ui_sprite_cfg_loader_load_world_from_cfg(
    ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, cfg_t cfg);

/*entity */
int ui_sprite_cfg_loader_load_entity_from_path(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, const char * src_path);

int ui_sprite_cfg_loader_load_entity_from_cfg(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, cfg_t cfg);

/*world_res*/
int ui_sprite_cfg_loader_add_resource_loader(ui_sprite_cfg_loader_t loader, const char * name, ui_sprite_cfg_load_resource_fun_t fun, void * ctx);
int ui_sprite_cfg_loader_remove_resource_loader(ui_sprite_cfg_loader_t loader, const char * name);

ui_sprite_world_res_t
ui_sprite_cfg_loader_load_resource_from_path(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_world_t world, const char * world_res_name, const char * src_path);

ui_sprite_world_res_t
ui_sprite_cfg_loader_load_resource_from_cfg(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_world_t world, const char * world_res_name, cfg_t cfg);

/*component*/
int ui_sprite_cfg_loader_add_comp_loader(ui_sprite_cfg_loader_t loader, const char * name, ui_sprite_cfg_load_comp_fun_t fun, void * ctx);
int ui_sprite_cfg_loader_remove_comp_loader(ui_sprite_cfg_loader_t loader, const char * name);

int ui_sprite_cfg_loader_load_components_from_path(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, const char * src_path);

int ui_sprite_cfg_loader_load_components_from_cfg(
    ui_sprite_cfg_loader_t loader, ui_sprite_entity_t entity, cfg_t cfg);

ui_sprite_component_t
ui_sprite_cfg_loader_load_component_from_path(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_entity_t entity, const char * comp_name, const char * src_path);

ui_sprite_component_t
ui_sprite_cfg_loader_load_component_from_cfg(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_entity_t entity, const char * comp_name, cfg_t cfg);

/*action*/
int ui_sprite_cfg_loader_add_action_loader(ui_sprite_cfg_loader_t loader, const char * name, ui_sprite_cfg_load_action_fun_t fun, void * ctx);
int ui_sprite_cfg_loader_remove_action_loader(ui_sprite_cfg_loader_t loader, const char * name);

ui_sprite_fsm_action_t
ui_sprite_cfg_loader_load_action_from_path(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_fsm_state_t fsm_state, const char * comp_name, const char * src_path);

ui_sprite_fsm_action_t
ui_sprite_cfg_loader_load_action_from_cfg(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_fsm_state_t fsm_state, const char * comp_name, cfg_t cfg);

/*fsms*/
int ui_sprite_cfg_load_fsm_from_path(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_ins_t fsm, const char * src_path);
int ui_sprite_cfg_load_fsm(ui_sprite_cfg_loader_t loader, ui_sprite_fsm_ins_t fsm, cfg_t cfg);
    
ui_sprite_fsm_ins_t ui_sprite_cfg_loader_load_fsm_proto_from_path(
    ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * name, const char * path);
ui_sprite_fsm_ins_t ui_sprite_cfg_loader_load_fsm_proto_from_cfg(
    ui_sprite_cfg_loader_t loader, ui_sprite_world_t world, const char * name, cfg_t cfg);

cfg_t ui_sprite_cfg_loader_find_cfg(ui_sprite_cfg_loader_t loader, const char * path);
    
#ifdef __cplusplus
}
#endif

#endif
