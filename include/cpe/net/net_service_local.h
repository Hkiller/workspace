#ifndef CPE_NET_SERVICE_LOCAL_H
#define CPE_NET_SERVICE_LOCAL_H
#include "net_service.h"

#ifdef __cplusplus
extern "C" {
#endif

net_svr_t
net_svr_create_local(
    net_mgr_t nmgr, 
    const char * name,
    net_chanel_t readChanel,
    net_chanel_t writeChanel,
    net_svr_process_fun_t);

#ifdef __cplusplus
}
#endif

#endif


