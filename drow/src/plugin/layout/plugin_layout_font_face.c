#include <assert.h>
#include "render/cache/ui_cache_font.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_meta_i.h"
#include "plugin_layout_font_element_i.h"

plugin_layout_font_face_t
plugin_layout_font_face_create(plugin_layout_module_t module, plugin_layout_font_id_t font_id) {
    plugin_layout_font_face_t face;
    plugin_layout_font_meta_t meta;

    meta = plugin_layout_font_meta_find_by_category(module, font_id->category);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_face_create: face category %d no meta support!", font_id->category);
        return NULL;
    }
    
    face = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_font_face) + meta->m_face_capacity);
    if (face == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_face_create: alloc fail!");
        return NULL;
    }

    face->m_module = module;
    face->m_id = *font_id;
    face->m_meta = meta;
    face->m_height = 0;
    face->m_ascender = 0;
    
    if (cpe_hash_table_init(
            &face->m_elements,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_layout_font_element_hash,
            (cpe_hash_eq_t) plugin_layout_font_element_eq,
            CPE_HASH_OBJ2ENTRY(plugin_layout_font_element, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "plugin_layout_font_face_create: init elements hash table fail!");
        mem_free(module->m_alloc, face);
        return NULL;
    }

    cpe_hash_entry_init(&face->m_hh);
    if (cpe_hash_table_insert(&module->m_font_faces, face) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_face_create: face %s-%d duplicate!",
            face->m_meta->m_name, face->m_id.size);
        cpe_hash_table_fini(&face->m_elements);
        mem_free(module->m_alloc, face);
        return NULL;
    }
    
    if (meta->m_init_face(meta->m_ctx, face) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_face_create: meta %s init face fail!", meta->m_name);
        cpe_hash_table_remove_by_ins(&module->m_font_faces, face);
        cpe_hash_table_fini(&face->m_elements);
        mem_free(module->m_alloc, face);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&meta->m_faces, face, m_next_for_meta);

    return face;
}

void plugin_layout_font_face_free(plugin_layout_font_face_t face) {
    plugin_layout_module_t module = face->m_module;

    plugin_layout_font_element_free_all(face);
    assert(cpe_hash_table_count(&face->m_elements) == 0);
    cpe_hash_table_fini(&face->m_elements);

    face->m_meta->m_fini_face(face->m_meta->m_ctx, face);

    TAILQ_REMOVE(&face->m_meta->m_faces, face, m_next_for_meta);

    cpe_hash_table_remove_by_ins(&module->m_font_faces, face);
    
    mem_free(module->m_alloc, face);
}

plugin_layout_font_face_t
plugin_layout_font_face_find(plugin_layout_module_t module, plugin_layout_font_id_t font_id) {
    struct plugin_layout_font_face key;
    key.m_id = *font_id;
    return cpe_hash_table_find(&module->m_font_faces, &key);
}

plugin_layout_font_face_t
plugin_layout_font_face_check_create(plugin_layout_module_t module, plugin_layout_font_id_t font_id) {
    plugin_layout_font_face_t face;

    face = plugin_layout_font_face_find(module, font_id);
    if (face == NULL) {
        face = plugin_layout_font_face_create(module, font_id);
    }

    return face;
}   

int plugin_layout_font_face_basic_layout(
    plugin_layout_font_face_t face, plugin_layout_render_t render,
    plugin_layout_font_draw_t font_draw, uint32_t const * text, size_t text_len,
    plugin_layout_render_group_t group)
{
    return face->m_meta->m_basic_layout(face->m_meta->m_ctx, face, render, font_draw, text, text_len, group);
}

void * plugin_layout_font_face_data(plugin_layout_font_face_t face) {
    return face + 1;
}

plugin_layout_font_face_t plugin_layout_font_face_from_data(void * data) {
    return ((plugin_layout_font_face_t)data) - 1;
}

uint32_t plugin_layout_font_face_hash(const plugin_layout_font_face_t face) {
    return ((uint32_t)face->m_id.category) << 24
        |  ((uint32_t)face->m_id.face) << 16
        |  ((uint32_t)face->m_id.size) << 8
        |  ((uint32_t)face->m_id.stroke_width);
}

int plugin_layout_font_face_eq(const plugin_layout_font_face_t l, const plugin_layout_font_face_t r) {
    return (l->m_id.category == r->m_id.category
            && l->m_id.face == r->m_id.face
            && l->m_id.size == r->m_id.size
            && l->m_id.stroke_width == r->m_id.stroke_width)
        ? 1
        : 0;
}
