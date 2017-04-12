#include <assert.h>
/* #include "freetype/ftoutln.h" */
/* #include "freetype/fttrigon.h" */
#include "freetype/ftstroke.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
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

static void plugin_layout_font_meta_font_raster_callback(int y, int count, const FT_Span* spans, void* user) {
    plugin_layout_font_meta_t meta = user;
    plugin_layout_font_meta_font_t meta_font = plugin_layout_font_meta_data(meta);
    plugin_layout_module_t module = meta->m_module;
    int i;
    for (i = 0; i < count; ++i) {
        plugin_layout_font_meta_font_spans_add(module, meta_font, spans[i].x, y, spans[i].len, spans[i].coverage);
    }
}

int plugin_layout_font_meta_font_load_element_stroke(plugin_layout_module_t module, plugin_layout_font_element_t element) {
    plugin_layout_font_element_font_t element_font = plugin_layout_font_element_data(element);
    plugin_layout_font_face_font_t face_font = plugin_layout_font_face_data(element->m_face);
    plugin_layout_font_meta_font_t meta_font = plugin_layout_font_meta_data(element->m_face->m_meta);
	FT_Raster_Params params;
    FT_Stroker stroker;
    FT_Glyph stroker_glyph;
    uint16_t normal_span_count;
    uint16_t i;
    ui_rect upload_rect;
    int16_t bitmap_lt_x, bitmap_lt_y, bitmap_rb_x, bitmap_rb_y, bitmap_row_count, bitmap_col_count;
    uint32_t bitmap_buf_size;
    char * bitmap_buf;
    
	bzero(&params, sizeof(FT_Raster_Params));
	params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
	params.gray_spans = plugin_layout_font_meta_font_raster_callback;
	params.user = element->m_face->m_meta;

    /*加载字体信息 */
    if (FT_Load_Char(face_font->m_face, element->m_charter, FT_LOAD_NO_BITMAP) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_meta_font_load_element: load font %s.%d (no bigmap) fail!",
            ui_cache_res_path(face_font->m_font_res), element->m_charter);
        return -1;
    }

    if (face_font->m_face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_meta_font_load_element: load font %s.%d (no bigmap) result is not outline!",
            ui_cache_res_path(face_font->m_font_res), element->m_charter);
        return -1;
    }

    meta_font->m_span_count = 0;
	FT_Outline_Render(meta_font->m_lib, &face_font->m_face->glyph->outline, &params);
    normal_span_count = meta_font->m_span_count;
    
    /* */
    FT_Stroker_New(meta_font->m_lib, &stroker);
    FT_Stroker_Set(stroker, element->m_face->m_id.stroke_width * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    
    if (FT_Get_Glyph(face_font->m_face->glyph, &stroker_glyph) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_load_element: copy border glyph fail!");
        FT_Stroker_Done(stroker);
        return -1;
    }
    
    FT_Glyph_StrokeBorder(&stroker_glyph, stroker, 0, 1);
    assert(stroker_glyph->format == FT_GLYPH_FORMAT_OUTLINE);
    FT_Outline_Render(meta_font->m_lib, &((FT_OutlineGlyph)stroker_glyph)->outline, &params);

    FT_Stroker_Done(stroker);
    FT_Done_Glyph(stroker_glyph);

    if (meta_font->m_span_count <= 0) return 0;
    
    for(i = 0; i < meta_font->m_span_count; ++i) {
        struct plugin_layout_font_meta_font_span * span = &meta_font->m_spans[i];
        int16_t rb_x = span->x + span->width - 1;
        
        if (i == 0) {
            bitmap_lt_x = span->x;
            bitmap_lt_y = span->y;
            bitmap_rb_x = rb_x;
            bitmap_rb_y = span->y;
        }
        else {
            if (span->x < bitmap_lt_x) bitmap_lt_x = span->x;
            if (span->y < bitmap_lt_y) bitmap_lt_y = span->y;
            if (rb_x > bitmap_rb_x) bitmap_rb_x = rb_x;
            if (span->y > bitmap_rb_y) bitmap_rb_y = span->y;
        }
    }
    bitmap_col_count = bitmap_rb_x - bitmap_lt_x + 1;
    bitmap_row_count = bitmap_rb_y - bitmap_lt_y + 1;
    bitmap_buf_size = (uint32_t)bitmap_row_count * (uint32_t)bitmap_col_count;
    
    //printf("xxxxx: load: rect=(%d,%d)-(%d,%d), size=(%d*%d)\n", bitmap_lt_x, bitmap_lt_y, bitmap_rb_x, bitmap_rb_y, bitmap_row_count, bitmap_col_count);

    if (bitmap_row_count <= 0 || bitmap_col_count <= 0) return 0;

    /*分配图片缓存 */
    mem_buffer_clear_data(&module->m_dump_buffer);
    bitmap_buf = mem_buffer_alloc(&module->m_dump_buffer, bitmap_buf_size);
    if (bitmap_buf == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_load_element: upload alloc buff %d fail!", bitmap_buf_size);
        return -1;
    }

    /*加载Normal部分 */
    bzero(bitmap_buf, bitmap_buf_size);
    for(i = 0; i < normal_span_count; ++i) {
        struct plugin_layout_font_meta_font_span * span = &meta_font->m_spans[i];
        uint16_t index = (bitmap_row_count - 1 - (span->y - bitmap_lt_y)) * bitmap_col_count + (span->x - bitmap_lt_x);
        uint16_t w;
        
        for (w = 0; w < span->width; w++) {
            bitmap_buf[index + w] = span->coverage;
        }
    }
    if (plugin_layout_font_cache_insert(
            module->m_font_cache, &upload_rect, bitmap_col_count, bitmap_row_count,
            bitmap_buf, bitmap_buf_size) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_sys_element_create: upload fail(normal)!");
        return -1;
    }
    element->m_texture_rect = upload_rect;
    
    /*加载Stroker部分 */
    bzero(bitmap_buf, bitmap_buf_size);
    for(; i < meta_font->m_span_count; ++i) {
        struct plugin_layout_font_meta_font_span * span = &meta_font->m_spans[i];
        uint16_t index = (bitmap_row_count - 1 - (span->y - bitmap_lt_y)) * bitmap_col_count + span->x - bitmap_lt_x;
        uint16_t w;
        
        for (w = 0; w < span->width; w++) {
            bitmap_buf[index + w] = span->coverage;
        }
    }
    if (plugin_layout_font_cache_insert(module->m_font_cache, &upload_rect, bitmap_col_count, bitmap_row_count, bitmap_buf, bitmap_buf_size) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_font_sys_element_create: upload fail(stroker)!");
        return -1;
    }

    element->m_render_width = ui_rect_width(&upload_rect);
    element->m_render_height = ui_rect_height(&upload_rect);
    element->m_bearing_x = face_font->m_face->glyph->metrics.horiBearingX / 64 - element->m_face->m_id.stroke_width;
    element->m_bearing_y = face_font->m_face->glyph->metrics.horiBearingY / 64 + element->m_face->m_id.stroke_width;
    element_font->m_outline_texture_rect = upload_rect;
    
    return 0;
}

