#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "render/model/ui_data_layout.h"
#include "plugin_ui_control_binding_i.h"
#include "plugin_ui_control_binding_use_slot_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_attr_meta_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_page_calc_i.h"
#include "plugin_ui_env_i.h"

static void plugin_ui_control_binding_clear(plugin_ui_control_binding_t binding);

plugin_ui_control_binding_t
plugin_ui_control_binding_create(plugin_ui_control_t control, const char * attr_name) {
    plugin_ui_env_t env = control->m_page->m_env;
    plugin_ui_control_binding_t binding;
    plugin_ui_control_attr_meta_t attr_meta;

    attr_meta = plugin_ui_control_attr_meta_find(control->m_meta, attr_name);
    if (attr_meta == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_binding_create: attr %s not exist in control type %s!",
            attr_name, ui_data_control_type_name(control->m_meta->m_type));
        return NULL;
    }

    binding = TAILQ_FIRST(&env->m_free_bindings);
    if (binding) {
        TAILQ_REMOVE(&env->m_free_bindings, binding, m_next_for_control);
    }
    else {
        binding = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_control_binding));
        if (binding == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_binding_create: alloc fail!");
            return NULL;
        }
    }

    binding->m_control = control;
    TAILQ_INIT(&binding->m_slots);
    binding->m_attr_meta = attr_meta;
    binding->m_function = NULL;
    
    binding->m_need_process = 1;
    TAILQ_INSERT_TAIL(&binding->m_control->m_page->m_need_process_bindings, binding, m_next_for_page);
    
    TAILQ_INSERT_TAIL(&control->m_bindings, binding, m_next_for_control);

    return binding;
}

void plugin_ui_control_binding_free(plugin_ui_control_binding_t binding) {
    plugin_ui_control_t control = binding->m_control;
    plugin_ui_env_t env = control->m_page->m_env;

    TAILQ_REMOVE(&control->m_bindings, binding, m_next_for_control);
    if (binding->m_need_process) {
        TAILQ_REMOVE(&binding->m_control->m_page->m_need_process_bindings, binding, m_next_for_page);
    }

    plugin_ui_control_binding_clear(binding);

    binding->m_control = (plugin_ui_control_t)env;
    TAILQ_INSERT_TAIL(&env->m_free_bindings, binding, m_next_for_control);
}

void plugin_ui_control_binding_real_free(plugin_ui_control_binding_t binding) {
    plugin_ui_env_t env = (plugin_ui_env_t)binding->m_control;

    TAILQ_REMOVE(&env->m_free_bindings, binding, m_next_for_control);

    mem_free(env->m_module->m_alloc, binding);
}

void plugin_ui_control_binding_clear(plugin_ui_control_binding_t binding) {
    plugin_ui_env_t env = binding->m_control->m_page->m_env;

    while(!TAILQ_EMPTY(&binding->m_slots)) {
        plugin_ui_control_binding_use_slot_free(TAILQ_FIRST(&binding->m_slots));
    }

    if (binding->m_function) {
        mem_free(env->m_module->m_alloc, binding->m_function);
        binding->m_function = NULL;
    }
}

const char * plugin_ui_control_binding_attr_name(plugin_ui_control_binding_t bining) {
    return bining->m_attr_meta->m_attr_name;
}

const char * plugin_ui_control_binding_finction(plugin_ui_control_binding_t binding) {
    return binding->m_function;
}

uint8_t plugin_ui_control_binding_need_process(plugin_ui_control_binding_t binding) {
    return binding->m_need_process;
}

void plugin_ui_control_binding_set_need_process(plugin_ui_control_binding_t binding, uint8_t need_process) {
    if (need_process) need_process = 1;

    if (binding->m_need_process == need_process) return;

    if (binding->m_need_process) {
        TAILQ_REMOVE(&binding->m_control->m_page->m_need_process_bindings, binding, m_next_for_page);
    }

    binding->m_need_process = need_process;

    if (binding->m_need_process) {
        TAILQ_INSERT_TAIL(&binding->m_control->m_page->m_need_process_bindings, binding, m_next_for_page);
    }
}

static void plugin_ui_control_binding_on_found_arg(void * ctx, xtoken_t arg) {
    char name_buf[128];
    const char * attr_name;
    plugin_ui_control_binding_t binding = ctx;
    plugin_ui_page_t page = binding->m_control->m_page;
    plugin_ui_env_t env = page->m_env;
    plugin_ui_control_binding_use_slot_t bindnig_use_slot;
    plugin_ui_page_slot_t slot;

    attr_name = xtoken_to_str(arg, name_buf, sizeof(name_buf));

    TAILQ_FOREACH(bindnig_use_slot, &binding->m_slots, m_next_for_binding) {
        if (strcmp(bindnig_use_slot->m_slot->m_name, attr_name) == 0) return;
    }

    slot = plugin_ui_page_slot_find(page, attr_name);
    if (slot == NULL) {
        slot = plugin_ui_page_slot_create(page, attr_name);
        if (slot == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_control_binding_set_function: analyze function %s: create slot for arg %s fail",
                binding->m_function + 1, attr_name);
            return;
        }
    }

    bindnig_use_slot = plugin_ui_control_binding_use_slot_create(binding, slot);
    if (bindnig_use_slot == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_control_binding_set_function: analyze function %s: create binding use slot for arg %s fail",
            binding->m_function + 1, attr_name);
        return;
    }
}

