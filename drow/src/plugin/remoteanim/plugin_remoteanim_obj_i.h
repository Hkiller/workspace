#ifndef PLUGIN_REMOTEANIM_OBJ_I_H
#define PLUGIN_REMOTEANIM_OBJ_I_H
#include "plugin/remoteanim/plugin_remoteanim_obj.h"
#include "plugin_remoteanim_module_i.h"

struct plugin_remoteanim_obj {
    plugin_remoteanim_module_t m_module;
    plugin_remoteanim_block_t m_block;
    uint8_t m_resize_to_default;
    TAILQ_ENTRY(plugin_remoteanim_obj) m_next_for_block;
    ui_runtime_render_obj_ref_t m_default;
};

int plugin_remoteanim_obj_register(plugin_remoteanim_module_t module);
void plugin_remoteanim_obj_unregister(plugin_remoteanim_module_t module);

#endif
