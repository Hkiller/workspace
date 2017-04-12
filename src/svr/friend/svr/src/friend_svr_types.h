#ifndef SVR_FRIEND_SVR_TYPES_H
#define SVR_FRIEND_SVR_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/friend/svr_friend_internal.h"
#include "protocol/svr/friend/svr_friend_meta.h"

typedef struct friend_svr * friend_svr_t;

typedef enum friend_svr_runing_mode {
    friend_svr_runing_mode_one_way,
    friend_svr_runing_mode_ack
} friend_svr_runing_mode_t;

struct friend_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    set_logic_sp_t m_set_sp;
    friend_svr_runing_mode_t m_runing_mode;
    int m_debug;

    logic_op_register_t m_op_register;
    set_logic_rsp_manage_t m_rsp_manage;
    mongo_cli_proxy_t m_db;

    /*record数据 */
    struct mem_buffer m_record_metalib;
    LPDRMETA m_record_meta;
    LPDRMETAENTRY m_record_id_entry;
    uint32_t m_record_id_start_pos;
    uint32_t m_record_id_capacity;
    LPDRMETAENTRY m_record_uid_entry;
    uint32_t m_record_uid_start_pos;
    LPDRMETAENTRY m_record_fuid_entry;
    uint32_t m_record_fuid_start_pos;
    uint32_t m_record_size;
    LPDRMETA m_record_list_meta;
    uint32_t m_record_data_start_pos;
    LPDRMETAENTRY m_record_list_count_entry;
    LPDRMETAENTRY m_record_list_data_entry;

    /*好友相关的数据 */
    LPDRMETA m_data_meta;
    LPDRMETAENTRY m_data_fuid_entry;
    uint32_t m_data_fuid_start_pos;
    uint32_t m_data_size;

    /*操作上下文相关的meta*/
    LPDRMETA m_meta_op_add_ctx;

    /*协议相关的meta */
    LPDRMETA m_meta_res_query;

    mongo_pkg_t m_mongo_pkg;
};

#endif
