#ifndef GDPP_APP_ATTR_REQUEST_H
#define GDPP_APP_ATTR_REQUEST_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gd/app_attr/app_attr_request.h"
#include "System.hpp"

namespace Gd { namespace AppAttr {

class AppAttrRequest : public Cpe::Utils::SimulateObject {
public:
    operator app_attr_request_t (void) const { return (app_attr_request_t)this; }

    uint32_t id(void) const { return app_attr_request_id(*this); }

    template<typename T>
    AppAttrRequest & onResult(
        T & r,
        void (T::*fun)(AppAttrRequest & request))
    {
#ifdef _MSC_VER
        return this->onResult(
            r, static_cast<AppAttrProcessFun>(fun)
            , *((AppAttrProcessor*)((void*)&r)));
#else
        return this->onResult(static_cast<AppAttrProcessor&>(r), static_cast<AppAttrProcessFun>(fun));
#endif
    }

    AppAttrRequest & formula(const char * name, const char * def);

    AppAttrRequest & onResult(
        AppAttrProcessor& realResponser, AppAttrProcessFun fun
#ifdef _MSC_VER
        , AppAttrProcessor& useResponser
#endif
        );

    AppAttrFormula & formula(const char * name);
    AppAttrFormula * findFormula(const char * name);
    
    static AppAttrRequest & _cast(app_attr_request_t request);
};

}}

#endif
