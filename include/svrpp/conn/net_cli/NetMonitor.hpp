#ifndef SVRPP_CONN_NET_CLI_MONITOR_H
#define SVRPP_CONN_NET_CLI_MONITOR_H
#include "System.hpp"

namespace Svr { namespace Conn {

class NetMonitor {
public:
    NetMonitor(NetClient & cli);
    virtual ~NetMonitor();

    virtual void onStateUpdate(NetClient & cli) = 0;

protected:
    NetClient & netClient(void) { return m_cli; }
    NetClient const & netClient(void) const { return m_cli; }

private:
    NetClient & m_cli;
};

}}

#endif
