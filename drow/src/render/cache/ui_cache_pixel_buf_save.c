#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "gd/app/app_context.h"
#include "ui_cache_pixel_buf_i.h"

int ui_cache_pixel_buf_save_to_file(
    ui_cache_pixel_buf_t buf, const char * pathname, error_monitor_t em, mem_allocrator_t tmp_alloc)
{
    const char * suffix;
    ui_cache_pixel_save_fun_t save_fun;
    vfs_file_t fp;
    struct vfs_write_stream fs;
    int rv;
    
    suffix = file_name_suffix(pathname);

    if (strcasecmp(suffix, "pzd") == 0) {
        save_fun = ui_cache_pixel_save_pzd;
    }
    else if (strcasecmp(suffix, "png") == 0) {
        save_fun = ui_cache_pixel_save_png;
    }
    else {
        CPE_ERROR(em, "pixel save from %s: suffix %s not support!", pathname, suffix);
        return -1;
    }

    fp = vfs_file_open(gd_app_vfs_mgr(buf->m_mgr->m_app), pathname, "wb");
    if (fp == NULL) {
        CPE_ERROR(em, "pixel save to %s: open file fail!", pathname);
        return -1;
    }

    vfs_write_stream_init(&fs, fp);

    rv = save_fun(buf, (write_stream_t)&fs, em, tmp_alloc);
    if (rv != 0) {
        CPE_ERROR(em, "pixel save from %s: save data fail, rv=%d!", pathname, rv);
        vfs_file_close(fp);
        return -1;
    }

    vfs_file_close(fp);
    return 0;
}
