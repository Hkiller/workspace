#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/vfs/vfs_builder.h"
#include "cpe/spack/spack_builder.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "convert_ctx.h"

const char * convert_save_package_spack_prepaire(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx)
{
    const char * output_path;
    vfs_builder_t builder;
    
    assert(builder_ctx);
    assert(*builder_ctx == NULL);

    mem_buffer_clear_data(path_buffer);
    mem_buffer_printf(path_buffer, "%s/packages/%s", ctx->m_output, plugin_package_package_name(package));
    output_path = mem_buffer_make_continuous(path_buffer, 0);

    builder = vfs_builder_create(gd_app_vfs_mgr(ctx->m_app), output_path);
    if (builder == NULL) {
        CPE_ERROR(ctx->m_em, "save pachage %s: mkdir %s fail", plugin_package_package_name(package), output_path);
        return NULL;
    }

    *builder_ctx = builder;
    
    return output_path;
}

static int convert_save_package_spack_build(
    convert_ctx_t ctx, vfs_builder_t builder, mem_buffer_t path_buffer, plugin_package_package_t package)
{
    spack_builder_t spacker;
    vfs_mgr_t vfs = gd_app_vfs_mgr(ctx->m_app);
    char * output_path;
    char * sep;
    vfs_file_t fp;
    struct vfs_write_stream fs;
    
    mem_buffer_clear_data(path_buffer);
    mem_buffer_printf(path_buffer, "%s/packages/%s.spack", ctx->m_output, plugin_package_package_name(package));
    output_path = mem_buffer_make_continuous(path_buffer, 0);

    sep = strrchr(output_path, '.');
    *sep = 0;
    spacker = spack_builder_create(vfs, output_path, ctx->m_alloc, ctx->m_em);
    if (spacker == NULL) {
        CPE_ERROR(ctx->m_em, "convert_save_package_spack_commit: create packer fail!");
        return -1;
    }

    if (spack_builder_add(spacker, vfs_builder_path(builder)) != 0) {
        CPE_ERROR(ctx->m_em, "convert_save_package_spack_commit: add path %s fail!", vfs_builder_path(builder));
        spack_builder_free(spacker);
        return -1;
    }
    *sep = '.';
    
    sep = strrchr(output_path, '/');
    *sep = 0;
    if (vfs_dir_mk_recursion(vfs, output_path) != 0) {
        CPE_ERROR(ctx->m_em, "convert_save_package_spack_commit: create output dir %s fail!", output_path);
        return -1;
    }
    *sep = '/';

    fp = vfs_file_open(vfs, output_path, "wb");
    if (fp == NULL) {
        CPE_ERROR(ctx->m_em, "convert_save_package_spack_commit: open result file %s fail!", output_path);
        spack_builder_free(spacker);
        return -1;
    }

    vfs_write_stream_init(&fs, fp);
    spack_builder_build(spacker, (write_stream_t)&fs);

    spack_builder_free(spacker);
    vfs_file_close(fp);

    return 0;
}

int convert_save_package_spack_commit(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx, uint8_t success)
{
    vfs_builder_t builder;
    
    assert(builder_ctx);

    if (*builder_ctx == NULL) return 0;

    builder = *builder_ctx;

    if (success) {
        convert_save_package_spack_build(ctx, builder, path_buffer, package);
    }
    
    vfs_builder_free(builder);
    *builder_ctx = NULL;
    
    return 0;
}
