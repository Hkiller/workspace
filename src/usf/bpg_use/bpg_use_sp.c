#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/nm/nm_read.h"
#include "cpe/nm/nm_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_use/bpg_use_sp.h"
#include "bpg_use_internal_types.h"

static void bpg_use_sp_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_bpg_use_sp = {
    "usf_bpg_use_sp",
    bpg_use_sp_clear
};

bpg_use_sp_t
bpg_use_sp_create(gd_app_context_t app, const char * name, bpg_pkg_manage_t pkg_manage, mem_allocrator_t alloc, error_monitor_t em) {
    bpg_use_sp_t sp;
    nm_node_t sp_node;

    assert(app);

    if (em  == NULL) em = gd_app_em(app);

    sp_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_use_pkg_chanel));
    if (sp_node == NULL) return NULL;

    sp = (bpg_use_sp_t)nm_node_data(sp_node);

    sp->m_pkg_manage = pkg_manage;
    sp->m_app = app;
    sp->m_alloc = gd_app_alloc(app);
    sp->m_em = em;
    sp->m_dsp = NULL;
    sp->m_pkg_buf = NULL;
    sp->m_debug = 0;
    sp->m_client_id = 0;

    mem_buffer_init(&sp->m_data_buf, gd_app_alloc(app));

    nm_node_set_type(sp_node, &s_nm_node_type_bpg_use_sp);
    
    return sp;
}

static void bpg_use_sp_clear(nm_node_t node) {
    bpg_use_sp_t sp;
    sp = (bpg_use_sp_t)nm_node_data(node);

    if (sp->m_pkg_buf) {
        dp_req_free(sp->m_pkg_buf);
        sp->m_pkg_buf = NULL;
    }

    if (sp->m_dsp != NULL) {
        bpg_pkg_dsp_free(sp->m_dsp);
        sp->m_dsp = NULL;
    }

    mem_buffer_clear(&sp->m_data_buf);
}

void bpg_use_sp_free(bpg_use_sp_t sp) {
    nm_node_t sp_node;
    assert(sp);

    sp_node = nm_node_from_data(sp);
    if (nm_node_type(sp_node) != &s_nm_node_type_bpg_use_sp) return;
    nm_node_free(sp_node);
}

bpg_use_sp_t
bpg_use_sp_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_use_sp) return NULL;
    return (bpg_use_sp_t)nm_node_data(node);
}

bpg_use_sp_t
bpg_use_sp_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_use_sp) return NULL;
    return (bpg_use_sp_t)nm_node_data(node);
}

uint64_t bpg_use_sp_client_id(bpg_use_sp_t sp) {
    return sp->m_client_id;
}

void bpg_use_sp_set_client_id(bpg_use_sp_t sp, uint64_t client_id) {
    sp->m_client_id = client_id;
}

const char * bpg_use_sp_name(bpg_use_sp_t sp) {
    return nm_node_name(nm_node_from_data(sp));
}

cpe_hash_string_t
bpg_use_sp_name_hs(bpg_use_sp_t sp) {
    return nm_node_name_hs(nm_node_from_data(sp));
}

int bpg_use_sp_set_send_to(bpg_use_sp_t sp, cfg_t send_to) {
    if (sp->m_dsp) bpg_pkg_dsp_free(sp->m_dsp);

    sp->m_dsp = bpg_pkg_dsp_create(gd_app_alloc(sp->m_app));
    if (sp->m_dsp == NULL) {
        CPE_ERROR(sp->m_em, "%s: bpg_use_sp_create: create dsp fail!", bpg_use_sp_name(sp));
        return -1;
    }

    if (bpg_pkg_dsp_load(sp->m_dsp, send_to, sp->m_em) != 0) {
        CPE_ERROR(sp->m_em, "%s: bpg_use_set_send_to: load dsp fail!", bpg_use_sp_name(sp));
        bpg_pkg_dsp_free(sp->m_dsp);
        sp->m_dsp = NULL;
        return -1;
    }

    return 0;
}

dp_req_t
bpg_use_sp_pkg_buf(bpg_use_sp_t sp, size_t capacity) {
    if (sp->m_pkg_buf && (bpg_pkg_body_capacity(sp->m_pkg_buf) < capacity)) {
        dp_req_free(sp->m_pkg_buf);
        sp->m_pkg_buf = NULL;
    }

    if (sp->m_pkg_buf == NULL) {
        sp->m_pkg_buf = bpg_pkg_create_with_body_by_data_capacity(sp->m_pkg_manage, capacity);
    }

    bpg_pkg_init(sp->m_pkg_buf);

    return sp->m_pkg_buf;
}

int bpg_use_sp_send(bpg_use_sp_t sp, dp_req_t pkg) {
    if (sp->m_client_id != 0) bpg_pkg_set_client_id(pkg, sp->m_client_id);
    return bpg_pkg_dsp_dispatch(sp->m_dsp, pkg, sp->m_em);
}

bpg_pkg_manage_t bpg_use_sp_pkg_manage(bpg_use_sp_t sp) {
    return sp->m_pkg_manage;
}

gd_app_context_t bpg_use_sp_app(bpg_use_sp_t sp) {
    return sp->m_app;
}

LPDRMETALIB bpg_use_sp_metalib(bpg_use_sp_t sp) {
    return bpg_pkg_manage_data_metalib(sp->m_pkg_manage);
}

LPDRMETA bpg_use_sp_meta(bpg_use_sp_t sp, const char * name) {
    LPDRMETALIB metalib = bpg_pkg_manage_data_metalib(sp->m_pkg_manage);
    return metalib == NULL
        ? NULL
        : dr_lib_find_meta_by_name(metalib, name);
}

void * bpg_use_sp_data_buf(bpg_use_sp_t sp, size_t capacity) {
    dp_req_t pkg = bpg_use_sp_pkg_buf(sp, capacity);
    void * r = bpg_pkg_body(pkg);
    bzero(r, capacity);
    return r;
}

EXPORT_DIRECTIVE
int bpg_use_sp_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    struct bpg_use_sp * sp;
    bpg_pkg_manage_t pkg_manage;

    pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pkg-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "pkg-manage", "default"));
        return -1;
    }

    sp =
        bpg_use_sp_create(
            app, 
            gd_app_module_name(module),
            pkg_manage,
            gd_app_alloc(app),
            gd_app_em(app));
    if (sp == NULL) return -1;

    if (bpg_use_sp_set_send_to(sp, cfg_find_cfg(cfg, "send-to")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to fail!", gd_app_module_name(module));
        bpg_use_sp_free(sp);
        return -1;
    }

    sp->m_debug = cfg_get_int32(cfg, "debug", 0);

    return 0;
}

EXPORT_DIRECTIVE
void bpg_use_sp_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_use_sp_t bpg_use_sp;

    bpg_use_sp = bpg_use_sp_find_nc(app, gd_app_module_name(module));
    if (bpg_use_sp) {
        bpg_use_sp_free(bpg_use_sp);
    }
}
