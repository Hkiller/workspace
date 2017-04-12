#ifndef UI_CACHE_SOUND_BUF_I_H
#define UI_CACHE_SOUND_BUF_I_H
#include "render/cache/ui_cache_sound_buf.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_sound_buf {
    ui_cache_manager_t m_mgr;
	ui_cache_sound_format_t m_format;

    uint32_t m_freq;
    uint32_t m_bytes_per_sec;
    uint8_t m_channel;
    ui_cache_sound_data_format_t m_data_format;
    uint32_t m_bits_per_sample;
    uint32_t m_data_size;
    void * m_data;
};

typedef int (*ui_cache_sound_load_fun_t)(ui_cache_sound_buf_t buf, vfs_file_t fs, error_monitor_t em, mem_allocrator_t tmp_alloc);
    
int ui_cache_sound_load_ogg(ui_cache_sound_buf_t buf, vfs_file_t fs, error_monitor_t em, mem_allocrator_t tmp_alloc);

#ifdef __cplusplus
}
#endif

#endif
