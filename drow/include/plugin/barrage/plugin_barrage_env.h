#ifndef DROW_PLUGIN_BARRAGE_ENV_H
#define DROW_PLUGIN_BARRAGE_ENV_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "plugin_barrage_types.h"
#include "protocol/plugin/barrage/barrage_common.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_barrage_env_t plugin_barrage_env_create(plugin_barrage_module_t module, plugin_chipmunk_env_t chipmunk_env);
void plugin_barrage_env_free(plugin_barrage_env_t env);

int plugin_barrage_env_update(plugin_barrage_env_t env, float delta, BARRAGE_RECT const * rect);

uint32_t plugin_barrage_env_cur_frame(plugin_barrage_env_t env);

float plugin_barrage_env_fps(plugin_barrage_env_t env);
void plugin_barrage_env_set_fps(plugin_barrage_env_t env, float fps);

uint32_t plugin_barrage_env_collision_type(plugin_barrage_env_t env);
uint32_t plugin_barrage_env_bullet_count(plugin_barrage_env_t env);
uint32_t plugin_barrage_env_emitter_count(plugin_barrage_env_t env);
uint32_t plugin_barrage_env_op_count(plugin_barrage_env_t env);

void plugin_barrage_env_bullets(plugin_barrage_bullet_it_t bullet_it, plugin_barrage_env_t env);

#ifdef __cplusplus
}
#endif

#endif

