#ifndef DROW_PLUGIN_CHIPMUNK_TYPES_H
#define DROW_PLUGIN_CHIPMUNK_TYPES_H
#include "cpe/pal/pal_types.h"
#include "render/model/ui_model_types.h"
#include "render/runtime/ui_runtime_module.h"
#include "protocol/plugin/chipmunk/chipmunk_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum chipmunk_obj_type {
    chipmunk_obj_type_dynamic = CHIPMUNK_OBJ_TYPE_DYNAMIC,
	chipmunk_obj_type_kinematic = CHIPMUNK_OBJ_TYPE_KINEMATIC,
	chipmunk_obj_type_static = CHIPMUNK_OBJ_TYPE_STATIC,
} chipmunk_obj_type_t;
    
/*plugin*/    
typedef struct plugin_chipmunk_module * plugin_chipmunk_module_t;

typedef struct plugin_chipmunk_env * plugin_chipmunk_env_t;
typedef struct plugin_chipmunk_env_updator * plugin_chipmunk_env_updator_t;
typedef struct plugin_chipmunk_render * plugin_chipmunk_render_t;

/**/
typedef struct plugin_chipmunk_data_scene * plugin_chipmunk_data_scene_t;
typedef struct plugin_chipmunk_data_body * plugin_chipmunk_data_body_t;
typedef struct plugin_chipmunk_data_body_it * plugin_chipmunk_data_body_it_t;
typedef struct plugin_chipmunk_data_fixture * plugin_chipmunk_data_fixture_t;
typedef struct plugin_chipmunk_data_fixture_it * plugin_chipmunk_data_fixture_it_t;
typedef struct plugin_chipmunk_data_polygon_node * plugin_chipmunk_data_polygon_node_t;
typedef struct plugin_chipmunk_data_constraint * plugin_chipmunk_data_constraint_t;
typedef struct plugin_chipmunk_data_constraint_it * plugin_chipmunk_data_constraint_it_t;

typedef void (*plugin_chipmunk_env_update_fun_t)(plugin_chipmunk_env_t env, void * ctx, float delta);

#ifdef __cplusplus
}
#endif

#endif
