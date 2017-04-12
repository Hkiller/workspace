#include <limits>
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpepp/dr/Data.hpp"
#include "gdpp/app/Log.hpp"
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "pluginpp/app_env/AppEnvService.hpp"

namespace Drow { namespace AppEnv {

struct AppEnvServiceProcessCtx {
    gd_app_context_t m_app;
    AppEnvProcessFun m_fun;
    AppEnvProcessor * m_useResponser;
};

void app_env_process_fun(void * ctx, uint32_t id, int rv, dr_data_t result) {
    AppEnvServiceProcessCtx * processCtx = (AppEnvServiceProcessCtx *)ctx;

    try {
        if (result) {
            Cpe::Dr::Data data(result->m_data, result->m_meta, result->m_size);
            (processCtx->m_useResponser->*(processCtx->m_fun))(id, rv, &data);
        }
        else {
            (processCtx->m_useResponser->*(processCtx->m_fun))(id, rv, NULL);
        }
    }
    APP_CTX_CATCH_EXCEPTION(processCtx->m_app, "process app_env:");
}

void app_env_ctx_free(void * ctx) {
    delete (AppEnvServiceProcessCtx *)ctx;
}

uint32_t AppEnvService::execute(
        LPDRMETA req_meta, void const * req_data, size_t req_data_size,
        AppEnvProcessor& realResponser, AppEnvProcessFun fun
#ifdef _MSC_VER
        , AppEnvProcessor& useResponser
#endif
        )
{
    AppEnvServiceProcessCtx * ctx = new AppEnvServiceProcessCtx;
    ctx->m_app = app();
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    uint32_t id;
    if (plugin_app_env_exec_request_asnyc(
            *this, &id,
            ctx, app_env_process_fun, app_env_ctx_free,
            req_meta, req_data, req_data_size) != 0)
    {
        delete ctx;
        return 0;
    }

    return id;
}

dr_data_t AppEnvService::execute(
    LPDRMETA req_meta, void const * req_data, size_t req_data_size, mem_buffer_t buffer, LPDRMETA res_meta)
{
    dr_data_t result = NULL;
    if (plugin_app_env_exec_request_sync(*this, &result, buffer, req_meta, req_data, req_data_size) != 0) {
        return NULL;
    }

    if (result && res_meta) {
        if (result->m_meta != res_meta && strcmp(dr_meta_name(result->m_meta), dr_meta_name(res_meta)) != 0) {
            APP_CTX_ERROR(
                app(), "AppEnvService: execute sync: expect result meta %s, but is %s",
                dr_meta_name(res_meta), dr_meta_name(result->m_meta));
            return NULL;
        }
    }

    return result; 
}


struct AppEnvServiceMonitorCtx {
    gd_app_context_t m_app;
    AppEnvMonitorFun m_fun;
    AppEnvProcessor * m_useResponser;
};

int app_env_monitor_fun(void * ctx, LPDRMETA res_meta, void const * res_data, size_t res_size) {
    AppEnvServiceMonitorCtx * monitorCtx = (AppEnvServiceMonitorCtx *)ctx;

    try {
        Cpe::Dr::ConstData data(res_data, res_meta, res_size);
        return (monitorCtx->m_useResponser->*(monitorCtx->m_fun))(&data);
    }
    APP_CTX_CATCH_EXCEPTION(monitorCtx->m_app, "app_env: monitor: ");
    
    return -1;
}

void app_env_monitor_ctx_free(void * ctx) {
    delete (AppEnvServiceMonitorCtx *)ctx;
}

void AppEnvService::addMonitor(
    const char * meta_name, AppEnvProcessor& realResponser, AppEnvMonitorFun fun
#ifdef _MSC_VER
    , AppEnvProcessor& useResponser
#endif
    )
{
    AppEnvServiceMonitorCtx * ctx = new AppEnvServiceMonitorCtx;
    ctx->m_app = app();
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    plugin_app_env_monitor_t monitor =
        plugin_app_env_monitor_create(
            *this, meta_name,
            ctx, app_env_monitor_fun, app_env_monitor_ctx_free);
    if (monitor == NULL) {
        delete ctx;
        throw ::std::runtime_error("AppEnvService::addMonitor: create monitor fail!");
    }
}

AppEnvService & AppEnvService::_cast(plugin_app_env_module_t mgr) {
    if (mgr == NULL) {
        throw ::std::runtime_error("AppEnvService::_cast: input mgr is NULL!");
    }

    return *(AppEnvService*)mgr;
}

AppEnvService & AppEnvService::instance(gd_app_context_t app, const char * name) {
    plugin_app_env_module_t mgr = plugin_app_env_module_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "app_env_module %s not exist!", name ? name : "default");
    }

    return *(AppEnvService*)mgr;
}

}}


