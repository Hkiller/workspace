#ifndef DROW_PLUGIN_UI_PHASE_POPUP_DEF_H
#define DROW_PLUGIN_UI_PHASE_POPUP_DEF_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_phase_use_popup_def_it {
    plugin_ui_phase_use_popup_def_t (*next)(struct plugin_ui_phase_use_popup_def_it * it);
    char m_data[64];
};
    
plugin_ui_phase_use_popup_def_t plugin_ui_phase_use_popup_def_create(plugin_ui_phase_t phase, plugin_ui_popup_def_t popup_def);
void plugin_ui_phase_use_popup_def_free(plugin_ui_phase_use_popup_def_t binding);

#define plugin_ui_phase_use_popup_def_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

