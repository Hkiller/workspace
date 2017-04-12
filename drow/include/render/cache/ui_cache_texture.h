#ifndef UI_CACHE_TEXTURE_H
#define UI_CACHE_TEXTURE_H
#include "render/utils/ui_utils_types.h"
#include "ui_cache_res.h"

#ifdef __cplusplus
extern "C" {
#endif
    
int ui_cache_texture_set_summary(ui_cache_res_t res, uint32_t width, uint32_t height, ui_cache_pixel_format_t format);
ui_cache_pixel_format_t ui_cache_texture_format(ui_cache_res_t res);
uint32_t ui_cache_texture_width(ui_cache_res_t res);
uint32_t ui_cache_texture_height(ui_cache_res_t res);
uint32_t ui_cache_texture_memory_size(ui_cache_res_t res);

uint8_t ui_cache_texture_is_dirty(ui_cache_res_t texture);
void ui_cache_texture_set_is_dirty(ui_cache_res_t texture, uint8_t is_dirty);

uint8_t ui_cache_texture_need_part_update(ui_cache_res_t texture);
void ui_cache_texture_set_need_part_update(ui_cache_res_t texture, uint8_t need_part_update);
    
uint8_t ui_cache_texture_keep_data_buf(ui_cache_res_t texture);
void ui_cache_texture_set_keep_data_buf(ui_cache_res_t texture, uint8_t keep_data_buf);    
ui_cache_pixel_buf_t ui_cache_texture_data_buf(ui_cache_res_t texture);
int ui_cache_texture_attach_data_buf(ui_cache_res_t texture, ui_cache_pixel_buf_t data_buf);

ui_cache_res_t
ui_cache_texture_create_with_buff(
    ui_cache_manager_t cache_mgr, ui_cache_pixel_format_t format,
    uint32_t width, uint32_t height, void * data, size_t data_size);

int ui_cache_texture_upload(
    ui_cache_res_t res, ui_rect_t rect, ui_cache_pixel_format_t format, void const * data, size_t data_size);
    
#ifdef __cplusplus
}
#endif

#endif

