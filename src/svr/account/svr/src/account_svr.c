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
#include "account_svr_module.h"
#include "account_svr_conn_info.h"
#include "account_svr_login_info.h"
#include "account_svr_ops.h"
#include "account_svr_backend.h"

extern char g_metalib_svr_account_pro[];
static void account_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_account_svr = {
    "svr_account_svr",
    account_svr_clear
};

static struct logic_op_register_def g_account_ops[] = {
    { "account_op_create", account_svr_op_create_send, account_svr_op_create_recv }
    , { "account_op_login", account_svr_op_login_send, account_svr_op_login_recv }
    , { "account_op_bind", account_svr_op_bind_send, account_svr_op_bind_recv }
    , { "account_op_unbind", account_svr_op_unbind_send, account_svr_op_unbind_recv }
    , { "account_op_query_by_logic_id", account_svr_op_query_by_logic_id_send, account_svr_op_query_by_logic_id_recv }
    , { "account_op_query_by_account_id", account_svr_op_query_by_account_id_send, account_svr_op_query_by_account_id_recv }
    , { "account_op_query_login_info", account_svr_op_query_login_info, NULL }
    , { "account_op_query_external_friends", account_svr_op_query_external_friends_send, account_svr_op_query_external_friends_recv }
};

/* static struct { */
/*     const char * name;  */
/*     int (*init)(account_svr_t svr); */
/*     void (*fini)(account_svr_t svr); */
/* } s_auto_reg_products[] = { */
/*     { "backend-device", account_svr_backend_device_install, account_svr_backend_device_uninstall }     */
/*     , { "backend-qihoo", account_svr_backend_qihoo_install, account_svr_backend_qihoo_uninstall }     */
/* }; */

#define ACCOUNT_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_account_pro, __name); \
    assert(svr-> __arg)

account_svr_t
account_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_sp_t set_sp,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mongo_id_generator_t id_generator,
    net_trans_manage_t trans_mgr,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct account_svr * svr;
    nm_node_t svr_node;
    //uint16_t component_pos;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct account_svr));
    if (svr_node == NULL) return NULL;

    svr = (account_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_set_sp = set_sp;
    svr->m_rsp_manage = rsp_manage;
    svr->m_db = db;
    svr->m_id_generator = id_generator;
    svr->m_trans_mgr = trans_mgr;
    svr->m_debug = 0;
    TAILQ_INIT(&svr->m_backends);
    TAILQ_INIT(&svr->m_free_login_infos);

    ACCOUNT_SVR_LOAD_META(m_meta_res_create, "svr_account_res_create");
    ACCOUNT_SVR_LOAD_META(m_meta_res_login, "svr_account_res_login");
    ACCOUNT_SVR_LOAD_META(m_meta_res_query_login_info, "svr_account_res_query_login_info");
    ACCOUNT_SVR_LOAD_META(m_meta_res_query_external_friends, "svr_account_res_query_external_friends");
    ACCOUNT_SVR_LOAD_META(m_meta_logic_id, "svr_account_logic_id");
    ACCOUNT_SVR_LOAD_META(m_meta_login_info, "svr_account_login_info");
    ACCOUNT_SVR_LOAD_META(m_meta_record_full, "svr_account_full");
    ACCOUNT_SVR_LOAD_META(m_meta_record_full_list, "svr_account_full_list");
    ACCOUNT_SVR_LOAD_META(m_meta_record_basic, "svr_account_basic");
    ACCOUNT_SVR_LOAD_META(m_meta_record_basic_list, "svr_account_basic_list");
    ACCOUNT_SVR_LOAD_META(m_meta_logic_id_list, "svr_account_logic_id_list");

    svr->m_op_register = logic_op_register_create(app, NULL, alloc, em);
    if (svr->m_op_register == NULL) {
        CPE_ERROR(em, "%s: create: create op_register fail!", name);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_account_infos,
            svr->m_alloc,
            (cpe_hash_fun_t) account_svr_account_info_hash,
            (cpe_hash_eq_t) account_svr_account_info_eq,
            CPE_HASH_OBJ2ENTRY(account_svr_account_info, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: create account infos hashtable fail!", name);
        logic_op_register_free(svr->m_op_register);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_login_infos,
            svr->m_alloc,
            (cpe_hash_fun_t) account_svr_login_info_hash,
            (cpe_hash_eq_t) account_svr_login_info_eq,
            CPE_HASH_OBJ2ENTRY(account_svr_login_info, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "%s: create: create token infos hashtable fail!", name);
        cpe_hash_table_fini(&svr->m_account_infos);
        logic_op_register_free(svr->m_op_register);
        nm_node_free(svr_node);
        return NULL;
    }

    if (logic_op_register_create_ops(
            svr->m_op_register,
            sizeof(g_account_ops) / sizeof(g_account_ops[0]),
            g_account_ops,
            svr) != 0)
    {
        CPE_ERROR(em, "%s: create: register account ops fail!", name);
        cpe_hash_table_fini(&svr->m_login_infos);
        cpe_hash_table_fini(&svr->m_account_infos);
        logic_op_register_free(svr->m_op_register);
        nm_node_free(svr_node);
        return NULL;
    }

    /* for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) { */
    /*     if (s_auto_reg_products[component_pos].init(svr) != 0) { */
    /*         CPE_ERROR(svr->m_em, "%s: create: regist product %s fail!", name, s_auto_reg_products[component_pos].name); */
    /*         for(; component_pos > 0; component_pos--) { */
    /*             s_auto_reg_products[component_pos - 1].fini(svr); */
    /*         } */
    /*         account_svr_account_info_free_all(svr); */
    /*         cpe_hash_table_fini(&svr->m_account_infos); */
    /*         nm_node_free(svr_node); */
    /*         return NULL; */
    /*     } */
    /* } */

    mem_buffer_init(&svr->m_dump_buffer, alloc);

    nm_node_set_type(svr_node, &s_nm_node_type_account_svr);

    return svr;
}

static void account_svr_clear(nm_node_t node) {
    account_svr_t svr;
    //uint16_t component_pos;

    svr = (account_svr_t)nm_node_data(node);
    /* for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) { */
    /*     s_auto_reg_products[component_pos - 1].fini(svr); */
    /* } */

    while(!TAILQ_EMPTY(&svr->m_backends)) {
        account_svr_backend_free(TAILQ_FIRST(&svr->m_backends));
    }

    account_svr_account_info_free_all(svr);
    cpe_hash_table_fini(&svr->m_account_infos);

    account_svr_login_info_free_all(svr);
    cpe_hash_table_fini(&svr->m_login_infos);

    while(!TAILQ_EMPTY(&svr->m_free_login_infos)) {
        account_svr_login_info_real_free(TAILQ_FIRST(&svr->m_free_login_infos));
    }

    mem_buffer_clear(&svr->m_dump_buffer);

    if (svr->m_op_register) {
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
    }
}

void account_svr_free(account_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_account_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t account_svr_app(account_svr_t svr) {
    return svr->m_app;
}

account_svr_t
account_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_account_svr) return NULL;
    return (account_svr_t)nm_node_data(node);
}

account_svr_t
account_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_account_svr) return NULL;
    return (account_svr_t)nm_node_data(node);
}

const char * account_svr_name(account_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
account_svr_name_hs(account_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t account_svr_cur_time(account_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}
