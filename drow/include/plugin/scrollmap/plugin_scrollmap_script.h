#ifndef PLUGIN_SCROLLMAP_SCRIPT_H
#define PLUGIN_SCROLLMAP_SCRIPT_H
#include "plugin_scrollmap_types.h"
#include "protocol/plugin/scrollmap/scrollmap_script.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_scrollmap_script_t
plugin_scrollmap_script_create(
    plugin_scrollmap_layer_t layer, SCROLLMAP_SCRIPT const * script, ui_vector_2_t pos);
void plugin_scrollmap_script_free(plugin_scrollmap_script_t script);

plugin_scrollmap_script_state_t plugin_scrollmap_script_state(plugin_scrollmap_script_t script);
plugin_scrollmap_layer_t plugin_scrollmap_script_layer(plugin_scrollmap_script_t script);
SCROLLMAP_SCRIPT const * plugin_scrollmap_script_data(plugin_scrollmap_script_t script);

ui_vector_2_t plugin_scrollmap_script_pos(plugin_scrollmap_script_t script);
void plugin_scrollmap_script_set_pos(plugin_scrollmap_script_t script, ui_vector_2_t pos);

plugin_scrollmap_range_t plugin_scrollmap_script_range(plugin_scrollmap_script_t script);
void plugin_scrollmap_script_set_range(plugin_scrollmap_script_t script, plugin_scrollmap_range_t range);

void plugin_scrollmap_script_print(write_stream_t s, plugin_scrollmap_script_t script);    
const char * plugin_scrollmap_script_dump(plugin_scrollmap_script_t script);
    
typedef uint8_t (*plugin_scrollmap_script_check_fun_t)(void * ctx, plugin_scrollmap_layer_t layer, SCROLLMAP_SCRIPT const * script);
void plugin_scrollmap_env_set_script_filter(plugin_scrollmap_env_t env, void * ctx, plugin_scrollmap_script_check_fun_t check_fun);

#ifdef __cplusplus
}
#endif

#endif
