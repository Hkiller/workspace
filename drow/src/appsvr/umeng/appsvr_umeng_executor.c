#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_calc.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "plugin/app_env/plugin_app_env_executor.h"
#include "appsvr_umeng_executor.h"

static struct appsvr_umeng_function_def {
    const char * m_name;
    const char * m_args[5];
} s_functions[] = {
    { "onProfileSignIn", { "S:provider", "S:id", NULL, NULL, NULL } }
    , { "onProfileSignOff", { NULL, NULL, NULL, NULL, NULL } }
    , { "startLevel", { "S:id", NULL, NULL, NULL, NULL } }
    , { "failLevel", { "S:id", NULL, NULL, NULL, NULL } }
    , { "finishLevel", { "S:id", NULL, NULL, NULL, NULL } }
    , { "setPlayerLevel", { "I:level", NULL, NULL, NULL, NULL } }
    , { "use", { "S:item", "I:count", "D:price", NULL, NULL } }
    , { "bonus", { "S:item", "I:count", "D:price", "I:reason", NULL } }
    , { "buy", { "S:item", "I:count", "D:price", NULL, NULL } }
    , { "pay", { "D:cash", "S:item", "I:count", "D:price", "I:source" } }
    , { "onEvent", { "S:id", "I:count", "S:attrs", NULL, NULL } }
    , { "reportError", { "S:error", NULL, NULL, NULL, NULL } }
};

static void appsvr_umeng_exec(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_umeng_executor_t executor = (appsvr_umeng_executor_t)ctx;
    appsvr_umeng_module_t module = executor->m_module;
    struct dr_data_source data_source;

    data_source.m_data.m_meta = req_meta;
    data_source.m_data.m_data = (void*)req_data;
    data_source.m_data.m_size = req_size;
    data_source.m_next = NULL;

    if (executor->m_condition) {
        if (!dr_calc_int8_with_dft(module->m_computer, executor->m_condition, &data_source, 0)) {
            if (module->m_debug >= 2) {
                CPE_INFO(module->m_em, "umeng: %s ==> %s ignore!", dr_meta_name(req_meta), executor->m_op_name);
            }
            return;
        }
    }

    appsvr_umeng_executor_backend_exec(executor, &data_source);
}

appsvr_umeng_executor_t
appsvr_umeng_executor_create(
    appsvr_umeng_module_t module, LPDRMETA meta,
    struct appsvr_umeng_function_def * function_def, const char * condition, cfg_t args)
{
    appsvr_umeng_executor_t umeng_executor;
    size_t condition_len = condition ? strlen(condition) + 1 : 0;
    
    umeng_executor = (appsvr_umeng_executor_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_umeng_executor) + condition_len);
    if (umeng_executor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_umeng_executor_create: alloc fail!");
        return NULL;
    }

    umeng_executor->m_module = module;
    umeng_executor->m_op_name = function_def->m_name;
    if (condition) {
        char * p = (char *)(umeng_executor + 1);
        memcpy(p, condition, condition_len);
        *cpe_str_trim_tail(p + condition_len - 1, p) = 0;
        umeng_executor->m_condition = (const char *)p;
    }
    else {
        umeng_executor->m_condition = NULL;
    }
        
    for(umeng_executor->m_arg_count = 0; umeng_executor->m_arg_count < CPE_ARRAY_SIZE(function_def->m_args); umeng_executor->m_arg_count++) {
        struct appsvr_umeng_executor_arg * arg;
        const char * arg_def = function_def->m_args[umeng_executor->m_arg_count];
        const char * arg_name;
        const char * arg_value;

        if (arg_def == NULL) break;

        arg = &umeng_executor->m_args[umeng_executor->m_arg_count];

        arg->m_type = arg_def[0];
        arg_name = arg_def + 2;
        
        arg_value = cfg_get_string(args, arg_name, NULL);
        if (arg_value == NULL) {
            CPE_ERROR(module->m_em, "appsvr_umeng_executor_create: %s: arg %s not configured!", function_def->m_name, arg_name);
            mem_free(module->m_alloc, umeng_executor);
            return NULL;
        }

        cpe_str_dup(arg->m_def, sizeof(arg->m_def), arg_value);
        *cpe_str_trim_tail(arg->m_def + strlen(arg->m_def), arg->m_def) = 0;
    }

    if (appsvr_umeng_executor_backend_init(umeng_executor) != 0) {
        mem_free(module->m_alloc, umeng_executor);
        return NULL;
    }
    
    umeng_executor->m_executor =
        plugin_app_env_executor_create_oneway(module->m_app_env, meta, umeng_executor, appsvr_umeng_exec);
    if (umeng_executor->m_executor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_umeng_executor_create: create executor fail!");
        appsvr_umeng_executor_backend_fini(umeng_executor);
        mem_free(module->m_alloc, umeng_executor);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&module->m_executors, umeng_executor, m_next);
    
    return umeng_executor;
}

