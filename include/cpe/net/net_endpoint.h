#ifndef CPE_NET_ENDPOINT_H
#define CPE_NET_ENDPOINT_H
#include "cpe/utils/memory.h"
#include "cpe/tl/tl_types.h"
#include "net_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*服务管理接口*/
net_ep_t net_ep_create(net_mgr_t nmgr);
void net_ep_free(net_ep_t ep);

net_ep_t net_ep_find(net_mgr_t nmgr, net_ep_id_t id);

net_mgr_t net_ep_mgr(net_ep_t ep);
net_ep_id_t net_ep_id(net_ep_t ep);
net_chanel_t net_ep_chanel_r(net_ep_t ep);
net_chanel_t net_ep_chanel_w(net_ep_t ep);
void net_ep_set_chanel_r(net_ep_t ep, net_chanel_t chanel);
void net_ep_set_chanel_w(net_ep_t ep, net_chanel_t chanel);

int net_ep_set_timeout(net_ep_t ep, tl_time_span_t span);
tl_time_span_t net_ep_timeout(net_ep_t ep);

void net_ep_close(net_ep_t ep);

int net_ep_is_open(net_ep_t ep);
net_connector_t net_ep_connector(net_ep_t ep);

void net_ep_set_processor(net_ep_t ep, net_process_fun_t process_fun, void * process_ctx);

void net_ep_set_status(net_ep_t ep, enum net_status status);

int net_ep_localname(net_ep_t ep, uint32_t * ip, uint16_t * port);
int net_ep_peername(net_ep_t ep, uint32_t * ip, uint16_t * port);

/*服务读写接口*/
int net_ep_send(net_ep_t ep, const void * buf, size_t size);
ssize_t net_ep_rece(net_ep_t ep, void * buf, size_t capacity);

size_t net_ep_size(net_ep_t ep);
void * net_ep_peek(net_ep_t ep, void * buf, size_t size);
void net_ep_erase(net_ep_t ep, size_t size);

const char * net_ep_event_str(net_ep_event_t evt);

#ifdef __cplusplus
}
#endif

#endif
