#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/app_env/plugin_app_env_request.h"
#include "appsvr_account_adapter_i.h"

appsvr_account_adapter_t
appsvr_account_adapter_create(
    appsvr_account_module_t module, uint8_t service_type, const char * service_name,
    size_t capacity, 
    appsvr_account_login_start_fun_t login_start_fun,
    appsvr_account_relogin_start_fun_t relogin_start_fun)
{
    appsvr_account_adapter_t adapter;

    adapter = mem_calloc(module->m_alloc, sizeof(struct appsvr_account_adapter) + capacity);

    if (adapter == NULL) {
        CPE_ERROR(module->m_em, "account: create adapter %s(%d): alloc fail!", service_name, service_type);
        return NULL;
    }

    adapter->m_module = module;
    adapter->m_service_type = service_type;
    cpe_str_dup(adapter->m_service_name, sizeof(adapter->m_service_name), service_name);
    adapter->m_fini = NULL;
    adapter->m_login_start_fun = login_start_fun;
    adapter->m_relogin_start_fun = relogin_start_fun;

    module->m_adapter_count++;

    TAILQ_INSERT_TAIL(&module->m_adapters, adapter, m_next);

    return adapter;
}

void appsvr_account_adapter_free(appsvr_account_adapter_t adapter) {
    appsvr_account_module_t module;

    module = adapter->m_module;

    if (module->m_runing_adapter == adapter) {
        module->m_runing_adapter = NULL;
        plugin_app_env_cancel_request_by_id(module->m_app_env, module->m_runing_id);
        module->m_runing_id = 0;
    }

    if (adapter->m_fini) adapter->m_fini(adapter);
    
    module->m_adapter_count--;
    TAILQ_REMOVE(&module->m_adapters, adapter, m_next);

    mem_free(module->m_alloc, adapter);
}

void appsvr_account_adapter_set_fini(appsvr_account_adapter_t adapter, appsvr_account_fini_fun_t fini) {
    adapter->m_fini = fini;
}

int appsvr_account_adapter_notify_login_result(
    appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN_RESULT const * result)
{
    appsvr_account_module_t module = adapter->m_module;
    int rv = 0;
    
    CPE_ERROR(module->m_em, "appsvr_account_adapter_notify_login_result: enter 1 !");
    if (module->m_runing_adapter != adapter) {
    CPE_ERROR(module->m_em, "appsvr_account_adapter_notify_login_result: enter 2 !");

        CPE_ERROR(module->m_em, "account: notify result: runing adapter mismatch!");
        return -1;
    }
    CPE_ERROR(module->m_em, "appsvr_account_adapter_notify_login_result: enter 3!");

    if (plugin_app_env_notify_request_result(
            adapter->m_module->m_app_env, module->m_runing_id,
            (int)result->result,
            module->m_meta_res_login, result, sizeof(*result))
        != 0)
    {
        CPE_ERROR(module->m_em, "account: notify result: notify result fail!");
        rv = -1;
    }

    module->m_runing_adapter = NULL;
    module->m_runing_id = 0;
    CPE_ERROR(module->m_em, "appsvr_account_adapter_notify_login_result: enter 4 !");

    return rv;
}

void * appsvr_account_adapter_data(appsvr_account_adapter_t adapter) {
    return adapter + 1;
}

