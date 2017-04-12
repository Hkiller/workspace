#include <stdexcept>
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "svrpp/conn/net_cli/NetMonitor.hpp" 
#include "svrpp/conn/net_cli/NetClient.hpp" 

namespace Svr { namespace Conn {

static void process_cli_state_update(conn_net_cli_t cli, void * ctx) {
    NetMonitor * monitor = (NetMonitor*)ctx;
    try {
        monitor->onStateUpdate(*(NetClient*)cli);
    }
    catch(::std::exception const & e) {
        APP_CTX_ERROR(conn_net_cli_app(cli), "process_cli_state_update: %s!", e.what());
    }
    catch(...) {
        APP_CTX_ERROR(conn_net_cli_app(cli), "process_cli_state_update: unknown exception!");
    }
}

NetMonitor::NetMonitor(NetClient & cli)
    : m_cli(cli)
{
    if (conn_net_cli_monitor_add(cli, process_cli_state_update, this) != 0) {
        APP_CTX_THROW_EXCEPTION(
            cli.app(),
            ::std::runtime_error,
            "Svr::Conn:Monitor regist fail!");
    }
}

NetMonitor::~NetMonitor() {
    conn_net_cli_monitor_remove(m_cli, process_cli_state_update, this);
}

}}
