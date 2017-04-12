#include "cpe/utils/string_utils.h"
#include "plugin_ui_state_node_page_i.h"

plugin_ui_state_node_page_t
plugin_ui_state_node_page_create(
    plugin_ui_state_node_t state_node, plugin_ui_page_t page, const char * before_page,
    plugin_ui_page_state_on_hide_fun_t on_hide_fun, 
    void * on_hide_ctx)
{
    plugin_ui_env_t env = page->m_env;
    plugin_ui_state_node_page_t node_page;

    node_page = TAILQ_FIRST(&env->m_free_node_pages);
    if (node_page) {
        TAILQ_REMOVE(&env->m_free_node_pages, node_page, m_next_for_state_node);
    }
    else {
        node_page = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_state_node_page));
        if (node_page == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_state_node_page_create: alloc fail");
            return NULL;
        }
    }

    node_page->m_state_node = state_node;
    node_page->m_page = page;
    node_page->m_on_hide_fun = on_hide_fun;
    node_page->m_on_hide_ctx = on_hide_ctx;
    
    if (before_page) {
        cpe_str_dup(node_page->m_before_page, sizeof(node_page->m_before_page), before_page);
    }
    else {
        node_page->m_before_page[0] = 0;
    }
    
    env->m_visible_pages_need_update = 1;

    TAILQ_INSERT_TAIL(&node_page->m_state_node->m_pages, node_page, m_next_for_state_node);
    TAILQ_INSERT_TAIL(&node_page->m_page->m_visible_in_states, node_page, m_next_for_page);

    return node_page;
}

plugin_ui_state_node_page_t
plugin_ui_state_node_page_find(plugin_ui_state_node_t state_node, plugin_ui_page_t page) {
    plugin_ui_state_node_page_t node_page;

    TAILQ_FOREACH(node_page, &page->m_visible_in_states, m_next_for_page) {
        if (node_page->m_state_node == state_node) return node_page;
    }

    return NULL;
}
    
void plugin_ui_state_node_page_free(plugin_ui_state_node_page_t node_page) {
    plugin_ui_env_t env = node_page->m_page->m_env;

    if (node_page->m_on_hide_fun) {
        node_page->m_on_hide_fun(node_page->m_on_hide_ctx, node_page->m_page, node_page->m_state_node);
    }
    
    env->m_visible_pages_need_update = 1;
    TAILQ_REMOVE(&node_page->m_state_node->m_pages, node_page, m_next_for_state_node);
    TAILQ_REMOVE(&node_page->m_page->m_visible_in_states, node_page, m_next_for_page);

    node_page->m_state_node = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_node_pages, node_page, m_next_for_state_node);
}

void plugin_ui_state_node_page_real_free(plugin_ui_state_node_page_t node_page) {
    plugin_ui_env_t env = (void*)node_page->m_state_node;

    TAILQ_REMOVE(&env->m_free_node_pages, node_page, m_next_for_state_node);

    mem_free(env->m_module->m_alloc, node_page);
}



