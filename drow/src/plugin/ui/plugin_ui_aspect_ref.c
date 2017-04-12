#include "plugin_ui_aspect_ref_i.h"
#include "plugin_ui_aspect_i.h"

plugin_ui_aspect_ref_t
plugin_ui_aspect_ref_create(
    plugin_ui_aspect_t aspect, plugin_ui_aspect_ref_list_t * list_on_aspect,
    void * obj, plugin_ui_aspect_ref_list_t * list_on_obj, uint8_t is_tie)
{
    plugin_ui_env_t env = aspect->m_env;
    plugin_ui_aspect_ref_t aspect_ref;
    
    aspect_ref = TAILQ_FIRST(&env->m_free_aspect_refs);
    if (aspect_ref) {
        TAILQ_REMOVE(&env->m_free_aspect_refs, aspect_ref, m_next_for_aspect);
    }
    else {
        aspect_ref = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_aspect_ref));
        if (aspect_ref == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_aspect_ref: create: alloc fail!");
            return NULL;
        }
    }

    aspect_ref->m_aspect = aspect;
    aspect_ref->m_obj = obj;
    aspect_ref->m_is_tie = is_tie;

    TAILQ_INSERT_TAIL(list_on_aspect, aspect_ref, m_next_for_aspect);
    TAILQ_INSERT_TAIL(list_on_obj, aspect_ref, m_next_for_obj);

    return aspect_ref;
}

void plugin_ui_aspect_ref_free(
    plugin_ui_aspect_ref_t ref,
    plugin_ui_aspect_ref_list_t * list_on_aspect,
    plugin_ui_aspect_ref_list_t * list_on_obj)
{
    plugin_ui_env_t env = ref->m_aspect->m_env;
    
    TAILQ_REMOVE(list_on_aspect, ref, m_next_for_aspect);
    TAILQ_REMOVE(list_on_obj, ref, m_next_for_obj);

    ref->m_aspect = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_aspect_refs, ref, m_next_for_aspect);
}
    
void plugin_ui_aspect_ref_real_free(plugin_ui_aspect_ref_t ref) {
    plugin_ui_env_t env = (void*)ref->m_aspect;

    TAILQ_REMOVE(&env->m_free_aspect_refs, ref, m_next_for_aspect);

    mem_free(env->m_module->m_alloc, ref);
}
