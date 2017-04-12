#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_page_slot_i.h"
#include "plugin_ui_control_binding_use_slot_i.h"

plugin_ui_page_slot_t
plugin_ui_page_slot_create(plugin_ui_page_t page, const char * name) {
    plugin_ui_env_t env = page->m_env;
    plugin_ui_page_slot_t slot;

    slot = TAILQ_FIRST(&env->m_free_page_slots);
    if (slot) {
        TAILQ_REMOVE(&env->m_free_page_slots, slot, m_next);
    }
    else {
        slot = (plugin_ui_page_slot_t)mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_page_slot));
        if (slot == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_page_slot_create: alloc fail");
            return NULL;
        }
    }

    slot->m_page = page;
    cpe_str_dup(slot->m_name, sizeof(slot->m_name), name);
    TAILQ_INIT(&slot->m_bindings);
    
    slot->m_value.m_type = CPE_DR_TYPE_STRING;
    slot->m_value.m_meta = NULL;
    slot->m_value.m_data = slot->m_inline_buf;
    slot->m_value.m_size = 1;
    slot->m_buf = NULL;
    slot->m_inline_buf[0] = 0;
    
    TAILQ_INSERT_TAIL(&page->m_slots, slot, m_next);

    return slot;
}

void plugin_ui_page_slot_free(plugin_ui_page_slot_t slot) {
    plugin_ui_env_t env = slot->m_page->m_env;

    if (slot->m_buf) {
        mem_free(env->m_module->m_alloc, slot->m_buf);
        slot->m_buf = NULL;
    }

    TAILQ_REMOVE(&slot->m_page->m_slots, slot, m_next);

    while(!TAILQ_EMPTY(&slot->m_bindings)) {
        plugin_ui_control_binding_free(TAILQ_FIRST(&slot->m_bindings)->m_binding);
    }

    slot->m_page = (plugin_ui_page_t)((void*)env);
    TAILQ_INSERT_TAIL(&env->m_free_page_slots, slot, m_next);
}

plugin_ui_page_slot_t plugin_ui_page_slot_find(plugin_ui_page_t page, const char * name) {
    plugin_ui_page_slot_t slot;

    TAILQ_FOREACH(slot, &page->m_slots, m_next) {
        if (strcmp(slot->m_name, name) == 0) return slot;
    }

    return NULL;
}

void plugin_ui_page_slot_real_free(plugin_ui_page_slot_t slot) {
    plugin_ui_env_t env = (plugin_ui_env_t)slot->m_page;

    TAILQ_REMOVE(&env->m_free_page_slots, slot, m_next);

    mem_free(env->m_module->m_alloc, slot);
}

const char * plugin_ui_page_slot_name(plugin_ui_page_slot_t slot) {
    return slot->m_name;
}

plugin_ui_page_t plugin_ui_page_slot_page(plugin_ui_page_slot_t slot) {
    return slot->m_page;
}

dr_value_t plugin_ui_page_slot_value(plugin_ui_page_slot_t slot) {
    return &slot->m_value;
}

int plugin_ui_page_slot_set_by_str(plugin_ui_page_slot_t slot, const char * str_value) {
    plugin_ui_env_t env = slot->m_page->m_env;
    
    if (slot->m_value.m_data == slot->m_inline_buf || slot->m_value.m_data == slot->m_buf) {
        size_t len = strlen(str_value) + 1;
        if (len <= sizeof(slot->m_inline_buf)) {
            slot->m_value.m_type = CPE_DR_TYPE_STRING;
            slot->m_value.m_meta = NULL;
            slot->m_value.m_data = slot->m_inline_buf;
            slot->m_value.m_size = len;
            memcpy(slot->m_inline_buf, str_value, len);

            if (slot->m_buf) {
                mem_free(env->m_module->m_alloc, slot->m_buf);
                slot->m_buf = NULL;
            }
        }
        else {
            if (slot->m_buf) {
                mem_free(env->m_module->m_alloc, slot->m_buf);
            }

            slot->m_buf = cpe_str_mem_dup(env->m_module->m_alloc, str_value);
            if (slot->m_buf == NULL) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_page_slot_set_by_str: %s.%s: dup str %s fail",
                    plugin_ui_page_name(slot->m_page), slot->m_name, str_value);

                slot->m_value.m_type = CPE_DR_TYPE_STRING;
                slot->m_value.m_meta = NULL;
                slot->m_value.m_data = slot->m_inline_buf;
                slot->m_value.m_size = 1;
                slot->m_inline_buf[0] = 0;
                
                return -1;
            }

            slot->m_value.m_type = CPE_DR_TYPE_STRING;
            slot->m_value.m_meta = NULL;
            slot->m_value.m_data = slot->m_buf;
            slot->m_value.m_size = strlen(str_value) + 1;
        }
    }
    else {
        if (slot->m_value.m_meta) {
            if (dr_json_read(slot->m_value.m_data, slot->m_value.m_size, str_value, slot->m_value.m_meta, env->m_module->m_em) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_page_slot_set_by_str: %s.%s: set data %s from json %s fail",
                    plugin_ui_page_name(slot->m_page), slot->m_name, dr_meta_name(slot->m_value.m_meta), str_value);
                return -1;
            }
        }
        else {
            if (dr_ctype_set_from_string(slot->m_value.m_data, slot->m_value.m_type, str_value, env->m_module->m_em) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "plugin_ui_page_slot_set_by_str: %s.%s: set %s data from %s fail",
                    plugin_ui_page_name(slot->m_page), slot->m_name, dr_type_name(slot->m_value.m_type), str_value);
                return -1;
            }
        }
    }

    plugin_ui_page_slot_set_binding_need_process(slot);

    return 0;
}

void plugin_ui_page_slot_sync_page_value(plugin_ui_page_slot_t slot) {
    //plugin_ui_page_t page = slot->m_page;
}

void plugin_ui_page_slot_set_binding_need_process(plugin_ui_page_slot_t slot) {
    plugin_ui_control_binding_use_slot_t binding;
    TAILQ_FOREACH(binding, &slot->m_bindings, m_next_for_slot) {
        plugin_ui_control_binding_set_need_process(binding->m_binding, 1);
    }
}
