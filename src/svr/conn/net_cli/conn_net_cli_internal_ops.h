#ifndef SVR_CONN_CLI_INTERNAL_OPS_H
#define SVR_CONN_CLI_INTERNAL_OPS_H
#include "conn_net_cli_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void conn_net_cli_disconnect(conn_net_cli_t cli);
void conn_net_cli_apply_evt(conn_net_cli_t cli, enum conn_net_cli_fsm_evt_type type);
int conn_net_cli_start_state_timer(conn_net_cli_t cli, tl_time_span_t span);
void conn_net_cli_stop_state_timer(conn_net_cli_t cli);

int conn_net_cli_set_ringbuf_size(conn_net_cli_t conn, size_t capacity);
void conn_net_cli_link_node_r(conn_net_cli_t cli, ringbuffer_block_t blk);
void conn_net_cli_link_node_w(conn_net_cli_t conn, ringbuffer_block_t blk);

void conn_net_cli_rw_cb(EV_P_ ev_io *w, int revents);
void conn_net_cli_start_watch(conn_net_cli_t cli);

dp_req_t conn_net_cli_outgoing_pkg_buf(conn_net_cli_t cli, size_t capacity);
conn_net_cli_pkg_t conn_net_cli_incoming_pkg(conn_net_cli_t cli);

/*fsm impl operations*/
int conn_net_cli_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int conn_net_cli_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int conn_net_cli_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int conn_net_cli_fsm_create_established(fsm_def_machine_t fsm_def, error_monitor_t em);


/*conn_net_cli_svr_stub operations*/
conn_net_cli_svr_stub_t conn_net_cli_svr_stub_create(conn_net_cli_t cli, const char * svr_type_name, uint16_t svr_type_id);
void conn_net_cli_svr_stub_free(struct conn_net_cli_svr_stub * svr);
void conn_net_cli_svr_stub_free_all(conn_net_cli_t cli);

int conn_net_cli_svr_stub_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em);

uint32_t conn_net_cli_svr_stub_hash(conn_net_cli_svr_stub_t svr);
int conn_net_cli_svr_stub_eq(conn_net_cli_svr_stub_t l, conn_net_cli_svr_stub_t r);

/*cmd info operations*/
conn_net_cli_cmd_info_t conn_net_cli_cmd_info_create(conn_net_cli_svr_stub_t stub, LPDRMETAENTRY entry);
void conn_net_cli_cmd_info_free_all(conn_net_cli_svr_stub_t stub);
conn_net_cli_cmd_info_t
conn_net_cli_cmd_info_find_by_name(conn_net_cli_svr_stub_t stub, const char * meta_name);

uint32_t conn_net_cli_cmd_info_hash(conn_net_cli_cmd_info_t cmd_info);
int conn_net_cli_cmd_info_eq(conn_net_cli_cmd_info_t l, conn_net_cli_cmd_info_t r);

/*conn_net_cli_monitor operations*/
void conn_net_cli_monitor_process(fsm_machine_t fsm_ins, void * ctx);
void conn_net_cli_monitor_free(conn_net_cli_monitor_t mon);
void conn_net_cli_monitor_free_all(conn_net_cli_t cli);

#ifdef __cplusplus
}
#endif

#endif
