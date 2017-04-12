#include "plugin_ui_phase_use_page_i.h"

plugin_ui_phase_use_page_t
plugin_ui_phase_use_page_create(plugin_ui_phase_t phase, plugin_ui_page_t page) {
    plugin_ui_env_t env = phase->m_env;
    plugin_ui_phase_use_page_t use_page;

    use_page = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_phase_use_page));
    if (use_page == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_use_page_create: alloc fail!");
        return NULL;
    }

    use_page->m_phase = phase;
    TAILQ_INSERT_TAIL(&phase->m_using_pages, use_page, m_next_for_phase);
    
    use_page->m_page = page;
    TAILQ_INSERT_TAIL(&page->m_used_by_phases, use_page, m_next_for_page);

    return use_page;
}

void plugin_ui_phase_use_page_free(plugin_ui_phase_use_page_t use_page) {
    plugin_ui_env_t env = use_page->m_phase->m_env;

    TAILQ_REMOVE(&use_page->m_phase->m_using_pages, use_page, m_next_for_phase);
    TAILQ_REMOVE(&use_page->m_page->m_used_by_phases, use_page, m_next_for_page);

    mem_free(env->m_module->m_alloc, use_page);
}

