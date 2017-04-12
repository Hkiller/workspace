#ifndef DROW_PLUGIN_UI_SEARCH_H
#define DROW_PLUGIN_UI_SEARCH_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/*navigation*/
plugin_ui_navigation_t
plugin_ui_search_next_navigation_to_state(
    plugin_ui_phase_node_t phase, const char * state);

#ifdef __cplusplus
}
#endif

#endif

