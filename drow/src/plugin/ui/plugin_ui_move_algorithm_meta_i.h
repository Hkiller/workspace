#ifndef PLUGIN_UI_ANIMATION_MOVE_ALGORITHM_META_I_H
#define PLUGIN_UI_ANIMATION_MOVE_ALGORITHM_META_I_H
#include "plugin/ui/plugin_ui_move_algorithm_meta.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_move_algorithm_meta {
    plugin_ui_module_t m_module;
    const char * m_name;
    struct cpe_hash_entry m_hh;
    plugin_ui_move_algorithm_list_t m_algorithms;
    
    void * m_ctx;
    size_t m_capacity;
    plugin_ui_move_alogrithm_init_fun_t m_init;
    plugin_ui_move_alogrithm_fini_fun_t m_fini;
    plugin_ui_move_alogrithm_calc_duration_fun_t m_calc_duration;
    plugin_ui_move_alogrithm_calc_pos_fun_t m_calc_pos;
    plugin_ui_move_algorithm_setup_fun_t m_setup;
};

void plugin_ui_move_algorithm_meta_free_all(const plugin_ui_module_t module);
    
uint32_t plugin_ui_move_algorithm_meta_hash(const plugin_ui_move_algorithm_meta_t meta);
int plugin_ui_move_algorithm_meta_eq(const plugin_ui_move_algorithm_meta_t l, const plugin_ui_move_algorithm_meta_t r);
    
#ifdef __cplusplus
}
#endif

#endif
