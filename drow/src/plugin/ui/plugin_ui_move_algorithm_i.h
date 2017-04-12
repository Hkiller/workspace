#ifndef PLUGIN_UI_ANIMATION_MOVE_ALGORITHM_I_H
#define PLUGIN_UI_ANIMATION_MOVE_ALGORITHM_I_H
#include "plugin/ui/plugin_ui_move_algorithm.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_move_algorithm {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_move_algorithm) m_next_for_env;
    uint32_t m_id;
    struct cpe_hash_entry m_hh;
    plugin_ui_move_algorithm_meta_t m_meta;
    TAILQ_ENTRY(plugin_ui_move_algorithm) m_next_for_meta;
    void * m_user_ctx;
    void (*m_user_on_fini)(void * m_user_ctx, plugin_ui_move_algorithm_t algorithm);
};

int plugin_ui_move_algorithm_setup(plugin_ui_move_algorithm_t move_algorithm, char * arg_buf_will_change);
    
uint32_t plugin_ui_move_algorithm_hash(const plugin_ui_move_algorithm_t meta);
int plugin_ui_move_algorithm_eq(const plugin_ui_move_algorithm_t l, const plugin_ui_move_algorithm_t r);

void plugin_ui_move_algorithm_real_free(plugin_ui_move_algorithm_t move_algorithm);
    
#ifdef __cplusplus
}
#endif

#endif
