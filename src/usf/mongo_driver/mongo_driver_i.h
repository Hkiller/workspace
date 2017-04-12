#ifndef USF_MONGO_DRIVER_DRIVER_I_H
#define USF_MONGO_DRIVER_DRIVER_I_H
#include "ev.h"
#include "bson.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/dp/dp_types.h"
#include "cpe/net/net_types.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_types.h"
#include "usf/mongo_driver/mongo_driver.h"

typedef struct mongo_server * mongo_server_t;
typedef struct mongo_connection * mongo_connection_t;

typedef TAILQ_HEAD(mongo_server_list, mongo_server) mongo_server_list_t;
typedef TAILQ_HEAD(mongo_connection_list, mongo_connection) mongo_connection_list_t;

#define MONGO_NO_SET_VERSION -1

struct mongo_driver {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_is_enable;
    int64_t m_select_threshold_ms;
    
    fsm_def_machine_t m_fsm_def;

    char m_user[64];
    char m_passwd[64];
    char m_source[64];
    char m_replica_set[64];
    char m_auth_mechanism[16];
    mongo_topology_type_t m_topology_type;
    int64_t m_max_set_version;
    bson_oid_t m_max_election_id;
    
    int m_server_count;
    mongo_server_list_t m_servers;
    mongo_server_t m_master_server;

    uint32_t m_max_connection_id;
    
    uint32_t m_read_block_size;
    uint32_t m_reconnect_span_s;
    uint32_t m_op_timeout_ms;

    cpe_hash_string_t m_incoming_send_to;
    dp_rsp_t m_outgoing_recv_at;

    size_t m_pkg_buf_max_size;
    mongo_pkg_t m_pkg_buf;

    ringbuffer_t m_ringbuf;
    struct ev_loop * m_ev_loop;
    struct ev_timer m_timer_event;
    
    struct mem_buffer m_dump_buffer;

    int m_debug;
};

/*driver ops*/
uint32_t mongo_driver_cur_time_s(mongo_driver_t driver);
uint64_t mongo_driver_cur_time_ms(mongo_driver_t driver);
int mongo_driver_on_send(dp_req_t req, void * ctx, error_monitor_t em);
int32_t mongoc_driver_lowest_max_wire_version(mongo_driver_t driver);
void mongo_driver_tick_once(mongo_driver_t driver);

void mongo_driver_topology_update(mongo_driver_t driver, mongo_server_t server);
void mongo_driver_topology_suitable_servers(
    mongo_server_t * servers, uint32_t * server_count,
    mongo_driver_t driver, uint8_t for_writes, mongo_read_mode_t read_mode);

#endif
