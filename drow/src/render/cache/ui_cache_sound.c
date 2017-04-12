#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "render/cache/ui_cache_texture.h"
#include "ui_cache_res_i.h"
#include "ui_cache_sound_buf_i.h"

int ui_cache_sound_do_load(ui_cache_manager_t mgr, ui_cache_res_t res, const char * root) {
    char path_buf[256];
    const char * path;

    assert(res->m_res_type == ui_cache_res_type_sound);

    if (res->m_sound.m_data_buff == NULL) {
        res->m_sound.m_data_buff = ui_cache_sound_buf_create(mgr);
        if (res->m_sound.m_data_buff == NULL) {
            CPE_ERROR(mgr->m_em, "sound %s: create sound buf fail", res->m_path);
            res->m_load_result = ui_cache_res_internal_error;
            return -1;
        }
    }
    
    path = ui_cache_res_path(res);
    if (path[0] != '/' && root && root[0] != 0) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s", root, path);
        path = path_buf;
    }

    if (ui_cache_sound_buf_load_from_file(res->m_sound.m_data_buff, path, mgr->m_em, mgr->m_alloc) != 0) {
        CPE_ERROR(mgr->m_em, "sound %s: load from file %s fail", res->m_path, path);
        res->m_load_result = ui_cache_res_internal_error;
        return -1;
    }

    return 0;
}

ui_cache_sound_buf_t ui_cache_sound_get_buf(ui_cache_res_t res) {
    if (res->m_res_type != ui_cache_res_type_sound) return NULL;
    if (res->m_load_state != ui_cache_res_loaded) return NULL;
    return res->m_sound.m_data_buff;
}
