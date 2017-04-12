#include <string.h>
#include <assert.h>
#include "cpe/utils/stream_mem.h"
#include "cpe/zip/zip_file.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_types.h"
#include "cfg_internal_ops.h"

struct cfg_zip_read_ctx {
    cfg_policy_t m_policy;
    cfg_t m_curent;
    error_monitor_t m_em;
    struct mem_buffer m_tbuffer;
    struct mem_buffer m_data_buffer;
    struct {
        cfg_t m_node;
    } m_node_stack[CPE_CFG_MAX_LEVEL];
    int m_node_stack_pos;
};

dir_visit_next_op_t cfg_read_zip_on_enter(const char * full, cpe_unzip_dir_t d, void * ctx) {
    struct cfg_zip_read_ctx * readCtx = (struct cfg_zip_read_ctx *)ctx;

    assert(readCtx);
    if (readCtx->m_curent == NULL || cfg_is_value(readCtx->m_curent)) return dir_visit_next_exit;

    ++readCtx->m_node_stack_pos;
    if (readCtx->m_node_stack_pos >= CPE_CFG_MAX_LEVEL) {
        readCtx->m_curent = NULL;
        return dir_visit_next_ignore;
    }

    readCtx->m_node_stack[readCtx->m_node_stack_pos].m_node = readCtx->m_curent;

    if (readCtx->m_curent->m_type == CPE_CFG_TYPE_STRUCT) {
        readCtx->m_curent = cfg_struct_add_struct(readCtx->m_curent, cpe_unzip_dir_name(d), readCtx->m_policy);
    }
    else {
        readCtx->m_curent = cfg_seq_add_struct(readCtx->m_curent);
    }

    return dir_visit_next_go;
}

dir_visit_next_op_t cfg_read_zip_on_leave(const char * full, cpe_unzip_dir_t d, void * ctx) {
    struct cfg_zip_read_ctx * readCtx = (struct cfg_zip_read_ctx *)ctx;

    assert(readCtx);
    if (readCtx->m_curent == NULL) return dir_visit_next_exit;

    if (readCtx->m_node_stack_pos >= 0 && readCtx->m_node_stack_pos < CPE_CFG_MAX_LEVEL) {
        readCtx->m_curent = readCtx->m_node_stack[readCtx->m_node_stack_pos].m_node;
    }
    else {
        readCtx->m_curent = NULL;
    }
    --readCtx->m_node_stack_pos;

    return dir_visit_next_go;
}

dir_visit_next_op_t cfg_read_zip_on_file(const char * full, cpe_unzip_file_t zf, void * ctx) {
    struct cfg_zip_read_ctx * readCtx = (struct cfg_zip_read_ctx *)ctx;
    struct read_stream_mem stream;
    const char * fileSuffix;

    assert(zf);

    if (readCtx->m_curent == NULL || cfg_is_value(readCtx->m_curent)) return dir_visit_next_ignore;

    fileSuffix = file_name_suffix(cpe_unzip_file_name(zf));
    if (strcmp(fileSuffix, "yml") != 0
        && strcmp(fileSuffix, "yaml") != 0
        )
    {
        return dir_visit_next_go;
    }

    CPE_ERROR_SET_FILE(readCtx->m_em, full);

    mem_buffer_clear_data(&readCtx->m_data_buffer);
    if (cpe_unzip_file_load_to_buffer(&readCtx->m_data_buffer, zf, readCtx->m_em) < 0) {
        CPE_ERROR_SET_FILE(readCtx->m_em, NULL);
        return dir_visit_next_go;
    }

    read_stream_mem_init(&stream, mem_buffer_make_continuous(&readCtx->m_data_buffer, 0), mem_buffer_size(&readCtx->m_data_buffer));

    cfg_yaml_read_with_name(
        readCtx->m_curent,
        file_name_base(cpe_unzip_file_name(zf), &readCtx->m_tbuffer),
        (read_stream_t)&stream,
        readCtx->m_policy,
        readCtx->m_em);
    
    CPE_ERROR_SET_FILE(readCtx->m_em, NULL);

    return dir_visit_next_go;
}

static void cfg_read_zip_i(cfg_t cfg, cpe_unzip_dir_t d, cfg_policy_t policy, error_monitor_t em, mem_allocrator_t talloc) {
    struct cfg_zip_read_ctx ctx;
    struct cpe_unzip_file_visitor zipVisitor;

    ctx.m_policy = policy;
    ctx.m_curent = cfg;
    ctx.m_em = em;
    ctx.m_node_stack_pos = -1;

    zipVisitor.on_dir_enter = cfg_read_zip_on_enter;
    zipVisitor.on_dir_leave = cfg_read_zip_on_leave;
    zipVisitor.on_file = cfg_read_zip_on_file;

    mem_buffer_init(&ctx.m_tbuffer, talloc);
    mem_buffer_init(&ctx.m_data_buffer, talloc);

    cpe_unzip_dir_search(&zipVisitor, &ctx, d, -1, em, NULL);

    mem_buffer_clear(&ctx.m_data_buffer);
    mem_buffer_clear(&ctx.m_tbuffer);
}

int cfg_read_zip_dir(cfg_t cfg, cpe_unzip_dir_t d, cfg_policy_t policy, error_monitor_t em, mem_allocrator_t talloc) {
    int ret = 0;
    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        cfg_read_zip_i(cfg, d, policy, em, talloc);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        cfg_read_zip_i(cfg, d, policy, &logError, talloc);
    }

    return ret;
}

int cfg_read_zip_file(cfg_t cfg, cpe_unzip_file_t zf, cfg_policy_t policy, error_monitor_t em) {
    struct mem_buffer buffer;
    struct read_stream_mem stream;
    int rv;

    CPE_ERROR_SET_FILE(em, cpe_unzip_file_name(zf));

    mem_buffer_init(&buffer, NULL);
    
    if (cpe_unzip_file_load_to_buffer(&buffer, zf, em) < 0) {
        CPE_ERROR(em, "read file to buffer fail!");
        CPE_ERROR_SET_FILE(em, NULL);
        return -1;
    }

    read_stream_mem_init(&stream, mem_buffer_make_continuous(&buffer, 0), mem_buffer_size(&buffer));

    rv = cfg_yaml_read(cfg, (read_stream_t)&stream, policy, em);

    mem_buffer_clear(&buffer);
    CPE_ERROR_SET_FILE(em, NULL);

    return rv;
}

int cfg_read_zip_bin_file(cfg_t cfg, cpe_unzip_file_t zf, cfg_policy_t policy, error_monitor_t em) {
    struct mem_buffer buffer;
    int rv;
    ssize_t fileSize;

    CPE_ERROR_SET_FILE(em, cpe_unzip_file_name(zf));

    mem_buffer_init(&buffer, NULL);
    
    fileSize = cpe_unzip_file_load_to_buffer(&buffer, zf, em);
    if (fileSize < 0) {
        CPE_ERROR(em, "read file to buffer fail!");
        CPE_ERROR_SET_FILE(em, NULL);
        return -1;
    }

    rv = cfg_bin_read(cfg, mem_buffer_make_continuous(&buffer, 0), fileSize, em);

    mem_buffer_clear(&buffer);
    CPE_ERROR_SET_FILE(em, NULL);

    return rv;
}
