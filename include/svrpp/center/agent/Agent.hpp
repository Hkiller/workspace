#ifndef SVRPP_CENTER_AGENT_AGENT_H
#define SVRPP_CENTER_AGENT_AGENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/center/agent/center_agent.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Center {

class Agent : public Cpe::Utils::SimulateObject {
public:
    operator center_agent_t() const { return (center_agent_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)center_agent_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)center_agent_app(*this); }

    static Agent & instance(gd_app_context_t app, const char * name = NULL);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
