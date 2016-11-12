#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/time_utils.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/logic_use/logic_op_register.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "svr/set/share/set_pkg.h"
#include "gift_svr_ops.h"
#include "gift_svr_generator.h"

extern char g_metalib_svr_gift_pro[];
static void gift_svr_clear(nm_node_t node);
static ptr_int_t gift_svr_tick(void * ctx, ptr_int_t arg, float delta_s);

struct nm_node_type s_nm_node_type_gift_svr = {
    "svr_gift_svr",
    gift_svr_clear
};

struct logic_op_register_def g_gift_ops[] = {
    { "gift_op_init", gift_svr_op_init_send, gift_svr_op_init_recv }
    , { "gift_op_check_state", gift_svr_op_check_state_send, NULL }
    , { "gift_op_expire", gift_svr_op_expire_send, gift_svr_op_expire_recv }
    , { "gift_op_generate", gift_svr_op_generate_send, gift_svr_op_generate_recv }
    , { "gift_op_update_generate", gift_svr_op_update_generate_send, gift_svr_op_update_generate_recv }
    , { "gift_op_query_generate", gift_svr_op_query_generate_send, NULL }
    , { "gift_op_query_use", gift_svr_op_update_generate_send, gift_svr_op_query_use_recv }
    , { "gift_op_use", gift_svr_op_use_send, gift_svr_op_use_recv }
};

#define GIFT_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_gift_pro, __name); \
    assert(svr-> __arg)

gift_svr_t
gift_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_logic_rsp_manage_t rsp_manage,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct gift_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct gift_svr));
    if (svr_node == NULL) return NULL;

    svr = (gift_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_rsp_manage = rsp_manage;
    svr->m_db = db;
    svr->m_debug = 0;
    svr->m_init_state = gift_svr_init_not_init;
    svr->m_max_generate_id = 0;
    svr->m_next_expire_time = 0;
    
    mem_buffer_init(&svr->m_record_metalib, alloc);
    svr->m_generate_record_meta = NULL;
    svr->m_generate_record_size = 0;
    svr->m_generate_record_id_entry = NULL;
    svr->m_generate_record_id_start_pos = 0;
    svr->m_generate_record_id_capacity = 0;
    svr->m_generate_record_list_meta = NULL;
    svr->m_generate_record_data_start_pos = 0;
    svr->m_generate_record_list_count_entry = NULL;
    svr->m_generate_record_list_data_entry = NULL;

    svr->m_data_meta = NULL;
    svr->m_data_size = 0;

    svr->m_generate_record_mgr = NULL;
    svr->m_generate_record_hash = NULL;

    TAILQ_INIT(&svr->m_generators);
    
    GIFT_SVR_LOAD_META(m_meta_res_generate, "svr_gift_res_generate");
    GIFT_SVR_LOAD_META(m_meta_res_query_generate, "svr_gift_res_query_generate");
    GIFT_SVR_LOAD_META(m_meta_res_query_use, "svr_gift_res_query_use");
    GIFT_SVR_LOAD_META(m_meta_res_use, "svr_gift_res_use");

    GIFT_SVR_LOAD_META(m_use_record_meta, "svr_gift_use_record");
    GIFT_SVR_LOAD_META(m_use_record_list_meta, "svr_gift_use_record_list");
    GIFT_SVR_LOAD_META(m_use_record_use_meta, "svr_gift_use_record_use");
    
    GIFT_SVR_LOAD_META(m_meta_op_generate_ctx, "svr_gift_op_generate_ctx");

    if (gd_app_tick_add(app, gift_svr_tick, svr, 0) != 0) {
        CPE_ERROR(em, "%s: create: add tick fail!", name);
        mem_buffer_clear(&svr->m_record_metalib);
        nm_node_free(svr_node);
        return NULL;
    }

    svr->m_op_register = NULL;

    nm_node_set_type(svr_node, &s_nm_node_type_gift_svr);

    return svr;
}

static void gift_svr_clear(nm_node_t node) {
    gift_svr_t svr;
    svr = (gift_svr_t)nm_node_data(node);

    gd_app_tick_remove(svr->m_app, gift_svr_tick, svr);

    if (svr->m_op_register) {
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
    }

    /*generate_record*/
    if (svr->m_generate_record_mgr) {
        aom_obj_mgr_free(svr->m_generate_record_mgr);
        svr->m_generate_record_mgr = NULL;
    }

    if (svr->m_generate_record_hash) {
        aom_obj_hash_table_free(svr->m_generate_record_hash);
        svr->m_generate_record_hash = NULL;
    }

    while(!TAILQ_EMPTY(&svr->m_generators)) {
        gift_svr_generator_free(TAILQ_FIRST(&svr->m_generators));
    }
    
    mem_buffer_clear(&svr->m_record_metalib);
}

