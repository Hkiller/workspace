#ifndef PLUGIN_UI_ENV_PACKAGE_MANAGED_QUEUE_I_H
#define PLUGIN_UI_ENV_PACKAGE_MANAGED_QUEUE_I_H
#include "plugin/ui/plugin_ui_package_queue_managed.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_package_queue_managed {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_package_queue_managed) m_next_for_env;
    plugin_package_queue_t m_package_queue;
    plugin_ui_package_queue_using_list_t m_phases;
};

#ifdef __cplusplus
}
#endif

#endif
