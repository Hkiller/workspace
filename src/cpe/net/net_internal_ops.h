#ifndef GD_NET_INTERNAL_OPS_H
#define GD_NET_INTERNAL_OPS_H
#include "net_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*connector op*/
uint32_t net_connector_hash(net_connector_t connector);
int net_connector_cmp(net_connector_t l, net_connector_t r);
void net_connector_on_disconnect(net_connector_t l);
void net_connectors_free(net_mgr_t nmgr);

/*listener op*/
uint32_t net_listener_hash(net_listener_t listener);
int net_listener_cmp(net_listener_t l, net_listener_t r);
void net_listeners_free(net_mgr_t nmgr);

/*chanel ops*/
#define net_chanel_read_from_buf(c, b, s) ((c)->m_type->read_from_buf(c, b, s))
#define net_chanel_write_to_buf(c, b, s) ((c)->m_type->write_to_buf(c, b, s))

/*ep ops*/
int net_ep_set_fd(net_ep_t ep, int fd);
void net_ep_close_i(net_ep_t ep, net_ep_event_t ev);

/*ep_pages op*/
net_ep_t net_ep_pages_alloc_ep(net_mgr_t nmgr);
void net_ep_pages_free_ep(net_ep_t);
void net_ep_pages_free(net_mgr_t nmgr);

/*tcp ops*/
void net_socket_close(int * fd, error_monitor_t em);
int net_socket_set_none_block(int fd, error_monitor_t em);
int net_socket_set_reuseaddr(int fd, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
