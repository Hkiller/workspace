#ifndef PLUGIN_MASK_RENDER_OBJ_I_H
#define PLUGIN_MASK_RENDER_OBJ_I_H
#include "render/utils/ui_color.h"
#include "plugin/mask/plugin_mask_render_obj.h"
#include "plugin_mask_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_mask_render_obj {
    plugin_mask_module_t m_module;
    plugin_mask_data_block_t m_data_block;
    ui_color m_color;
    ui_cache_res_t m_res;
};

int plugin_mask_render_obj_regist(plugin_mask_module_t module);
void plugin_mask_render_obj_unregist(plugin_mask_module_t module);

#ifdef __cplusplus
}
#endif

#endif
