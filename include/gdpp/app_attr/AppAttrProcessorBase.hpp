#ifndef GDPP_APP_ATTR_PROCESSORBASE_H
#define GDPP_APP_ATTR_PROCESSORBASE_H
#include "AppAttrManager.hpp"
#include "AppAttrProcessor.hpp"
#include "AppAttrRequest.hpp"
#include "AppAttrFormula.hpp"

namespace Gd { namespace AppAttr {

class AppAttrProcessorBase : public AppAttrProcessor {
public:
    AppAttrProcessorBase(AppAttrManager & mgr);
    ~AppAttrProcessorBase();

    AppAttrRequest & startAppAttrRequest(void) { return m_mgr.startAppAttrRequest(); }

    void removeAppAttrRequest(uint32_t request_id) { m_mgr.removeAppAttrRequest(request_id); }
    
private:
    AppAttrManager & m_mgr;
};

}}

#endif
