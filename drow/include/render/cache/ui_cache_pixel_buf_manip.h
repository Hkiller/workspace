#ifndef UI_CACHE_PIXEL_BUF_MANIP_H
#define UI_CACHE_PIXEL_BUF_MANIP_H
#include "cpe/utils/md5.h"
#include "render/utils/ui_rect.h"
#include "ui_cache_pixel_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_pixel_buf_rect {
    uint32_t level;
    uint32_t boundary_lt;
    uint32_t boundary_tp;
    uint32_t boundary_bm;
    uint32_t boundary_rt;
};

int ui_cache_pixel_buf_rect_is_alpha_zero(
    uint8_t * result, ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect);

int ui_cache_pixel_buf_rect_copy(
    ui_cache_pixel_buf_t to_buf, ui_cache_pixel_buf_rect_t to_rect,
    ui_cache_pixel_buf_t from_buf, ui_cache_pixel_buf_rect_t from_rect,
    error_monitor_t em);

int ui_cache_pixel_buf_rect_copy_from_data(
    ui_cache_pixel_buf_t to_buf, ui_cache_pixel_buf_rect_t to_rect,
    void const * data, size_t data_size, error_monitor_t em);
    
int ui_cache_pixel_buf_rect_op(
    ui_cache_pixel_buf_t to_buf, ui_cache_pixel_buf_rect_t to_rect,
    ui_cache_pixel_buf_t from_buf, ui_cache_pixel_buf_rect_t from_rect,
    ui_cache_pixel_rect_flip_type_t flip_type, ui_cache_pixel_rect_angle_type_t angle_type, 
    error_monitor_t em);
    
int ui_cache_pixel_buf_rect_clear(
    ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect, error_monitor_t em);

int ui_cache_pixel_buf_rect_md5(
    cpe_md5_value_t result, ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect, error_monitor_t em);
    
int ui_cache_pixel_buf_rect_op_inline(
    ui_cache_pixel_buf_t buf, ui_cache_pixel_buf_rect_t rect,
    ui_cache_pixel_rect_flip_type_t flip_type, ui_cache_pixel_rect_angle_type_t angle_type, error_monitor_t em);

void ui_cache_pixel_op_netative(uint8_t * flip_type, uint8_t * angle_type);
void ui_cache_pixel_op_regular(uint8_t * flip_type, uint8_t * angle_type);
uint8_t ui_cache_pixel_op_is_regular(uint8_t flip_type, uint8_t angle_type);

#ifdef __cplusplus
}
#endif

#endif

