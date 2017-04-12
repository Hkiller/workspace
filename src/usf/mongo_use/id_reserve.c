#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic_use/logic_op_async.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "usf/mongo_use/id_generator.h"
#include "mongo_use_internal_ops.h"
#include "protocol/mongo_use/mongo_use.h"

extern unsigned char g_metalib_mongo_use_data[];    

static logic_op_exec_result_t
mongo_id_reserve_send(logic_context_t ctx, logic_stack_node_t stack_noe, void * user_data, cfg_t cfg);
static logic_op_exec_result_t
mongo_id_reserve_recv(logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require, void * user_data, cfg_t cfg);

struct mongo_id_reserve_op {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    mongo_id_generator_t m_id_generator;
    logic_executor_type_t m_executor_type;

    int m_debug;
};

static void mongo_id_reserve_op_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_mongo_id_reserve_op = {
    "usf_mongo_id_reserve_op",
    mongo_id_reserve_op_clear
};

struct mongo_id_reserve_op *
mongo_id_reserve_op_create(
    gd_app_context_t app,
    const char * name,
    mongo_id_generator_t generator,
    logic_manage_t logic_manage,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct mongo_id_reserve_op * op;
    nm_node_t op_node;

    op_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct mongo_id_reserve_op));
    if (op_node == NULL) return NULL;

    op = (struct mongo_id_reserve_op *)nm_node_data(op_node);
    bzero(op, sizeof(struct mongo_id_reserve_op));

    op->m_app = app;
    op->m_alloc = alloc;
    op->m_em = em;
    op->m_debug = 0;
    op->m_id_generator = generator;

    op->m_executor_type =
        logic_op_async_type_create(
            app,
            NULL,
            name,
            mongo_id_reserve_send,
            mongo_id_reserve_recv,
            op,
            NULL,
            gd_app_em(app));
    if (op->m_executor_type == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: create executor fail!",
            name);
        nm_node_free(op_node);
        return NULL;
    }

    nm_node_set_type(op_node, &s_nm_node_type_mongo_id_reserve_op);

    return op;
} 

static void mongo_id_reserve_op_clear(nm_node_t node) {
    struct mongo_id_reserve_op * op;

    op = (struct mongo_id_reserve_op *)nm_node_data(node);

    logic_executor_type_free(op->m_executor_type);
    op->m_executor_type = NULL;
}

void mongo_id_reserve_op_free(struct mongo_id_reserve_op * op) {
    nm_node_t op_node;
    assert(op);

    op_node = nm_node_from_data(op);
    if (nm_node_type(op_node) != &s_nm_node_type_mongo_id_reserve_op) return;
    nm_node_free(op_node);
}

struct mongo_id_reserve_op *
mongo_id_reserve_op_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_id_reserve_op) return NULL;
    return (struct mongo_id_reserve_op *)nm_node_data(node);
}

struct mongo_id_reserve_op *
mongo_id_reserve_op_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_mongo_id_reserve_op) return NULL;
    return (struct mongo_id_reserve_op *)nm_node_data(node);
}

const char * mongo_id_reserve_op_name(struct mongo_id_reserve_op * op) {
    return nm_node_name(nm_node_from_data(op));
}

static int mongo_id_reserve_send_update_pkg(
    struct mongo_id_reserve_op * op,
    logic_require_t require,
    struct mongo_id_info * id_info,
    uint32_t reserve_count)
{
    mongo_pkg_t pkg;
    LPDRMETALIB metalib = (LPDRMETALIB)g_metalib_mongo_use_data;
    LPDRMETA meta = dr_lib_find_meta_by_name(metalib, "SysIdInfoList");

    pkg = mongo_cli_proxy_pkg_buf(op->m_id_generator->m_mongo_cli);
    if (pkg == NULL) {
        CPE_ERROR(op->m_em, "%s: send update pkg: get pkg fail", mongo_id_reserve_op_name(op));
        return -1;
    }

    mongo_pkg_cmd_init(pkg);

    if (
        mongo_pkg_doc_open(pkg) != 0
        || mongo_pkg_append_string(pkg, "findandmodify", "SysIdInfo") != 0
        || mongo_pkg_append_start_object(pkg, "query") != 0
        ||   mongo_pkg_append_string(pkg, "_id", id_info->m_name) != 0
        || mongo_pkg_append_finish_object(pkg) != 0
        || mongo_pkg_append_start_object(pkg, "update") != 0
        ||   mongo_pkg_append_start_object(pkg, "$inc") != 0
        ||   mongo_pkg_append_int64(pkg, "id_value", reserve_count) != 0
        ||   mongo_pkg_append_finish_object(pkg) != 0
        || mongo_pkg_append_finish_object(pkg) != 0
        || mongo_pkg_doc_close(pkg) != 0
        )
    {
        CPE_ERROR(op->m_em, "%s: send update pkg: build pkg fail", mongo_id_reserve_op_name(op));
        return -1;
    }

    if (mongo_cli_proxy_send(op->m_id_generator->m_mongo_cli, pkg, require, meta, 1, NULL, NULL, 0) != 0) {
        CPE_ERROR(op->m_em, "%s: send update pkg: send request fail", mongo_id_reserve_op_name(op));
        return -1;
    }

    if (op->m_debug) {
        CPE_ERROR(op->m_em, "%s: send update pkg: send request success", mongo_id_reserve_op_name(op));
    }

    return 0;
}

