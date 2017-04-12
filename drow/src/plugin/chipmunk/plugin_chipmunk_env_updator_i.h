#ifndef PLUGIN_CHIPMUNK_ENV_UPDATOR_I_H
#define PLUGIN_CHIPMUNK_ENV_UPDATOR_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/dr/dr_types.h"
#include "chipmunk/chipmunk_private.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_env_updator {
    plugin_chipmunk_env_t m_env;
    TAILQ_ENTRY(plugin_chipmunk_env_updator) m_next;
    plugin_chipmunk_env_update_fun_t m_fun;
    void * m_ctx;
};

void plugin_chipmunk_env_updator_real_free(plugin_chipmunk_env_updator_t updator);

#ifdef __cplusplus
}
#endif

#endif
