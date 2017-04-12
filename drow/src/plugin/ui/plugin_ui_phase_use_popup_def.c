#include "plugin_ui_phase_use_popup_def_i.h"

plugin_ui_phase_use_popup_def_t
plugin_ui_phase_use_popup_def_create(plugin_ui_phase_t phase, plugin_ui_popup_def_t popup_def) {
    plugin_ui_env_t env = phase->m_env;
    plugin_ui_phase_use_popup_def_t use_popup_def;

    use_popup_def = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_phase_use_popup_def));
    if (use_popup_def == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_use_popup_def_create: alloc fail!");
        return NULL;
    }

    use_popup_def->m_phase = phase;
    TAILQ_INSERT_TAIL(&phase->m_using_popup_defs, use_popup_def, m_next_for_phase);
    
    use_popup_def->m_popup_def = popup_def;
    TAILQ_INSERT_TAIL(&popup_def->m_used_by_phases, use_popup_def, m_next_for_popup_def);

    return use_popup_def;
}

void plugin_ui_phase_use_popup_def_free(plugin_ui_phase_use_popup_def_t use_popup_def) {
    plugin_ui_env_t env = use_popup_def->m_phase->m_env;

    TAILQ_REMOVE(&use_popup_def->m_phase->m_using_popup_defs, use_popup_def, m_next_for_phase);
    TAILQ_REMOVE(&use_popup_def->m_popup_def->m_used_by_phases, use_popup_def, m_next_for_popup_def);

    mem_free(env->m_module->m_alloc, use_popup_def);
}

