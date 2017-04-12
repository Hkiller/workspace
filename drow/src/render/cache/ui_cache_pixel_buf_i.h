#ifndef UI_CACHE_PIXEL_BUF_I_H
#define UI_CACHE_PIXEL_BUF_I_H
#include "render/cache/ui_cache_pixel_buf.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_pixel_level_info {
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_buf_offset;
    uint32_t m_buf_size;
};
    
struct ui_cache_pixel_buf {
    ui_cache_manager_t m_mgr;

	ui_cache_pixel_format_t	m_format;
	uint32_t m_stride;
    uint32_t m_level_count;
    struct ui_cache_pixel_level_info m_level_infos[16];

    uint32_t m_total_buf_size;
	void * m_pixel_buf;
};

typedef int (*ui_cache_pixel_load_fun_t)(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
typedef int (*ui_cache_pixel_save_fun_t)(ui_cache_pixel_buf_t buf, write_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
    
int ui_cache_pixel_load_pzd(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
int ui_cache_pixel_save_pzd(ui_cache_pixel_buf_t buf, write_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
    
int ui_cache_pixel_load_png(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
int ui_cache_pixel_save_png(ui_cache_pixel_buf_t buf, write_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);

int ui_cache_pixel_load_jpg(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
int ui_cache_pixel_save_jpg(ui_cache_pixel_buf_t buf, write_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc);
    
#ifdef __cplusplus
}
#endif

#endif
