#ifndef PLUGIN_UI_PHASE_USE_PAGE_I_H
#define PLUGIN_UI_PHASE_USE_PAGE_I_H
#include "plugin/ui/plugin_ui_phase_use_page.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_phase_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_phase_use_page {
    plugin_ui_page_t m_page;
    TAILQ_ENTRY(plugin_ui_phase_use_page) m_next_for_page;
    plugin_ui_phase_t m_phase;
    TAILQ_ENTRY(plugin_ui_phase_use_page) m_next_for_phase;
};

#ifdef __cplusplus
}
#endif

#endif
