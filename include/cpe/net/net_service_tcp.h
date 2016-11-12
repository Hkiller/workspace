#ifndef CPE_NET_SERVICE_TCP_H
#define CPE_NET_SERVICE_TCP_H
#include "net_service.h"

#ifdef __cplusplus
extern "C" {
#endif

/*tcp client operations*/
net_svr_t
net_svr_create_tcp_client(
    net_mgr_t nmgr, 
    const char * name,
    net_chanel_t readChanel,
    net_chanel_t writeChanel,
    const char * target_ip,
    short target_port);

const char * net_svr_tcp_client_target_ip(net_svr_t svr);
short net_svr_tcp_client_target_port(net_svr_t svr);

/*tcp server operations*/
net_svr_t
net_svr_create_tcp_server(
    net_mgr_t nmgr, 
    const char * name,
    net_chanel_t readChanel,
    net_chanel_t writeChanel,
    const char * ip,
    short port);

const char * net_svr_tcp_server_ip(net_svr_t svr);
short net_svr_tcp_server_port(net_svr_t svr);

#ifdef __cplusplus
}
#endif

#endif
