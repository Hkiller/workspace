#ifndef DROW_PLUGIN_CHIPMUNK_ENV_UPDATOR_H
#define DROW_PLUGIN_CHIPMUNK_ENV_UPDATOR_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_env_updator_t plugin_chipmunk_env_updator_create(
    plugin_chipmunk_env_t env, plugin_chipmunk_env_update_fun_t fun, void * ctx);
void plugin_chipmunk_env_updator_free(plugin_chipmunk_env_updator_t updator);

#ifdef __cplusplus
}
#endif

#endif
