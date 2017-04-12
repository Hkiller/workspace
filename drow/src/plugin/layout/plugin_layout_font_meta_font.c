#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_font.h"
#include "plugin_layout_font_meta_font_i.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_meta_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_render_i.h"

static int plugin_layout_font_meta_font_init_meta(void * ctx, plugin_layout_font_meta_t meta) {
    plugin_layout_module_t module = ctx;
    plugin_layout_font_meta_font_t meta_font = plugin_layout_font_meta_data(meta);

    if (FT_Init_FreeType(&meta_font->m_lib) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_register: init free type fail!");
        return -1;
    }

    meta_font->m_spans = NULL;
    meta_font->m_span_capacity = 0;
    meta_font->m_span_count = 0;

    return 0;
}

static void plugin_layout_font_meta_font_fini_meta(void * ctx, plugin_layout_font_meta_t meta) {
    plugin_layout_module_t module = ctx;
    plugin_layout_font_meta_font_t meta_font = plugin_layout_font_meta_data(meta);
    FT_Done_FreeType(meta_font->m_lib);

    if (meta_font->m_spans) {
        mem_free(module->m_alloc, meta_font->m_spans);
        meta_font->m_spans = NULL;
    }
}

static void plugin_layout_font_meta_font_on_cache_clear(void * ctx, plugin_layout_font_meta_t meta) {
    plugin_layout_font_face_t face;
    
    TAILQ_FOREACH(face, &meta->m_faces, m_next_for_meta) {
        struct cpe_hash_it element_it;
        plugin_layout_font_element_t element;

        cpe_hash_it_init(&element_it, &face->m_elements);

        while((element = cpe_hash_it_next(&element_it))) {
            plugin_layout_font_element_font_t element_font = plugin_layout_font_element_data(element);

            element_font->m_is_loaded = 0;
            element->m_texture_rect = UI_RECT_ZERO;
            element_font->m_outline_texture_rect = UI_RECT_ZERO;
        }
    }
}

int plugin_layout_font_meta_font_spans_add(plugin_layout_module_t module, plugin_layout_font_meta_font_t meta_font, int16_t x, int16_t y, int16_t width, uint8_t coverage) {
    struct plugin_layout_font_meta_font_span * span;
    
    if (meta_font->m_span_count >= meta_font->m_span_capacity) {
        uint16_t capacity = meta_font->m_span_capacity < 128 ? 128 : meta_font->m_span_capacity * 2;
        struct plugin_layout_font_meta_font_span * buf;

        buf = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_font_meta_font_span) * capacity);
        if (buf == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_font_meta_font: add span: alloc fail, capacity=%d!", capacity);
            return -1;
        }

        if (meta_font->m_span_count) {
            assert(meta_font->m_spans);
            memcpy(buf, meta_font->m_spans, sizeof(struct plugin_layout_font_meta_font_span) * meta_font->m_span_count);
        }

        if (meta_font->m_spans) mem_free(module->m_alloc, meta_font->m_spans);
        meta_font->m_spans = buf;
        meta_font->m_span_capacity = capacity;
    }

    span = &meta_font->m_spans[meta_font->m_span_count++];
    span->x = x;
    span->y = y;
    span->width = width;
    span->coverage = coverage;

    return 0;
}

static int plugin_layout_font_meta_font_init_face(void * ctx, plugin_layout_font_face_t face) {
    plugin_layout_module_t module = ctx;
    plugin_layout_font_meta_font_t meta_font = plugin_layout_font_meta_data(face->m_meta);
    struct plugin_layout_font_face_font * face_font = plugin_layout_font_face_data(face);
    char font_path[64];
    uint32_t font_size;
    
    snprintf(font_path, sizeof(font_path), "sysfont/sysFont_%d.ttf", face->m_id.face + 1);
    face_font->m_font_res = ui_cache_res_find_by_path(module->m_cache_mgr, font_path);
    if (face_font->m_font_res == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_init_face: font res %s not exist", font_path);
        return -1;
    }

    if (ui_cache_res_load_state(face_font->m_font_res) != ui_cache_res_loaded) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_init_face: font res %s not loaded", font_path);
        return -1;
    }

    if (FT_New_Memory_Face(
            meta_font->m_lib,
            (const FT_Byte*)ui_cache_font_data(face_font->m_font_res),
            ui_cache_font_data_size(face_font->m_font_res),
            0, &face_font->m_face) != 0)
    {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_init_face: face %s.%d crate fail", font_path, face->m_id.size);
        return -1;
    }

    font_size = (((uint32_t)face->m_id.size) << 6) - (uint32_t)face->m_id.stroke_width * 2u;
    if (FT_Set_Char_Size(face_font->m_face, 0, font_size, 72, 72) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_init_face: face %s.%d set char size fail", font_path, face->m_id.size);
        FT_Done_Face(face_font->m_face);
        return -1;
    }

    face_font->m_hb_font = hb_ft_font_create(face_font->m_face, NULL);
    if (face_font->m_hb_font == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_init_face: face %s.%d crate hb font fail", font_path, face->m_id.size);
        FT_Done_Face(face_font->m_face);
        return -1;
    }

    face->m_ascender = (face_font->m_face->size->metrics.ascender >> 6) + face->m_id.stroke_width * 1u;
    face->m_height = (face_font->m_face->size->metrics.height >> 6) + face->m_id.stroke_width * 2u;
    
    return 0;
}

