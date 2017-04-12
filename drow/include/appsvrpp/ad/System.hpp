#ifndef APPSVRPP_AD_SYSTEM_H
#define APPSVRPP_AD_SYSTEM_H
#include "appsvr/ad/appsvr_ad_types.h"

namespace AppSvr { namespace Ad {

class AdManager;
class AdProcessor;
class AdProcessorBase;

typedef void (AdProcessor::*AdProcessFun)(uint32_t request_id, appsvr_ad_result_t result);

}}

#endif