int plugin_ui_control_binding_set_function(plugin_ui_control_binding_t binding, const char * function) {
    plugin_ui_env_t env = binding->m_control->m_page->m_env;

    plugin_ui_control_binding_clear(binding);

    if (function == NULL) return 0;

    binding->m_function = cpe_str_mem_dup_trim(env->m_module->m_alloc, function);
    if (binding->m_function == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_binding_set_function: alloc for function fail");
        return -1;
    }

    if (binding->m_function[0] == ':') {
        if (xcomputer_visit_args(env->m_module->m_computer, binding->m_function + 1, binding, plugin_ui_control_binding_on_found_arg) != 0) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_binding_set_function: analize function %s fail", binding->m_function + 1);
            plugin_ui_control_binding_clear(binding);
            return -1;
        }
    }

    return 0;
}

int plugin_ui_control_binding_apply(plugin_ui_control_binding_t binding) {
    plugin_ui_env_t env = binding->m_control->m_page->m_env;
    struct dr_value v;

    if (binding->m_function == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s: no function", binding->m_attr_meta->m_attr_name);
        return -1;
    }

    if (binding->m_function[0] != ':') {
        v.m_type = CPE_DR_TYPE_STRING;
        v.m_meta = NULL;
        v.m_data = (void*)binding->m_function;
        v.m_size = strlen(binding->m_function) + 1;
    
        if (binding->m_attr_meta->m_setter(binding->m_control, &v) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s set value from str %s fail!",
                binding->m_attr_meta->m_attr_name, binding->m_function);
            return -1;
        }
    }
    else {
        xcomputer_t computer = env->m_module->m_computer;
        xtoken_t value;
        
        value = plugin_ui_page_calc_i(binding->m_control->m_page, binding->m_function + 1, NULL);
        if (value == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s calc from %s fail!",
                binding->m_attr_meta->m_attr_name, binding->m_function + 1);
            return -1;
        }

        switch(xtoken_data_type(value)) {
        case xtoken_data_int: {
            int64_t rv;

            if (xtoken_try_to_int64(value, &rv) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s read value int fail!",
                    binding->m_attr_meta->m_attr_name);
                xcomputer_free_token(computer, value);
                return -1;
            }

            v.m_type = CPE_DR_TYPE_INT64;
            v.m_meta = NULL;
            v.m_data = (void*)&rv;
            v.m_size = sizeof(rv);

            if (binding->m_attr_meta->m_setter(binding->m_control, &v) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s set value from int "FMT_INT64_T" fail!",
                    binding->m_attr_meta->m_attr_name, rv);
                xcomputer_free_token(computer, value);
                return -1;
            }
            break;
        }
        case xtoken_data_double: {
            double rv;

            if (xtoken_try_to_double(value, &rv) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s read value double fail!",
                    binding->m_attr_meta->m_attr_name);
                xcomputer_free_token(computer, value);
                return -1;
            }

            v.m_type = CPE_DR_TYPE_DOUBLE;
            v.m_meta = NULL;
            v.m_data = (void*)&rv;
            v.m_size = sizeof(rv);
            
            if (binding->m_attr_meta->m_setter(binding->m_control, &v) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s set value from double %f fail!",
                    binding->m_attr_meta->m_attr_name, rv);
                xcomputer_free_token(computer, value);
                return -1;
            }
            break;
        }
        case xtoken_data_str: {
            const char * r = xtoken_try_to_str(value);
            if (r == NULL) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s read string fail!",
                    binding->m_attr_meta->m_attr_name);
                xcomputer_free_token(computer, value);
                return -1;
            }

            v.m_type = CPE_DR_TYPE_STRING;
            v.m_meta = NULL;
            v.m_data = (void*)r;
            v.m_size = strlen(r) + 1;
            
            if (binding->m_attr_meta->m_setter(binding->m_control, &v) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s set value from str %s fail!",
                    binding->m_attr_meta->m_attr_name, r);
                xcomputer_free_token(computer, value);
                return -1;
            }
            break;
        }
        default:
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_control_binding_apply: binding of %s unknown calc result token type!",
                binding->m_attr_meta->m_attr_name);
            xcomputer_free_token(computer, value);
            return -1;
        }
        
        xcomputer_free_token(computer, value);
    }
    
    return 0;
}
