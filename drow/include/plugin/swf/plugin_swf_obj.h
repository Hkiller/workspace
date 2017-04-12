#ifndef PLUGIN_SWF_OBJ_H
#define PLUGIN_SWF_OBJ_H
#include "render/model/ui_model_types.h"
#include "plugin_swf_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
plugin_swf_module_t  plugin_swf_obj_module(plugin_swf_obj_t obj);

int plugin_swf_obj_set_data(plugin_swf_obj_t obj, plugin_swf_data_t data);

int plugin_swf_obj_modify(plugin_swf_obj_t obj, const char * modification);
    
#ifdef __cplusplus
}
#endif

#endif
