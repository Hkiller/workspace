#ifndef SVR_SET_BPG_INTERNAL_OPS_H
#define SVR_SET_BPG_INTERNAL_OPS_H
#include "set_bpg_internal_types.h"

set_bpg_chanel_t
set_bpg_chanel_create(
    gd_app_context_t app,
    const char * name,
    bpg_pkg_manage_t bpg_pkg_manage,
    mem_allocrator_t alloc,
    error_monitor_t em);
void set_bpg_chanel_free(set_bpg_chanel_t mgr);

set_bpg_chanel_t
set_bpg_chanel_find(gd_app_context_t app, cpe_hash_string_t name);
set_bpg_chanel_t
set_bpg_chanel_find_nc(gd_app_context_t app, const char * name);

const char * set_bpg_chanel_name(set_bpg_chanel_t mgr);

int set_bpg_chanel_set_incoming_recv_at(set_bpg_chanel_t sp, const char * recv_at);
int set_bpg_chanel_set_incoming_dispatch_to(set_bpg_chanel_t sp, const char * dispatch_to);
int set_bpg_chanel_set_outgoing_recv_at(set_bpg_chanel_t sp, const char * recv_at);
int set_bpg_chanel_set_outgoing_dispatch_to(set_bpg_chanel_t sp, const char * dispatch_to);

int set_bpg_chanel_incoming_recv(dp_req_t req, void * ctx, error_monitor_t em);
int set_bpg_chanel_outgoing_recv(dp_req_t req, void * ctx, error_monitor_t em);

#endif
