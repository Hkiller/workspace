#ifndef PLUGIN_UI_PHASE_USE_POPUP_DEF_I_H
#define PLUGIN_UI_PHASE_USE_POPUP_DEF_I_H
#include "plugin/ui/plugin_ui_phase_use_popup_def.h"
#include "plugin_ui_popup_def_i.h"
#include "plugin_ui_phase_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_phase_use_popup_def {
    plugin_ui_popup_def_t m_popup_def;
    TAILQ_ENTRY(plugin_ui_phase_use_popup_def) m_next_for_popup_def;
    plugin_ui_phase_t m_phase;
    TAILQ_ENTRY(plugin_ui_phase_use_popup_def) m_next_for_phase;
};

#ifdef __cplusplus
}
#endif

#endif
