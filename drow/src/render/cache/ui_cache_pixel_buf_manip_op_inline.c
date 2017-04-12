#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_cache_pixel_format_i.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "ui_cache_pixel_buf_i.h"

#define cache_pixel_buf_rect_op_swap(__l, __r)          \
    for(swap_c = 0; swap_c < buf->m_stride; swap_c++) { \
        (__l)[swap_c] ^= (__r)[swap_c];                 \
        (__r)[swap_c] ^= (__l)[swap_c];                 \
        (__l)[swap_c] ^= (__r)[swap_c];                 \
    }

#define cache_pixel_buf_rect_op_copy(__to, __from)      \
    for(swap_c = 0; swap_c < buf->m_stride; swap_c++) { \
        (__to)[swap_c] = (__from)[swap_c];              \
    }

#define cache_pixel_buf_rect_op_rotate(__p0, __p1, __p2, __p3)   \
    do {                                                         \
    char rotate_buf[8];                                          \
    cache_pixel_buf_rect_op_copy(rotate_buf, (__p3));            \
    cache_pixel_buf_rect_op_copy((__p3), (__p2));                \
    cache_pixel_buf_rect_op_copy((__p2), (__p1));                \
    cache_pixel_buf_rect_op_copy((__p1), (__p0));                \
    cache_pixel_buf_rect_op_copy((__p0), rotate_buf);            \
    } while(0)


#define cache_pixel_buf_rect_point_pos(__x, __y)        \
    (data_buf + row_size * (rect->boundary_tp + (__y)) + buf->m_stride * (rect->boundary_lt + (__x)))

