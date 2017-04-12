#include <limits>
#include "gdpp/app/Log.hpp"
#include "gd/app_attr/app_attr_module.h"
#include "gd/app_attr/app_attr_request.h"
#include "gdpp/app_attr/AppAttrManager.hpp"
#include "gdpp/app_attr/AppAttrProcessor.hpp"
#include "gdpp/app_attr/AppAttrRequest.hpp"

namespace Gd { namespace AppAttr {

AppAttrRequest &
AppAttrManager::startAppAttrRequest(void) {
    app_attr_request_t request = app_attr_request_create(*this);
    if (request == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "startAppAttrRequest: create request fail");
    }

    return AppAttrRequest::_cast(request);
}

void AppAttrManager::removeAppAttrRequest(uint32_t request_id) {
    app_attr_request_remove_by_id(*this, request_id);
}

void AppAttrManager::removeAppAttrRequests(AppAttrProcessor & realResponser) {
    app_attr_request_remove_by_ctx(*this, &realResponser);
}

AppAttrManager & AppAttrManager::_cast(app_attr_module_t mgr) {
    if (mgr == NULL) {
        throw ::std::runtime_error("Gd::Evt::AppAttrManager::_cast: input mgr is NULL!");
    }

    return *(AppAttrManager*)mgr;
}

AppAttrManager & AppAttrManager::instance(gd_app_context_t app, const char * name) {
    app_attr_module_t mgr = app_attr_module_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "app_attr_module %s not exist!", name ? name : "default");
    }

    return *(AppAttrManager*)mgr;
}

}}


