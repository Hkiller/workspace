#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "gd/app/app_context.h"
#include "ui_cache_pixel_buf_i.h"

int ui_cache_pixel_buf_load_from_file(
    ui_cache_pixel_buf_t buf, const char * pathname, error_monitor_t em, mem_allocrator_t tmp_alloc)
{
    const char * suffix;
    ui_cache_pixel_load_fun_t load_fun;
    vfs_file_t fp;
    struct vfs_read_stream fs;
    int rv;

    suffix = file_name_suffix(pathname);

    if (strcasecmp(suffix, "pzd") == 0) {
        load_fun = ui_cache_pixel_load_png;
    }
    else if (strcasecmp(suffix, "png") == 0) {
        load_fun = ui_cache_pixel_load_png;
    }
    else {
        CPE_ERROR(em, "pixel load from %s: suffix %s not support!", pathname, suffix);
        return -1;
    }

    fp = vfs_file_open(gd_app_vfs_mgr(buf->m_mgr->m_app), pathname, "rb");
    if (fp == NULL) {
        CPE_ERROR(em, "pixel load from %s: open file fail, errno=%d (%s)!", pathname, errno, strerror(errno));
        return -1;
    }

    vfs_read_stream_init(&fs, fp);

    rv = load_fun(buf, (read_stream_t)&fs, em, tmp_alloc);
    if (rv != 0) {
        CPE_ERROR(em, "pixel load from %s: load data fail, rv=%d!", pathname, rv);
        vfs_file_close(fp);
        return -1;
    }

    vfs_file_close(fp);
    return 0;
}

int ui_cache_pixel_buf_load_from_data(
    ui_cache_pixel_buf_t buf, void const * i_data, size_t data_capacity, error_monitor_t em, mem_allocrator_t tmp_alloc)
{
    ui_cache_pixel_load_fun_t load_fun;
    struct read_stream_mem rs = CPE_READ_STREAM_MEM_INITIALIZER(i_data, data_capacity);
    const unsigned char * data = i_data;
    int rv;

    if (data_capacity >= 4 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') {
        load_fun = ui_cache_pixel_load_png;
    }
    else if (data_capacity >= 11
             && data[0] == 0xff && data[1] == 0xd8 /*tart of Image (SOI) marker -- two bytes (FFD8)*/
             && data[2] == 0xff && data[3] == 0xe0 /*JFIF marker (FFE0)*/
             && data[6] == 'J' && data[7] == 'F' && data[8] == 'I' && data[9] == 'F' && data[10] == 0 /*JFIF*/
        )
    {
        load_fun = ui_cache_pixel_load_jpg;
    }
    else {
        CPE_ERROR(em, "pixel load from data: format not support!");
        return -1;
    }

    rv = load_fun(buf, (read_stream_t)&rs, em, tmp_alloc);
    if (rv != 0) {
        CPE_ERROR(em, "pixel load from data: load data fail, rv=%d!", rv);
        return -1;
    }

    return 0;
}
