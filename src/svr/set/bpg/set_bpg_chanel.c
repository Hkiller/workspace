#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "svr/set/share/set_pkg.h"
#include "set_bpg_internal_ops.h"

static void set_bpg_chanel_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_set_bpg_chanel = {
    "svr_set_bpg_chanel",
    set_bpg_chanel_clear
};

set_bpg_chanel_t
set_bpg_chanel_create(
    gd_app_context_t app,
    const char * name,
    bpg_pkg_manage_t bpg_pkg_manage,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct set_bpg_chanel * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct set_bpg_chanel));
    if (mgr_node == NULL) return NULL;

    mgr = (set_bpg_chanel_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_pkg_max_size = 1 * 1024 * 1024;

    mgr->m_bpg_pkg_manage = bpg_pkg_manage;
    mgr->m_bpg_head = NULL;

    mgr->m_set_head = NULL;

    mgr->m_outgoing_buf = NULL;
    mgr->m_outgoing_dispatch_to = NULL;
    mgr->m_outgoing_recv_at = NULL;

    mgr->m_incoming_buf = NULL;
    mgr->m_incoming_dispatch_to = NULL;
    mgr->m_incoming_recv_at = NULL;

    mem_buffer_init(&mgr->m_dump_buffer, alloc);

    nm_node_set_type(mgr_node, &s_nm_node_type_set_bpg_chanel);

    return mgr;
}

static void set_bpg_chanel_clear(nm_node_t node) {
    set_bpg_chanel_t mgr;
    mgr = (set_bpg_chanel_t)nm_node_data(node);

    if (mgr->m_incoming_recv_at) {
        dp_rsp_free(mgr->m_incoming_recv_at);
        mgr->m_incoming_recv_at = NULL;
    }

    if (mgr->m_incoming_dispatch_to) {
        mem_free(mgr->m_alloc, mgr->m_incoming_dispatch_to);
        mgr->m_incoming_dispatch_to = NULL;
    }

    if (mgr->m_outgoing_recv_at) {
        dp_rsp_free(mgr->m_outgoing_recv_at);
        mgr->m_outgoing_recv_at = NULL;
    }

    if (mgr->m_outgoing_dispatch_to) {
        mem_free(mgr->m_alloc, mgr->m_outgoing_dispatch_to);
        mgr->m_outgoing_dispatch_to = NULL;
    }

    if (mgr->m_incoming_buf) {
        dp_req_free(mgr->m_incoming_buf);
        mgr->m_incoming_buf = NULL;
    }

    if (mgr->m_outgoing_buf) {
        dp_req_free(mgr->m_outgoing_buf);
        mgr->m_outgoing_buf = NULL;
    }

    if (mgr->m_bpg_head) {
        bpg_pkg_free(mgr->m_bpg_head);
        mgr->m_bpg_head = NULL;
    }

    if (mgr->m_set_head) {
        dp_req_free(mgr->m_set_head);
        mgr->m_set_head = NULL;
    }

    mem_buffer_clear(&mgr->m_dump_buffer);
}

gd_app_context_t set_bpg_chanel_app(set_bpg_chanel_t mgr) {
    return mgr->m_app;
}

void set_bpg_chanel_free(set_bpg_chanel_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_set_bpg_chanel) return;
    nm_node_free(mgr_node);
}

set_bpg_chanel_t
set_bpg_chanel_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_bpg_chanel) return NULL;
    return (set_bpg_chanel_t)nm_node_data(node);
}

set_bpg_chanel_t
set_bpg_chanel_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_bpg_chanel) return NULL;
    return (set_bpg_chanel_t)nm_node_data(node);
}

const char * set_bpg_chanel_name(set_bpg_chanel_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
set_bpg_chanel_name_hs(set_bpg_chanel_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

int set_bpg_chanel_set_incoming_dispatch_to(set_bpg_chanel_t sp, const char * incoming_dispatch_to) {
    cpe_hash_string_t new_incoming_dispatch_to = cpe_hs_create(sp->m_alloc, incoming_dispatch_to);
    if (new_incoming_dispatch_to == NULL) return -1;

    if (sp->m_incoming_dispatch_to) mem_free(sp->m_alloc, sp->m_incoming_dispatch_to);
    sp->m_incoming_dispatch_to = new_incoming_dispatch_to;

    return 0;
}

cpe_hash_string_t set_bpg_chanel_incoming_dispatch_to(set_bpg_chanel_t sp) {
    return sp->m_incoming_dispatch_to;
}

int set_bpg_chanel_set_incoming_recv_at(set_bpg_chanel_t sp, const char * incoming_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.incoming-recv-rsp", set_bpg_chanel_name(sp));

    if (sp->m_incoming_recv_at) dp_rsp_free(sp->m_incoming_recv_at);

    sp->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(sp->m_app), name_buf);
    if (sp->m_incoming_recv_at == NULL) return -1;

    dp_rsp_set_processor(sp->m_incoming_recv_at, set_bpg_chanel_incoming_recv, sp);

    if (dp_rsp_bind_string(sp->m_incoming_recv_at, incoming_recv_at, sp->m_em) != 0) {
        CPE_ERROR(
            sp->m_em, "%s: set incoming_recv_at: bind to %s fail!",
            set_bpg_chanel_name(sp), incoming_recv_at);
        dp_rsp_free(sp->m_incoming_recv_at);
        sp->m_incoming_recv_at = NULL;
        return -1;
    }

    return 0;
}

int set_bpg_chanel_set_outgoing_dispatch_to(set_bpg_chanel_t sp, const char * outgoing_dispatch_to) {
    cpe_hash_string_t new_outgoing_dispatch_to = cpe_hs_create(sp->m_alloc, outgoing_dispatch_to);
    if (new_outgoing_dispatch_to == NULL) return -1;

    if (sp->m_outgoing_dispatch_to) mem_free(sp->m_alloc, sp->m_outgoing_dispatch_to);
    sp->m_outgoing_dispatch_to = new_outgoing_dispatch_to;

    return 0;
}

cpe_hash_string_t set_bpg_chanel_outgoing_dispatch_to(set_bpg_chanel_t sp) {
    return sp->m_outgoing_dispatch_to;
}

int set_bpg_chanel_set_outgoing_recv_at(set_bpg_chanel_t sp, const char * outgoing_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.outgoing-recv-rsp", set_bpg_chanel_name(sp));

    if (sp->m_outgoing_recv_at) dp_rsp_free(sp->m_outgoing_recv_at);

    sp->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(sp->m_app), name_buf);
    if (sp->m_outgoing_recv_at == NULL) return -1;

    dp_rsp_set_processor(sp->m_outgoing_recv_at, set_bpg_chanel_outgoing_recv, sp);

    if (dp_rsp_bind_string(sp->m_outgoing_recv_at, outgoing_recv_at, sp->m_em) != 0) {
        CPE_ERROR(
            sp->m_em, "%s: set outgoing_recv_at: bind to %s fail!",
            set_bpg_chanel_name(sp), outgoing_recv_at);
        dp_rsp_free(sp->m_outgoing_recv_at);
        sp->m_outgoing_recv_at = NULL;
        return -1;
    }

    return 0;
}
