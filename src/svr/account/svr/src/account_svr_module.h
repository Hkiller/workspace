#ifndef SVR_ACCOUNT_SVR_MODULE_H
#define SVR_ACCOUNT_SVR_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "gd/timer/timer_types.h"
#include "gd/net_trans/net_trans_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/mongo_use_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/account/svr_account_internal.h"

typedef struct account_svr * account_svr_t;
typedef struct account_svr_backend * account_svr_backend_t;
typedef struct account_svr_account_info * account_svr_account_info_t;
typedef struct account_svr_login_info * account_svr_login_info_t;

typedef TAILQ_HEAD(account_svr_backend_list, account_svr_backend) account_svr_backend_list_t;
typedef TAILQ_HEAD(account_svr_login_info_list, account_svr_login_info) account_svr_login_info_list_t;

struct account_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    set_logic_sp_t m_set_sp;
    int m_debug;

    logic_op_register_t m_op_register;
    set_logic_rsp_manage_t m_rsp_manage;
    mongo_cli_proxy_t m_db;
    mongo_id_generator_t m_id_generator;

    account_svr_backend_list_t m_backends;

    net_trans_manage_t m_trans_mgr;
    
    struct cpe_hash_table m_account_infos;
    struct cpe_hash_table m_login_infos;

    LPDRMETA m_meta_res_login;
    LPDRMETA m_meta_res_create;
    LPDRMETA m_meta_res_query_login_info;
    LPDRMETA m_meta_res_query_external_friends;
    LPDRMETA m_meta_logic_id;
    LPDRMETA m_meta_login_info;
    LPDRMETA m_meta_record_full;
    LPDRMETA m_meta_record_full_list;
    LPDRMETA m_meta_record_basic;
    LPDRMETA m_meta_record_basic_list;
    LPDRMETA m_meta_logic_id_list;
    
    account_svr_login_info_list_t m_free_login_infos;

    struct mem_buffer m_dump_buffer;
};

/*operations of account_svr */
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
    error_monitor_t em);

void account_svr_free(account_svr_t svr);

account_svr_t account_svr_find(gd_app_context_t app, cpe_hash_string_t name);
account_svr_t account_svr_find_nc(gd_app_context_t app, const char * name);
const char * account_svr_name(account_svr_t svr);

/*conn svr ops*/
int account_svr_is_conn_svr(account_svr_t svr, uint16_t svr_type_id);
int account_svr_conn_bind_account(
    account_svr_t svr, logic_context_t ctx,
    uint16_t conn_svr_id, uint16_t conn_svr_type, uint64_t account_id, uint16_t account_state,
    uint8_t device_category, uint8_t device_cap, const char * chanel);
int account_svr_conn_get_conn_info(account_svr_t svr, logic_context_t ctx, uint32_t * conn_id, uint64_t * account_id);

uint8_t account_svr_account_type_from_str(const char * str);

#endif
