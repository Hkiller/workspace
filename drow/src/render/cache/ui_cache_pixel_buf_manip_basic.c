#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "ui_cache_pixel_format_i.h"
#include "ui_cache_pixel_buf_i.h"

int ui_cache_pixel_buf_rect_is_alpha_zero(uint8_t * result, ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect) {
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + buf->m_format;
    ui_cache_pixel_level_info_t level_info;
    char * data_buf;
    uint32_t row_size;
    uint32_t col_start;
    uint32_t col_end;
    uint32_t row;
    char * row_buf;

    assert(rect);
    assert(buf);

    if (format_info->compressed) return -1;
    if (rect->level >= buf->m_level_count) return -1;

    level_info = &buf->m_level_infos[rect->level];
    data_buf = ui_cache_pixel_buf_level_buf(buf, rect->level);
    row_size = buf->m_stride * level_info->m_width;
    col_start = buf->m_stride * rect->boundary_lt;
    col_end = buf->m_stride * rect->boundary_rt;

    for(row = rect->boundary_tp, row_buf = data_buf + row_size * row;
        row < rect->boundary_bm;
        row++, row_buf += row_size)
    {
        char * row_col_start = row_buf + col_start;
        char * row_col_end = row_buf + col_end;

        while(row_col_start < row_col_end) {
            if ((*(row_col_start + format_info->alpha_start) & format_info->alpha_mask) != 0) {
                *result = 0;
                return 0;
            }
            row_col_start += buf->m_stride;
        }
    }

    *result = 1;
    return 0;
}

int ui_cache_pixel_buf_rect_copy(
    ui_cache_pixel_buf_t to_buf, ui_cache_pixel_buf_rect_t to_rect,
    ui_cache_pixel_buf_t from_buf, ui_cache_pixel_buf_rect_t from_rect,
    error_monitor_t em)
{
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + to_buf->m_format;

    ui_cache_pixel_level_info_t to_level_info;
    char * to_data_buf;
    uint32_t to_col_start;
    uint32_t to_row_size;
    uint32_t to_row;

    ui_cache_pixel_level_info_t from_level_info;
    char * from_data_buf;
    uint32_t from_col_start;
    uint32_t from_row_size;

    uint32_t copy_size;
    
    if (to_buf->m_format != from_buf->m_format) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_copy: format mismatch %d <==> %d", to_buf->m_format, from_buf->m_format);
        return -1;
    }

    if (format_info->compressed) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_copy: format %d is compressed", to_buf->m_format);
        return -1;
    }

    if ((from_rect->boundary_bm - from_rect->boundary_tp) != (to_rect->boundary_bm - to_rect->boundary_tp)) {
        CPE_ERROR(
            em, "ui_cache_pixel_buf_rect_copy: copy height mismatch, %d ==> %d",
            from_rect->boundary_bm - from_rect->boundary_tp,
            to_rect->boundary_bm - to_rect->boundary_tp);
        return -1;
    }

    if ((from_rect->boundary_rt - from_rect->boundary_lt) != (to_rect->boundary_rt - to_rect->boundary_lt)) {
        CPE_ERROR(
            em, "ui_cache_pixel_buf_rect_copy: copy width mismatch, %d ==> %d",
            from_rect->boundary_rt - from_rect->boundary_lt,
            to_rect->boundary_rt - to_rect->boundary_lt);
        return -1;
    }

    copy_size = from_buf->m_stride * (from_rect->boundary_rt - from_rect->boundary_lt);

    to_level_info = &to_buf->m_level_infos[to_rect->level];
    to_data_buf = ui_cache_pixel_buf_level_buf(to_buf, to_rect->level);
    to_row_size = to_buf->m_stride * to_level_info->m_width;
    to_col_start = to_buf->m_stride * to_rect->boundary_lt;
    
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
    from_col_start = from_buf->m_stride * from_rect->boundary_lt;

    for(to_row = to_rect->boundary_tp
            , to_data_buf = to_data_buf + to_row_size * to_rect->boundary_tp
            , from_data_buf = from_data_buf + from_row_size * from_rect->boundary_tp
            ;
        to_row < to_rect->boundary_bm
            ;
        to_row++
            , to_data_buf += to_row_size
            , from_data_buf += from_row_size)
    {
        memcpy(to_data_buf + to_col_start, from_data_buf + from_col_start, copy_size);
    }
    
    return 0;
}

