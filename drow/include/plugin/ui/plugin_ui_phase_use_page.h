#ifndef DROW_PLUGIN_UI_PHASE_PAGE_H
#define DROW_PLUGIN_UI_PHASE_PAGE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_phase_use_page_it {
    plugin_ui_phase_use_page_t (*next)(struct plugin_ui_phase_use_page_it * it);
    char m_data[64];
};
    
plugin_ui_phase_use_page_t plugin_ui_phase_use_page_create(plugin_ui_phase_t phase, plugin_ui_page_t page);
void plugin_ui_phase_use_page_free(plugin_ui_phase_use_page_t binding);

#define plugin_ui_phase_use_page_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