int ui_cache_pixel_buf_rect_op_inline(
    ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect,
    ui_cache_pixel_rect_flip_type_t flip_type, ui_cache_pixel_rect_angle_type_t angle_type, error_monitor_t em)
{
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + buf->m_format;
    ui_cache_pixel_level_info_t level_info;
    char * data_buf;
    uint32_t col_start;
    uint32_t col_end;
    uint32_t row_size;
    uint32_t op_row;
    uint32_t op_col;
    uint32_t op_width;
    uint32_t op_height;
    char * swap_l; char * swap_r; uint8_t swap_c;

    if (angle_type == ui_cache_pixel_rect_angle_type_180) {
        flip_type ^= ui_cache_pixel_rect_flip_type_xy;
    }
    else if (angle_type == ui_cache_pixel_rect_angle_type_270) {
        flip_type ^= ui_cache_pixel_rect_flip_type_xy;
        angle_type = ui_cache_pixel_rect_angle_type_90;
    }

    if (format_info->compressed) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: format %d is compressed", buf->m_format);
        return -1;
    }

    if (rect->level >= buf->m_level_count) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: level %d overflow, level-count=%d", rect->level, buf->m_level_count);
        return -1;
    }

    level_info = &buf->m_level_infos[rect->level];

    if (rect->boundary_bm > level_info->m_height) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: rect bm %d overflow", rect->boundary_bm);
        return -1;
    }

    if (rect->boundary_rt > level_info->m_width) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_op: rect rt %d overflow", rect->boundary_rt);
        return -1;
    }

    op_width = rect->boundary_rt - rect->boundary_lt;
    op_height = rect->boundary_bm - rect->boundary_tp;

    if (op_width != op_height && angle_type != ui_cache_pixel_rect_angle_type_none) {
        CPE_ERROR(
            em, "ui_cache_pixel_buf_rect_op: rect %d-%d can`t rotate",
            rect->boundary_rt - rect->boundary_lt,
            rect->boundary_bm - rect->boundary_tp);
        return -1;
    }

    data_buf = ui_cache_pixel_buf_level_buf(buf, rect->level);
    row_size = buf->m_stride * level_info->m_width;
    col_start = buf->m_stride * rect->boundary_lt;
    col_end = buf->m_stride * rect->boundary_rt;

    if (angle_type == ui_cache_pixel_rect_angle_type_none) {
        switch(flip_type) {
        case ui_cache_pixel_rect_flip_type_x: {
            char * row_data_begin;
            for(op_row = 0, row_data_begin = data_buf + row_size * rect->boundary_tp;
                op_row < op_height;
                op_row++, row_data_begin += row_size)
            {
                swap_l = row_data_begin + col_start;
                swap_r = row_data_begin + col_end - buf->m_stride;

                while(swap_l < swap_r) {
                    cache_pixel_buf_rect_op_swap(swap_l, swap_r);
                    swap_l += buf->m_stride;
                    swap_r -= buf->m_stride;
                }
            }
            break;
        }
        case ui_cache_pixel_rect_flip_type_y: {
            char * col_data_begin;
            uint32_t col_diff = row_size * (op_height - 1);

            for(op_col = 0, col_data_begin = data_buf + row_size * rect->boundary_tp + col_start;
                op_col < op_width;
                op_col++, col_data_begin += buf->m_stride)
            {
                swap_l = col_data_begin;
                swap_r = col_data_begin + col_diff;

                while(swap_l < swap_r) {
                    cache_pixel_buf_rect_op_swap(swap_l, swap_r);
                    swap_l += row_size;
                    swap_r -= row_size;
                }
            }
            break;
        }
        case ui_cache_pixel_rect_flip_type_xy: {
            char * row_data_begin;
            uint32_t half_height = op_height / 2;

            for(op_row = 0, row_data_begin = data_buf + row_size * rect->boundary_tp;
                op_row < half_height;
                op_row++, row_data_begin += row_size)
            {
                swap_l = row_data_begin + col_start;
                swap_r = row_data_begin + row_size * (op_height - op_row * 2) - buf->m_stride;

                for(op_col = 0; op_col < op_width; ++op_col) {
                    cache_pixel_buf_rect_op_swap(swap_l, swap_r);
                    swap_l += buf->m_stride;
                    swap_r -= buf->m_stride;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    else {
        uint32_t circle;
        uint32_t circle_count = op_height / 2;
        char * p0, *p1, *p2, *p3;

        switch(flip_type) {
        case ui_cache_pixel_rect_flip_type_x: {
            uint32_t i;

            for(circle = 0; circle < circle_count; ++circle) {
                uint32_t side_size = op_height - 2 * circle;
                uint32_t op_end = circle + side_size - 1;

                assert(2 * circle < op_height);

                for(i = 0; i < side_size - 1; ++i) {
                    swap_l = cache_pixel_buf_rect_point_pos(circle + i, circle);
                    swap_r = cache_pixel_buf_rect_point_pos(op_end, op_end - i);
                    cache_pixel_buf_rect_op_swap(swap_l, swap_r);
                }

                for(i = 1; i < side_size - 1; ++i) {
                    swap_l = cache_pixel_buf_rect_point_pos(circle, circle + i);
                    swap_r = cache_pixel_buf_rect_point_pos(op_end - i, op_end);
                    cache_pixel_buf_rect_op_swap(swap_l, swap_r);
                }
            }
            break;
        }
        case ui_cache_pixel_rect_flip_type_y: {
            uint32_t i;

            for(circle = 0; circle < circle_count; ++circle) {
                uint32_t side_size = op_height - 2 * circle;
                uint32_t op_end = circle + side_size - 1;

                assert(2 * circle < op_height);

                for(i = 1; i < side_size; ++i) {
                    p0 = cache_pixel_buf_rect_point_pos(circle + i, circle);
                    p1 = cache_pixel_buf_rect_point_pos(circle, circle + i);
                    cache_pixel_buf_rect_op_swap(p0, p1); 
                }

                for(i = 1; i < side_size - 1; ++i) {
                    p2 = cache_pixel_buf_rect_point_pos(op_end - i, op_end);
                    p3 = cache_pixel_buf_rect_point_pos(op_end, op_end - i);
                    cache_pixel_buf_rect_op_swap(p2, p3);
                }
            }
            break;
        }
        case ui_cache_pixel_rect_flip_type_xy: {
            uint32_t i;

            for(circle = 0; circle < circle_count; ++circle) {
                uint32_t side_size = op_height - 2 * circle;
                uint32_t op_count = side_size - 1;
                uint32_t op_end = circle + side_size - 1;

                assert(2 * circle < op_height);

                for(i = 0; i < op_count; ++i) {
                    p0 = cache_pixel_buf_rect_point_pos(circle + i, circle);
                    p1 = cache_pixel_buf_rect_point_pos(op_end, circle + i);
                    p2 = cache_pixel_buf_rect_point_pos(op_end - i, op_end);
                    p3 = cache_pixel_buf_rect_point_pos(circle, op_end - i);
                    cache_pixel_buf_rect_op_rotate(p3, p2, p1, p0);                    
                }
            }
            break;
        }
        default: {
            uint32_t i;

            for(circle = 0; circle < circle_count; ++circle) {
                uint32_t side_size = op_height - 2 * circle;
                uint32_t op_count = side_size - 1;
                uint32_t op_end = circle + side_size - 1;

                assert(2 * circle < op_height);

                for(i = 0; i < op_count; ++i) {
                    p0 = cache_pixel_buf_rect_point_pos(circle + i, circle);
                    p1 = cache_pixel_buf_rect_point_pos(op_end, circle + i);
                    p2 = cache_pixel_buf_rect_point_pos(op_end - i, op_end);
                    p3 = cache_pixel_buf_rect_point_pos(circle, op_end - i);
                    cache_pixel_buf_rect_op_rotate(p0, p1, p2, p3);                    
                }
            }
            break;
        }
        }
    }

    return 0;
}
