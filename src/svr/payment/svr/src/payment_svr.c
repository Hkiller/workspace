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
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/logic_use/logic_op_register.h"
#include "svr/set/share/set_pkg.h"
#include "payment_svr.h"
#include "payment_svr_ops.h"
#include "payment_svr_adapter.h"
#include "payment_svr_waiting.h"

extern char g_metalib_svr_payment_pro[];
static void payment_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_payment_svr = {
    "svr_payment_svr",
    payment_svr_clear
};

struct logic_op_register_def g_payment_ops[] = {
    { "payment_op_get_balance", payment_svr_op_get_balance_send, payment_svr_op_get_balance_recv }
    , { "payment_op_pay", payment_svr_op_pay_send, payment_svr_op_pay_recv }
    , { "payment_op_recharge_begin", payment_svr_op_recharge_begin_send, payment_svr_op_recharge_begin_recv }
    , { "payment_op_recharge_commit", payment_svr_op_recharge_commit_send, payment_svr_op_recharge_commit_recv }
    , { "payment_op_notify", payment_svr_op_notify_send, payment_svr_op_notify_recv }
};

#define PAYMENT_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_payment_pro, __name); \
    assert(svr-> __arg)

payment_svr_t
payment_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    net_trans_manage_t trans_mgr,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct payment_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct payment_svr));
    if (svr_node == NULL) return NULL;

    svr = (payment_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_set_sp = set_sp;
    svr->m_rsp_manage = rsp_manage;
    svr->m_trans_mgr = trans_mgr;
    svr->m_db = db;
    svr->m_debug = 0;

    svr->m_bag_info_count = 0;
    svr->m_bag_infos = NULL;

    svr->m_iap_svr_type = 0;
    svr->m_iap_meta_req_validate = NULL;

    PAYMENT_SVR_LOAD_META(m_meta_recharge_record, "payment_recharge_record");
    PAYMENT_SVR_LOAD_META(m_meta_recharge_record_list, "payment_recharge_record_list");
    PAYMENT_SVR_LOAD_META(m_meta_data_list, "payment_data_list");
    PAYMENT_SVR_LOAD_META(m_meta_bill_data, "payment_bill_data");
    PAYMENT_SVR_LOAD_META(m_meta_money_group, "svr_payment_money_group");
    PAYMENT_SVR_LOAD_META(m_meta_res_get_balance, "svr_payment_res_get_balance");
    PAYMENT_SVR_LOAD_META(m_meta_res_recharge_begin, "svr_payment_res_recharge_begin");
    PAYMENT_SVR_LOAD_META(m_meta_res_recharge_commit, "svr_payment_res_recharge_commit");
    PAYMENT_SVR_LOAD_META(m_meta_res_pay, "svr_payment_res_pay");
    PAYMENT_SVR_LOAD_META(m_meta_vendor_record, "svr_payment_vendor_record");
    
    PAYMENT_SVR_LOAD_META(m_meta_iapppay_record, "svr_payment_iapppay_record");
    PAYMENT_SVR_LOAD_META(m_meta_iapppay_error, "svr_payment_iapppay_error");

    if (cpe_hash_table_init(
            &svr->m_waitings,
            svr->m_alloc,
            (cpe_hash_fun_t) payment_svr_waiting_hash,
            (cpe_hash_eq_t) payment_svr_waiting_eq,
            CPE_HASH_OBJ2ENTRY(payment_svr_waiting, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    svr->m_op_register = logic_op_register_create(app, NULL, alloc, em);
    if (svr->m_op_register == NULL) {
        CPE_ERROR(em, "%s: create: create op_register fail!", name);
        cpe_hash_table_fini(&svr->m_waitings);
        nm_node_free(svr_node);
        return NULL;
    }

    if (logic_op_register_create_ops(
            svr->m_op_register,
            sizeof(g_payment_ops) / sizeof(g_payment_ops[0]), 
            g_payment_ops,
            svr) != 0)
    {
        CPE_ERROR(em, "%s: create: register payment ops fail!", name);
        logic_op_register_free(svr->m_op_register);
        cpe_hash_table_fini(&svr->m_waitings);
        nm_node_free(svr_node);
        return NULL;
    }

    mem_buffer_init(&svr->m_buffer, alloc);
    mem_buffer_init(&svr->m_sign_buffer, alloc);
    mem_buffer_init(&svr->m_rsa_buffer, alloc);
    TAILQ_INIT(&svr->m_adapters);
    
    nm_node_set_type(svr_node, &s_nm_node_type_payment_svr);

    return svr;
}

static void payment_svr_clear(nm_node_t node) {
    payment_svr_t svr;
    svr = (payment_svr_t)nm_node_data(node);

    if (svr->m_op_register) {
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
    }

    if (svr->m_bag_infos) {
        mem_free(svr->m_alloc, svr->m_bag_infos);
        svr->m_bag_infos = NULL;
        svr->m_bag_info_count = 0;
    }

    while(!TAILQ_EMPTY(&svr->m_adapters)) {
        payment_svr_adapter_free(TAILQ_FIRST(&svr->m_adapters));
    }

    payment_svr_waiting_free_all(svr);
    cpe_hash_table_fini(&svr->m_waitings);
    
    mem_buffer_clear(&svr->m_buffer);
    mem_buffer_clear(&svr->m_sign_buffer);
    mem_buffer_clear(&svr->m_rsa_buffer);
}

void payment_svr_free(payment_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_payment_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t payment_svr_app(payment_svr_t svr) {
    return svr->m_app;
}

payment_svr_t
payment_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_payment_svr) return NULL;
    return (payment_svr_t)nm_node_data(node);
}

payment_svr_t
payment_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_payment_svr) return NULL;
    return (payment_svr_t)nm_node_data(node);
}

const char * payment_svr_name(payment_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
payment_svr_name_hs(payment_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t payment_svr_cur_time(payment_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int payment_svr_op_validate_money_types(payment_svr_t svr, BAG_INFO * bag_info, SVR_PAYMENT_MONEY_GROUP const * moneies) {
    uint8_t i;
    for(i = 0; i < moneies->count; ++i) {
        SVR_PAYMENT_MONEY const * money = moneies->datas + i;
        
        if (money->type < PAYMENT_MONEY_TYPE_MIN
            || money->type >= PAYMENT_MONEY_TYPE_MAX
            || (money->type - PAYMENT_MONEY_TYPE_MIN) >= bag_info->money_type_count)
        {
            CPE_ERROR(svr->m_em, "%s: validate_money_types: money type %d error!", payment_svr_name(svr), money->type);
            return -1;
        }
    }

    return 0;
}
