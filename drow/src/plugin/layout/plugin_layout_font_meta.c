#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_module.h"
#include "plugin_layout_font_meta_i.h"
#include "plugin_layout_font_face_i.h"

plugin_layout_font_meta_t
plugin_layout_font_meta_create(
    plugin_layout_module_t module,
    plugin_layout_font_category_t category,
    const char * name,
    void * ctx,
    uint32_t meta_capacity,
    plugin_layout_font_meta_init_fun_t init_meta,
    plugin_layout_font_meta_fini_fun_t fini_meta,
    plugin_layout_font_meta_on_cache_clear_fun_t on_cache_clear,
    uint32_t face_capacity,
    plugin_layout_font_face_init_fun_t init_face,
    plugin_layout_font_face_fini_fun_t fini_face,
    uint32_t element_capacity,
    plugin_layout_font_element_init_fun_t init_element,
    plugin_layout_font_element_fini_fun_t fini_element,
    plugin_layout_font_element_render_fun_t render_element,
    /*layout*/
    plugin_layout_font_meta_basic_layout_fun_t basic_layout)
{
    plugin_layout_font_meta_t meta;

    if (!TAILQ_EMPTY(&module->m_layouts) || !TAILQ_EMPTY(&module->m_free_layouts)) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_create: already have layout!");
        return NULL;
    }
    
    meta = mem_calloc(module->m_alloc, sizeof(struct plugin_layout_font_meta) + meta_capacity);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_create: alloc fail!");
        return NULL;
    }

    meta->m_module = module;
    meta->m_category = category;
    cpe_str_dup(meta->m_name, sizeof(meta->m_name), name);
    meta->m_face_capacity = face_capacity;
    meta->m_ctx = ctx;
    meta->m_meta_capacity = meta_capacity;
    meta->m_init_meta = init_meta;
    meta->m_fini_meta = fini_meta;
    meta->m_on_cache_clear = on_cache_clear;
    meta->m_face_capacity = face_capacity;
    meta->m_init_face = init_face;
    meta->m_fini_face = fini_face;
    meta->m_element_capacity = element_capacity;
    meta->m_init_element = init_element;
    meta->m_fini_element = fini_element;
    meta->m_render_element = render_element;
    meta->m_basic_layout = basic_layout;
    TAILQ_INIT(&meta->m_faces);

    if (meta->m_init_meta(ctx, meta) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_create: meta %s init fail!", name);
        mem_free(module->m_alloc, meta);
        return NULL;
    }

    if (element_capacity > module->m_max_element_capacity) {
        module->m_max_element_capacity = element_capacity;
    }
    
    TAILQ_INSERT_TAIL(&module->m_font_metas, meta, m_next);
        
    return meta;
}

void plugin_layout_font_meta_free(plugin_layout_font_meta_t meta) {
    plugin_layout_module_t module = meta->m_module;

    while(!TAILQ_EMPTY(&meta->m_faces)) {
        plugin_layout_font_face_free(TAILQ_FIRST(&meta->m_faces));
    }
    
    TAILQ_REMOVE(&module->m_font_metas, meta, m_next);

    meta->m_fini_meta(meta->m_ctx, meta);
    
    mem_free(module->m_alloc, meta);
}

plugin_layout_font_meta_t
plugin_layout_font_meta_find_by_category(plugin_layout_module_t module, plugin_layout_font_category_t category) {
    plugin_layout_font_meta_t meta;

    TAILQ_FOREACH(meta, &module->m_font_metas, m_next) {
        if (meta->m_category == category) return meta;
    }

    return NULL;
}

plugin_layout_font_meta_t
plugin_layout_font_meta_find_by_name(plugin_layout_module_t module, const char * name) {
    plugin_layout_font_meta_t meta;

    TAILQ_FOREACH(meta, &module->m_font_metas, m_next) {
        if (strcmp(meta->m_name, name) == 0) return meta;
    }

    return NULL;
}

void * plugin_layout_font_meta_data(plugin_layout_font_meta_t meta) {
    return meta + 1;
}
