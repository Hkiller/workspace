#include "gdpp/app_attr/AppAttrProcessor.hpp"
#include "gdpp/app_attr/AppAttrProcessorBase.hpp"

namespace Gd { namespace AppAttr {

AppAttrProcessor::~AppAttrProcessor() {
}

AppAttrProcessorBase::AppAttrProcessorBase(AppAttrManager & mgr)
    : m_mgr(mgr)
{
}

AppAttrProcessorBase::~AppAttrProcessorBase() {
    m_mgr.removeAppAttrRequests(*this);
}

}}

