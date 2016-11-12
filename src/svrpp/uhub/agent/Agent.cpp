#include <stdexcept>
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "svrpp/uhub/agent/Agent.hpp" 

namespace Svr { namespace UHub {

Agent & Agent::instance(gd_app_context_t app, const char * name) {
    uhub_agent_t agent = uhub_agent_find_nc(app, name);
    if (agent == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "uhub_agent %s not exist!", name);
    }

    return *(Agent*)agent;
}

void Agent::sendNotifyPkg(dp_req_t pkg, void const * carry_data, size_t carry_data_size) {
    if (uhub_agent_send_notify_pkg(*this, pkg, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Agent %s: send notify pkg fail!", name());
    }
}

void Agent::sendNotifyData(
    void const * data, uint16_t data_size, LPDRMETA meta,
    void const * carry_data, size_t carry_data_size)
{
    if (uhub_agent_send_notify_data(*this, data, data_size, meta, carry_data, carry_data_size) != 0) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "Agent %s: send notify data fail!", name());
    }
}

}}
