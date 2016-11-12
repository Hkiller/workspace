#ifndef SVR_PAYMENT_SVR_TYPES_H
#define SVR_PAYMENT_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "gd/net_trans/net_trans_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/payment/svr_payment_internal.h"
#include "protocol/svr/payment/svr_payment_meta.h"
#include "protocol/svr/payment/svr_payment_pro.h"

typedef struct payment_svr * payment_svr_t;
typedef struct payment_svr_adapter * payment_svr_adapter_t;
typedef struct payment_svr_adapter_type * payment_svr_adapter_type_t;
typedef struct payment_svr_waiting * payment_svr_waiting_t;
typedef TAILQ_HEAD(payment_svr_adapter_list, payment_svr_adapter) payment_svr_adapter_list_t;

struct payment_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    set_logic_sp_t m_set_sp;
    int m_debug;

    logic_op_register_t m_op_register;
    set_logic_rsp_manage_t m_rsp_manage;
    net_trans_manage_t m_trans_mgr;
    mongo_cli_proxy_t m_db;

    LPDRMETA m_meta_recharge_record;
    LPDRMETA m_meta_recharge_record_list;
    LPDRMETA m_meta_data_list;
    LPDRMETA m_meta_bill_data;
    LPDRMETA m_meta_money_group;
    LPDRMETA m_meta_res_get_balance;
    LPDRMETA m_meta_res_recharge_begin;
    LPDRMETA m_meta_res_recharge_commit;
    LPDRMETA m_meta_res_pay;

    LPDRMETA m_meta_vendor_record;
    
    LPDRMETA m_meta_iapppay_record;
    LPDRMETA m_meta_iapppay_error;

    uint32_t m_bag_info_count;
    BAG_INFO * m_bag_infos;

    struct cpe_hash_table m_waitings;
    payment_svr_adapter_list_t m_adapters;

    struct mem_buffer m_buffer;
    struct mem_buffer m_sign_buffer;
    struct mem_buffer m_rsa_buffer;

    /*for iap svr*/
    uint16_t m_iap_svr_type;
    LPDRMETA m_iap_meta_req_validate;
};

/*operations of payment_svr */
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
    error_monitor_t em);

void payment_svr_free(payment_svr_t svr);

payment_svr_t payment_svr_find(gd_app_context_t app, cpe_hash_string_t name);
payment_svr_t payment_svr_find_nc(gd_app_context_t app, const char * name);
const char * payment_svr_name(payment_svr_t svr);

uint32_t payment_svr_cur_time(payment_svr_t svr);

/*util functions*/
int payment_svr_op_validate_money_types(payment_svr_t svr, BAG_INFO * bag_info, SVR_PAYMENT_MONEY_GROUP const * moneies);

#endif
