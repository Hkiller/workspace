#ifndef DROW_PLUGIN_UI_MOVE_ALGORITHM_H
#define DROW_PLUGIN_UI_MOVE_ALGORITHM_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_move_algorithm_t
plugin_ui_move_algorithm_create(plugin_ui_env_t env, plugin_ui_move_algorithm_meta_t meta);

plugin_ui_move_algorithm_t
plugin_ui_move_algorithm_create_by_type_name(plugin_ui_env_t env, const char * type_name);

plugin_ui_env_t plugin_ui_move_algorithm_env(plugin_ui_move_algorithm_t move_algorithm);
uint32_t plugin_ui_move_algorithm_id(plugin_ui_move_algorithm_t move_algorithm);

void * plugin_ui_move_algorithm_data(plugin_ui_move_algorithm_t move_algorithm);
plugin_ui_move_algorithm_t plugin_ui_move_algorithm_from_data(void * data);

plugin_ui_move_algorithm_t plugin_ui_move_algorithm_find(plugin_ui_env_t env, uint32_t move_algorithm_id);
    
void plugin_ui_move_algorithm_free(plugin_ui_move_algorithm_t algorithm);

#ifdef __cplusplus
}
#endif

#endif

