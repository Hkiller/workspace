#ifndef SVR_GIFT_SVR_TYPES_H
#define SVR_GIFT_SVR_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/aom/aom_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "usf/logic_use/logic_use_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "svr/set/logic/set_logic_types.h"
#include "protocol/svr/gift/svr_gift_internal.h"
#include "protocol/svr/gift/svr_gift_meta.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gift_svr * gift_svr_t;
typedef struct gift_svr_generator * gift_svr_generator_t;
typedef struct gift_svr_generator_block * gift_svr_generator_block_t;

typedef TAILQ_HEAD(gift_svr_generator_list, gift_svr_generator) gift_svr_generator_list_t;

typedef enum gift_svr_init_state {
    gift_svr_init_not_init,
    gift_svr_init_loading,
    gift_svr_init_success,
} gift_svr_init_state_t;

struct gift_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;
    gift_svr_init_state_t m_init_state;
    uint32_t m_max_generate_id;

    logic_op_register_t m_op_register;
    set_logic_rsp_manage_t m_rsp_manage;
    mongo_cli_proxy_t m_db;

    gift_svr_generator_list_t m_generators;
    
    /*record数据 */
    struct mem_buffer m_record_metalib;
    LPDRMETA m_generate_record_meta;
    LPDRMETAENTRY m_generate_record_id_entry;
    uint32_t m_generate_record_id_start_pos;
    uint32_t m_generate_record_id_capacity;
    uint32_t m_generate_record_size;
    LPDRMETA m_generate_record_list_meta;
    uint32_t m_generate_record_data_start_pos;
    LPDRMETAENTRY m_generate_record_list_count_entry;
    LPDRMETAENTRY m_generate_record_list_data_entry;

    /*use相关数据 */
    LPDRMETA m_use_record_meta;
    LPDRMETA m_use_record_list_meta;
    LPDRMETA m_use_record_use_meta;
    
    /*好友相关的数据 */
    LPDRMETA m_data_meta;
    uint32_t m_data_size;

    /*操作上下文相关的meta*/
    LPDRMETA m_meta_op_generate_ctx;
    
    /*协议相关的meta */
    LPDRMETA m_meta_res_generate;
    LPDRMETA m_meta_res_query_generate;
    LPDRMETA m_meta_res_query_use;
    LPDRMETA m_meta_res_use;

    uint8_t m_expire_op_in_process;
    uint32_t m_next_expire_time;
    aom_obj_mgr_t m_generate_record_mgr;
    aom_obj_hash_table_t m_generate_record_hash;
    
    mongo_pkg_t m_mongo_pkg;
};

/*operations of gift_svr */
gift_svr_t
gift_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em);

void gift_svr_free(gift_svr_t svr);

uint32_t gift_svr_cur_time(gift_svr_t svr);

gift_svr_t gift_svr_find(gd_app_context_t app, cpe_hash_string_t name);
gift_svr_t gift_svr_find_nc(gd_app_context_t app, const char * name);
const char * gift_svr_name(gift_svr_t svr);

int gift_svr_install_ops(gift_svr_t svr);
int gift_svr_start_tick(gift_svr_t svr);

#ifdef __cplusplus
}
#endif
    
#endif