static int mongo_id_reserve_send_insert_pkg(
    struct mongo_id_reserve_op * op,
    logic_require_t require,
    struct mongo_id_info * id_info,
    uint32_t reserve_count)
{
    LPDRMETALIB metalib = (LPDRMETALIB)g_metalib_mongo_use_data;
    LPDRMETA meta = dr_lib_find_meta_by_name(metalib, "SysIdInfo");
    mongo_pkg_t pkg;
    SYSIDINFO data;

    pkg = mongo_cli_proxy_pkg_buf(op->m_id_generator->m_mongo_cli);
    if (pkg == NULL) {
        CPE_ERROR(op->m_em, "%s: send insert pkg: get pkg fail", mongo_id_reserve_op_name(op));
        return -1;
    }

    mongo_pkg_init(pkg);
    mongo_pkg_set_op(pkg, mongo_db_op_insert);
    mongo_pkg_set_collection(pkg, "SysIdInfo");

    cpe_str_dup(data._id, sizeof(data._id), id_info->m_name);
    data.id_value = id_info->m_id_start + reserve_count;

    if (mongo_pkg_doc_append(pkg, meta, &data, sizeof(data)) != 0) {
        CPE_ERROR(op->m_em, "%s: send insert pkg: build pkg fail", mongo_id_reserve_op_name(op));
        return -1;
    }

    if (mongo_cli_proxy_send(op->m_id_generator->m_mongo_cli, pkg, require, meta, 1, NULL, NULL, 0) != 0) {
        CPE_ERROR(op->m_em, "%s: send insert pkg: send request fail", mongo_id_reserve_op_name(op));
        return -1;
    }

    if (op->m_debug) {
        CPE_ERROR(op->m_em, "%s: send insert pkg: send request success", mongo_id_reserve_op_name(op));
    }

    return 0;
}

