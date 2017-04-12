#include <assert.h>
#include <errno.h>
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "convert_ctx.h"

int convert_save_runing(convert_ctx_t ctx) {
    char path_buf[256];
    
    snprintf(path_buf, sizeof(path_buf), "%s/runtime.bin", ctx->m_output);

    return cfg_bin_write_file(ctx->m_runing, gd_app_vfs_mgr(ctx->m_app), path_buf, ctx->m_em);

    /* do { */
    /*     struct vfs_write_stream ws; */
    /*     snprintf(path_buf, sizeof(path_buf), "%s/runtime.yml", ctx->m_output); */
    /*     output_file = vfs_file_open(gd_app_vfs_mgr(ctx->m_app), path_buf, "wb"); */
    /*     vfs_write_stream_init(&ws, output_file); */
    /*     cfg_yaml_write((write_stream_t)&ws, ctx->m_runing, ctx->m_em); */
    /*     vfs_file_close(output_file); */
    /* } while(0); */
}
