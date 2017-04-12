#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_type.h"

static void logic_op_dump_all(logic_context_t context, cfg_t root) {
    struct mem_buffer buffer;
    gd_app_context_t app = logic_context_app(context);

    mem_buffer_init(&buffer, NULL);

    APP_CTX_INFO(app, "dump context\n%s", cfg_dump(root, &buffer, 4, 4));

    mem_buffer_clear(&buffer);
}

static void logic_op_dump_part(logic_context_t context, cfg_t root, const char * path) {
    gd_app_context_t app = logic_context_app(context);
    cfg_t child = cfg_find_cfg(root, path);
    if (child) {
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, NULL);
        APP_CTX_INFO(app, "    %s: %s", path, cfg_dump_inline(child, &buffer));
        mem_buffer_clear(&buffer);
    }
    else {
        APP_CTX_INFO(app, "    %s: NULL", path);
    }
}

logic_op_exec_result_t logic_use_op_dump(logic_context_t context, logic_stack_node_t stack_node, void * user_data, cfg_t args) {
    gd_app_context_t app = logic_context_app(context);
    cfg_t dump_data;
    cfg_t parts_cfg;

    if (cfg_get_int32(args, "dump-in-debug", 1)) {
        if (!logic_context_flag_is_enable(context, logic_context_flag_debug)) {
            return logic_op_exec_result_true;
        }
    }

    dump_data = cfg_create(gd_app_alloc(app));
    if (dump_data == NULL) {
        APP_CTX_ERROR(app, "logicuse_op_dump: create dump data fail!");
        return logic_op_exec_result_null;
    }

    logic_context_data_dump_to_cfg(context, dump_data);

    parts_cfg = cfg_find_cfg(args, "parts");
    if (parts_cfg == NULL) {
        logic_op_dump_all(context, dump_data);
    }
    else {
        struct cfg_it it;
        cfg_t child;
        cfg_it_init(&it, parts_cfg);

        while((child = cfg_it_next(&it))) {
            const char * path = cfg_as_string(child, NULL);
            if (path) {
                logic_op_dump_part(context, cfg_child_only(dump_data), path);
            }
        }
    }

    cfg_free(dump_data);

    return logic_op_exec_result_true;

};
