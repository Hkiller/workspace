#ifndef UI_SPRITE_PARTICLE_TYPES_H
#define UI_SPRITE_PARTICLE_TYPES_H
#include "plugin/particle/plugin_particle_types.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_particle_module * ui_sprite_particle_module_t;
typedef struct ui_sprite_particle_set_value * ui_sprite_particle_set_value_t;
typedef struct ui_sprite_particle_clear_particle * ui_sprite_particle_clear_particle_t;
typedef struct ui_sprite_particle_enable_emitter * ui_sprite_particle_enable_emitter_t;
typedef struct ui_sprite_particle_disable_emitter * ui_sprite_particle_disable_emitter_t;
typedef struct ui_sprite_particle_kill_emitter * ui_sprite_particle_kill_emitter_t;    
typedef struct ui_sprite_particle_bind_emitter_part * ui_sprite_particle_bind_emitter_part_t;
typedef struct ui_sprite_particle_gen_entity * ui_sprite_particle_gen_entity_t;
typedef struct ui_sprite_particle_enable * ui_sprite_particle_enable_t;
typedef struct ui_sprite_particle_disable * ui_sprite_particle_disable_t;
typedef struct ui_sprite_particle_controled_obj * ui_sprite_particle_controled_obj_t;

#ifdef __cplusplus
}
#endif

#endif
