#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/logic_use/logic_op_register.h"
#include "svr/set/share/set_pkg.h"
#include "friend_svr_ops.h"

extern char g_metalib_svr_friend_pro[];
static void friend_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_friend_svr = {
    "svr_friend_svr",
    friend_svr_clear
};

struct logic_op_register_def g_friend_ops[] = {
    { "friend_op_query", 
      friend_svr_op_query_send,
      friend_svr_op_query_recv }
    , { "friend_op_query_data", 
      friend_svr_op_query_data_send,
      friend_svr_op_query_data_recv }
    , { "friend_op_add", 
      friend_svr_op_add_send,
      friend_svr_op_add_recv }
    , { "friend_op_remove", 
      friend_svr_op_remove_send,
      friend_svr_op_remove_recv }
    , { "friend_op_ack", 
      friend_svr_op_ack_send,
      friend_svr_op_ack_recv }
    , { "friend_op_sync", 
      friend_svr_op_sync_send,
      friend_svr_op_sync_recv }
};

#define FRIEND_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_friend_pro, __name); \
    assert(svr-> __arg)

friend_svr_t
friend_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct friend_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct friend_svr));
    if (svr_node == NULL) return NULL;

    svr = (friend_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_set_sp = set_sp;
    svr->m_rsp_manage = rsp_manage;
    svr->m_db = db;
    svr->m_debug = 0;
    svr->m_runing_mode = friend_svr_runing_mode_one_way;

    mem_buffer_init(&svr->m_record_metalib, alloc);
    svr->m_record_meta = NULL;
    svr->m_record_size = 0;
    svr->m_record_id_entry = NULL;
    svr->m_record_id_start_pos = 0;
    svr->m_record_id_capacity = 0;
    svr->m_record_uid_entry = NULL;
    svr->m_record_uid_start_pos = 0;
    svr->m_record_fuid_entry = NULL;
    svr->m_record_fuid_start_pos = 0;
    svr->m_record_list_meta = NULL;
    svr->m_record_data_start_pos = 0;
    svr->m_record_list_count_entry = NULL;
    svr->m_record_list_data_entry = NULL;

    svr->m_data_meta = NULL;
    svr->m_data_fuid_entry = NULL;
    svr->m_data_fuid_start_pos = 0;
    svr->m_data_size = 0;

    FRIEND_SVR_LOAD_META(m_meta_op_add_ctx, "svr_friend_op_add_ctx");

    FRIEND_SVR_LOAD_META(m_meta_res_query, "svr_friend_res_query");

    svr->m_op_register = logic_op_register_create(app, NULL, alloc, em);
    if (svr->m_op_register == NULL) {
        CPE_ERROR(em, "%s: create: create op_register fail!", name);
        mem_buffer_clear(&svr->m_record_metalib);
        nm_node_free(svr_node);
        return NULL;
    }

    if (logic_op_register_create_ops(
            svr->m_op_register,
            sizeof(g_friend_ops) / sizeof(g_friend_ops[0]), 
            g_friend_ops,
            svr) != 0)
    {
        CPE_ERROR(em, "%s: create: register friend ops fail!", name);
        mem_buffer_clear(&svr->m_record_metalib);
        logic_op_register_free(svr->m_op_register);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_friend_svr);

    return svr;
}

static void friend_svr_clear(nm_node_t node) {
    friend_svr_t svr;
    svr = (friend_svr_t)nm_node_data(node);

    if (svr->m_op_register) {
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
    }

    mem_buffer_clear(&svr->m_record_metalib);
}

void friend_svr_free(friend_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_friend_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t friend_svr_app(friend_svr_t svr) {
    return svr->m_app;
}

friend_svr_t
friend_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_friend_svr) return NULL;
    return (friend_svr_t)nm_node_data(node);
}

friend_svr_t
friend_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_friend_svr) return NULL;
    return (friend_svr_t)nm_node_data(node);
}

const char * friend_svr_name(friend_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
friend_svr_name_hs(friend_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t friend_svr_cur_time(friend_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

uint64_t friend_svr_record_fuid(friend_svr_t svr, void * record) {
    uint64_t fuid;
    int r = dr_entry_try_read_uint64(&fuid, ((char *)record) + svr->m_record_fuid_start_pos, svr->m_record_fuid_entry, svr->m_em);
    assert(r == 0);
    return fuid;
}

void friend_svr_record_set_fuid(friend_svr_t svr, uint64_t fuid,void * record) {
    int r = dr_entry_set_from_uint64((char*)record + svr->m_record_fuid_start_pos, fuid, svr->m_record_fuid_entry, svr->m_em);
    assert(r == 0);
}

uint64_t friend_svr_record_uid(friend_svr_t svr, void * record) {
    return ((SVR_FRIEND_RECORD *)record)->user_id;
    /* uint64_t uid; */
    /* int r = dr_entry_try_read_uint64(&uid, ((char *)record) + svr->m_record_uid_start_pos, svr->m_record_uid_entry, svr->m_em); */
    /* assert(r == 0); */
    /* return uid; */
}

void friend_svr_record_set_uid(friend_svr_t svr, uint64_t uid,void * record) {
    ((SVR_FRIEND_RECORD *)record)->user_id = uid;
    /* int r = dr_entry_set_from_uint64((char *)record + svr->m_record_uid_start_pos, uid, svr->m_record_uid_entry, svr->m_em); */
    /* assert(r == 0); */
}

uint8_t friend_svr_record_state(friend_svr_t svr, void * record) {
    return ((SVR_FRIEND_RECORD *)record)->state;
}

void friend_svr_record_set_state(friend_svr_t svr, uint8_t state, void * record) {
    ((SVR_FRIEND_RECORD *)record)->state = state;
}

int friend_svr_record_build_id(friend_svr_t svr, void * record) {
    uint64_t uid = friend_svr_record_uid(svr, record);
    uint64_t fuid = friend_svr_record_fuid(svr, record);

    if (uid == 0) {
        CPE_ERROR(svr->m_em, "%s: set_record_id: uid is 0!", friend_svr_name(svr));
        return -1;
    }

    if (fuid == 0) {
        CPE_ERROR(svr->m_em, "%s: set_record_id: fuid is 0!", friend_svr_name(svr));
        return -1;
    }

    snprintf(
        ((char *)record) + svr->m_record_id_start_pos,
        svr->m_record_id_capacity,
        FMT_UINT64_T"-"FMT_UINT64_T, uid, fuid);

    return 0;
}
