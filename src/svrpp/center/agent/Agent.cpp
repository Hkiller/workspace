#include <stdexcept>
#include "gdpp/app/Log.hpp"
#include "svrpp/center/agent/Agent.hpp" 

namespace Svr { namespace Center {

Agent & Agent::instance(gd_app_context_t app, const char * name) {
    center_agent_t agent = center_agent_find_nc(app, name);
    if (agent == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "center_agent %s not exist!", name);
    }

    return *(Agent*)agent;
}

}}
