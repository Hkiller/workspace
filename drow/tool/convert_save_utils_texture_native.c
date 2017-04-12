#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/vfs/vfs_stream.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "convert_ctx.h"

extern int ui_cache_pixel_save_png(ui_cache_pixel_buf_t buf, write_stream_t ws, error_monitor_t em, mem_allocrator_t tmp_alloc);
int convert_save_res_png(convert_ctx_t ctx, vfs_file_t output, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf) {
    struct vfs_write_stream fs = VFS_WRITE_STREAM_INITIALIZER(output);
    int rv;

    rv = ui_cache_pixel_save_png(res_buf, (write_stream_t)&fs, ctx->m_em, NULL);
    if (rv != 0) {
        CPE_ERROR(ctx->m_em, "convert save res png: save png fail, rv=%d!", rv);
        return -1;
    }

    return 0;
}
