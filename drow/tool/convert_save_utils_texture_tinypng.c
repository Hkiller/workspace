#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "convert_ctx.h"

extern int ui_cache_pixel_save_png(ui_cache_pixel_buf_t buf, write_stream_t ws, error_monitor_t em, mem_allocrator_t tmp_alloc);
int convert_save_res_tinypng(convert_ctx_t ctx, vfs_file_t output, ui_cache_res_t res, ui_cache_pixel_buf_t res_buf) {
    struct mem_buffer origin_buf;
    struct mem_buffer output_buf;
    struct write_stream_buffer origin_s;
    uint8_t use_cache;
    
    mem_buffer_init(&origin_buf, NULL);
    write_stream_buffer_init(&origin_s, &origin_buf);

    mem_buffer_init(&output_buf, NULL);
    
    if (ui_cache_pixel_save_png(res_buf, (write_stream_t)&origin_s, ctx->m_em, NULL) != 0) {
        CPE_ERROR(ctx->m_em, "tinypng: %s: read origin texture data!", ui_cache_res_path(res));
        goto SAVE_ERROR;
    }

    printf("tinypng: %s: %d ==> ", ui_cache_res_path(res), (int)mem_buffer_size(&origin_buf));
    if (convert_tinypng_convert(ctx, &origin_buf, &output_buf, &use_cache) != 0) goto SAVE_ERROR;

    printf("%d (%.2f%%) done%s\n",
           (int)mem_buffer_size(&output_buf),
           ((float)mem_buffer_size(&output_buf) / (float)mem_buffer_size(&origin_buf)) * 100.0f,
           use_cache ? "(cached)" : "");
    
    if (vfs_file_write(output, mem_buffer_make_continuous(&output_buf, 0), mem_buffer_size(&output_buf)) != mem_buffer_size(&output_buf)) {
        CPE_ERROR(ctx->m_em, "tinypng: %s: save output fail!", ui_cache_res_path(res));
        goto SAVE_ERROR;
    }

    mem_buffer_clear(&origin_buf);
    mem_buffer_clear(&output_buf);
    return 0;

SAVE_ERROR:
    mem_buffer_clear(&origin_buf);
    mem_buffer_clear(&output_buf);
    return -1;
}

