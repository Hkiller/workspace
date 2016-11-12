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
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/logic_use/logic_op_register.h"
#include "svr/set/share/set_pkg.h"
#include "mail_svr_ops.h"

extern char g_metalib_svr_mail_pro[];
static void mail_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_mail_svr = {
    "svr_mail_svr",
    mail_svr_clear
};

struct logic_op_register_def g_mail_ops[] = {
    { "mail_op_send_mail",
      mail_svr_op_send_send,
      mail_svr_op_send_recv }
    , { "mail_op_remove_mail",
      mail_svr_op_remove_send,
      mail_svr_op_remove_recv }
    , { "mail_op_update_mail",
      mail_svr_op_update_send,
      mail_svr_op_update_recv }
    , { "mail_op_query_full",
      mail_svr_op_query_full_send,
      mail_svr_op_query_full_recv }
    , { "mail_op_query_basic",
      mail_svr_op_query_basic_send,
      mail_svr_op_query_basic_recv }
    , { "mail_op_query_detail",
      mail_svr_op_query_detail_send,
      mail_svr_op_query_detail_recv }
    , {"mail_op_send_global_mail",
      mail_svr_op_send_global_send,
      mail_svr_op_send_global_recv }
    , {"mail_op_query_global",
      mail_svr_op_query_global_send,
      mail_svr_op_query_global_recv}
};

#define FRIEND_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_mail_pro, __name); \
    assert(svr-> __arg)

mail_svr_t
mail_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mongo_id_generator_t id_generator,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct mail_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct mail_svr));
    if (svr_node == NULL) return NULL;

    svr = (mail_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_set_sp = set_sp;
    svr->m_rsp_manage = rsp_manage;
    svr->m_db = db;
    svr->m_id_generator = id_generator;
    svr->m_debug = 0;

    mem_buffer_init(&svr->m_record_metalib, alloc);
    mem_buffer_init(&svr->m_record_global_metalib, alloc);

    svr->m_record_tmpl_meta = NULL;
    svr->m_record_meta = NULL;
    svr->m_record_size = 0;
    svr->m_record_sender_start_pos = 0;
    svr->m_record_sender_capacity = 0;
    svr->m_record_sender_entry = NULL;
    svr->m_record_attach_start_pos = 0;
    svr->m_record_attach_capacity = 0;
    svr->m_record_attach_entry = NULL;

    svr->m_record_list_meta = NULL;
    svr->m_record_list_count_entry = NULL;
    svr->m_record_list_data_entry = NULL;

    svr->m_record_global_tmpl_meta = NULL;
    svr->m_record_global_meta = NULL;
    svr->m_record_global_size = 0;

    svr->m_record_global_attach_start_pos = 0;
    svr->m_record_global_attach_capacity = 0;
    svr->m_record_global_attach_entry = NULL;

    svr->m_record_global_list_meta = NULL;
    svr->m_record_global_list_count_entry = NULL;
    svr->m_record_global_list_data_entry = NULL;

    svr->m_sender_meta = NULL;
    svr->m_attach_meta = NULL;
    svr->m_attach_global_meta = NULL;

    //svr->m_record_global_mgr = NULL;
    //svr->m_record_global_hash = NULL;

    FRIEND_SVR_LOAD_META(m_meta_res_send, "svr_mail_res_send_mail");
    FRIEND_SVR_LOAD_META(m_meta_res_query_full, "svr_mail_res_query_mail_full");
    FRIEND_SVR_LOAD_META(m_meta_res_query_basic, "svr_mail_res_query_mail_basic");
    FRIEND_SVR_LOAD_META(m_meta_res_query_detail, "svr_mail_res_query_mail_detail");

    svr->m_op_register = logic_op_register_create(app, NULL, alloc, em);
    if (svr->m_op_register == NULL) {
        CPE_ERROR(em, "%s: create: create op_register fail!", name);
        mem_buffer_clear(&svr->m_record_metalib);
        nm_node_free(svr_node);
        return NULL;
    }

    if (logic_op_register_create_ops(
            svr->m_op_register,
            sizeof(g_mail_ops) / sizeof(g_mail_ops[0]),
            g_mail_ops,
            svr) != 0)
    {
        CPE_ERROR(em, "%s: create: register mail ops fail!", name);
        mem_buffer_clear(&svr->m_record_metalib);
        logic_op_register_free(svr->m_op_register);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_mail_svr);

    return svr;
}

static void mail_svr_clear(nm_node_t node) {
    mail_svr_t svr;
    svr = (mail_svr_t)nm_node_data(node);

    if (svr->m_op_register) {
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
    }

    /*record_global*/
    /*if(svr->m_record_global_mgr) {
        aom_obj_mgr_free(svr->m_record_global_mgr);
        svr->m_record_global_mgr = NULL;
    }

    if(svr->m_record_global_hash) {
        aom_obj_hash_table_free(svr->m_record_global_hash);
        svr->m_record_global_hash = NULL;
    }*/

    mem_buffer_clear(&svr->m_record_metalib);
}

void mail_svr_free(mail_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_mail_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t mail_svr_app(mail_svr_t svr) {
    return svr->m_app;
}

mail_svr_t
mail_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mail_svr) return NULL;
    return (mail_svr_t)nm_node_data(node);
}

mail_svr_t
mail_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mail_svr) return NULL;
    return (mail_svr_t)nm_node_data(node);
}

const char * mail_svr_name(mail_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
mail_svr_name_hs(mail_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t mail_svr_cur_time(mail_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}
