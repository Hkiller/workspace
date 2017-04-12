#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_visitor.h"
#include "spack_builder_i.h"
#include "spack_building_file_i.h"
#include "spack_proto_i.h"

spack_builder_t spack_builder_create(vfs_mgr_t vfs, const char * root, mem_allocrator_t alloc, error_monitor_t em) {
    spack_builder_t builder;
    size_t root_size = root ? (strlen(root) + 1) : 0;
    
    builder = mem_alloc(alloc, sizeof(struct spack_builder) + root_size);
    if (builder == NULL) {
        CPE_ERROR(em, "spack_builder_create: alloc builder fail!");
        return NULL;
    }

    if (root) {
        assert(root_size > 0);
        memcpy(builder + 1, root, root_size);
    }
    
    builder->m_vfs = vfs;
    builder->m_root = root ? (void*)(builder + 1) : NULL;
    builder->m_alloc = alloc;
    builder->m_em = em;

    if (cpe_hash_table_init(
            &builder->m_files,
            alloc,
            (cpe_hash_fun_t) spack_building_file_hash,
            (cpe_hash_eq_t) spack_building_file_eq,
            CPE_HASH_OBJ2ENTRY(spack_building_file, m_hh),
            -1) != 0)
    {
        mem_free(alloc, builder);
        return NULL;
    }

    mem_buffer_init(&builder->m_string_buff, alloc);

    return builder;
}

void spack_builder_free(spack_builder_t builder) {
    spack_building_file_free_all(builder);
    mem_buffer_clear(&builder->m_string_buff);
    mem_free(builder->m_alloc, builder);
}

int spack_builder_add_file(spack_builder_t builder, const char * path) {
    vfs_file_t vfs_file;
    ssize_t data_size;

    if (builder->m_root && !cpe_str_start_with(path, builder->m_root)) return 0;
    
    vfs_file = vfs_file_open(builder->m_vfs, path, "rb");
    if (vfs_file == NULL) {
        CPE_ERROR(builder->m_em, "spack_builder_add: open %s fail!", path);
        return -1;
    }
    
    data_size = vfs_file_size(vfs_file);
    if (data_size < 0) {
        CPE_ERROR(builder->m_em, "spack_builder_add: read %s size fail!", path);
        vfs_file_close(vfs_file);
        return -1;
    }
    vfs_file_close(vfs_file);

    if (spack_building_file_create(builder, mem_buffer_strdup(&builder->m_string_buff, path), (uint32_t)data_size) == NULL) {
        CPE_ERROR(builder->m_em, "spack_builder_add: create file fail!");
        return -1;
    }

    return 0;
}

struct spack_builder_search_ctx {
    spack_builder_t m_builder;
    int m_rv;
};

static vfs_visitor_next_op_t spack_builder_search_on_file(vfs_mgr_t vfs, const char * full, const char * base, void * i_ctx) {
    struct spack_builder_search_ctx * ctx = i_ctx;
    spack_builder_add_file(ctx->m_builder, full);
    return vfs_visitor_next_go;
}

int spack_builder_add(spack_builder_t builder, const char * path) {
    if (vfs_dir_exist(builder->m_vfs, path)) {
        struct vfs_visitor dir_walker;
        struct spack_builder_search_ctx ctx;
        
        dir_walker.on_dir_enter = NULL;
        dir_walker.on_dir_leave = NULL;
        dir_walker.on_file = spack_builder_search_on_file;

        ctx.m_builder = builder;
        ctx.m_rv = 0;
        
        vfs_search_dir(builder->m_vfs, &dir_walker, &ctx, path, 20);
        return ctx.m_rv;
    }
    else {
        return spack_builder_add_file(builder, path);
    }
}

static int spack_builder_append_uint32(spack_builder_t builder, write_stream_t ws, uint32_t value, uint32_t * size) {
    uint32_t buf;
    CPE_COPY_HTON32(&buf, &value);
    if (stream_write(ws, &buf, sizeof(buf)) < sizeof(buf)) {
        CPE_ERROR(builder->m_em, "spack_builder_build: write data fail!");
        return -1;
    }

    *size += sizeof(buf);
    return 0;
}

