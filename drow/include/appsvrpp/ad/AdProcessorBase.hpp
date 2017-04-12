#ifndef APPSVRPP_AD_PROCESSORBASE_H
#define APPSVRPP_AD_PROCESSORBASE_H
#include "AdManager.hpp"
#include "AdProcessor.hpp"

namespace AppSvr { namespace Ad {

class AdProcessorBase : public AdProcessor {
public:
    AdProcessorBase(AdManager & mgr);
    ~AdProcessorBase();

    template<typename T>
    uint32_t startAd(
        const char * action_name,
        T & r,
        void (T::*fun)(uint32_t request_id, appsvr_ad_result_t result))
    {
        return m_mgr.startAd(action_name, r, fun);
    }

    uint32_t startAd(const char * action_name) {
        return m_mgr.startAd(action_name);
    }

    void removeAdAction(uint32_t action_id) { m_mgr.removeAdAction(action_id); }
    
private:
    AdManager & m_mgr;
};

}}

#endif