int plugin_layout_font_meta_font_load_element_normal(plugin_layout_module_t module, plugin_layout_font_element_t element) {
    plugin_layout_font_face_font_t face_font = plugin_layout_font_face_data(element->m_face);
    FT_GlyphSlot ft_slot;
    FT_Bitmap ft_bitmap;
    ui_rect upload_rect;

    /*加载字体信息 */
    if (FT_Load_Char(face_font->m_face, element->m_charter, FT_LOAD_RENDER) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_meta_font_load_element: load font %s.%d fail!",
            ui_cache_res_path(face_font->m_font_res), element->m_charter);
        return -1;
    }

    ft_slot = face_font->m_face->glyph;
        
	ft_bitmap = ft_slot->bitmap;
    if (ft_bitmap.width <= 0 || ft_bitmap.rows <= 0) return 0;
    
    if (ft_bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        uint32_t width_in_bytes = (ft_bitmap.width + 7) >> 3;
        uint32_t w, h;
        char * buff;
        size_t buff_size;

        buff_size = ft_bitmap.rows * ft_bitmap.width;
        mem_buffer_clear_data(&module->m_dump_buffer);
        buff = mem_buffer_alloc(&module->m_dump_buffer, buff_size);
        if (buff == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_font_meta_font_load_element: upload alloc buff %d fail!", (int)buff_size);
            return -1;
        }
        
        for (h = 0; h < (uint32_t)ft_bitmap.rows; ++h) {
            for (w = 0; w < width_in_bytes; ++w) {
                uint32_t index  = h * ft_bitmap.width + w;
                uint32_t offset = w & 0x7;

                if (ft_bitmap.buffer[(w >> 3) + width_in_bytes * h] & (128 >> offset)) {
                    buff[index] = 255;
                }
                else {
                    buff[index] = 0;
                }
            }
        }

        if (plugin_layout_font_cache_insert(module->m_font_cache, &upload_rect, ft_bitmap.width, ft_bitmap.rows, buff, buff_size) != 0) {
            CPE_ERROR(module->m_em, "plugin_layout_font_sys_element_create: upload fail!");
            return -1;
        }
    }
    else if (ft_bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        if (plugin_layout_font_cache_insert(module->m_font_cache, &upload_rect, ft_bitmap.width, ft_bitmap.rows, ft_bitmap.buffer, ft_bitmap.rows * ft_bitmap.width) != 0) {
            CPE_ERROR(module->m_em, "plugin_layout_font_sys_element_create: upload(native) fail!");
            return -1;
        }
    }
    else {
        CPE_ERROR(module->m_em, "plugin_layout_font_sys_element_create: not support bitmap format %d!", ft_bitmap.pixel_mode);
        return -1;
    }

    element->m_render_width = ft_slot->bitmap.width;
    element->m_render_height = ft_slot->bitmap.rows;
    element->m_bearing_x = ft_slot->metrics.horiBearingX / 64;
    element->m_bearing_y = ft_slot->metrics.horiBearingY / 64;
    element->m_texture_rect = upload_rect;

    return 0;
}
