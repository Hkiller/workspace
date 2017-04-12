#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "svr/conn/net_cli/conn_net_cli_svr_stub.h"
#include "conn_net_cli_internal_ops.h"

conn_net_cli_svr_stub_t
conn_net_cli_svr_stub_create(conn_net_cli_t cli, const char * svr_type_name, uint16_t svr_type_id) {
    conn_net_cli_svr_stub_t svr;
    size_t name_len = strlen(svr_type_name) + 1;

    if (conn_net_cli_svr_stub_lsearch_by_name(cli, svr_type_name) != NULL) {
        CPE_ERROR(cli->m_em, "%s: create cli svr %s: name duplicate!", conn_net_cli_name(cli), svr_type_name);
        return NULL;
    }

    svr = mem_alloc(cli->m_alloc, sizeof(struct conn_net_cli_svr_stub) + name_len);
    if (svr == NULL) return NULL;

    svr->m_cli = cli;
    svr->m_svr_type_name = (char*)(svr + 1);
    svr->m_svr_type_id = svr_type_id;

    memcpy(svr->m_svr_type_name, svr_type_name, name_len);

    svr->m_pkg_meta = NULL;
    svr->m_pkg_cmd_entry = NULL;
    svr->m_pkg_data_entry = NULL;

    svr->m_error_pkg_meta = NULL;
    svr->m_error_pkg_cmd = 0;
    svr->m_error_pkg_error_entry = NULL;

    svr->m_response_dispatch_to = NULL;
    svr->m_notify_dispatch_to = NULL;
    svr->m_outgoing_recv_at = NULL;

    if (cpe_hash_table_init(
            &svr->m_cmds,
            svr->m_cli->m_alloc,
            (cpe_hash_fun_t) conn_net_cli_cmd_info_hash,
            (cpe_hash_eq_t) conn_net_cli_cmd_info_eq,
            CPE_HASH_OBJ2ENTRY(conn_net_cli_cmd_info, m_hh),
            -1) != 0)
    {
        CPE_ERROR(
            svr->m_cli->m_em, "%s: create svr stub %s(%d): init cmd hash table fail!",
            conn_net_cli_name(svr->m_cli), svr_type_name, svr_type_id);
        mem_free(cli->m_alloc, svr);
        return NULL;
    }

    cpe_hash_entry_init(&svr->m_hh);
    if (cpe_hash_table_insert_unique(&cli->m_svrs, svr) != 0) {
        CPE_ERROR(
            cli->m_em, "%s: create svr: insert fail, svr_type_id %d already exist!",
            conn_net_cli_name(cli), svr_type_id);
        mem_free(cli->m_alloc, svr);
        return NULL;
    }

    return svr;
}

void conn_net_cli_svr_stub_free(struct conn_net_cli_svr_stub * svr) {
    conn_net_cli_t cli = svr->m_cli;

    conn_net_cli_cmd_info_free_all(svr);

    if (svr->m_response_dispatch_to) {
        mem_free(cli->m_alloc, svr->m_response_dispatch_to);
        svr->m_response_dispatch_to = NULL;
    }

    if (svr->m_notify_dispatch_to) {
        mem_free(cli->m_alloc, svr->m_notify_dispatch_to);
        svr->m_notify_dispatch_to = NULL;
    }

    if (svr->m_outgoing_recv_at) {
        dp_rsp_free(svr->m_outgoing_recv_at);
        svr->m_outgoing_recv_at = NULL;
    }

    cpe_hash_table_fini(&svr->m_cmds);

    cpe_hash_table_remove_by_ins(&cli->m_svrs, svr);

    mem_free(cli->m_alloc, svr);
}

void conn_net_cli_svr_stub_free_all(conn_net_cli_t cli) {
    struct cpe_hash_it svr_it;
    conn_net_cli_svr_stub_t svr;

    cpe_hash_it_init(&svr_it, &cli->m_svrs);

    svr = cpe_hash_it_next(&svr_it);
    while(svr) {
        conn_net_cli_svr_stub_t next = cpe_hash_it_next(&svr_it);
        conn_net_cli_svr_stub_free(svr);
        svr = next;
    }
}

const char * conn_net_cli_svr_stub_type_name(conn_net_cli_svr_stub_t svr) {
    return svr->m_svr_type_name;
}

conn_net_cli_svr_stub_t conn_net_cli_svr_stub_lsearch_by_name(conn_net_cli_t cli, const char * name) {
    struct cpe_hash_it svr_it;
    conn_net_cli_svr_stub_t svr;

    cpe_hash_it_init(&svr_it, &cli->m_svrs);

    while((svr = cpe_hash_it_next(&svr_it))) {
        if (strcmp(svr->m_svr_type_name, name) == 0) return svr;
    }

    return NULL;
}

