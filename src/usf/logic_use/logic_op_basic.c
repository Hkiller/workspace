#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"

logic_op_exec_result_t logic_use_op_dump(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args);

logic_op_exec_result_t logic_use_op_false(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    return logic_op_exec_result_false;

};

logic_op_exec_result_t logic_use_op_true(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    return logic_op_exec_result_true;

};

logic_op_exec_result_t logic_use_op_null(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    return logic_op_exec_result_null;

};

EXPORT_DIRECTIVE
int logic_op_basic_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    logic_executor_type_t type_dump;
    logic_executor_type_t type_true;
    logic_executor_type_t type_false;
    logic_executor_type_t type_null;

    type_dump = logic_executor_type_create_global(
        app,
        cfg_get_string(cfg, "type-group", NULL),
        "dump",
        logic_use_op_dump,
        NULL,
        NULL,
        gd_app_em(app));

    type_true = logic_executor_type_create_global(
        app,
        cfg_get_string(cfg, "type-group", NULL),
        "true",
        logic_use_op_true,
        NULL,
        NULL,
        gd_app_em(app));

    type_false = logic_executor_type_create_global(
        app,
        cfg_get_string(cfg, "type-group", NULL),
        "false",
        logic_use_op_false,
        NULL,
        NULL,
        gd_app_em(app));

    type_null = logic_executor_type_create_global(
        app,
        cfg_get_string(cfg, "type-group", NULL),
        "null",
        logic_use_op_null,
        NULL,
        NULL,
        gd_app_em(app));

    if (type_dump && type_true && type_false && type_null) {
        return 0;
    }
    else {
        return -1;
    }
}

EXPORT_DIRECTIVE
void logic_op_basic_app_fini(gd_app_context_t app, gd_app_module_t module) {
}
