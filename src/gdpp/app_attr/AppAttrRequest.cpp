#include <limits>
#include "gdpp/app/Log.hpp"
#include "gd/app_attr/app_attr_module.h"
#include "gd/app_attr/app_attr_request.h"
#include "gdpp/app_attr/AppAttrRequest.hpp"
#include "gdpp/app_attr/AppAttrProcessor.hpp"
#include "gdpp/app_attr/AppAttrFormula.hpp"

namespace Gd { namespace AppAttr {

struct AppAttrRequestProcessCtx {
    gd_app_context_t m_app;
    AppAttrProcessFun m_fun;
    AppAttrProcessor * m_useResponser;
};

static void app_attr_request_response_fun(void * ctx, app_attr_request_t request, void * arg) {
    AppAttrRequestProcessCtx * processCtx = (AppAttrRequestProcessCtx *)arg;
    try {
        (processCtx->m_useResponser->*(processCtx->m_fun))(*(AppAttrRequest *)request);
    }
    APP_CTX_CATCH_EXCEPTION(processCtx->m_app, "ad on result:");
}

static void app_attr_request_response_ctx_free(void * ctx) {
    delete (AppAttrRequestProcessCtx *)ctx;
}

AppAttrRequest & AppAttrRequest::formula(const char * name, const char * def) {
    app_attr_formula_t formula = app_attr_formula_create(*this, name, def);
    if (formula == NULL) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(*this));
        
        app_attr_request_free(*this);
        
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "AppAttrRequest::fail to add formula %s: %s!", name, def);
    }

    return *this;
}

AppAttrFormula & AppAttrRequest::formula(const char * name) {
    app_attr_formula_t formula = app_attr_formula_find(*this, name);
    if (formula == NULL) {
        gd_app_context_t app = app_attr_module_app(app_attr_request_module(*this));
        
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "AppAttrRequest formula %s not exist!", name);
    }

    return *(AppAttrFormula*)formula;
}

AppAttrFormula * AppAttrRequest::findFormula(const char * name) {
    app_attr_formula_t formula = app_attr_formula_find(*this, name);
    if (formula == NULL) return NULL;
    return (AppAttrFormula*)formula;
}

AppAttrRequest & AppAttrRequest::onResult(
    AppAttrProcessor& realResponser, AppAttrProcessFun fun
#ifdef _MSC_VER
    , AppAttrProcessor& useResponser
#endif
    )
{
    gd_app_context_t app = app_attr_module_app(app_attr_request_module(*this));
    
    AppAttrRequestProcessCtx * ctx = new AppAttrRequestProcessCtx;
    ctx->m_app = app;
    ctx->m_fun = fun;
#ifdef _MSC_VER
    ctx->m_useResponser = &useResponser;
#else
    ctx->m_useResponser = &realResponser;
#endif

    if (app_attr_request_set_result_processor(
            *this, &realResponser, app_attr_request_response_fun, ctx, app_attr_request_response_ctx_free) != 0)
    {
        delete ctx;
        app_attr_request_free(*this);
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "AppAttrRequest::start action fail!");
    }

    return *this;
}

AppAttrRequest & AppAttrRequest::_cast(app_attr_request_t request) {
    if (request == NULL) {
        throw ::std::runtime_error("Gd::Evt::AppAttrRequest::_cast: request is NULL!");
    }

    return *(AppAttrRequest*)request;
}

}}


