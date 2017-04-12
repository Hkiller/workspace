#ifndef UI_SPRITE_PARTICLE_MODULE_I_H
#define UI_SPRITE_PARTICLE_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_particle/ui_sprite_particle_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_particle_bind_emitter_part_binding * ui_sprite_particle_bind_emitter_part_binding_t;
typedef TAILQ_HEAD(
    ui_sprite_particle_bind_emitter_part_binding_list,
    ui_sprite_particle_bind_emitter_part_binding) ui_sprite_particle_bind_emitter_part_binding_list_t;

typedef struct ui_sprite_particle_gen_entity_slot * ui_sprite_particle_gen_entity_slot_t;
    
struct ui_sprite_particle_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_runtime_module_t m_runtime;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    ui_sprite_render_module_t m_sprite_render;
    error_monitor_t m_em;
    int m_debug;

    ui_sprite_particle_bind_emitter_part_binding_list_t m_free_bind_emitter_part_binding;
};

#ifdef __cplusplus
}
#endif

#endif
