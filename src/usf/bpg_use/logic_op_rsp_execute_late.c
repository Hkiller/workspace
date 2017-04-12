#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"

static
logic_op_exec_result_t
do_rsp_execute_late(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    logic_context_t late_context;
    bpg_rsp_manage_t rsp_manage;
    const char * rsp_manage_name;
    const char * rsp_name;

    rsp_name = cfg_get_string(args, "name", NULL);
    if (rsp_name == NULL) {
        APP_CTX_ERROR(
            logic_context_app(context), "rsp_execute_late: arg 'name' not configured!");
        return logic_op_exec_result_null;
    }

    rsp_manage_name = cfg_get_string(args, "rsp-manage", NULL);
    rsp_manage =
        bpg_rsp_manage_find_nc(logic_context_app(context), rsp_manage_name);
    if (rsp_manage == NULL) {
        APP_CTX_ERROR(
            logic_context_app(context), "rsp_execute_late: rsp-manage %s not exist!",
            rsp_manage_name ? rsp_manage_name : "default");
        return logic_op_exec_result_null;
    } 

    late_context = 
        bpg_rsp_manage_create_op_by_name(
            rsp_manage, context, rsp_name, gd_app_em(logic_context_app(context)));
    if (late_context == NULL) {
        APP_CTX_ERROR(
            logic_context_app(context), "rsp_execute_late: create context for %s.%s not exist!",
            rsp_manage_name ? rsp_manage_name : "default", rsp_name);
        return logic_op_exec_result_null;
    }

    return logic_op_exec_result_true;
};

EXPORT_DIRECTIVE
int logic_op_rsp_execute_late_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    logic_executor_type_t type;

    type = logic_executor_type_create_global(
        app,
        cfg_get_string(cfg, "type-group", NULL),
        gd_app_module_name(module),
        do_rsp_execute_late,
        NULL,
        NULL,
        gd_app_em(app));

    return type ? 0 : -1;
}

EXPORT_DIRECTIVE
void logic_op_rsp_execute_late_app_fini(gd_app_context_t app, gd_app_module_t module) {
}
