#ifndef PLUGIN_LAYOUT_LAYOUT_META_I_H
#define PLUGIN_LAYOUT_LAYOUT_META_I_H
#include "plugin/layout/plugin_layout_layout_meta.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_layout_meta {
    plugin_layout_module_t m_module;
    const char * m_name;
    uint32_t m_data_capacity;
    cpe_hash_entry m_hh_for_module;
    plugin_layout_layout_init_fun_t m_init;
    plugin_layout_layout_fini_fun_t m_fini;
    plugin_layout_layout_setup_fun_t m_setup;
    plugin_layout_layout_analize_fun_t m_analize;
    plugin_layout_layout_layout_fun_t m_layout;
    plugin_layout_layout_update_fun_t m_update;
};

void plugin_layout_layout_meta_free_all(const plugin_layout_module_t module);
    
uint32_t plugin_layout_layout_meta_hash(const plugin_layout_layout_meta_t meta);
int plugin_layout_layout_meta_eq(const plugin_layout_layout_meta_t l, const plugin_layout_layout_meta_t r);
    
#ifdef __cplusplus
}
#endif

#endif