static void plugin_layout_font_meta_font_fini_face(void * ctx, plugin_layout_font_face_t face) {
    struct plugin_layout_font_face_font * face_font = plugin_layout_font_face_data(face);
    hb_font_destroy(face_font->m_hb_font);
    FT_Done_Face(face_font->m_face);
}

static int plugin_layout_font_meta_font_init_element(void * ctx, plugin_layout_font_element_t element) {
    plugin_layout_module_t module = ctx;
    plugin_layout_font_element_font_t element_font = plugin_layout_font_element_data(element);
    int rv;
    
    element_font->m_outline_texture_rect = UI_RECT_ZERO;

    rv = element->m_face->m_id.stroke_width > 0
        ? plugin_layout_font_meta_font_load_element_stroke(module, element)
        : plugin_layout_font_meta_font_load_element_normal(module, element);
    if (rv == 0) {
        element_font->m_is_loaded = 1;
    }

    return rv;
}

static void plugin_layout_font_meta_font_fini_element(void * ctx, plugin_layout_font_element_t element) {
}

static void plugin_layout_font_meta_font_render_element(
    void * ctx, plugin_layout_font_element_t element, plugin_layout_font_draw_t font_draw, ui_rect_t target_rect,
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd,
    ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_layout_module_t module = ctx;
    plugin_layout_font_element_font_t element_font = plugin_layout_font_element_data(element);
    
    if (!element_font->m_is_loaded) {
        int rv = 
            element->m_face->m_id.stroke_width > 0
            ? plugin_layout_font_meta_font_load_element_stroke(module, element)
            : plugin_layout_font_meta_font_load_element_normal(module, element);
        
        if (rv != 0) return;

        element_font->m_is_loaded = 1;
    }

    if (element->m_face->m_id.stroke_width > 0 && ui_rect_is_valid(&element_font->m_outline_texture_rect)) {
        plugin_layout_render_render_element(
            target_rect, &element_font->m_outline_texture_rect,
            module->m_font_cache->m_texture, ui_runtime_render_filter_nearest,
            ui_runtime_render_program_buildin_add, &font_draw->stroke_color,
            render, batch_cmd, clip_rect, second_color, transform);
    }
    
    if (ui_rect_is_valid(&element->m_texture_rect)) {
        plugin_layout_render_render_element(
            target_rect, &element->m_texture_rect,
            module->m_font_cache->m_texture, ui_runtime_render_filter_nearest,
            ui_runtime_render_program_buildin_add, &font_draw->color,
            render,  batch_cmd, clip_rect, second_color, transform);
    }
}

int plugin_layout_font_meta_font_register(plugin_layout_module_t module) {
    plugin_layout_font_meta_t meta;

    meta = plugin_layout_font_meta_create(
        module, plugin_layout_font_category_font, "font",
        module,
        sizeof(struct plugin_layout_font_meta_font),
        plugin_layout_font_meta_font_init_meta,
        plugin_layout_font_meta_font_fini_meta,
        plugin_layout_font_meta_font_on_cache_clear,
        sizeof(struct plugin_layout_font_face_font),
        plugin_layout_font_meta_font_init_face,
        plugin_layout_font_meta_font_fini_face,
        sizeof(struct plugin_layout_font_element_font),
        plugin_layout_font_meta_font_init_element,
        plugin_layout_font_meta_font_fini_element,
        plugin_layout_font_meta_font_render_element,
        /*layout*/
        plugin_layout_font_meta_font_basic_layout);

    return meta ? 0 : -1;
}

void plugin_layout_font_meta_font_unregister(plugin_layout_module_t module) {
    plugin_layout_font_meta_t meta;

    meta = plugin_layout_font_meta_find_by_name(module, "font");
    if (meta) {
        plugin_layout_font_meta_free(meta);
    }
}

