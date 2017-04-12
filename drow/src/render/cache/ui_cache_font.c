#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "cpe/vfs/vfs_file.h"
#include "gd/app/app_context.h"
#include "render/cache/ui_cache_texture.h"
#include "ui_cache_res_i.h"

int ui_cache_font_do_load(ui_cache_manager_t mgr, ui_cache_res_t res, const char * root) {
    char path_buf[256];
    const char * path;
    ssize_t data_size;
    vfs_file_t fp;
    
    assert(res->m_res_type == ui_cache_res_type_font);

    path = ui_cache_res_path(res);
    if (path[0] != '/' && root && root[0] != 0) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s", root, path);
        path = path_buf;
    }


    fp = vfs_file_open(gd_app_vfs_mgr(mgr->m_app), path, "rb");
    if (fp == NULL) {
        CPE_ERROR(mgr->m_em, "font %s: load from file %s fail", res->m_path, path);
        res->m_load_result = ui_cache_res_internal_error;
        return -1;
    }

    data_size = vfs_file_size(fp);
    if (data_size < 0) {
        CPE_ERROR(mgr->m_em, "font %s: get file %s size fail", res->m_path, path);
        res->m_load_result = ui_cache_res_internal_error;
        vfs_file_close(fp);
        return -1;
    }

    if (res->m_font.m_data) {
        mem_free(mgr->m_alloc, res->m_font.m_data);
        res->m_font.m_data_size = 0;
    }

    res->m_font.m_data = mem_alloc(mgr->m_alloc, data_size);
    if (res->m_font.m_data == NULL) {
        CPE_ERROR(mgr->m_em, "font %s: alloc buf(size=%d) fail", res->m_path, (int)data_size);
        res->m_load_result = ui_cache_res_internal_error;
        vfs_file_close(fp);
        return -1;
    }

    if (vfs_file_read(fp, res->m_font.m_data, data_size) != data_size) {
        CPE_ERROR(mgr->m_em, "font %s: read from %s fail", res->m_path, path);
        res->m_load_result = ui_cache_res_internal_error;
        mem_free(mgr->m_alloc, res->m_font.m_data);
        res->m_font.m_data = NULL;
        vfs_file_close(fp);
        return -1;
    }
    res->m_font.m_data_size = (uint32_t)data_size;
    
    vfs_file_close(fp);
    return 0;
}

uint32_t ui_cache_font_data_size(ui_cache_res_t res) {
    assert(res->m_res_type == ui_cache_res_type_font);
    return res->m_font.m_data_size;
}

void * ui_cache_font_data(ui_cache_res_t res) {
    return res->m_font.m_data;
}

