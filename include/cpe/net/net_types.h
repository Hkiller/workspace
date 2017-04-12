#ifndef CPE_NET_TYPES_H
#define CPE_NET_TYPES_H
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    net_connector_state_disable = -1
    , net_connector_state_idle = 0
    , net_connector_state_connecting
    , net_connector_state_connected
    , net_connector_state_error
} net_connector_state_t;

typedef enum net_chanel_type_id {
    net_chanel_type_id_queue
    , net_chanel_type_id_ring
} net_chanel_type_id_t;

typedef uint32_t net_ep_id_t;

typedef enum net_chanel_state {
    net_chanel_state_empty,
    net_chanel_state_have_data,
    net_chanel_state_full
} net_chanel_state_t;

typedef struct net_mgr * net_mgr_t;
typedef struct net_chanel_type * net_chanel_type_t;
typedef struct net_chanel * net_chanel_t;
typedef struct net_ep * net_ep_t;
typedef struct net_listener * net_listener_t;
typedef struct net_connector * net_connector_t;

typedef void (*net_accept_fun_t)(net_listener_t listener, net_ep_t ep, void * ctx);

typedef enum net_ep_event {
    net_ep_event_read
    , net_ep_event_open
    , net_ep_event_close_by_user
    , net_ep_event_close_by_peer
    , net_ep_event_close_by_error
    , net_ep_event_close_by_shutdown
} net_ep_event_t ;

typedef void (*net_process_fun_t)(net_ep_t ep, void * ctx, net_ep_event_t event);

typedef void (*net_connector_state_monitor_fun_t)(net_connector_t connector, void * ctx);

typedef void (*net_run_tick_fun_t)(void * ctx);

struct ev_loop;

enum net_status {
	NET_INVALID = 0,
	NET_CONNECTION,
	NET_REMOVE_AFTER_SEND,
};

#ifdef __cplusplus
}
#endif

#endif
