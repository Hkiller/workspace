#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/net/net_listener.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "rank_f_svr_ops.h"

extern char g_metalib_svr_rank_f_pro[];
static void rank_f_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_rank_f_svr = {
    "svr_rank_f_svr",
    rank_f_svr_clear
};

#define RANK_F_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_rank_f_pro, __name); \
    assert(svr-> __arg)

rank_f_svr_t
rank_f_svr_create(gd_app_context_t app, const char * name, set_svr_stub_t stub, mem_allocrator_t alloc, error_monitor_t em) {
    struct rank_f_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct rank_f_svr));
    if (svr_node == NULL) return NULL;

    svr = (rank_f_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;
    svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    svr->m_recv_at = NULL;

    svr->m_data_meta = NULL;
    svr->m_data_size = 0;
    bzero(&svr->m_index_infos, sizeof(svr->m_index_infos));

    RANK_F_SVR_LOAD_META(m_pkg_meta_res_error, "svr_rank_f_res_error");
    RANK_F_SVR_LOAD_META(m_pkg_meta_res_query, "svr_rank_f_res_query");
    RANK_F_SVR_LOAD_META(m_pkg_meta_res_query_with_data, "svr_rank_f_res_query_with_data");

    mem_buffer_init(&svr->m_record_metalib, alloc);
    svr->m_record_meta = NULL;
    svr->m_record_size = 0;
    svr->m_record_mgr = NULL;
    svr->m_record_buf = NULL;

    svr->m_page_size = 4 * 1024 * 1024;

    /*index*/
    svr->m_index_page_count = 0;
    svr->m_index_count = 0;
    svr->m_index_free_count = 0;
    svr->m_index_using_count = 0;

    TAILQ_INIT(&svr->m_indexes_for_check);
    if (cpe_hash_table_init(
            &svr->m_indexes,
            alloc,
            (cpe_hash_fun_t) rank_f_svr_index_hash,
            (cpe_hash_eq_t) rank_f_svr_index_eq,
            CPE_HASH_OBJ2ENTRY(rank_f_svr_index, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    TAILQ_INIT(&svr->m_free_indexes);
    TAILQ_INIT(&svr->m_index_heads);

    /*buf*/
    svr->m_buf_page_count = 0;
    svr->m_buf_count = 0;
    svr->m_buf_free_count = 0;
    svr->m_buf_using_count = 0;

    svr->m_free_bufs = NULL;
    svr->m_buf_heads = NULL;

    nm_node_set_type(svr_node, &s_nm_node_type_rank_f_svr);

    return svr;
}

static void rank_f_svr_clear(nm_node_t node) {
    rank_f_svr_t svr;
    svr = (rank_f_svr_t)nm_node_data(node);

    if (svr->m_recv_at != NULL) {
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);
        assert(timer_mgr);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    /*index*/
    rank_f_svr_index_free_all(svr);
    assert(TAILQ_EMPTY(&svr->m_indexes_for_check));
    assert(TAILQ_EMPTY(&svr->m_free_indexes));
    assert(TAILQ_EMPTY(&svr->m_index_heads));
    cpe_hash_table_fini(&svr->m_indexes);

    /*buf*/
    rank_f_svr_index_buf_release_all(svr);
    assert(svr->m_free_bufs == NULL);
    assert(svr->m_buf_heads == NULL);

    /*record*/
    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_buf) {
        mem_free(svr->m_alloc, svr->m_record_buf);
        svr->m_record_buf = NULL;
    }

    mem_buffer_clear(&svr->m_record_metalib);
}

void rank_f_svr_free(rank_f_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_rank_f_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t rank_f_svr_app(rank_f_svr_t svr) {
    return svr->m_app;
}

rank_f_svr_t
rank_f_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_rank_f_svr) return NULL;
    return (rank_f_svr_t)nm_node_data(node);
}

rank_f_svr_t
rank_f_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_rank_f_svr) return NULL;
    return (rank_f_svr_t)nm_node_data(node);
}

