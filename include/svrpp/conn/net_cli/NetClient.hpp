#ifndef SVRPP_CONN_CLI_CLIENT_H
#define SVRPP_CONN_CLI_CLIENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Conn {

class NetClient : public Cpe::Utils::SimulateObject {
public:
    operator conn_net_cli_t() const { return (conn_net_cli_t)this; }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)conn_net_cli_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)conn_net_cli_app(*this); }

    const char * name(void) const { return conn_net_cli_name(*this); }


    const char * ip(void) const { return conn_net_cli_svr_ip(*this); }
    uint16_t port(void) const { return conn_net_cli_svr_port(*this); }
    void setSvr(const char * ip, uint16_t port);

    conn_net_cli_state_t state(void) const { return conn_net_cli_state(*this); }
    void enable(void) { conn_net_cli_enable(*this); }
    void disable(void) { conn_net_cli_disable(*this); }

    void sendPkg(uint16_t to_svr, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len);
    void sendData(uint16_t to_svr, uint32_t sn, LPDRMETA meta, void const * data, uint16_t data_len);
    void sendCmd(uint16_t to_svr, uint32_t sn, uint32_t cmd);

    static NetClient & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
