#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_file.h"
#include "gd/app/app_context.h"
#include "ui_cache_sound_buf_i.h"

int ui_cache_sound_buf_load_from_file(
    ui_cache_sound_buf_t buf, const char * pathname, error_monitor_t em, mem_allocrator_t tmp_alloc)
{
    const char * suffix;
    ui_cache_sound_load_fun_t load_fun;
    vfs_file_t fp;
    int rv;

    if (buf->m_data) {
        CPE_ERROR(em, "sound load from %s: buf is already loaded!", pathname);
        return -1;
    }

    suffix = file_name_suffix(pathname);

    if (strcasecmp(suffix, "ogg") == 0) {
        load_fun = ui_cache_sound_load_ogg;
    }
    else {
        CPE_ERROR(em, "sound load from %s: suffix %s not support!", pathname, suffix);
        return -1;
    }

    fp = vfs_file_open(gd_app_vfs_mgr(buf->m_mgr->m_app), pathname, "rb");
    if (fp == NULL) {
        CPE_ERROR(em, "sound load from %s: open file fail!", pathname);
        return -1;
    }

    rv = load_fun(buf, fp, em, tmp_alloc);
    if (rv != 0) {
        CPE_ERROR(em, "sound load from %s: load data fail, rv=%d!", pathname, rv);
        buf->m_freq = 0;
        buf->m_bytes_per_sec = 0;
        buf->m_channel = 0;
        buf->m_data_format = 0;
        buf->m_bits_per_sample = 0;
        buf->m_data_size = 0;
        if (buf->m_data) mem_free(buf->m_mgr->m_alloc, buf->m_data);
        buf->m_data = NULL;

        vfs_file_close(fp);
        return -1;
    }

    vfs_file_close(fp);
    return 0;
}
