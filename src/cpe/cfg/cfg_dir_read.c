#include <string.h>
#include <assert.h>
#include "cpe/utils/file.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/vfs/vfs_visitor.h"
#include "cfg_internal_types.h"
#include "cfg_internal_ops.h"

struct cfg_dir_read_ctx {
    cfg_policy_t m_policy;
    cfg_t m_curent;
    error_monitor_t m_em;
    struct mem_buffer m_tbuffer;

    struct {
        cfg_t m_node;
    } m_node_stack[CPE_CFG_MAX_LEVEL];
    int m_node_stack_pos;
};

vfs_visitor_next_op_t cfg_read_dir_on_enter(vfs_mgr_t vfs, const char * full, const char * base, void * ctx) {
    struct cfg_dir_read_ctx * read_ctx = (struct cfg_dir_read_ctx *)ctx;

    assert(read_ctx);
    if (read_ctx->m_curent == NULL || cfg_is_value(read_ctx->m_curent)) return vfs_visitor_next_exit;

    ++read_ctx->m_node_stack_pos;
    if (read_ctx->m_node_stack_pos >= CPE_CFG_MAX_LEVEL) {
        read_ctx->m_curent = NULL;
        return vfs_visitor_next_ignore;
    }

    read_ctx->m_node_stack[read_ctx->m_node_stack_pos].m_node = read_ctx->m_curent;

    if (read_ctx->m_curent->m_type == CPE_CFG_TYPE_STRUCT) {
        read_ctx->m_curent = cfg_struct_add_struct(read_ctx->m_curent, base, read_ctx->m_policy);
    }
    else {
        read_ctx->m_curent = cfg_seq_add_struct(read_ctx->m_curent);
    }

    return vfs_visitor_next_go;
}

vfs_visitor_next_op_t cfg_read_dir_on_leave(vfs_mgr_t vfs, const char * full, const char * base, void * ctx) {
    struct cfg_dir_read_ctx * read_ctx = (struct cfg_dir_read_ctx *)ctx;

    assert(read_ctx);
    if (read_ctx->m_curent == NULL) return vfs_visitor_next_exit;

    if (read_ctx->m_node_stack_pos >= 0 && read_ctx->m_node_stack_pos < CPE_CFG_MAX_LEVEL) {
        read_ctx->m_curent = read_ctx->m_node_stack[read_ctx->m_node_stack_pos].m_node;
    }
    else {
        read_ctx->m_curent = NULL;
    }
    --read_ctx->m_node_stack_pos;

    return vfs_visitor_next_go;
}

vfs_visitor_next_op_t cfg_read_dir_on_file(vfs_mgr_t vfs, const char * full, const char * base, void * ctx) {
    struct cfg_dir_read_ctx * read_ctx = (struct cfg_dir_read_ctx *)ctx;
    struct vfs_read_stream fstream;
    vfs_file_t fp;
    const char * suffix;

    if (read_ctx->m_curent == NULL || cfg_is_value(read_ctx->m_curent)) return vfs_visitor_next_ignore;

    suffix = file_name_suffix(base);

    if (strcmp(suffix, "yml") == 0 || strcmp(suffix, "yaml") == 0) {
        fp = vfs_file_open(vfs, full, "r");
        if (fp == NULL) {
            return vfs_visitor_next_go;
        }

        CPE_ERROR_SET_FILE(read_ctx->m_em, full);

        vfs_read_stream_init(&fstream, fp);

        cfg_yaml_read_with_name(
            read_ctx->m_curent,
            file_name_base(base, &read_ctx->m_tbuffer),
            (read_stream_t)&fstream,
            read_ctx->m_policy,
            read_ctx->m_em);
    
        CPE_ERROR_SET_FILE(read_ctx->m_em, NULL);

        vfs_file_close(fp);

        return vfs_visitor_next_go;
    }
    else if (strcmp(suffix, "json") == 0) {
        fp = vfs_file_open(vfs, full, "r");
        if (fp == NULL) {
            return vfs_visitor_next_go;
        }

        CPE_ERROR_SET_FILE(read_ctx->m_em, full);

        vfs_read_stream_init(&fstream, fp);

        cfg_json_read_with_name(
            read_ctx->m_curent,
            file_name_base(base, &read_ctx->m_tbuffer),
            (read_stream_t)&fstream,
            read_ctx->m_policy,
            read_ctx->m_em);
    
        CPE_ERROR_SET_FILE(read_ctx->m_em, NULL);

        vfs_file_close(fp);

        return vfs_visitor_next_go;
    }
    else {
    }
    
    return vfs_visitor_next_go;
}

static void cfg_read_dir_i(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em, mem_allocrator_t talloc) {
    struct cfg_dir_read_ctx ctx;
    struct vfs_visitor visitor;
    
    ctx.m_policy = policy;
    ctx.m_curent = cfg;
    ctx.m_em = em;
    ctx.m_node_stack_pos = -1;

    visitor.on_dir_enter = cfg_read_dir_on_enter;
    visitor.on_dir_leave = cfg_read_dir_on_leave;
    visitor.on_file = cfg_read_dir_on_file;

    mem_buffer_init(&ctx.m_tbuffer, talloc);
    vfs_search_dir(vfs, &visitor, &ctx, path, -1);
    mem_buffer_clear(&ctx.m_tbuffer);
}

int cfg_read_dir(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em, mem_allocrator_t talloc) {
    int ret = 0;
    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        cfg_read_dir_i(cfg, vfs, path, policy, em, talloc);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        cfg_read_dir_i(cfg, vfs, path, policy, &logError, talloc);
    }

    return ret;
}
