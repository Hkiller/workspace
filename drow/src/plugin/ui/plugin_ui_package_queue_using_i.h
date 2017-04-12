#ifndef PLUGIN_UI_ENV_PACKAGE_QUEUE_USING_I_H
#define PLUGIN_UI_ENV_PACKAGE_QUEUE_USING_I_H
#include "plugin/ui/plugin_ui_package_queue_using.h"
#include "plugin_ui_package_queue_managed_i.h"
#include "plugin_ui_phase_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_package_queue_using {
    plugin_ui_package_queue_managed_t m_queue;
    TAILQ_ENTRY(plugin_ui_package_queue_using) m_next_for_queue;
    plugin_ui_phase_t m_phase;
    TAILQ_ENTRY(plugin_ui_package_queue_using) m_next_for_phase;
    uint32_t m_limit;
};

#ifdef __cplusplus
}
#endif

#endif
