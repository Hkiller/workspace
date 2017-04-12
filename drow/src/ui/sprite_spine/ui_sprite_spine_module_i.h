#ifndef UI_SPRITE_SPINE_MODULE_I_H
#define UI_SPRITE_SPINE_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_spine/ui_sprite_spine_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_spine_bind_parts_binding * ui_sprite_spine_bind_parts_binding_t;
typedef TAILQ_HEAD(ui_sprite_spine_bind_parts_binding_list, ui_sprite_spine_bind_parts_binding) ui_sprite_spine_bind_parts_binding_list_t;
    
typedef struct ui_sprite_spine_follow_parts_binding * ui_sprite_spine_follow_parts_binding_t;
typedef TAILQ_HEAD(ui_sprite_spine_follow_parts_binding_list, ui_sprite_spine_follow_parts_binding) ui_sprite_spine_follow_parts_binding_list_t;    

typedef struct ui_sprite_spine_control_entity_slot * ui_sprite_spine_control_entity_slot_t;
typedef TAILQ_HEAD(ui_sprite_spine_control_entity_slot_list, ui_sprite_spine_control_entity_slot) ui_sprite_spine_control_entity_slot_list_t;
typedef TAILQ_HEAD(ui_sprite_spine_controled_obj_list, ui_sprite_spine_controled_obj) ui_sprite_spine_controled_obj_list_t;
    
struct ui_sprite_spine_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_runtime_module_t m_runtime;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    ui_sprite_render_module_t m_sprite_render;
    ui_sprite_tri_module_t m_tri;
    error_monitor_t m_em;
    int m_debug;
    struct mem_buffer m_dump_buffer;
    ui_sprite_spine_bind_parts_binding_list_t m_free_bind_parts_binding;
    ui_sprite_spine_control_entity_slot_list_t m_free_control_entity_slots;
    ui_sprite_spine_follow_parts_binding_list_t m_free_follow_parts_binding;
};

const char * ui_sprite_spine_module_analize_name(ui_sprite_spine_module_t module, const char * name, char * * args);

    
#ifdef __cplusplus
}
#endif

#endif
