#ifndef UI_PLUGIN_REMOTEANIM_OBJ_H
#define UI_PLUGIN_REMOTEANIM_OBJ_H
#include "plugin_remoteanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_obj_ref_t plugin_remoteanim_obj_default(plugin_remoteanim_obj_t obj);
void plugin_remoteanim_obj_set_default(plugin_remoteanim_obj_t obj, ui_runtime_render_obj_ref_t obj_ref);
int plugin_remoteanim_obj_set_default_by_res(plugin_remoteanim_obj_t obj, const char * res, char * addition_args);

plugin_remoteanim_block_t plugin_remoteanim_obj_block(plugin_remoteanim_obj_t obj);
void plugin_remoteanim_obj_set_block(plugin_remoteanim_obj_t obj, plugin_remoteanim_block_t block);
int plugin_remoteanim_obj_set_block_by_def(plugin_remoteanim_obj_t obj, const char * block_name, const char * url);
    
#ifdef __cplusplus
}
#endif

#endif

