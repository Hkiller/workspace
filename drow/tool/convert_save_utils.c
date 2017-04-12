#include <errno.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_file.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "convert_ctx.h"

static int convert_copy_src(convert_ctx_t ctx, const char * output, ui_data_src_t src, const char * postfix) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(ctx->m_app);
    struct mem_buffer base_path_buff;
    const char * base_path;
    char from_path[256];
    char to_path[256];
    char * p;

    mem_buffer_init(&base_path_buff, NULL);
    base_path = ui_data_src_path_dump(&base_path_buff, src);
    
    snprintf(from_path, sizeof(from_path), "%s/%s.%s", ctx->m_root, base_path, postfix);
    snprintf(to_path, sizeof(to_path), "%s/%s.%s", output ? output : ctx->m_output, base_path, postfix);

    mem_buffer_clear(&base_path_buff);
    
    if ((p = strrchr(to_path, '/'))) {
        *p = 0;
        if (vfs_dir_mk_recursion(vfs, to_path) != 0) {
            CPE_ERROR(ctx->m_em, "create dir %s fail, errno=%d (%s)!", to_path, errno, strerror(errno));
            return -1;
        }
        *p = '/';
    }

    if (vfs_file_copy(vfs, to_path, from_path) < 0) {
        CPE_ERROR(ctx->m_em, "copy file %s ==> %s fail, errno=%d (%s)!", from_path, to_path, errno, strerror(errno));
        return -1;
    }

    return 0;
}

int convert_save_src(convert_ctx_t ctx, const char * output_path, ui_data_src_t src) {
    if (ui_data_src_type(src) == ui_data_src_type_spine_skeleton) {
        return convert_copy_src(ctx, output_path, src, "spine");
    }
    else if (ui_data_src_type(src) == ui_data_src_type_swf) {
        return convert_copy_src(ctx, output_path, src, "swf");
    }
    else if (ui_data_src_type(src) == ui_data_src_type_dir) {
        return 0;
    }
    else {
        if (ui_data_src_save(src, output_path, ctx->m_em) != 0) {
            CPE_ERROR(
                ctx->m_em, "save src %s %s fail",
                ui_data_src_type_name(ui_data_src_type(src)),
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
            return -1;
        }
        else {
            return 0;
        }
    }
}

static int convert_copy_res(convert_ctx_t ctx, const char * output, ui_cache_res_t res) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(ctx->m_app);
    char from_path[256];
    char to_path[256];    
    char * p;
    
    snprintf(from_path, sizeof(from_path), "%s/%s", ctx->m_root, ui_cache_res_path(res));
    snprintf(to_path, sizeof(to_path), "%s/%s", output, ui_cache_res_path(res));

    if ((p = strrchr(to_path, '/'))) {
        *p = 0;
        if (vfs_dir_mk_recursion(vfs, to_path) != 0) {
            CPE_ERROR(ctx->m_em, "create dir %s fail, errno=%d (%s)!", to_path, errno, strerror(errno));
            return -1;
        }
        *p = '/';
    }

    if (vfs_file_copy(vfs, to_path, from_path) < 0) {
        CPE_ERROR(ctx->m_em, "copy file %s ==> %s fail, errno=%d (%s)!", from_path, to_path, errno, strerror(errno));
        return -1;
    }

    return 0;
}

extern int convert_save_res_pngquant(convert_ctx_t ctx, vfs_file_t output, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf);
extern int convert_save_res_png(convert_ctx_t ctx, vfs_file_t output, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf);
extern int convert_save_res_tinypng(convert_ctx_t ctx, vfs_file_t output, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf);
static int convert_save_res_texture(convert_ctx_t ctx, const char * output, ui_cache_res_t res) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(ctx->m_app);
    ui_cache_pixel_buf_t res_buf;
    uint8_t i;
    char path_buf[512];
    char * sep;
    vfs_file_t output_file;
    char head_buf[32];

    snprintf(path_buf, sizeof(path_buf), "%s/%s", output, ui_cache_res_path(res));
    if ((sep = strrchr(path_buf, '/'))) {
        *sep = 0;
        if (vfs_dir_mk_recursion(vfs, path_buf) != 0) {
            CPE_ERROR(
                ctx->m_em, "convert res: %s: create dir %s fail, errno=%d (%s)!",
                ui_cache_res_path(res), path_buf, errno, strerror(errno));
            return -1;
        }
        *sep = '/';
    }

    if ((sep = strrchr(path_buf, '.'))) {
        snprintf(sep, sizeof(path_buf) - (sep - path_buf), ".pzd");
    }

    output_file = vfs_file_open(vfs, path_buf, "wb");
    if (output_file == NULL) {
        CPE_ERROR(
            ctx->m_em, "convert res: %s: create output file %s fail, errno=%d (%s)!",
            ui_cache_res_path(res), path_buf, errno, strerror(errno));
        return -1;
    }

    for(i = 0; i < CPE_ARRAY_SIZE(head_buf); ++i) {
        head_buf[i] = (char)rand();
    }

    res_buf = ui_cache_texture_data_buf(res);
    if (res_buf == NULL) {
        if (ui_cache_res_load_sync(res, NULL) != 0) {
            CPE_ERROR(ctx->m_em, "convert res: res %s load fail!", ui_cache_res_path(res));
            vfs_file_close(output_file);
            return -1;
        }

        res_buf = ui_cache_texture_data_buf(res);
        if (res_buf == NULL) {
            CPE_ERROR(ctx->m_em, "convert res: res %s load success, no buff!", ui_cache_res_path(res));
            vfs_file_close(output_file);
            return -1;
        }
    }

    if (ctx->m_texture_compress == NULL) {
        if (convert_save_res_png(ctx, output_file, res, res_buf) != 0) {
            vfs_file_close(output_file);
            return -1;
        }
    }
    else if (strcmp(ctx->m_texture_compress, "tinypng") == 0) {
        if (convert_save_res_tinypng(ctx, output_file, res, res_buf) != 0) {
            vfs_file_close(output_file);
            return -1;
        }
    }
    else {
        CPE_ERROR(ctx->m_em, "convert res: unknown texture compress type %s", ctx->m_texture_compress);
        vfs_file_close(output_file);
        return -1;
    }

    vfs_file_close(output_file);
    return 0;
}

int convert_save_res(convert_ctx_t ctx, const char * output, ui_cache_res_t res) {
    if (ui_cache_res_type(res) != ui_cache_res_type_texture) {
        return convert_copy_res(ctx, output, res);
    }
    else {
        return convert_save_res_texture(ctx, output, res);
    }
}


