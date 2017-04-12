#ifndef UI_SPRITE_CHIPMUNK_MODULE_I_H
#define UI_SPRITE_CHIPMUNK_MODULE_I_H
#include "chipmunk/chipmunk_private.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_touch_trace * ui_sprite_chipmunk_touch_trace_t;
typedef struct ui_sprite_chipmunk_touch_responser * ui_sprite_chipmunk_touch_responser_t;
typedef struct ui_sprite_chipmunk_touch_binding * ui_sprite_chipmunk_touch_binding_t;
    
    
typedef struct ui_sprite_chipmunk_obj_runtime_group * ui_sprite_chipmunk_obj_runtime_group_t;
typedef struct ui_sprite_chipmunk_obj_body_group * ui_sprite_chipmunk_obj_body_group_t;
typedef struct ui_sprite_chipmunk_obj_body_group_binding * ui_sprite_chipmunk_obj_body_group_binding_t;

typedef TAILQ_HEAD(ui_sprite_chipmunk_touch_trace_list, ui_sprite_chipmunk_touch_trace) ui_sprite_chipmunk_touch_trace_list_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_touch_responser_list, ui_sprite_chipmunk_touch_responser) ui_sprite_chipmunk_touch_responser_list_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_touch_binding_list, ui_sprite_chipmunk_touch_binding) ui_sprite_chipmunk_touch_binding_list_t;
    
typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_runtime_group_list, ui_sprite_chipmunk_obj_runtime_group) ui_sprite_chipmunk_obj_runtime_group_list_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_updator_list, ui_sprite_chipmunk_obj_updator) ui_sprite_chipmunk_obj_updator_list_t;

typedef TAILQ_HEAD(
    ui_sprite_chipmunk_obj_body_group_binding_list,
    ui_sprite_chipmunk_obj_body_group_binding) ui_sprite_chipmunk_obj_body_group_binding_list_t;

typedef TAILQ_HEAD(ui_sprite_chipmunk_tri_scope_list, ui_sprite_chipmunk_tri_scope) ui_sprite_chipmunk_tri_scope_list_t;
    
struct ui_sprite_chipmunk_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_runtime_module_t m_runtime;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    plugin_chipmunk_module_t m_chipmunk_module;
    ui_sprite_render_module_t m_sprite_render;
    ui_sprite_tri_module_t m_tri;
    error_monitor_t m_em;
    int m_debug;
    LPDRMETA m_meta_chipmunk_obj_data;
    LPDRMETA m_meta_chipmunk_collision_data;
    LPDRMETA m_meta_chipmunk_move_state;
    ui_sprite_chipmunk_obj_body_group_binding_list_t m_free_body_group_bindings;
    ui_sprite_chipmunk_tri_scope_list_t m_free_tri_scopes;
    struct mem_buffer m_dump_buffer;
};

tl_time_t ui_sprite_chipmunk_module_cur_time(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
