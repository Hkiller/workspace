#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_visitor.h"
#include "convert_ctx.h"

struct convert_load_textures_ctx {
    convert_ctx_t m_ctx;
    int m_rv;
};

static vfs_visitor_next_op_t convert_load_textures_on_file(vfs_mgr_t mgr, const char * full, const char * base, void * input_ctx) {
    struct convert_load_textures_ctx * search_ctx = (struct convert_load_textures_ctx *)input_ctx;
    convert_ctx_t ctx = search_ctx->m_ctx;
    const char * path = full + strlen(ctx->m_root) + 1;
    const char * suffix = file_name_suffix(base);
    ui_cache_res_type_t res_type;
    ui_cache_res_t res;

    if (strcasecmp(suffix, "ogg") == 0) {
        res_type = ui_cache_res_type_sound;
    }
    else if (strcasecmp(suffix, "png") == 0) {
        res_type = ui_cache_res_type_texture;
    }
    else if (strcasecmp(suffix, "ttf") == 0) {
        res_type = ui_cache_res_type_font;
    }
    else {
        return vfs_visitor_next_go;
    }

    res = ui_cache_res_find_by_path(ctx->m_cache_mgr, path);
    if (res) {
        if (ui_cache_res_type(res) != res_type) {
            CPE_INFO(ctx->m_em, "res %s type error, expect %d, but %d!", path, res_type, ui_cache_res_type(res));
        }
        return vfs_visitor_next_go;
    }
    
    res = ui_cache_res_create(ctx->m_cache_mgr, res_type);
    if (res == NULL) {
        CPE_ERROR(ctx->m_em, "create res %s type %d fail!", path, res_type);
        search_ctx->m_rv = -1;
        return vfs_visitor_next_go;
    }

    if (ui_cache_res_set_path(res, path) != 0) {
        CPE_ERROR(ctx->m_em, "set res path to %s fail!", path);
        search_ctx->m_rv = -1;
        return vfs_visitor_next_go;
    }
    
    return vfs_visitor_next_go;
}


int convert_load_textures(convert_ctx_t ctx) {
    struct convert_load_textures_ctx search_ctx = { ctx, 0 };
    struct vfs_visitor dv = { NULL, NULL, convert_load_textures_on_file };
    vfs_search_dir(gd_app_vfs_mgr(ctx->m_app), &dv, &search_ctx, ctx->m_root, 15);
    return search_ctx.m_rv;
}
