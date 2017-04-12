#ifndef DROW_PLUGIN_CHIPMUNK_ENV_H
#define DROW_PLUGIN_CHIPMUNK_ENV_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_env_t plugin_chipmunk_env_create(plugin_chipmunk_module_t module);
void plugin_chipmunk_env_free(plugin_chipmunk_env_t env);

/*cpSpace*/ void * plugin_chipmunk_env_space(plugin_chipmunk_env_t env);

int plugin_chipmunk_env_register_collision_type(uint32_t * type, plugin_chipmunk_env_t env, const char * collision_type_name);
    
int plugin_chipmunk_env_mask_register(plugin_chipmunk_env_t env, const char * layer_name);
void plugin_chipmunk_env_mask_unregister(plugin_chipmunk_env_t env, const char * laye_name);
uint32_t plugin_chipmunk_env_mask(plugin_chipmunk_env_t env, const char * laye_name);
int plugin_chipmunk_env_masks(plugin_chipmunk_env_t env, uint32_t * masks, const char * laye_name);

float plugin_chipmunk_env_time_scale(plugin_chipmunk_env_t env);
void plugin_chipmunk_env_set_time_scale(plugin_chipmunk_env_t env, float time_scale);
    
float plugin_chipmunk_env_ptm(plugin_chipmunk_env_t env);
void plugin_chipmunk_env_set_ptm(plugin_chipmunk_env_t env, float ptm);
    
float plugin_chipmunk_env_step_duration(plugin_chipmunk_env_t env);
void plugin_chipmunk_env_set_step_duration(plugin_chipmunk_env_t env, float step_duration);

void plugin_chipmunk_env_set_gravity(plugin_chipmunk_env_t env, float gravity_x, float gravity_y);

void plugin_chipmunk_env_set_spatial_hash(plugin_chipmunk_env_t env, float dim, uint32_t count);
    
int plugin_chipmunk_env_update(plugin_chipmunk_env_t env, float delta);

#ifdef __cplusplus
}
#endif

#endif