static logic_op_exec_result_t
mongo_id_reserve_send(logic_context_t ctx, logic_stack_node_t stack_noe, void * user_data, cfg_t cfg) {
    logic_require_t require;
    struct mongo_id_reserve_op * op = (struct mongo_id_reserve_op *)user_data;
    const char * id_name = cfg_get_string(cfg, "id-name", NULL);
    uint32_t reserve_count = cfg_get_uint32(cfg, "reserve-count", 0);
    struct mongo_id_info * id_info;

    if (id_name == NULL) {
        CPE_ERROR(op->m_em, "%s: id-name not configured", mongo_id_reserve_op_name(op));
        return logic_op_exec_result_null;
    }

    id_info = mongo_id_info_find(op->m_id_generator, id_name);
    if (id_info == NULL) {
        CPE_ERROR(
            op->m_em, "%s: id-name %s not exist",
            mongo_id_reserve_op_name(op), id_name);
        return logic_op_exec_result_null;
    }

    if (id_info->m_left_id_count >= reserve_count) return logic_op_exec_result_true;

    if (id_info->m_id_inc > reserve_count) {
        reserve_count = id_info->m_id_inc;
    }

    if (reserve_count == 0) {
        CPE_ERROR(op->m_em, "%s: reserve-count not configured", mongo_id_reserve_op_name(op));
        return logic_op_exec_result_null;
    }
    else if (reserve_count > 4096 * 1024) {
        CPE_ERROR(op->m_em, "%s: reserve-count %d error", mongo_id_reserve_op_name(op), reserve_count);
        return logic_op_exec_result_null;
    }

    if (mongo_id_info_have_waiting_require(op->m_id_generator, id_info)) {
        require = logic_require_create(stack_noe, "id-reserve-wait");
        if (require == NULL) {
            CPE_ERROR(op->m_em, "%s: create require %s fail!", mongo_id_reserve_op_name(op), "id-reserve-wait");
            return logic_op_exec_result_null;
        }

        if (logic_require_queue_add(id_info->m_waiting_queue, logic_require_id(require), NULL, 0) != 0) {
            CPE_ERROR(op->m_em, "%s: add waiting require to queue fail!", mongo_id_reserve_op_name(op));
            logic_require_free(require);
            return logic_op_exec_result_null;
        }

        if (op->m_debug) {
            CPE_INFO(
                op->m_em, "%s: wait, require-id=%d\n", 
                mongo_id_reserve_op_name(op), logic_require_id(require));
        }
    }
    else {
        require = logic_require_create(stack_noe, "id-reserve-query");
        if (require == NULL) {
            CPE_ERROR(op->m_em, "%s: create require %s fail!", mongo_id_reserve_op_name(op), "id-reserve-query");
            return logic_op_exec_result_null;
        }

        if (mongo_id_reserve_send_update_pkg(op, require, id_info, reserve_count) != 0) {
            return logic_op_exec_result_null;
        }

        id_info->m_processing_require = logic_require_id(require);

        if (op->m_debug) {
            CPE_INFO(
                op->m_em, "%s: execute, require-id=%d\n", 
                mongo_id_reserve_op_name(op), logic_require_id(require));
        }
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
mongo_id_reserve_recv_query(
    logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require,
    struct mongo_id_reserve_op * op, struct mongo_id_info * id_info, uint32_t reserve_count, cfg_t cfg)
{
    logic_data_t result_data;
    SYSIDINFO * result;

    if (logic_require_state(require) != logic_require_state_done) {
        CPE_ERROR(
            op->m_em, "%s: %s: require-id=%d, state=%d, error=%d!",
            mongo_id_reserve_op_name(op), logic_require_name(require),
            logic_require_id(require), logic_require_state(require), logic_require_error(require)); 
        return logic_op_exec_result_null;
    }

    result_data = logic_require_data_find(require, "SysIdInfoList");
    if (result_data == NULL) {
        CPE_ERROR(op->m_em, "%s: %s: result not exist!", mongo_id_reserve_op_name(op), logic_require_name(require));
        return logic_op_exec_result_null;
    }

    if (logic_data_record_count(result_data) == 0) {
        logic_require_free(require);
        require = logic_require_create(stack_noe, "id-reserve-insert");
        if (require == NULL) {
            CPE_ERROR(op->m_em, "%s: create require id-reserve-insert fail!", mongo_id_reserve_op_name(op)); 
            return logic_op_exec_result_null;
        }

        id_info->m_processing_require = logic_require_id(require);
        if (mongo_id_reserve_send_insert_pkg(op, require, id_info, reserve_count) != 0) {
            logic_require_error(require);
            return logic_op_exec_result_null;
        }
        else {
            return logic_op_exec_result_true;
        }                
    }

    result = (SYSIDINFO *)logic_data_record_at(result_data, 0);

    id_info->m_next_id = result->id_value - reserve_count;
    id_info->m_left_id_count = reserve_count;

    if (op->m_debug) {
        CPE_INFO(
            op->m_em, "%s: reserve success, next-id="FMT_UINT64_T", count=%u!",
            mongo_id_reserve_op_name(op), id_info->m_next_id, id_info->m_left_id_count);
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
mongo_id_reserve_recv_insert(
    logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require,
    struct mongo_id_reserve_op * op, struct mongo_id_info * id_info, uint32_t reserve_count, cfg_t cfg)
{
    if (logic_require_state(require) != logic_require_state_done) {
        if (logic_require_error(require) == 123 /*TODO: */) {
            logic_require_free(require);
            id_info->m_processing_require = INVALID_LOGIC_REQUIRE_ID;
            return mongo_id_reserve_send(ctx, stack_noe, op, cfg);
        }
        else {
            CPE_ERROR(
                op->m_em, "%s: %s: require-id=%d, state=%d, error=%d!",
                mongo_id_reserve_op_name(op), logic_require_name(require),
                logic_require_id(require), logic_require_state(require), logic_require_error(require)); 
            return logic_op_exec_result_null;
        }
    }

    id_info->m_next_id = id_info->m_id_start;
    id_info->m_left_id_count = reserve_count;

    if (op->m_debug) {
        CPE_INFO(
            op->m_em, "%s: reserve success, next-id="FMT_UINT64_T", count=%u!",
            mongo_id_reserve_op_name(op), id_info->m_next_id, id_info->m_left_id_count);
    }

    return logic_op_exec_result_true;
}

static logic_op_exec_result_t
mongo_id_reserve_recv_wait(
    logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require,
    struct mongo_id_reserve_op * op, cfg_t cfg)
{
    if (logic_require_state(require) != logic_require_state_done) {
        CPE_ERROR(
            op->m_em, "%s: %s, require-id=%d, state=%d!",
            mongo_id_reserve_op_name(op),
            logic_require_name(require),
            logic_require_id(require),
            logic_require_state(require)); 

        return logic_op_exec_result_null;
    }

    logic_require_free(require);
    return mongo_id_reserve_send(ctx, stack_noe, op, cfg);
}

static logic_op_exec_result_t
mongo_id_reserve_recv(logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require, void * user_data, cfg_t cfg) {
    struct mongo_id_reserve_op * op = (struct mongo_id_reserve_op *)user_data;
    const char * id_name;
    uint32_t reserve_count;
    struct mongo_id_info * id_info;
    const char * require_name;
    logic_op_exec_result_t rv = logic_op_exec_result_null;

    require_name = logic_require_name(require);
    if (strcmp(require_name, "id-reserve-wait") == 0) {
        return mongo_id_reserve_recv_wait(ctx, stack_noe, require, op, cfg);
    }

    id_name = cfg_get_string(cfg, "id-name", NULL);
    reserve_count = cfg_get_uint32(cfg, "reserve-count", 0);
    id_info = mongo_id_info_find(op->m_id_generator, id_name);
    if (id_info == NULL) {
        CPE_ERROR(
            op->m_em, "%s: id-name %s not exist",
            mongo_id_reserve_op_name(op), id_name);
        return logic_op_exec_result_null;
    }

    if (id_info->m_id_inc > reserve_count) {
        reserve_count = id_info->m_id_inc;
    }

    if (strcmp(require_name, "id-reserve-query") == 0) {
        rv = mongo_id_reserve_recv_query(ctx, stack_noe, require, op, id_info, reserve_count, cfg);
    }
    else if (strcmp(require_name, "id-reserve-insert") == 0) {
        rv = mongo_id_reserve_recv_insert(ctx, stack_noe, require, op, id_info, reserve_count, cfg);
    }
    else {
        CPE_ERROR(
            op->m_em, "%s: %s, unknown require type!",
            mongo_id_reserve_op_name(op), logic_require_name(require));
        rv = logic_op_exec_result_null;
    }

    if (!mongo_id_info_have_waiting_require(op->m_id_generator, id_info)) {
        id_info->m_processing_require = INVALID_LOGIC_REQUIRE_ID;
        logic_require_queue_notify_all(id_info->m_waiting_queue, 0);

        if (op->m_debug) {
            CPE_ERROR(
                op->m_em, "%s: %s: weakup all requires!",
                mongo_id_reserve_op_name(op), id_info->m_name);
        }
    }

    return rv;
}

EXPORT_DIRECTIVE
int mongo_id_reserve_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    mongo_id_generator_t generator;
    logic_manage_t logic_manage;
    const char * generator_name;
    struct mongo_id_reserve_op * op;

    generator_name = cfg_get_string(cfg, "id-generator", NULL);
    if (generator_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: id-generator not configured!",
            gd_app_module_name(module));
        return -1;
    }

    generator = mongo_id_generator_find_nc(app, generator_name);
    if (generator == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s create: id-generator %s not exist!",
            gd_app_module_name(module), generator_name);
        return -1;
    }

    logic_manage = logic_manage_find_nc(app, cfg_get_string(cfg, "logic-manage", NULL));
    if (logic_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: logic-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "logic-manage", "default"));
        return -1;
    }

    op = mongo_id_reserve_op_create(app, gd_app_module_name(module), generator, logic_manage, gd_app_alloc(app), gd_app_em(app));
    if (op == NULL) return -1;

    op->m_debug = cfg_get_int32(cfg, "debug", op->m_debug);

    return 0;
}

EXPORT_DIRECTIVE
void mongo_id_reserve_app_fini(gd_app_context_t app, gd_app_module_t module) {
    struct mongo_id_reserve_op * op;

    op = mongo_id_reserve_op_find_nc(app, gd_app_module_name(module));
    if (op) {
        mongo_id_reserve_op_free(op);
    }
}