conn_net_cli_svr_stub_t conn_net_cli_svr_stub_find_by_id(conn_net_cli_t cli, uint16_t svr_type_id) {
    struct conn_net_cli_svr_stub key;
    key.m_svr_type_id = svr_type_id;
    return cpe_hash_table_find(&cli->m_svrs, &key);
}

cpe_hash_string_t conn_net_cli_svr_stub_response_dispatch_to(conn_net_cli_svr_stub_t svr) {
    return svr->m_response_dispatch_to;
}

int conn_net_cli_svr_stub_set_response_dispatch_to(struct conn_net_cli_svr_stub * svr, const char * response_dispatch_to) {
    conn_net_cli_t cli = svr->m_cli;
    cpe_hash_string_t new_response_dispatch_to = cpe_hs_create(cli->m_alloc, response_dispatch_to);
    if (new_response_dispatch_to == NULL) return -1;

    if (svr->m_response_dispatch_to) mem_free(cli->m_alloc, svr->m_response_dispatch_to);
    svr->m_response_dispatch_to = new_response_dispatch_to;

    return 0;
}

cpe_hash_string_t conn_net_cli_svr_stub_notify_dispatch_to(conn_net_cli_svr_stub_t svr) {
    return svr->m_notify_dispatch_to;
}

int conn_net_cli_svr_stub_set_notify_dispatch_to(struct conn_net_cli_svr_stub * svr, const char * notify_dispatch_to) {
    conn_net_cli_t cli = svr->m_cli;
    cpe_hash_string_t new_notify_dispatch_to = cpe_hs_create(cli->m_alloc, notify_dispatch_to);
    if (new_notify_dispatch_to == NULL) return -1;

    if (svr->m_notify_dispatch_to) mem_free(cli->m_alloc, svr->m_notify_dispatch_to);
    svr->m_notify_dispatch_to = new_notify_dispatch_to;

    return 0;
}

int conn_net_cli_svr_stub_set_outgoing_recv_at(conn_net_cli_svr_stub_t svr, const char * outgoing_recv_at) {
    conn_net_cli_t cli = svr->m_cli;
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.outgoing-recv-rsp", conn_net_cli_svr_stub_type_name(svr));

    if (svr->m_outgoing_recv_at) dp_rsp_free(svr->m_outgoing_recv_at);

    svr->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(cli->m_app), name_buf);
    if (svr->m_outgoing_recv_at == NULL) return -1;

    dp_rsp_set_processor(svr->m_outgoing_recv_at, conn_net_cli_svr_stub_outgoing_recv, svr);

    if (dp_rsp_bind_string(svr->m_outgoing_recv_at, outgoing_recv_at, cli->m_em) != 0) {
        CPE_ERROR(
            cli->m_em, "%s: set outgoing_recv_at: bind to %s fail!",
            conn_net_cli_svr_stub_type_name(svr), outgoing_recv_at);
        dp_rsp_free(svr->m_outgoing_recv_at);
        svr->m_outgoing_recv_at = NULL;
        return -1;
    }

    return 0;
}

LPDRMETA conn_net_cli_svr_stub_find_data_meta_by_cmd(conn_net_cli_svr_stub_t svr_info, uint32_t cmd) {
    LPDRMETA union_meta;
    LPDRMETAENTRY data_entry;

    if (svr_info->m_pkg_data_entry == NULL) return NULL;

    union_meta = dr_entry_ref_meta(svr_info->m_pkg_data_entry);
    if (union_meta == NULL) return NULL;

    data_entry = dr_meta_find_entry_by_id(union_meta, cmd);
    if (data_entry == NULL) return NULL;

    return dr_entry_ref_meta(data_entry);
}

LPDRMETA conn_net_cli_svr_stub_error_pkg_meta(conn_net_cli_svr_stub_t svr_info) {
    return svr_info->m_error_pkg_meta;
}

uint32_t conn_net_cli_svr_stub_error_pkg_cmd(conn_net_cli_svr_stub_t svr_info) {
    return svr_info->m_error_pkg_cmd;
}

LPDRMETAENTRY conn_net_cli_svr_stub_error_pkg_errno_entry(conn_net_cli_svr_stub_t svr_info) {
    return svr_info->m_error_pkg_error_entry;
}

uint32_t conn_net_cli_svr_stub_hash(conn_net_cli_svr_stub_t svr) {
    return svr->m_svr_type_id;
}

int conn_net_cli_svr_stub_eq(conn_net_cli_svr_stub_t l, conn_net_cli_svr_stub_t r) {
    return l->m_svr_type_id == r->m_svr_type_id;
}
