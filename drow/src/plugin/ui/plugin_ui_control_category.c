#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin_ui_control_category_i.h"

plugin_ui_control_category_t
plugin_ui_control_category_create(plugin_ui_env_t env, const char * prefix) {
    plugin_ui_control_category_t category;

    if (plugin_ui_control_category_find(env, prefix) != NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_category_create: prefix %s already exist!", prefix);
        return NULL;
    }

    category = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_control_category));
    if (category == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_category_create: alloc fail!");
        return NULL;
    }

    category->m_env = env;
    cpe_str_dup(category->m_prefix, sizeof(category->m_prefix), prefix);
    category->m_click_audio = NULL;
    
    TAILQ_INSERT_TAIL(&env->m_control_categories, category, m_next);

    return category;
}

void plugin_ui_control_category_free(plugin_ui_control_category_t category) {
    plugin_ui_env_t env = category->m_env;
    
    TAILQ_REMOVE(&env->m_control_categories, category, m_next);

    if (category->m_click_audio) {
        mem_free(env->m_module->m_alloc, category->m_click_audio);
        category->m_click_audio = NULL;
    }
            
    mem_free(env->m_module->m_alloc, category);
}

plugin_ui_control_category_t
plugin_ui_control_category_find(plugin_ui_env_t env, const char * prefix) {
    plugin_ui_control_category_t category;

    TAILQ_FOREACH(category, &env->m_control_categories, m_next) {
        if (strcmp(category->m_prefix, prefix) == 0) return category;
    }
    
    return NULL;
}

int plugin_ui_control_category_set_click_audio(plugin_ui_control_category_t category, const char * res) {
    plugin_ui_env_t env = category->m_env;
    
    if (category->m_click_audio) {
        mem_free(env->m_module->m_alloc, category->m_click_audio);
    }

    if (res) {
        category->m_click_audio = cpe_str_mem_dup(env->m_module->m_alloc, res);
    }
    else {
        category->m_click_audio = NULL;
    }
        
    return 0;
}
