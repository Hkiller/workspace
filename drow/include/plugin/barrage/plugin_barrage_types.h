#ifndef DROW_PLUGIN_BARRAGE_TYPES_H
#define DROW_PLUGIN_BARRAGE_TYPES_H
#include "cpe/pal/pal_types.h"
#include "render/model/ui_model_types.h"
#include "render/cache/ui_cache_types.h"
#include "render/runtime/ui_runtime_types.h"
#include "plugin/chipmunk/plugin_chipmunk_types.h"
#include "plugin/particle/plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_barrage_data_emitter_flip_type {
    plugin_barrage_data_emitter_flip_type_none = 1,
    plugin_barrage_data_emitter_flip_type_x = 2,
    plugin_barrage_data_emitter_flip_type_y = 3,
    plugin_barrage_data_emitter_flip_type_xy = 4,
} plugin_barrage_data_emitter_flip_type_t;

typedef enum plugin_barrage_bullet_state {
    plugin_barrage_bullet_state_active,
    plugin_barrage_bullet_state_colliede,    
} plugin_barrage_bullet_state_t;

typedef struct plugin_barrage_module * plugin_barrage_module_t;

typedef struct plugin_barrage_data_barrage * plugin_barrage_data_barrage_t;
typedef struct plugin_barrage_data_emitter * plugin_barrage_data_emitter_t;
typedef struct plugin_barrage_data_emitter_it * plugin_barrage_data_emitter_it_t;
typedef struct plugin_barrage_data_emitter_trigger * plugin_barrage_data_emitter_trigger_t;
typedef struct plugin_barrage_data_emitter_trigger_it * plugin_barrage_data_emitter_trigger_it_t;
typedef struct plugin_barrage_data_bullet_trigger * plugin_barrage_data_bullet_trigger_t;
typedef struct plugin_barrage_data_bullet_trigger_it * plugin_barrage_data_bullet_trigger_it_t;

typedef struct plugin_barrage_env * plugin_barrage_env_t;
typedef struct plugin_barrage_group * plugin_barrage_group_t;
typedef struct plugin_barrage_barrage * plugin_barrage_barrage_t;
typedef struct plugin_barrage_emitter * plugin_barrage_emitter_t;
typedef struct plugin_barrage_emitter_it * plugin_barrage_emitter_it_t;
typedef struct plugin_barrage_bullet * plugin_barrage_bullet_t;
typedef struct plugin_barrage_bullet_it * plugin_barrage_bullet_it_t;

typedef struct plugin_barrage_render * plugin_barrage_render_t;

typedef int (*plugin_barrage_target_fun_t)(void * ctx, ui_vector_2_t pos);

#ifdef __cplusplus
}
#endif

#endif