void gift_svr_free(gift_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_gift_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t gift_svr_app(gift_svr_t svr) {
    return svr->m_app;
}

gift_svr_t
gift_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gift_svr) return NULL;
    return (gift_svr_t)nm_node_data(node);
}

gift_svr_t
gift_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_gift_svr) return NULL;
    return (gift_svr_t)nm_node_data(node);
}

const char * gift_svr_name(gift_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
gift_svr_name_hs(gift_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t gift_svr_cur_time(gift_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

static ptr_int_t gift_svr_tick(void * ctx, ptr_int_t arg, float delta_s) {
    gift_svr_t svr = ctx;
    char time_str_buf[64];
    
    /*在数据库连接以后执行初始化过程 */
    if (svr->m_init_state == gift_svr_init_not_init) {
        if (mongo_driver_is_readable(mongo_cli_proxy_driver(svr->m_db))) {
            logic_context_t context;

            context = set_logic_rsp_manage_create_op_by_name(svr->m_rsp_manage, NULL, "internal_init", svr->m_em);
            if (context == NULL) {
                CPE_ERROR(svr->m_em, "%s: init state: create op fail!", gift_svr_name(svr));
            }
        }
    }

    /*设置好最近过期时间 */
    if (svr->m_next_expire_time == 0 && aom_obj_mgr_allocked_obj_count(svr->m_generate_record_mgr) > 0) {
        struct aom_obj_it obj_it;
        SVR_GIFT_GENERATE_RECORD const * next_record = NULL;
        SVR_GIFT_GENERATE_RECORD const * record_common;
        
        aom_objs(svr->m_generate_record_mgr, &obj_it);
        while((record_common = aom_obj_it_next(&obj_it))) {
            if (record_common->expire_time == 0) continue;
            if (svr->m_next_expire_time == 0 || record_common->expire_time < svr->m_next_expire_time) {
                svr->m_next_expire_time = record_common->expire_time;
                next_record = record_common;
            }
        }        

        if (next_record) {
            CPE_INFO(
                svr->m_em, "%s: tick: found next expire time %s, generate-id=%d!",
                gift_svr_name(svr),
                time_to_str((time_t)svr->m_next_expire_time, time_str_buf, sizeof(time_str_buf)), next_record->_id);
        }
        else {
            assert(svr->m_next_expire_time == 0);
        }
    }

    /*检查并启动过期处理 */
    if (!svr->m_expire_op_in_process
        && svr->m_next_expire_time > 0
        && svr->m_next_expire_time < gift_svr_cur_time(svr))
    {
        logic_context_t context;
        
        context = set_logic_rsp_manage_create_op_by_name(svr->m_rsp_manage, NULL, "internal_expire", svr->m_em);
        if (context == NULL) {
            CPE_ERROR(svr->m_em, "%s: tick: start process expire fail!", gift_svr_name(svr));
        }
        else {
            CPE_INFO(
                svr->m_em, "%s: tick: start process expire time %s!",
                gift_svr_name(svr),
                time_to_str((time_t)svr->m_next_expire_time, time_str_buf, sizeof(time_str_buf)));
            svr->m_expire_op_in_process = 1;
        }
    }
    
    return 0;
}

int gift_svr_install_ops(gift_svr_t svr) {
    assert(svr->m_op_register == NULL);
    
    svr->m_op_register = logic_op_register_create(svr->m_app, NULL, svr->m_alloc, svr->m_em);
    if (svr->m_op_register == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: create op_register fail!", gift_svr_name(svr));
        return -1;
    }

    if (logic_op_register_create_ops(
            svr->m_op_register,
            sizeof(g_gift_ops) / sizeof(g_gift_ops[0]),
            g_gift_ops,
            svr) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: create: register gift ops fail!", gift_svr_name(svr));
        logic_op_register_free(svr->m_op_register);
        svr->m_op_register = NULL;
        return -1;
    }

    return 0;
}

int gift_svr_start_tick(gift_svr_t svr) {
    if (gd_app_tick_add(svr->m_app, gift_svr_tick, svr, 0) != 0) {
        CPE_ERROR(svr->m_em, "%s: add tick fail!", gift_svr_name(svr));
        return -1;
    }

    return 0;
}    
