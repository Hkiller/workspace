#include <limits>
#include "gdpp/app/Log.hpp"
#include "appsvr/ad/appsvr_ad_module.h"
#include "appsvrpp/ad/AdManager.hpp"
#include "appsvrpp/ad/AdProcessor.hpp"

namespace AppSvr { namespace Ad {

struct AdManagerProcessCtx {
    gd_app_context_t m_app;
    AdProcessFun m_fun;
    AdProcessor * m_useResponser;
};

static void ad_on_response_fun(void * ctx, void * arg, uint32_t req_id, appsvr_ad_result_t result) {
    AdManagerProcessCtx * processCtx = (AdManagerProcessCtx *)arg;
    try {
        (processCtx->m_useResponser->*(processCtx->m_fun))(req_id, result);
    }
    APP_CTX_CATCH_EXCEPTION(processCtx->m_app, "ad on result:");
}

static void ad_ctx_free(void * ctx) {
    delete (AdManagerProcessCtx *)ctx;
}

uint32_t AdManager::startAd(
    const char * action_name,
    AdProcessor& realResponser, AdProcessFun fun
#ifdef _MSC_VER
    , AdProcessor& useResponser
#endif
    )
{
    AdManagerProcessCtx * ctx = new AdManagerProcessCtx;
    ctx->m_app = app();
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    uint32_t action_id;
    if (appsvr_ad_module_start(*this, action_name, &action_id, &realResponser, ad_on_response_fun, ctx, ad_ctx_free) != 0) {
        delete ctx;
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: AdManager::start action fail!", name());
    }

    return action_id;
}

uint32_t AdManager::startAd(const char * action_name) {
    uint32_t action_id;
    if (appsvr_ad_module_start(*this, action_name, &action_id, NULL, NULL, NULL, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "%s: AdManager::start action fail!", name());
    }

    return action_id;
}

void AdManager::removeAdAction(uint32_t action_id) {
    appsvr_ad_module_remove_by_id(*this, action_id);
}

void AdManager::removeAdActions(AdProcessor & realResponser) {
    appsvr_ad_module_remove_by_ctx(*this, &realResponser);
}

AdManager & AdManager::_cast(appsvr_ad_module_t mgr) {
    if (mgr == NULL) {
        throw ::std::runtime_error("Gd::Evt::AdManager::_cast: input mgr is NULL!");
    }

    return *(AdManager*)mgr;
}

AdManager & AdManager::instance(gd_app_context_t app, const char * name) {
    appsvr_ad_module_t mgr = appsvr_ad_module_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "appsvr_ad_module %s not exist!", name ? name : "default");
    }

    return *(AdManager*)mgr;
}

}}


