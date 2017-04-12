#include "appsvrpp/ad/AdProcessor.hpp"
#include "appsvrpp/ad/AdProcessorBase.hpp"

namespace AppSvr { namespace Ad {

AdProcessor::~AdProcessor() {
}

AdProcessorBase::AdProcessorBase(AdManager & mgr)
    : m_mgr(mgr)
{
}

AdProcessorBase::~AdProcessorBase() {
    m_mgr.removeAdActions(*this);
}

}}

