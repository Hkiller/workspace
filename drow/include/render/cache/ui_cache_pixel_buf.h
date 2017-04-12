#ifndef UI_CACHE_PICTURE_H
#define UI_CACHE_PICTURE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_cache_pixel_buf_t ui_cache_pixel_buf_create(ui_cache_manager_t mgr);
void ui_cache_pixel_buf_free(ui_cache_pixel_buf_t buf);

int ui_cache_pixel_buf_pixel_buf_create(
    ui_cache_pixel_buf_t buf,
    uint32_t width, uint32_t height, ui_cache_pixel_format_t format,
    uint32_t mipmap_levels);

void ui_cache_pixel_buf_pixel_buf_destory(ui_cache_pixel_buf_t buf);

ui_cache_pixel_format_t	ui_cache_pixel_buf_format(ui_cache_pixel_buf_t buf);
uint32_t ui_cache_pixel_buf_stride(ui_cache_pixel_buf_t buf);

uint32_t ui_cache_pixel_buf_total_buf_size(ui_cache_pixel_buf_t buf);
void * ui_cache_pixel_buf_pixel_buf(ui_cache_pixel_buf_t buf);

uint32_t ui_cache_pixel_buf_level_count(ui_cache_pixel_buf_t buf);
ui_cache_pixel_level_info_t ui_cache_pixel_buf_level_info_at(ui_cache_pixel_buf_t buf, uint8_t pos);

uint32_t ui_cache_pixel_buf_level_width(ui_cache_pixel_level_info_t level_info);
uint32_t ui_cache_pixel_buf_level_height(ui_cache_pixel_level_info_t level_info);
uint32_t ui_cache_pixel_buf_level_buf_offset(ui_cache_pixel_level_info_t level_info);
uint32_t ui_cache_pixel_buf_level_buf_size(ui_cache_pixel_level_info_t level_info);
void * ui_cache_pixel_buf_level_buf(ui_cache_pixel_buf_t buf, uint32_t level);
    
void * ui_cache_pixel_buf_pixel_at(ui_cache_pixel_buf_t buf, uint32_t level, uint32_t x, uint32_t y);
uint32_t ui_cache_pixel_buf_pixel_value_at(
    ui_cache_pixel_buf_t buf, uint32_t level, uint32_t x, uint32_t y, ui_cache_pixel_field_t field);

const char * ui_cache_pixel_field_to_str(ui_cache_pixel_field_t field);
int ui_cache_pixel_field_from_str(const char * str, ui_cache_pixel_field_t * field);
    
ui_cache_pixel_buf_t ui_cache_pixel_buf_resample(ui_cache_pixel_buf_t pixel_buf, uint8_t scale_level);
    
/*support call from multi thread*/
int ui_cache_pixel_buf_load_from_data(
    ui_cache_pixel_buf_t buf, void const * data, size_t data_capacity, error_monitor_t em, mem_allocrator_t tmp_alloc);
    
int ui_cache_pixel_buf_load_from_file(
    ui_cache_pixel_buf_t buf, const char * pathname, error_monitor_t em, mem_allocrator_t tmp_alloc);

int ui_cache_pixel_buf_save_to_file(
    ui_cache_pixel_buf_t buf, const char * pathname, error_monitor_t em, mem_allocrator_t tmp_alloc);

/**/
void ui_cache_pixel_software_resample(
    uint8_t * dst_data, int dst_width, int dst_height,
    uint8_t * src_data, int src_width, int src_height, int src_pitch, 
    int bytes_per_pixel);
    
#ifdef __cplusplus
}
#endif

#endif
