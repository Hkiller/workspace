#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_popup_def_binding_attr_i.h"
#include "plugin_ui_popup_def_binding_i.h"

plugin_ui_popup_def_binding_attr_t
plugin_ui_popup_def_binding_attr_create(plugin_ui_popup_def_binding_t binding, const char * attr_name, const char * attr_value) {
    plugin_ui_env_t env = binding->m_popup_def->m_env;
    plugin_ui_popup_def_binding_attr_t binding_attr;
    size_t name_len = strlen(attr_name) + 1;
    size_t value_len = strlen(attr_value) + 1;

    binding_attr = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_popup_def_binding_attr) + name_len + value_len);
    if (binding_attr == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_def_binding_attr_create: alloc fail!");
        return NULL;
    }

    binding_attr->m_binding = binding;
    TAILQ_INSERT_TAIL(&binding->m_binding_attrs, binding_attr, m_next);

    binding_attr->m_attr_name = (void*)(binding_attr + 1);
    memcpy((void *)binding_attr->m_attr_name, attr_name, name_len);
    
    binding_attr->m_attr_value = binding_attr->m_attr_name + name_len;
    memcpy((void *)binding_attr->m_attr_value, attr_value, value_len);
    
    return binding_attr;
}

void plugin_ui_popup_def_binding_attr_free(plugin_ui_popup_def_binding_attr_t binding_attr) {
    plugin_ui_env_t env = binding_attr->m_binding->m_popup_def->m_env;

    TAILQ_REMOVE(&binding_attr->m_binding->m_binding_attrs, binding_attr, m_next);

    mem_free(env->m_module->m_alloc, binding_attr);
}