static int spack_builder_append_string(spack_builder_t builder, write_stream_t ws, const char * value, uint32_t * size) {
    size_t len = strlen(value) + 1;
    if (stream_write(ws, value, len) != len) {
        CPE_ERROR(builder->m_em, "spack_builder_build: write string fail!");
        return -1;
    }

    *size += len;
    return 0;
}

static int spack_builder_append_file(spack_builder_t builder, write_stream_t ws, const char * path, uint32_t * size) {
    vfs_file_t vfs_file;
    char buf[1024];
    size_t total_size;
    
    vfs_file = vfs_file_open(builder->m_vfs, path, "rb");
    if (vfs_file == NULL) {
        CPE_ERROR(builder->m_em, "spack_builder_build: open %s fail!", path);
        return -1;
    }

    total_size = vfs_file_size(vfs_file);
    while(total_size > 0) {
        size_t require_sz = total_size > CPE_ARRAY_SIZE(buf) ? CPE_ARRAY_SIZE(buf) : total_size;
        ssize_t read_sz = vfs_file_read(vfs_file, buf, require_sz);
        if (read_sz < 0) {
            CPE_ERROR(builder->m_em, "spack_builder_build: read %s fail!", path);
            vfs_file_close(vfs_file);
            return -1;
        }
        
        if (stream_write(ws, buf, read_sz) != read_sz) {
            CPE_ERROR(builder->m_em, "spack_builder_build: write data fail!");
            vfs_file_close(vfs_file);
            return -1;
        }

        *size += read_sz;
        total_size -= read_sz;
    }
    
    vfs_file_close(vfs_file);
    return 0;
}

int spack_builder_build(spack_builder_t builder, write_stream_t ws) {
    struct cpe_hash_it file_it;
    spack_building_file_t file;
    uint32_t entry_count = (uint32_t)cpe_hash_table_count(&builder->m_files);
    uint32_t pos = 0;
    uint32_t string_start;
    uint32_t data_start;
    size_t root_len;
    
    if (spack_builder_append_uint32(builder, ws, 0, &pos) != 0) return -1; /*magic*/
    if (spack_builder_append_uint32(builder, ws, entry_count, &pos) != 0) return -1; /*entry count*/

    root_len = builder->m_root ? (strlen(builder->m_root)) + 1 : 0;
    
    /*构造entry列表 */
    /*    预先计算内容快的大小，方便构造文件头 */
    string_start = pos + sizeof(struct spack_entry) * entry_count;
    data_start = string_start;
    cpe_hash_it_init(&file_it, &builder->m_files);
    while((file = cpe_hash_it_next(&file_it))) {
        data_start += strlen(file->m_path) + 1 - root_len;
    }
    cpe_hash_it_init(&file_it, &builder->m_files);
    while((file = cpe_hash_it_next(&file_it))) {
        size_t len = strlen(file->m_path) + 1 - root_len;
        
        if (spack_builder_append_uint32(builder, ws, string_start, &pos) != 0
            || spack_builder_append_uint32(builder, ws, data_start, &pos) != 0
            || spack_builder_append_uint32(builder, ws, file->m_size, &pos) != 0) return -1;
        string_start += len;
        data_start += file->m_size;
    }

    /*构造string列表 */
    cpe_hash_it_init(&file_it, &builder->m_files);
    while((file = cpe_hash_it_next(&file_it))) {
        if (spack_builder_append_string(builder, ws, file->m_path + root_len, &pos) != 0) return -1;
    }
    
    /*构造数据 */
    cpe_hash_it_init(&file_it, &builder->m_files);
    while((file = cpe_hash_it_next(&file_it))) {
        if (spack_builder_append_file(builder, ws, file->m_path, &pos) != 0) return -1;
    }

    return 0;
}
