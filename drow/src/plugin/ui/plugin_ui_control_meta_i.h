#ifndef PLUGIN_UI_CONTROL_META_I_H
#define PLUGIN_UI_CONTROL_META_I_H
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_meta_event_slot {
    plugin_ui_event_scope_t m_scope;
    plugin_ui_event_fun_t m_fun;
};

struct plugin_ui_control_meta {
    plugin_ui_module_t m_module;
    uint8_t m_type;
    plugin_ui_control_list_t m_controls;
    struct cpe_hash_table m_attr_metas;
    uint32_t m_product_capacity;
    plugin_ui_control_init_fun_t m_init;
    plugin_ui_control_fini_fun_t m_fini;
    plugin_ui_control_load_fun_t m_load;
    plugin_ui_control_update_fun_t m_update;
    plugin_ui_control_layout_fun_t m_layout;
    
    plugin_ui_control_event_fun_t m_on_self_loaded;

    struct plugin_ui_control_meta_event_slot m_event_slots[plugin_ui_event_max - plugin_ui_event_min];
};

int plugin_ui_control_meta_buff_init(plugin_ui_module_t module);
void plugin_ui_control_meta_buff_fini(plugin_ui_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
