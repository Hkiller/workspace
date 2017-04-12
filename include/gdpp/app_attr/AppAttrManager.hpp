#ifndef GDPP_APP_ATTR_MANAGER_H
#define GDPP_APP_ATTR_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "gd/app_attr/app_attr_module.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
# pragma warning(disable:4407)
#endif

namespace Gd { namespace AppAttr {

class AppAttrManager : public Cpe::Utils::SimulateObject {
public:
    operator app_attr_module_t (void) const { return (app_attr_module_t)this; }

    const char * name(void) const { return app_attr_module_name(*this); }
    
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(app_attr_module_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(app_attr_module_app(*this)); }

    AppAttrRequest & startAppAttrRequest(void);

    void removeAppAttrRequest(uint32_t request_id);
    void removeAppAttrRequests(AppAttrProcessor & realResponser);
    
    static AppAttrManager & instance(gd_app_context_t app, const char * name = NULL);
    static AppAttrManager & _cast(app_attr_module_t module);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
