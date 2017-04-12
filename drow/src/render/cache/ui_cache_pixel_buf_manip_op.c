#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_cache_pixel_format_i.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "ui_cache_pixel_buf_i.h"

#define cache_pixel_buf_rect_op_copy(__from_x, __from_y, __to_x, __to_y) \
    do {                                                                \
        uint8_t swap_c;                                                 \
        char * __from = (from_data_buf + from_row_size * (from_rect->boundary_tp + (__from_y)) \
                         + format_info->stride * (from_rect->boundary_lt + (__from_x))); \
        char * __to = (to_data_buf + to_row_size * (to_rect->boundary_tp + (__to_y)) \
                       + format_info->stride * (to_rect->boundary_lt + (__to_x))); \
        for(swap_c = 0; swap_c < format_info->stride; swap_c++) {       \
            (__to)[swap_c] = (__from)[swap_c];                          \
        }                                                               \
    } while(0)

int ui_cache_pixel_buf_rect_op(
    ui_cache_pixel_buf_t to_buf, ui_cache_pixel_buf_rect_t to_rect,
    ui_cache_pixel_buf_t from_buf, ui_cache_pixel_buf_rect_t from_rect,
    ui_cache_pixel_rect_flip_type_t flip_type, ui_cache_pixel_rect_angle_type_t angle_type, 
    error_monitor_t em)
{
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + to_buf->m_format;

    ui_cache_pixel_level_info_t to_level_info;
    char * to_data_buf;
    uint32_t to_row_size;
    uint32_t to_width = to_rect->boundary_rt - to_rect->boundary_lt;
    uint32_t to_height = to_rect->boundary_bm - to_rect->boundary_tp;

    ui_cache_pixel_level_info_t from_level_info;
    char * from_data_buf;
    uint32_t from_row_size;
    uint32_t from_width = from_rect->boundary_rt - from_rect->boundary_lt;
    uint32_t from_height = from_rect->boundary_bm - from_rect->boundary_tp;
    
    uint32_t x;
    uint32_t y;

    if (to_buf->m_format != from_buf->m_format) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: format mismatch %d <==> %d", to_buf->m_format, from_buf->m_format);
        return -1;
    }

    if (format_info->compressed) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: format %d is compressed", to_buf->m_format);
        return -1;
    }

    if (angle_type == ui_cache_pixel_rect_angle_type_180) {
        flip_type ^= ui_cache_pixel_rect_flip_type_xy;
    }
    else if (angle_type == ui_cache_pixel_rect_angle_type_270) {
        flip_type ^= ui_cache_pixel_rect_flip_type_xy;
        angle_type = ui_cache_pixel_rect_angle_type_90;
    }

    if (angle_type == ui_cache_pixel_rect_angle_type_none) {
        if (from_height != to_height) {
            CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: op height mismatch, %d ==> %d", from_height, to_height);
            return -1;
        }

        if (from_width != to_width) {
            CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: op width mismatch, %d ==> %d", from_width, to_width);
            return -1;
        }
    }
    else {
        if (from_height != to_width) {
            CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: op height mismatch, %d ==> %d", from_height, to_width);
            return -1;
        }

        if (from_width != to_height) {
            CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: op width mismatch, %d ==> %d", from_width, to_height);
            return -1;
        }
    }

    to_level_info = &to_buf->m_level_infos[to_rect->level];
    to_data_buf = ui_cache_pixel_buf_level_buf(to_buf, to_rect->level);
    to_row_size = to_buf->m_stride * to_level_info->m_width;
    
    if (to_rect->boundary_bm > to_level_info->m_height) {
        CPE_ERROR(em, "ui_cache_pixel_buf_to_rect_clear: to_rect bm %d overflow, height=%d", to_rect->boundary_bm, to_level_info->m_height);
        return -1;
    }

    if (to_rect->boundary_rt > to_level_info->m_width) {
        CPE_ERROR(em, "ui_cache_pixel_buf_to_rect_clear: to_rect rt %d overflow, width=%d", to_rect->boundary_rt, to_level_info->m_width);
        return -1;
    }

    from_level_info = &from_buf->m_level_infos[from_rect->level];
    from_data_buf = ui_cache_pixel_buf_level_buf(from_buf, from_rect->level);
    from_row_size = from_buf->m_stride * from_level_info->m_width;

    if (angle_type == ui_cache_pixel_rect_angle_type_none) {
        switch(flip_type) {
        case ui_cache_pixel_rect_flip_type_x:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, from_width - x - 1, y);
                }
            }
            break;
        case ui_cache_pixel_rect_flip_type_y:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, x, from_height - y - 1);
                }
            }
            break;
        case ui_cache_pixel_rect_flip_type_xy:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, from_width - x - 1, from_height - y - 1);
                }
            }
            break;
        default:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, x, y);
                }
            }
            break;
        }
    }
    else {
        switch(flip_type) {
        case ui_cache_pixel_rect_flip_type_x:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, from_height - y - 1, from_width - x - 1);
                }
            }
            break;
        case ui_cache_pixel_rect_flip_type_y:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, y, x);
                }
            }
            break;
        case ui_cache_pixel_rect_flip_type_xy:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, y, from_width - x - 1);
                }
            }
            break;
        default:
            for(x = 0; x < from_width; ++x) {
                for(y = 0; y < from_height; ++y) {
                    cache_pixel_buf_rect_op_copy(x, y, from_height - y - 1, x);
                }
            }
            break;
        }
    }

    return 0;
}
