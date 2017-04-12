#ifndef PLUGIN_UI_CONTROL_ATTR_META_I_H
#define PLUGIN_UI_CONTROL_ATTR_META_I_H
#include "plugin/ui/plugin_ui_control_attr_meta.h"
#include "plugin_ui_control_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_attr_meta {
    plugin_ui_control_meta_t m_control_meta;
    struct cpe_hash_entry m_hh;
    char m_attr_name[32];
    plugin_ui_control_attr_set_fun_t m_setter;
    plugin_ui_control_attr_get_fun_t m_getter;
};

void plugin_ui_control_attr_meta_free_all(plugin_ui_control_meta_t meta);

int plugin_ui_control_attr_meta_create_basics(plugin_ui_control_meta_t meta);

uint32_t plugin_ui_control_attr_meta_hash(const plugin_ui_control_attr_meta_t attr_meta);
int plugin_ui_control_attr_meta_eq(const plugin_ui_control_attr_meta_t l, const plugin_ui_control_attr_meta_t r);
    
#ifdef __cplusplus
}
#endif

#endif
