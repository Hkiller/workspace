#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "plugin/app_env/plugin_app_env_executor.h"
#include "appsvr_account_executor.h"
#include "appsvr_account_adapter_i.h"

int appsvr_account_executor_do_query_services(
    void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size,
    dr_data_t * r, mem_buffer_t result_alloc)
{
    appsvr_account_module_t module = ctx;
    size_t capacity = sizeof(APPSVR_ACCOUNT_SERVICE_LIST)
        + module->m_adapter_count * sizeof(APPSVR_ACCOUNT_SERVICE_INFO);
    dr_data_t result;
    APPSVR_ACCOUNT_SERVICE_LIST * res_list;
    appsvr_account_adapter_t adapter;
    
    mem_buffer_clear_data(result_alloc);
    result = mem_buffer_alloc(result_alloc, sizeof(struct dr_data) + capacity);

    result->m_meta = module->m_meta_res_query_services;
    result->m_data = (void*)(result + 1);
    result->m_size = capacity;

    res_list = result->m_data;
    res_list->service_count = 0;

    TAILQ_FOREACH(adapter, &module->m_adapters, m_next) {
        APPSVR_ACCOUNT_SERVICE_INFO * service_info = &res_list->services[res_list->service_count++];
        service_info->type = adapter->m_service_type;
        service_info->support_relogin = adapter->m_relogin_start_fun ? 1 : 0;
    }

    *r = result;
    
    return 0;
}

int appsvr_account_executor_do_login(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size, uint32_t request_id) {
    appsvr_account_module_t module = ctx;
    APPSVR_ACCOUNT_LOGIN const * req = (APPSVR_ACCOUNT_LOGIN const *)req_data;
    appsvr_account_adapter_t adapter = NULL;
    
    if (module->m_runing_adapter) {
        CPE_ERROR(
            module->m_em, "account: login: request %d adapter %s(%d) already runing!",
            module->m_runing_id, module->m_runing_adapter->m_service_name, module->m_runing_adapter->m_service_type);
        return -1;
    }
    
    assert(module->m_runing_id == 0);

    TAILQ_FOREACH(adapter, &module->m_adapters, m_next) {
        if (adapter->m_service_type == req->account_type) break;
    }

    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "account: login: service %d not exist!", req->account_type);
        return -1;
    }

    if (adapter->m_login_start_fun(adapter, req) != 0) {
        CPE_ERROR(
            module->m_em, "account: login: request %d adapter %s(%d) start fail!",
            request_id, adapter->m_service_name, adapter->m_service_type);
        return -1;
    }

    module->m_runing_id = request_id;
    module->m_runing_adapter = adapter;

    if (module->m_debug) {
        CPE_INFO(
            module->m_em, "account: login: request %d adapter %s(%d) start success!",
            request_id, adapter->m_service_name, adapter->m_service_type);
    }
    
    return 0;
}

int appsvr_account_executor_do_relogin(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size, uint32_t request_id) {
    appsvr_account_module_t module = ctx;
    APPSVR_ACCOUNT_RELOGIN const * req = (APPSVR_ACCOUNT_RELOGIN const *)req_data;
    appsvr_account_adapter_t adapter = NULL;
    
    if (module->m_runing_adapter) {
        CPE_ERROR(
            module->m_em, "account: relogin: request %d adapter %s(%d) already runing!",
            module->m_runing_id, module->m_runing_adapter->m_service_name, module->m_runing_adapter->m_service_type);
        return -1;
    }
    
    assert(module->m_runing_id == 0);

    TAILQ_FOREACH(adapter, &module->m_adapters, m_next) {
        if (adapter->m_service_type == req->account_type) break;
    }

    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "account: relogin: service %d not exist!", req->account_type);
        return -1;
    }

    if (adapter->m_relogin_start_fun == NULL) {
        CPE_ERROR(module->m_em, "account: relogin: service %d not support relogin!", req->account_type);
        return -1;
    }

    if (adapter->m_relogin_start_fun(adapter, req) != 0) {
        CPE_ERROR(
            module->m_em, "account: relogin: request %d adapter %s(%d) start fail!",
            request_id, adapter->m_service_name, adapter->m_service_type);
        return -1;
    }

    module->m_runing_id = request_id;
    module->m_runing_adapter = adapter;

    if (module->m_debug) {
        CPE_INFO(
            module->m_em, "account: relogin: request %d adapter %s(%d) start success!",
            request_id, adapter->m_service_name, adapter->m_service_type);
    }
    
    return 0;
}

int appsvr_account_executor_init(appsvr_account_module_t module) {

    if (plugin_app_env_executor_create_async(
            module->m_app_env, module->m_meta_req_login, module, appsvr_account_executor_do_login) == NULL
        || plugin_app_env_executor_create_async(
            module->m_app_env, module->m_meta_req_relogin, module, appsvr_account_executor_do_relogin) == NULL
        || plugin_app_env_executor_create_sync(
            module->m_app_env, module->m_meta_req_query_services, module, appsvr_account_executor_do_query_services) == NULL
        )
    {
        CPE_ERROR(module->m_em, "appsvr_umeng_executor_create: create executor for %s fail!", dr_meta_name(module->m_meta_req_login));
        return -1;
    }

    return 0;
}

void appsvr_account_executor_fini(appsvr_account_module_t module) {
    plugin_app_env_executor_free_by_ctx(module->m_app_env, module);
}

