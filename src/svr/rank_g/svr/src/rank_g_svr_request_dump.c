#include <assert.h> 
#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data.h"
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_rank_tree.h"
#include "rank_g_svr_rank_tree_node.h"

static int rank_g_svr_op_dump_records(rank_g_svr_t svr, const char * root_path, const char * app_name) {
    FILE * fp;
    char path_buf[128];
    struct aom_obj_it record_it;
    void * record;

    if (app_name[0]) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s_records.txt", root_path, app_name);
    }
    else {
        snprintf(path_buf, sizeof(path_buf), "%s/records.txt", root_path);
    }
    
    fp = file_stream_open(path_buf, "w", svr->m_em);
    if (fp == NULL) {
        CPE_ERROR(
            svr->m_em, "rank_g_svr_op_dump_records: open file %s fail, rv=%d (%s)!",
            path_buf, errno, strerror(errno));
        return -1;
    }

    aom_objs(svr->m_record_mgr, &record_it);
    while((record = aom_obj_it_next(&record_it))) {
        uint32_t record_idx = (uint32_t)aom_obj_index(svr->m_record_mgr, record);
        fprintf(fp, "%d: %s\n", record_idx, rank_g_svr_record_dump(svr, record));
    }
    
    file_stream_close(fp, svr->m_em);
    
    return 0;
}

static int rank_g_svr_op_dump_index(rank_g_svr_t svr, const char * root_path, const char * app_name, rank_g_svr_index_t index) {
    FILE * fp;
    char path_buf[128];
    rt_node_t node;
    uint32_t i;

    if (app_name[0]) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s_index_%d.txt", root_path, app_name, index->m_id);
    }
    else {
        snprintf(path_buf, sizeof(path_buf), "%s/index_%d.txt", root_path, index->m_id);
    }
    
    fp = file_stream_open(path_buf, "w", svr->m_em);
    if (fp == NULL) {
        CPE_ERROR(
            svr->m_em, "rank_g_svr_op_dump_index: open file %s fail, rv=%d (%s)!",
            path_buf, errno, strerror(errno));
        return -1;
    }

    i = 0;
    for(node = rt_first(index->m_rank_tree); node; node = rt_next(index->m_rank_tree, node)) {
        void * record = aom_obj_get(svr->m_record_mgr, node->m_record_id);
        fprintf(fp, "%d: (value=%d, idx=%d): %s\n", i, node->m_value, node->m_record_id, rank_g_svr_record_dump(svr, record));
        i++;
    }
    
    file_stream_close(fp, svr->m_em);
    
    return 0;
}

logic_op_exec_result_t
rank_g_svr_op_dump_send(logic_context_t ctx, logic_stack_node_t stack, void * user_data, cfg_t cfg) {
    rank_g_svr_t svr = user_data;
    const char * app_name = gd_app_argc(svr->m_app) > 0 ? file_name_no_dir(gd_app_argv(svr->m_app)[0]) : "";
    const char * root_path;
    rank_g_svr_index_t index;
    int rv = 0;

    root_path = gd_app_arg_find(svr->m_app, "--log-dir");
    if (root_path == NULL) {
        CPE_ERROR(svr->m_em, "rank_g_svr_op_dump: no root dir!");
        logic_context_errno_set(ctx, SVR_RANK_G_ERROR_INTERNAL);
        return logic_op_exec_result_false;
    }

    if (rank_g_svr_op_dump_records(svr, root_path, app_name) != 0) {
        rv = -1;
    }

    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        if (rank_g_svr_op_dump_index(svr, root_path, app_name, index) != 0) rv = -1;
    }
    
    if (rv) {
        logic_context_errno_set(ctx, rv);
        return logic_op_exec_result_false;
    }

    return logic_op_exec_result_true;
}
