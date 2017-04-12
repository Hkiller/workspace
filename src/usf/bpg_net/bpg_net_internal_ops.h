#ifndef USF_BPG_NET_INTERNAL_OPS_H
#define USF_BPG_NET_INTERNAL_OPS_H
#include "cpe/net/net_types.h"
#include "bpg_net_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*agent process*/
void bpg_net_agent_accept(net_listener_t listener, net_ep_t ep, void * ctx);
int bpg_net_agent_reply(dp_req_t req, void * ctx, error_monitor_t em);
dp_req_t bpg_net_agent_req_buf(bpg_net_agent_t mgr);
int bpg_net_agent_notify_client(dp_req_t req, void * ctx, error_monitor_t em);

/*client process*/
int bpg_net_client_ep_init(bpg_net_client_t client, net_ep_t ep, size_t read_chanel_size, size_t write_chanel_size);
dp_req_t bpg_net_client_req_buf(bpg_net_client_t mgr);
int bpg_net_client_send(dp_req_t req, void * ctx, error_monitor_t em);

int bpg_net_client_save_require_id(bpg_net_client_t client, logic_require_id_t id);
int bpg_net_client_remove_require_id(bpg_net_client_t client, logic_require_id_t id);
void bpg_net_client_notify_all_require_disconnect(bpg_net_client_t client);

#ifdef __cplusplus
}
#endif

#endif
