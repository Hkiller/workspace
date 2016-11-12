#ifndef SVR_MAIL_SVR_TYPES_H
#define SVR_MAIL_SVR_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/mongo_use_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/mail/svr_mail_internal.h"

typedef struct mail_svr * mail_svr_t;

struct mail_svr {
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

    /*record数据 */
    struct mem_buffer m_record_metalib;
    LPDRMETA m_record_tmpl_meta;
    LPDRMETA m_record_meta;
    uint32_t m_record_size;

    uint32_t m_record_sender_start_pos;
    uint32_t m_record_sender_capacity;
    LPDRMETAENTRY m_record_sender_entry;

    uint32_t m_record_attach_start_pos;
    uint32_t m_record_attach_capacity;
    LPDRMETAENTRY m_record_attach_entry;

    /*record_list*/
    LPDRMETA m_record_list_meta;
    LPDRMETAENTRY m_record_list_count_entry;
    LPDRMETAENTRY m_record_list_data_entry;

    /*record global*/
    struct mem_buffer m_record_global_metalib;
    LPDRMETA m_record_global_tmpl_meta;
    LPDRMETA m_record_global_meta;
    uint32_t m_record_global_size;

    uint32_t m_record_global_attach_start_pos;
    uint32_t m_record_global_attach_capacity;
    LPDRMETAENTRY m_record_global_attach_entry;

    LPDRMETA m_record_global_list_meta;
    LPDRMETAENTRY m_record_global_list_count_entry;
    LPDRMETAENTRY m_record_global_list_data_entry;

    LPDRMETA m_sender_meta;
    LPDRMETA m_attach_meta;
    LPDRMETA m_attach_global_meta;
    /*record global hash*/
    //aom_obj_mgr_t m_record_global_mgr;
    //aom_obj_hash_table_t m_record_global_hash;

    /*协议相关的meta */
    LPDRMETA m_meta_res_send;
    LPDRMETA m_meta_res_query_full;
    LPDRMETA m_meta_res_query_basic;
    LPDRMETA m_meta_res_query_detail;

    mongo_pkg_t m_mongo_pkg;
};

#endif