const char * rank_f_svr_name(rank_f_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
rank_f_svr_name_hs(rank_f_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t rank_f_svr_cur_time(rank_f_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int rank_f_svr_request_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int rank_f_svr_set_request_recv_at(rank_f_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.rank_f.request", rank_f_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: rank_f_svr_set_recv_at: create rsp fail!",
            rank_f_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, rank_f_svr_request_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: rank_f_svr_set_recv_at: bind rsp to %s fail!",
            rank_f_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}

void rank_f_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg);
int rank_f_svr_set_check_span(rank_f_svr_t svr, uint32_t span_ms) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);

    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: set check span: timer_mgr not exist!", rank_f_svr_name(svr));
        return -1;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    if (gd_timer_mgr_regist_timer(timer_mgr, &svr->m_check_timer_id, rank_f_svr_timer, svr, NULL, NULL, span_ms, span_ms, -1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set check span: create timer fail!", rank_f_svr_name(svr));
        return -1;
    }

    return 0;
}

int rank_f_svr_record_init_from_mem(rank_f_svr_t svr, size_t memory_size) {
    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_buf) {
        mem_free(svr->m_alloc, svr->m_record_buf);
        svr->m_record_buf = NULL;
    }
    
    svr->m_record_buf = mem_alloc(svr->m_alloc, memory_size);
    if (svr->m_record_buf == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from mem: alloc buf fail, size=%d!",
            rank_f_svr_name(svr), (int)memory_size);
        return -1;
    }

    if (aom_obj_mgr_buf_init(svr->m_record_meta, svr->m_record_buf, memory_size, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em,  "%s: init user data from mem: init buf fail!", rank_f_svr_name(svr));
        return -1;
    }

    svr->m_record_mgr = aom_obj_mgr_create(svr->m_alloc, svr->m_record_buf, memory_size, svr->m_em);
    if (svr->m_record_mgr == NULL) {
        CPE_ERROR(svr->m_em,  "%s: init user data from mem: create aom obj mgr fail!", rank_f_svr_name(svr));
        return -1;
    }

    return 0;
}

int rank_f_svr_record_init_from_shm(rank_f_svr_t svr, int shm_key) {
    cpe_shm_id_t shmid;
    cpe_shmid_ds shm_info;
    void * data;

    if (svr->m_record_mgr) {
        aom_obj_mgr_free(svr->m_record_mgr);
        svr->m_record_mgr = NULL;
    }

    if (svr->m_record_buf) {
        mem_free(svr->m_alloc, svr->m_record_buf);
        svr->m_record_buf = NULL;
    }
    
    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from shm: get shm (key=%d) fail, errno=%d (%s)",
            rank_f_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from shm: get shm info (key=%d) fail, errno=%d (%s)",
            rank_f_svr_name(svr), shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: init user data from shm: attach shm (key=%d, size=%d) fail, errno=%d (%s)",
            rank_f_svr_name(svr), shm_key, shmid, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    svr->m_record_mgr = aom_obj_mgr_create(svr->m_alloc, data, shm_info.shm_segsz, svr->m_em);
    if (svr->m_record_mgr == NULL) {
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init user data from shm: create grp obj mgr (from shm) fail!", rank_f_svr_name(svr));
        return -1;
    }

    if (!dr_meta_compatible(svr->m_record_meta, aom_obj_mgr_meta(svr->m_record_mgr))) {
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init user data from shm: aom grp meta not compatable!", rank_f_svr_name(svr));
        return -1;
    }

    return 0;
}

void rank_f_svr_send_error_response(rank_f_svr_t svr, dp_req_t pkg_head, int16_t error) {
    if (set_pkg_sn(pkg_head)) {
        SVR_RANK_F_RES_ERROR pkg;
        pkg.error = error;

        if (set_svr_stub_send_response_data(
                svr->m_stub,
                set_pkg_from_svr_type(pkg_head), set_pkg_from_svr_id(pkg_head), set_pkg_sn(pkg_head),
                &pkg, sizeof(pkg), svr->m_pkg_meta_res_error,
                NULL, 0)
            != 0)
        {
            CPE_ERROR(svr->m_em, "%s: send error response: send pkg fail!", rank_f_svr_name(svr));
            return;
        }
    }
}

void * rank_f_svr_make_response(dp_req_t * res_body, rank_f_svr_t svr, dp_req_t req_body, size_t more_capacity) {
    SVR_RANK_F_PKG * pkg;
    dp_req_t response_body = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_RANK_F_PKG) + more_capacity);
    if (response_body == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: make response: get response buf fail, capacity=%d!",
            rank_f_svr_name(svr), (int)(sizeof(SVR_RANK_F_PKG) + more_capacity));
        return NULL;
    }

    if (set_pkg_init_response(response_body, req_body) != 0) {
        CPE_ERROR(svr->m_em, "%s: make response: init response fail!", rank_f_svr_name(svr));
        return NULL;
    }

    pkg = dp_req_data(response_body);
    assert(pkg);

    pkg->cmd = ((SVR_RANK_F_PKG *)dp_req_data(req_body))->cmd + 1;

    if (res_body) *res_body = response_body;

    return &pkg->data;
}

uint16_t rank_f_svr_gid_start_pos(rank_f_svr_t svr) {
    rank_f_svr_index_info_t gid_index_info = svr->m_index_infos + 0;

    assert(gid_index_info);
    assert(gid_index_info->m_svr);
    assert(gid_index_info->m_sorter_count == 1);

    return gid_index_info->m_sorters[0].m_data_start_pos;
}
