#include "plugin_layout_animation_meta_i.h"
#include "plugin_layout_animation_i.h"

plugin_layout_animation_meta_t
plugin_layout_animation_meta_create(
    plugin_layout_module_t module,
    const char * type_name,
    void * ctx,
    /*animation*/
    size_t anim_capacity,
    plugin_layout_animation_init_fun_t init_fun,
    plugin_layout_animation_free_fun_t fini_fun,
    plugin_layout_animation_layout_fun_t layout_fun,
    plugin_layout_animation_update_fun_t update_fun,
    plugin_layout_animation_render_fun_t render_fun)
{
    plugin_layout_animation_meta_t meta;

    meta = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_animation_meta));
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_animation_type: alloc fail!");
        return NULL;
    }

    meta->m_module = module;
    meta->m_name = type_name;
    meta->m_anim_capacity = anim_capacity;
    meta->m_ctx = ctx;
    meta->m_init_fun = init_fun;
    meta->m_fini_fun = fini_fun;
    meta->m_layout_fun = layout_fun;
    meta->m_update_fun = update_fun;
    meta->m_render_fun = render_fun;
    TAILQ_INIT(&meta->m_animations);
    
    cpe_hash_entry_init(&meta->m_hh);
    if (cpe_hash_table_insert(&module->m_animation_metas, meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_animation_meta_create: meta name %s duplicate!", meta->m_name);
        mem_free(module->m_alloc, meta);
        return NULL;
    }

    if (anim_capacity > module->m_animation_max_capacity) {
        module->m_animation_max_capacity = anim_capacity;
    }

    return meta;
}

void plugin_layout_animation_meta_free(plugin_layout_animation_meta_t meta) {
    plugin_layout_module_t module = meta->m_module;

    cpe_hash_table_remove_by_ins(&module->m_animation_metas, meta);

    while(!TAILQ_EMPTY(&meta->m_animations)) {
        plugin_layout_animation_free(TAILQ_FIRST(&meta->m_animations));
    }

    mem_free(module->m_alloc, meta);
}

uint32_t plugin_layout_animation_meta_hash(const plugin_layout_animation_meta_t meta) {
    return cpe_hash_str(meta->m_name, strlen(meta->m_name));
}

int plugin_layout_animation_meta_eq(const plugin_layout_animation_meta_t l, const plugin_layout_animation_meta_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

plugin_layout_animation_meta_t
plugin_layout_animation_meta_find(plugin_layout_module_t module, const char * name) {
    struct plugin_layout_animation_meta key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_animation_metas, &key);
}

void plugin_layout_animation_meta_free_all(const plugin_layout_module_t module) {
    struct cpe_hash_it animation_meta_it;
    plugin_layout_animation_meta_t animation_meta;

    cpe_hash_it_init(&animation_meta_it, &module->m_animation_metas);

    animation_meta = cpe_hash_it_next(&animation_meta_it);
    while (animation_meta) {
        plugin_layout_animation_meta_t next = cpe_hash_it_next(&animation_meta_it);
        plugin_layout_animation_meta_free(animation_meta);
        animation_meta = next;
    }
}

