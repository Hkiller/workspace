#ifndef UI_PLUGIN_PARTICLE_TYPES_H
#define UI_PLUGIN_PARTICLE_TYPES_H
#include "render/runtime/ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_particle_module * plugin_particle_module_t;
typedef struct plugin_particle_extern * plugin_particle_extern_t;

/*data*/
typedef struct plugin_particle_data * plugin_particle_data_t;
typedef struct plugin_particle_data_emitter * plugin_particle_data_emitter_t;
typedef struct plugin_particle_data_emitter_it * plugin_particle_data_emitter_it_t;
typedef struct plugin_particle_data_mod * plugin_particle_data_mod_t;
typedef struct plugin_particle_data_mod_it * plugin_particle_data_mod_it_t;
typedef struct plugin_particle_data_curve * plugin_particle_data_curve_t;

typedef struct plugin_particle_obj * plugin_particle_obj_t;
typedef struct plugin_particle_obj_particle * plugin_particle_obj_particle_t;
typedef struct plugin_particle_obj_particle_it * plugin_particle_obj_particle_it_t;
typedef struct plugin_particle_obj_emitter * plugin_particle_obj_emitter_t;
typedef struct plugin_particle_obj_emitter_it * plugin_particle_obj_emitter_it_t;
typedef struct plugin_particle_obj_plugin * plugin_particle_obj_plugin_t;
typedef struct plugin_particle_obj_plugin_it * plugin_particle_obj_plugin_it_t;
typedef struct plugin_particle_obj_plugin_data * plugin_particle_obj_plugin_data_t;    

typedef enum plugin_particle_obj_emitter_use_state {
    plugin_particle_obj_emitter_use_state_suspend,
    plugin_particle_obj_emitter_use_state_active,
    plugin_particle_obj_emitter_use_state_passive,
} plugin_particle_obj_emitter_use_state_t;

typedef enum plugin_particle_obj_emitter_texture_mode {
    plugin_particle_obj_emitter_texture_mode_basic,
    plugin_particle_obj_emitter_texture_mode_tiled,
    plugin_particle_obj_emitter_texture_mode_scroll,
} plugin_particle_obj_emitter_texture_mode_t;

typedef enum plugin_particle_obj_emitter_lifecircle {
    plugin_particle_obj_emitter_lifecircle_basic,
    plugin_particle_obj_emitter_lifecircle_remove_on_complete,
} plugin_particle_obj_emitter_lifecircle_t;
    
#ifdef __cplusplus
}
#endif

#endif