int ui_cache_pixel_buf_rect_copy_from_data(
    ui_cache_pixel_buf_t to_buf, ui_cache_pixel_buf_rect_t to_rect,
    void const * data, size_t data_size, error_monitor_t em)
{
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + to_buf->m_format;

    ui_cache_pixel_level_info_t to_level_info;
    char * to_data_buf;
    uint32_t to_col_start;
    uint32_t to_row_size;
    uint32_t to_row;

    char const * from_data_buf;
    uint32_t copy_row;
    uint32_t copy_col;
    uint32_t copy_line_size;
    
    if (format_info->compressed) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_copy_from_data: format %d is compressed", to_buf->m_format);
        return -1;
    }

    copy_col = to_rect->boundary_rt - to_rect->boundary_lt;
    copy_row = to_rect->boundary_bm - to_rect->boundary_tp;
    copy_line_size = copy_col * format_info->stride;

    if (copy_line_size * copy_row != data_size) {
        CPE_ERROR(
            em, "ui_cache_pixel_buf_rect_copy: copy data size mismatch, width=%d, height=%d, stride=%d, require-size=%d, input-size=%d",
            copy_col, copy_row, format_info->stride, copy_line_size * copy_row, (int)data_size);
        return -1;
    }

    to_level_info = &to_buf->m_level_infos[to_rect->level];
    to_data_buf = ui_cache_pixel_buf_level_buf(to_buf, to_rect->level);
    to_row_size = to_buf->m_stride * to_level_info->m_width;
    to_col_start = to_buf->m_stride * to_rect->boundary_lt;
    
    if (to_rect->boundary_bm > to_level_info->m_height) {
        CPE_ERROR(em, "ui_cache_pixel_buf_to_rect_clear: to_rect bm %d overflow, height=%d", to_rect->boundary_bm, to_level_info->m_height);
        return -1;
    }

    if (to_rect->boundary_rt > to_level_info->m_width) {
        CPE_ERROR(em, "ui_cache_pixel_buf_to_rect_clear: to_rect rt %d overflow, width=%d", to_rect->boundary_rt, to_level_info->m_width);
        return -1;
    }

    for(to_row = to_rect->boundary_tp
            , to_data_buf = to_data_buf + to_row_size * to_rect->boundary_tp
            , from_data_buf = data
            ;
        to_row < to_rect->boundary_bm
            ;
        to_row++
            , to_data_buf += to_row_size
            , from_data_buf += copy_line_size)
    {
        memcpy(to_data_buf + to_col_start, from_data_buf, copy_line_size);
    }
    
    return 0;
}

int ui_cache_pixel_buf_rect_clear(ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect, error_monitor_t em) {
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + buf->m_format;
    ui_cache_pixel_level_info_t level_info;
    char * data_buf;
    uint32_t col_start;
    uint32_t row_size;
    uint32_t row;
    uint32_t clear_size;
    
    if (format_info->compressed) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_clear: format %d is compressed", buf->m_format);
        return -1;
    }

    clear_size = buf->m_stride * (rect->boundary_rt - rect->boundary_lt);

    level_info = &buf->m_level_infos[rect->level];
    data_buf = ui_cache_pixel_buf_level_buf(buf, rect->level);
    row_size = buf->m_stride * level_info->m_width;
    col_start = buf->m_stride * rect->boundary_lt;

    if (rect->boundary_bm > level_info->m_height) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_clear: rect bm %d overflow", rect->boundary_bm);
        return -1;
    }

    if (rect->boundary_rt > level_info->m_width) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_clear: rect rt %d overflow", rect->boundary_rt);
        return -1;
    }

    for(row = rect->boundary_tp, data_buf = data_buf + row_size * rect->boundary_tp;
        row < rect->boundary_bm;
        row++, data_buf += row_size)
    {
        bzero(data_buf + col_start, clear_size);
    }
    
    return 0;
}

