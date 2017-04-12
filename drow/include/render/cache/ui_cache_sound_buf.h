#ifndef UI_CACHE_SOUND_BUF_H
#define UI_CACHE_SOUND_BUF_H
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_cache_sound_buf_t ui_cache_sound_buf_create(ui_cache_manager_t mgr);
void ui_cache_sound_buf_free(ui_cache_sound_buf_t buf);

uint32_t ui_cache_sound_buf_freq(ui_cache_sound_buf_t buf);
uint32_t ui_cache_sound_buf_bytes_per_sec(ui_cache_sound_buf_t buf);
uint8_t ui_cache_sound_buf_channel(ui_cache_sound_buf_t buf);
ui_cache_sound_data_format_t ui_cache_sound_buf_data_format(ui_cache_sound_buf_t buf);
uint32_t ui_cache_sound_buf_bits_per_sample(ui_cache_sound_buf_t buf);
uint32_t ui_cache_sound_buf_data_size(ui_cache_sound_buf_t buf);
void * ui_cache_sound_buf_data(ui_cache_sound_buf_t buf);
    
/*support call from multi thread*/
int ui_cache_sound_buf_load_from_file(
    ui_cache_sound_buf_t buf, const char * pathname, error_monitor_t em, mem_allocrator_t tmp_alloc);

#ifdef __cplusplus
}
#endif

#endif

