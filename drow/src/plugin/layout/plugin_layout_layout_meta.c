#include <assert.h>
#include "gd/app/app_module.h"
#include "plugin_layout_layout_meta_i.h"

plugin_layout_layout_meta_t
plugin_layout_layout_meta_create(
    plugin_layout_module_t module,
    const char * layout_name,
    uint32_t data_capacity,
    plugin_layout_layout_init_fun_t init_fun,
    plugin_layout_layout_fini_fun_t fini_fun,
    plugin_layout_layout_setup_fun_t setup_fun,
    plugin_layout_layout_analize_fun_t analize_fun,
    plugin_layout_layout_layout_fun_t layout_fun,
    plugin_layout_layout_update_fun_t update_fun)
{
    plugin_layout_layout_meta_t meta;
    size_t name_len = strlen(layout_name) + 1;

    if (!TAILQ_EMPTY(&module->m_layouts) || !TAILQ_EMPTY(&module->m_free_layouts)) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_meta_create: already have layout!");
        return NULL;
    }
    
    meta = mem_calloc(module->m_alloc, sizeof(struct plugin_layout_layout_meta) + name_len);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_meta_create: alloc fail!");
        return NULL;
    }

    memcpy(meta + 1, layout_name, name_len);
    
    meta->m_module = module;
    meta->m_name = (void*)(meta + 1);
    meta->m_data_capacity = data_capacity;
    meta->m_init = init_fun;
    meta->m_fini = fini_fun;
    meta->m_setup = setup_fun;
    meta->m_analize = analize_fun;
    meta->m_layout = layout_fun;
    meta->m_update = update_fun;
    
    cpe_hash_entry_init(&meta->m_hh_for_module);
    if (cpe_hash_table_insert(&module->m_layout_metas, meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_meta_create: meta name %s duplicate!", meta->m_name);
        mem_free(module->m_alloc, meta);
        return NULL;
    }

    if (data_capacity > module->m_layout_data_capacity) {
        module->m_layout_data_capacity = data_capacity;
    }
    
    return meta;
}

void plugin_layout_layout_meta_free(plugin_layout_layout_meta_t meta) {
    plugin_layout_module_t module = meta->m_module;
    
    cpe_hash_table_remove_by_ins(&module->m_layout_metas, meta);

    mem_free(module->m_alloc, meta);
}

plugin_layout_layout_meta_t
plugin_layout_layout_meta_find(plugin_layout_module_t module, const char * name) {
    struct plugin_layout_layout_meta key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_layout_metas, &key);
}

void plugin_layout_layout_meta_free_all(const plugin_layout_module_t module) {
    struct cpe_hash_it layout_meta_it;
    plugin_layout_layout_meta_t layout_meta;

    cpe_hash_it_init(&layout_meta_it, &module->m_layout_metas);

    layout_meta = cpe_hash_it_next(&layout_meta_it);
    while (layout_meta) {
        plugin_layout_layout_meta_t next = cpe_hash_it_next(&layout_meta_it);
        plugin_layout_layout_meta_free(layout_meta);
        layout_meta = next;
    }
}

uint32_t plugin_layout_layout_meta_hash(const plugin_layout_layout_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int plugin_layout_layout_meta_eq(const plugin_layout_layout_meta_t l, const plugin_layout_layout_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