void appsvr_umeng_executor_free(appsvr_umeng_executor_t executor) {
    appsvr_umeng_module_t module = executor->m_module;

    assert(executor->m_executor);
    plugin_app_env_executor_free(executor->m_executor);
    executor->m_executor = NULL;

    appsvr_umeng_executor_backend_fini(executor);
    
    TAILQ_REMOVE(&module->m_executors, executor, m_next);

    mem_free(module->m_alloc, executor);
}

struct appsvr_umeng_function_def *
appsvr_umeng_function_def_find(const char * name) {
    uint8_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(s_functions); ++i) {
        struct appsvr_umeng_function_def * def = &s_functions[i];
        if (strcmp(def->m_name, name) == 0) return def;
    }
    
    return NULL;
}

int appsvr_umeng_load_executors(appsvr_umeng_module_t module, cfg_t cfg) {
    dr_store_manage_t store_manage;
    dr_store_t store;
    const char * store_name;
    LPDRMETALIB metalib;
    struct cfg_it data_it;
    cfg_t data_cfg;
    
    store_manage = dr_store_manage_find_nc(module->m_app, NULL);
    if (store_manage == NULL) {
        CPE_ERROR(module->m_em, "umeng: load executors: dr store manage not exist!");
        return -1;
    }
    
    store_name = cfg_get_string(cfg, "dr-store", NULL);
    if (store_name == NULL) {
        CPE_ERROR(module->m_em, "umeng: load executors: dr-store not configured!");
        return -1;
    }

    store = dr_store_find(store_manage, store_name);
    if (store == NULL) {
        CPE_ERROR(module->m_em, "umeng: load executors: dr-store %s not exist!", store_name);
        return -1;
    }

    metalib = dr_store_lib(store);
    if (metalib == NULL) {
        CPE_ERROR(module->m_em, "umeng: load executors: dr-store %s not loaded!", store_name);
        return -1;
    }

    cfg_it_init(&data_it, cfg_find_cfg(cfg, "executors"));
    while((data_cfg = cfg_it_next(&data_it))) {
        const char * meta_name;
        LPDRMETA meta;
        struct cfg_it op_it;
        cfg_t op_cfg;

        data_cfg = cfg_child_only(data_cfg);
        if (data_cfg == NULL) {
            CPE_ERROR(module->m_em, "umeng: load executors: data cfg format error!");
            return -1;
        }

        meta_name = cfg_name(data_cfg);
        meta = dr_lib_find_meta_by_name(metalib, meta_name);
        if (meta == NULL) {
            CPE_ERROR(module->m_em, "umeng: load executors: dr-store %s no meta %s!", store_name, meta_name);
            return -1;
        }

        cfg_it_init(&op_it, data_cfg);
        while((op_cfg = cfg_it_next(&op_it))) {
            appsvr_umeng_executor_t executor;
            struct appsvr_umeng_function_def * function_def;
            cfg_t op_args;
            const char * op_name;
            const char * op_condition;
            
            if (cfg_type(op_cfg) == CPE_CFG_TYPE_STRING) {
                op_name = cfg_as_string(op_cfg, NULL);
                op_args = NULL;
                op_condition = NULL;
                assert(op_name);
            }
            else if (cfg_type(op_cfg) == CPE_CFG_TYPE_STRUCT) {
                op_cfg = cfg_child_only(op_cfg);
                op_name = cfg_name(op_cfg);
                op_args = cfg_find_cfg(op_cfg, "args");
                op_condition = cfg_get_string(op_cfg, "condition", NULL);
            }
            else {
                CPE_ERROR(module->m_em, "umeng: load executors: data %s: unknown op cfg type %d!", meta_name, cfg_type(op_cfg));
                return -1;
            }
            
            function_def = appsvr_umeng_function_def_find(op_name);
            if (function_def == NULL) {
                CPE_ERROR(module->m_em, "umeng: load executors: data %s: op %s not exist!", meta_name, op_name);
                return -1;
            }
            
            executor = appsvr_umeng_executor_create(module, meta, function_def, op_condition, op_args);
            if (executor == NULL) {
                return -1;
            }
        }
    }
    
    return 0;
}
