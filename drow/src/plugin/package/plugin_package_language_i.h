#ifndef PLUGIN_UI_PHASE_LANGUAGE_I_H
#define PLUGIN_UI_PHASE_LANGUAGE_I_H
#include "plugin/package/plugin_package_language.h"
#include "plugin_package_package_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_language {
    plugin_package_package_t m_package;
    TAILQ_ENTRY(plugin_package_language) m_next_for_package;
    ui_data_language_t m_data_language;
    ui_cache_group_t m_resources;
    ui_data_src_group_t m_srcs;
};

#ifdef __cplusplus
}
#endif

#endif
