#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_ui_control_meta_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_attr_meta_i.h"
#include "plugin_ui_module_i.h"
#include "plugin_ui_env_i.h"

plugin_ui_control_meta_t
plugin_ui_control_meta_create(
    plugin_ui_module_t module,
    uint8_t type,
    uint32_t data_capacity,
    plugin_ui_control_init_fun_t init_fun,
    plugin_ui_control_fini_fun_t fini_fun,
    plugin_ui_control_load_fun_t load_fun,
    plugin_ui_control_update_fun_t update_fun)
{
    plugin_ui_control_meta_t meta;
    plugin_ui_env_t env;

    TAILQ_FOREACH(env, &module->m_envs, m_next_for_module) {
        if (!TAILQ_EMPTY(&env->m_pages)) {
            CPE_ERROR(module->m_em, "plugin_ui_control_meta_create: control type %d is unknown!", type);
            return NULL;
        }
    }
    
    if (type < UI_CONTROL_TYPE_MIN || type >= UI_CONTROL_TYPE_MAX) {
        CPE_ERROR(module->m_em, "plugin_ui_control_meta_create: control type %d is unknown!", type);
        return NULL;
    }

    meta = module->m_control_metas + (type - UI_CONTROL_TYPE_MIN);
    if (meta->m_type != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_module_register_control: control type %d is already register!", type);
        return NULL;
    }

    bzero(meta, sizeof(*meta));

    meta->m_module = module;
    meta->m_type = type;
    meta->m_product_capacity = data_capacity;
    meta->m_init = init_fun;
    meta->m_fini = fini_fun;
    meta->m_load = load_fun;
    meta->m_update = update_fun;
    TAILQ_INIT(&meta->m_controls);
    
    if (cpe_hash_table_init(
            &meta->m_attr_metas,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_ui_control_attr_meta_hash,
            (cpe_hash_eq_t) plugin_ui_control_attr_meta_eq,
            CPE_HASH_OBJ2ENTRY(plugin_ui_control_attr_meta, m_hh),
            -1) != 0)
    {
        bzero(meta, sizeof(*meta));
        return NULL;
    }

    if (plugin_ui_control_attr_meta_create_basics(meta) != 0) {
        plugin_ui_control_attr_meta_free_all(meta);
        cpe_hash_table_fini(&meta->m_attr_metas);
        bzero(meta, sizeof(*meta));
        return NULL;
    }

    if (data_capacity > module->m_control_max_capacity) {
        module->m_control_max_capacity = data_capacity;
    }

    return meta;
}

void plugin_ui_module_unregister_control(plugin_ui_module_t module, uint8_t type) {
    plugin_ui_control_meta_t meta;
    
    if (type < UI_CONTROL_TYPE_MIN || type >= UI_CONTROL_TYPE_MAX) {
        CPE_ERROR(module->m_em, "plugin_ui_module_unregister_control: control type %d is unknown!", type);
        return;
    }

    meta = module->m_control_metas + (type - UI_CONTROL_TYPE_MIN);

    while(!TAILQ_EMPTY(&meta->m_controls)) {
        plugin_ui_control_free(TAILQ_FIRST(&meta->m_controls));
    }
    
    plugin_ui_control_attr_meta_free_all(meta);
    cpe_hash_table_fini(&meta->m_attr_metas);
    
    bzero(meta, sizeof(*meta));
}

plugin_ui_control_meta_t
plugin_ui_control_meta_find(plugin_ui_module_t module, uint8_t type) {
    plugin_ui_control_meta_t meta;
    
    if (type < UI_CONTROL_TYPE_MIN || type >= UI_CONTROL_TYPE_MAX) {
        CPE_ERROR(module->m_em, "plugin_ui_control_meta_create: control type %d is unknown!", type);
        return NULL;
    }

    meta = module->m_control_metas + (type - UI_CONTROL_TYPE_MIN);
    if (meta->m_type != type) return NULL;

    return meta;
}

void plugin_ui_control_meta_set_layout(plugin_ui_control_meta_t meta, plugin_ui_control_layout_fun_t layout) {
    meta->m_layout = layout;
}

void plugin_ui_control_meta_set_on_self_loaded(plugin_ui_control_meta_t meta, plugin_ui_control_event_fun_t on_self_loaded) {
    meta->m_on_self_loaded = on_self_loaded;
}

void plugin_ui_control_meta_set_event_fun(
    plugin_ui_control_meta_t meta, plugin_ui_event_t evt, plugin_ui_event_scope_t scope, plugin_ui_event_fun_t fun)
{
    struct plugin_ui_control_meta_event_slot * slot;
    
    assert(evt >= plugin_ui_event_min && evt < plugin_ui_event_max);
    
    slot = meta->m_event_slots + (evt - plugin_ui_event_min);
    slot->m_fun = fun;
    slot->m_scope = scope;
}

int plugin_ui_control_meta_buff_init(plugin_ui_module_t module) {
    assert(module->m_control_metas == NULL);

    module->m_control_metas = mem_calloc(module->m_alloc, sizeof(struct plugin_ui_control_meta) * (UI_CONTROL_TYPE_MAX - UI_CONTROL_TYPE_MIN));
    if (module->m_control_metas == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_control_meta_buff_init: alloc fail!");
        return -1;
    }
    
    return 0;
}

void plugin_ui_control_meta_buff_fini(plugin_ui_module_t module) {
    uint8_t type;

    assert(module->m_control_metas);

    for(type = UI_CONTROL_TYPE_MIN; type < UI_CONTROL_TYPE_MAX; ++type) {
        plugin_ui_control_meta_t meta = module->m_control_metas + (type - UI_CONTROL_TYPE_MIN);
        if (meta->m_type) {
            assert(meta->m_type == type);
            plugin_ui_module_unregister_control(module, type);
        }
    }

    mem_free(module->m_alloc, module->m_control_metas);
    module->m_control_metas = NULL;
}
