#ifndef APPSVRPP_AD_MANAGER_H
#define APPSVRPP_AD_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "appsvr/ad/appsvr_ad_module.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
# pragma warning(disable:4407)
#endif

namespace AppSvr { namespace Ad {

class AdManager : public Cpe::Utils::SimulateObject {
public:
    operator appsvr_ad_module_t (void) const { return (appsvr_ad_module_t)this; }

    const char * name(void) const { return appsvr_ad_module_name(*this); }
    
    Gd::App::Application & app(void) { return Gd::App::Application::_cast(appsvr_ad_module_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(appsvr_ad_module_app(*this)); }

    template<typename T>
    uint32_t startAd(
        const char * action_name, 
        T & r,
        void (T::*fun)(uint32_t request_id, appsvr_ad_result_t result))
    {
#ifdef _MSC_VER
        return this->startAd(
            action_name, r, static_cast<AdProcessFun>(fun)
            , *((AdProcessor*)((void*)&r)));
#else
        return this->startAd(action_name, static_cast<AdProcessor&>(r), static_cast<AdProcessFun>(fun));
#endif
    }
    
    uint32_t startAd(
        const char * action_name,
        AdProcessor& realResponser, AdProcessFun fun
#ifdef _MSC_VER
        , AdProcessor& useResponser
#endif
        );

    uint32_t startAd(const char * action_name);

    void removeAdAction(uint32_t action_id);
    void removeAdActions(AdProcessor & realResponser);
    
    static AdManager & instance(gd_app_context_t app, const char * name = NULL);
    static AdManager & _cast(appsvr_ad_module_t module);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
