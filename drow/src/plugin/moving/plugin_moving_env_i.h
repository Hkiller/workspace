#ifndef PLUGIN_MOVING_ENV_I_H
#define PLUGIN_MOVING_ENV_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/dr/dr_types.h"
#include "plugin/moving/plugin_moving_env.h"
#include "plugin_moving_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_moving_node_list, plugin_moving_node) plugin_moving_node_list_t;
typedef TAILQ_HEAD(plugin_moving_control_list, plugin_moving_control) plugin_moving_control_list_t;

struct plugin_moving_env {
    plugin_moving_module_t m_module;
    int m_debug;

    plugin_moving_control_list_t m_controls;
    plugin_moving_control_list_t m_free_controls;
    plugin_moving_node_list_t m_free_nodes;
};

#ifdef __cplusplus
}
#endif

#endif
