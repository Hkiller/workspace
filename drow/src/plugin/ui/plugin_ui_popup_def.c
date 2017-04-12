#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_popup_def_i.h"
#include "plugin_ui_popup_def_binding_i.h"
#include "plugin_ui_popup_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_page_meta_i.h"
#include "plugin_ui_phase_use_popup_def_i.h"

plugin_ui_popup_def_t
plugin_ui_popup_def_create(plugin_ui_env_t env, const char * popup_def_name) {
    plugin_ui_popup_def_t popup_def;

    popup_def = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_popup_def)  + env->m_backend->popup_def_capacity);
    if (popup_def == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_def_create: alloc fail!");
        return NULL;
    }

    popup_def->m_env = env;
    cpe_str_dup(popup_def->m_name, sizeof(popup_def->m_name), popup_def_name);

    popup_def->m_page_meta = NULL;
    popup_def->m_data_meta = NULL;
    popup_def->m_page_load_from = NULL;
    popup_def->m_layer = 0;
    popup_def->m_lifecircle = 0.0f;
    TAILQ_INIT(&popup_def->m_bindings);
    TAILQ_INIT(&popup_def->m_used_by_phases);

    if (env->m_backend->popup_def_init(env->m_backend->ctx, env, popup_def) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_def_create: backend init popup fail!");
        mem_free(env->m_module->m_alloc, popup_def);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&env->m_popup_defs, popup_def, m_next);

    return popup_def;
}

void plugin_ui_popup_def_free(plugin_ui_popup_def_t popup_def) {
    plugin_ui_env_t env = popup_def->m_env;

    env->m_backend->popup_def_fini(env->m_backend->ctx, env, popup_def);
    
    while(!TAILQ_EMPTY(&popup_def->m_used_by_phases)) {
        plugin_ui_phase_use_popup_def_free(TAILQ_FIRST(&popup_def->m_used_by_phases));
    }

    TAILQ_REMOVE(&env->m_popup_defs, popup_def, m_next);

    while(!TAILQ_EMPTY(&popup_def->m_bindings)) {
        plugin_ui_popup_def_binding_free(TAILQ_FIRST(&popup_def->m_bindings));
    }

    if (popup_def->m_page_load_from) {
        mem_free(env->m_module->m_alloc, popup_def->m_page_load_from);
        popup_def->m_page_load_from = NULL;
    }

    mem_free(env->m_module->m_alloc, popup_def);
}

plugin_ui_popup_def_t plugin_ui_popup_def_find(plugin_ui_env_t env, const char * popup_def_name) {
    plugin_ui_popup_def_t popup_def;

    TAILQ_FOREACH(popup_def, &env->m_popup_defs, m_next) {
        if (strcmp(popup_def->m_name, popup_def_name) == 0) return popup_def;
    }

    return NULL;
}

const char * plugin_ui_popup_def_name(plugin_ui_popup_def_t popup_def) {
    return popup_def->m_name;
}

int16_t plugin_ui_popup_def_layer(plugin_ui_popup_def_t popup_def) {
    return popup_def->m_layer;
}

void plugin_ui_popup_def_set_layer(plugin_ui_popup_def_t popup_def, int16_t layer) {
    popup_def->m_layer = layer;
}

float plugin_ui_popup_def_lifecircle(plugin_ui_popup_def_t popup_def) {
    return popup_def->m_lifecircle;
}

void plugin_ui_popup_def_set_lifecircle(plugin_ui_popup_def_t popup_def, float lifecircle) {
    popup_def->m_lifecircle = lifecircle;
}

plugin_ui_page_meta_t plugin_ui_popup_def_page_meta(plugin_ui_popup_def_t popup_def) {
    return popup_def->m_page_meta;
}

void plugin_ui_popup_def_set_page_meta(plugin_ui_popup_def_t popup_def, plugin_ui_page_meta_t page_meta) {
    popup_def->m_page_meta = page_meta;
}

LPDRMETA plugin_ui_popup_def_data_meta(plugin_ui_popup_def_t popup_def) {
    return popup_def->m_data_meta;
}

void plugin_ui_popup_def_set_data_meta(plugin_ui_popup_def_t popup_def, LPDRMETA data_meta) {
    popup_def->m_data_meta = data_meta;
}

const char * plugin_ui_popup_def_page_load_from(plugin_ui_popup_def_t popup_def) {
    return popup_def->m_page_load_from;
}

int plugin_ui_popup_def_set_page_load_from_by_path(plugin_ui_popup_def_t popup_def, const char * i_load_from) {
    plugin_ui_module_t module = popup_def->m_env->m_module;
    char * load_from;
    const char * endp;

    endp = strrchr(i_load_from, '/');
    if (endp == NULL) endp = i_load_from;
    endp = strchr(endp, '.');
    if (endp == NULL) endp = i_load_from + strlen(i_load_from);

    load_from = cpe_str_mem_dup_range(module->m_alloc, i_load_from, endp);
    if (load_from == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_ui_popup_def_set_page_load_from_by_path: %s: alloc fail!",
            popup_def->m_name);
        return -1;
    }

    if (popup_def->m_page_load_from) {
        mem_free(module->m_alloc, popup_def->m_page_load_from);
    }
    popup_def->m_page_load_from = load_from;
    
    return 0;
}

plugin_ui_popup_t plugin_ui_popup_def_create_popup(plugin_ui_popup_def_t popup_def) {
    plugin_ui_env_t env = popup_def->m_env;
    plugin_ui_popup_t popup;
    plugin_ui_popup_def_binding_t binding;

    popup = plugin_ui_popup_create(env, popup_def->m_name, popup_def->m_page_meta, popup_def->m_data_meta);
    if (popup == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_popup_def_create_popup: %s: create popup fail!",
            popup_def->m_name);
        return NULL;
    }
    popup->m_def = popup_def;

    if (popup_def->m_page_load_from) {
        ui_data_src_t load_from =
            ui_data_src_find_by_path(env->m_module->m_data_mgr, popup_def->m_page_load_from, ui_data_src_type_layout);
        if (load_from == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_popup_def_create_popup: %s: src %s not exist!",
                popup_def->m_name, popup_def->m_page_load_from);
            plugin_ui_popup_free(popup);
            return NULL;
        }

        plugin_ui_page_set_src(popup->m_page, load_from);

        if (plugin_ui_page_load(popup->m_page) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_popup_def_create_popup: %s: load res fail!",
                popup_def->m_name);
            plugin_ui_popup_free(popup);
            return NULL;
        }
    }

    if (popup_def->m_lifecircle) {
        plugin_ui_popup_set_lifecircle(popup, popup_def->m_lifecircle);
    }

    if (popup_def->m_layer) {
        plugin_ui_popup_set_layer(popup, popup_def->m_layer);
    }
    
    TAILQ_FOREACH(binding, &popup_def->m_bindings, m_next) {
        if (plugin_ui_popup_def_binding_apply(binding, popup) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_popup_def_create_popup: %s: apply binding fail!",
                popup_def->m_name);
            plugin_ui_popup_free(popup);
            return NULL;
        }
    }

    return popup;
}

void * plugin_ui_popup_def_data(plugin_ui_popup_def_t popup_def) {
    return popup_def + 1;
}