int ui_cache_pixel_buf_rect_md5(
    cpe_md5_value_t result, ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect, error_monitor_t em)
{
    struct ui_cache_pixel_format_info * format_info = g_ui_cache_pixel_format_infos + buf->m_format;
    ui_cache_pixel_level_info_t level_info;
    char * data_buf;
    uint32_t col_start;
    uint32_t row_size;
    uint32_t row;
    uint32_t md5_size;
    struct cpe_md5_ctx md5_ctx;

    if (format_info->compressed) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_md5: format %d is compressed", buf->m_format);
        return -1;
    }

    md5_size = buf->m_stride * (rect->boundary_rt - rect->boundary_lt);

    level_info = &buf->m_level_infos[rect->level];
    data_buf = ui_cache_pixel_buf_level_buf(buf, rect->level);
    row_size = buf->m_stride * level_info->m_width;
    col_start = buf->m_stride * rect->boundary_lt;

    if (rect->boundary_bm > level_info->m_height) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_md5: rect bm %d overflow, height=%d", rect->boundary_bm, level_info->m_height);
        return -1;
    }

    if (rect->boundary_rt > level_info->m_width) {
        CPE_ERROR(em, "ui_cache_pixel_buf_rect_md5: rect rt %d overflow, width=%d", rect->boundary_rt, level_info->m_width);
        return -1;
    }

    cpe_md5_ctx_init(&md5_ctx);
    for(row = rect->boundary_tp, data_buf = data_buf + row_size * rect->boundary_tp;
        row < rect->boundary_bm;
        row++, data_buf += row_size)
    {
        cpe_md5_ctx_update(&md5_ctx, data_buf + col_start, md5_size);
    }
    cpe_md5_ctx_final(&md5_ctx);

    *result = md5_ctx.value;
    return 0;
}

void ui_cache_pixel_op_netative(uint8_t * flip_type, uint8_t * angle_type) {
    ui_cache_pixel_op_regular(flip_type, angle_type);

    switch(*flip_type) {
    case ui_cache_pixel_rect_flip_type_none:
        *angle_type = (4 - (*angle_type)) % 4;
        ui_cache_pixel_op_regular(flip_type, angle_type);
        break;;
    case ui_cache_pixel_rect_flip_type_x:
    case ui_cache_pixel_rect_flip_type_y:
        break;
    case ui_cache_pixel_rect_flip_type_xy:
        if(*angle_type == ui_cache_pixel_rect_angle_type_90) {
            *flip_type = ui_cache_pixel_rect_flip_type_none;
        }
        break;
    default:
        assert(0);
        break;
    }
}

void ui_cache_pixel_op_regular(uint8_t * flip_type, uint8_t * angle_type) {
    if (*angle_type == ui_cache_pixel_rect_angle_type_180) {
        *flip_type ^= ui_cache_pixel_rect_flip_type_xy;
    }
    else if (*angle_type == ui_cache_pixel_rect_angle_type_270) {
        *flip_type ^= ui_cache_pixel_rect_flip_type_xy;
        *angle_type = ui_cache_pixel_rect_angle_type_90;
    }
}

uint8_t ui_cache_pixel_op_is_regular(uint8_t flip_type, uint8_t angle_type) {
    return
        (angle_type == ui_cache_pixel_rect_angle_type_none || angle_type == ui_cache_pixel_rect_angle_type_90)
        && flip_type < 4;
}
