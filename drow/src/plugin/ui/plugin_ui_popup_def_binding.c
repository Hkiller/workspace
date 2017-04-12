#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_binding.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin_ui_popup_def_binding_i.h"
#include "plugin_ui_popup_def_binding_attr_i.h"
#include "plugin_ui_popup_i.h"

plugin_ui_popup_def_binding_t
plugin_ui_popup_def_binding_create(plugin_ui_popup_def_t popup_def, const char * control_name) {
    plugin_ui_env_t env = popup_def->m_env;
    size_t control_name_len = strlen(control_name) + 1;
    plugin_ui_popup_def_binding_t binding;

    binding = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_popup_def_binding) + control_name_len);
    if (binding == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_def_binding_create: alloc fail!");
        return NULL;
    }

    memcpy(binding + 1, control_name, control_name_len);

    binding->m_popup_def = popup_def;
    binding->m_control = (void*)(binding + 1);
    binding->m_on_click_close = 0;
    binding->m_on_click_action = NULL;
    TAILQ_INIT(&binding->m_binding_attrs);
    
    TAILQ_INSERT_TAIL(&popup_def->m_bindings, binding, m_next);

    return binding;
}

void plugin_ui_popup_def_binding_free(plugin_ui_popup_def_binding_t binding) {
    plugin_ui_env_t env = binding->m_popup_def->m_env;
    
    TAILQ_REMOVE(&binding->m_popup_def->m_bindings, binding, m_next);

    if (binding->m_on_click_action) {
        mem_free(env->m_module->m_alloc, binding->m_on_click_action);
        binding->m_on_click_action = NULL;
    }

    while(!TAILQ_EMPTY(&binding->m_binding_attrs)) {
        plugin_ui_popup_def_binding_attr_free(TAILQ_FIRST(&binding->m_binding_attrs));
    }
    
    mem_free(env->m_module->m_alloc, binding);
}

const char * plugin_ui_popup_def_binding_on_click_action(plugin_ui_popup_def_binding_t binding) {
    return binding->m_on_click_action;
}

int plugin_ui_popup_def_binding_set_on_click_action(plugin_ui_popup_def_binding_t binding, const char * res) {
    plugin_ui_env_t env = binding->m_popup_def->m_env;

    if (binding->m_on_click_action) {
        mem_free(env->m_module->m_alloc, binding->m_on_click_action);
    }

    if (res) {
        binding->m_on_click_action = cpe_str_mem_dup(env->m_module->m_alloc, res);
        if (binding->m_on_click_action == NULL) return -1;
    }
    else {
        binding->m_on_click_action = NULL;
    }

    return 0;
}

uint8_t plugin_ui_popup_def_binding_on_click_close(plugin_ui_popup_def_binding_t binding) {
    return binding->m_on_click_close;
}

void plugin_ui_popup_def_binding_set_on_click_close(plugin_ui_popup_def_binding_t binding, uint8_t on_click_close) {
    binding->m_on_click_close = on_click_close;
}

int plugin_ui_popup_def_binding_apply(plugin_ui_popup_def_binding_t binding, plugin_ui_popup_t popup) {
    plugin_ui_env_t env = binding->m_popup_def->m_env;
    plugin_ui_control_t control;
    plugin_ui_popup_def_binding_attr_t binding_attr;

    control = plugin_ui_control_find_child_by_path(plugin_ui_page_root_control(popup->m_page), binding->m_control);
    if (control == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_popup_def_binding_apply: %s: control %s not exist!",
            binding->m_popup_def->m_name, binding->m_control);
        return -1;
    }

    if (binding->m_on_click_close) {
        if (plugin_ui_popup_set_close_on_click(popup, control) != 0) return -1;
    }

    if (binding->m_on_click_action) {
        if (plugin_ui_popup_set_action_on_click(popup, control, binding->m_on_click_action) != 0) return -1;
    }

    TAILQ_FOREACH(binding_attr, &binding->m_binding_attrs, m_next) {
        plugin_ui_control_binding_t control_binding;
        
        control_binding = plugin_ui_control_binding_create(control, binding_attr->m_attr_name);
        if (control_binding == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_popup_def_binding_apply: %s: control %s: attr %s: create control binding fail!",
                binding->m_popup_def->m_name, binding->m_control, binding_attr->m_attr_name);
            return -1;
        }

        if (plugin_ui_control_binding_set_function(control_binding, binding_attr->m_attr_value) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_popup_def_binding_apply: %s: control %s: attr %s: set function %s fail!",
                binding->m_popup_def->m_name, binding->m_control, binding_attr->m_attr_name, binding_attr->m_attr_value);
            return -1;
        }
    }

    return 0;
}
