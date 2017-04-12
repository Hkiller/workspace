#include <assert.h>
#include "gd/app/app_module.h"
#include "plugin_ui_page_meta_i.h"
#include "plugin_ui_page_i.h"

plugin_ui_page_meta_t
plugin_ui_page_meta_create(
    plugin_ui_module_t module,
    const char * page_type_name,
    uint32_t data_capacity,
    plugin_ui_page_init_fun_t init_fun,
    plugin_ui_page_fini_fun_t fini_fun)
{
    plugin_ui_page_meta_t meta;
    size_t name_len = strlen(page_type_name) + 1;

    meta = mem_calloc(module->m_alloc, sizeof(struct plugin_ui_page_meta) + name_len);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_page_meta_create: alloc fail!");
        return NULL;
    }

    memcpy(meta + 1, page_type_name, name_len);
    
    meta->m_module = module;
    meta->m_name = (void*)(meta + 1);
    meta->m_data_capacity = data_capacity;
    meta->m_init = init_fun;
    meta->m_fini = fini_fun;
    meta->m_page_count = 0;
    TAILQ_INIT(&meta->m_pages);

    cpe_hash_entry_init(&meta->m_hh_for_module);
    if (cpe_hash_table_insert(&module->m_page_metas, meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_page_meta_create: meta name %s duplicate!", meta->m_name);
        mem_free(module->m_alloc, meta);
        return NULL;
    }

    return meta;
}

void plugin_ui_page_meta_free(plugin_ui_page_meta_t meta) {
    plugin_ui_module_t module = meta->m_module;
    
    while(!TAILQ_EMPTY(&meta->m_pages)) {
        plugin_ui_page_free(TAILQ_FIRST(&meta->m_pages));
    }
    assert(meta->m_page_count == 0);

    cpe_hash_table_remove_by_ins(&module->m_page_metas, meta);

    mem_free(module->m_alloc, meta);
}

plugin_ui_page_meta_t
plugin_ui_page_meta_find(plugin_ui_module_t module, const char * name) {
    struct plugin_ui_page_meta key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_page_metas, &key);
}

void plugin_ui_page_meta_free_all(const plugin_ui_module_t module) {
    struct cpe_hash_it page_meta_it;
    plugin_ui_page_meta_t page_meta;

    cpe_hash_it_init(&page_meta_it, &module->m_page_metas);

    page_meta = cpe_hash_it_next(&page_meta_it);
    while (page_meta) {
        plugin_ui_page_meta_t next = cpe_hash_it_next(&page_meta_it);
        plugin_ui_page_meta_free(page_meta);
        page_meta = next;
    }
}

void plugin_ui_page_meta_set_on_update(plugin_ui_page_meta_t meta, plugin_ui_page_update_fun_t on_update) {
    meta->m_on_update = on_update;
}

void plugin_ui_page_meta_set_on_changed(plugin_ui_page_meta_t meta, plugin_ui_page_event_fun_t on_changed) {
    meta->m_on_changed = on_changed;
}

void plugin_ui_page_meta_set_on_hide(plugin_ui_page_meta_t meta, plugin_ui_page_event_fun_t on_hide) {
    meta->m_on_hide = on_hide;
}

void plugin_ui_page_meta_set_on_load(plugin_ui_page_meta_t meta, plugin_ui_page_event_fun_t on_load) {
    meta->m_on_load = on_load;
}

uint32_t plugin_ui_page_meta_hash(const plugin_ui_page_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int plugin_ui_page_meta_eq(const plugin_ui_page_meta_t l, const plugin_ui_page_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
    
plugin_ui_page_meta_t
plugin_ui_page_meta_load_from_module(plugin_ui_module_t module, const char * type_name) {
    plugin_ui_page_meta_t meta;
    gd_app_module_t page_module = gd_app_install_module(module->m_app, type_name, type_name, NULL, NULL);
    if (page_module == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_page_meta_load_from_module: create module %s fail", type_name);
        return NULL;
    }

    meta = plugin_ui_page_meta_find(module, type_name);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_page_meta_load_from_module: module %s not provider meta", type_name);
        gd_app_uninstall_module(module->m_app, type_name);
        return NULL;
    }

    return meta;
}
