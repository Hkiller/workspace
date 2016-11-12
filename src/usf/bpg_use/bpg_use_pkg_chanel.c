#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg.h"
#include "cpe/nm/nm_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_use/bpg_use_pkg_chanel.h"
#include "bpg_use_internal_types.h"

static void bpg_use_pkg_chanel_clear(nm_node_t node);
static int bpg_use_pkg_chanel_incoming_rsp(dp_req_t req, void * ctx, error_monitor_t em);
static int bpg_use_pkg_chanel_outgoing_rsp(dp_req_t req, void * ctx, error_monitor_t em);

struct nm_node_type s_nm_node_type_bpg_use_pkg_chanel = {
    "usf_bpg_use_pkg_chanel",
    bpg_use_pkg_chanel_clear
};

bpg_use_pkg_chanel_t
bpg_use_pkg_chanel_create(
    gd_app_context_t app,
    const char * name,
    bpg_pkg_manage_t pkg_manage,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    bpg_use_pkg_chanel_t chanel;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct bpg_use_pkg_chanel));
    if (mgr_node == NULL) return NULL;

    chanel = (bpg_use_pkg_chanel_t)nm_node_data(mgr_node);

    chanel->m_app = app;
    chanel->m_alloc = alloc;
    chanel->m_em = em;
    chanel->m_debug = 0;
    chanel->m_pkg_manage = pkg_manage;

    chanel->m_outgoing_send_to = NULL;
    chanel->m_outgoing_recv_at = NULL;
    chanel->m_outgoing_buf = NULL;

    chanel->m_incoming_send_to = NULL;
    chanel->m_incoming_recv_at = NULL;
    chanel->m_incoming_buf = NULL;

    nm_node_set_type(mgr_node, &s_nm_node_type_bpg_use_pkg_chanel);

    return chanel;
}

static void bpg_use_pkg_chanel_clear(nm_node_t node) {
    bpg_use_pkg_chanel_t chanel;
    chanel = (bpg_use_pkg_chanel_t)nm_node_data(node);

    /*outgoing */
    if (chanel->m_outgoing_buf) {
        dp_req_free(chanel->m_outgoing_buf);
        chanel->m_outgoing_buf = NULL;
    }

    if (chanel->m_outgoing_send_to != NULL) {
        mem_free(chanel->m_alloc, chanel->m_outgoing_send_to);
        chanel->m_outgoing_send_to = NULL;
    }

    if (chanel->m_outgoing_recv_at != NULL) {
        dp_rsp_free(chanel->m_outgoing_recv_at);
        chanel->m_outgoing_recv_at = NULL;
    }

    /*incoming */
    if (chanel->m_incoming_buf) {
        dp_req_free(chanel->m_incoming_buf);
        chanel->m_incoming_buf = NULL;
    }

    if (chanel->m_incoming_send_to != NULL) {
        bpg_pkg_dsp_free(chanel->m_incoming_send_to);
        chanel->m_incoming_send_to = NULL;
    }

    if (chanel->m_incoming_recv_at != NULL) {
        dp_rsp_free(chanel->m_incoming_recv_at);
        chanel->m_incoming_recv_at = NULL;
    }
}

gd_app_context_t bpg_use_pkg_chanel_app(bpg_use_pkg_chanel_t chanel) {
    return chanel->m_app;
}

void bpg_use_pkg_chanel_free(bpg_use_pkg_chanel_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_bpg_use_pkg_chanel) return;
    nm_node_free(mgr_node);
}

bpg_use_pkg_chanel_t
bpg_use_pkg_chanel_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_use_pkg_chanel) return NULL;
    return (bpg_use_pkg_chanel_t)nm_node_data(node);
}

bpg_use_pkg_chanel_t
bpg_use_pkg_chanel_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_bpg_use_pkg_chanel) return NULL;
    return (bpg_use_pkg_chanel_t)nm_node_data(node);
}

const char * bpg_use_pkg_chanel_name(bpg_use_pkg_chanel_t chanel) {
    return nm_node_name(nm_node_from_data(chanel));
}

cpe_hash_string_t
bpg_use_pkg_chanel_name_hs(bpg_use_pkg_chanel_t chanel) {
    return nm_node_name_hs(nm_node_from_data(chanel));
}

int bpg_use_pkg_chanel_outgoing_set_send_to(bpg_use_pkg_chanel_t chanel, const char * name) {
    if (chanel->m_outgoing_send_to != NULL) {
        mem_free(chanel->m_alloc, chanel->m_outgoing_send_to);
        chanel->m_outgoing_send_to = NULL;
    }

    chanel->m_outgoing_send_to = cpe_hs_create(chanel->m_alloc, name);
    if (chanel->m_outgoing_send_to == NULL) return -1;

    return 0;
}

int bpg_use_pkg_chanel_outgoing_set_recv_at(bpg_use_pkg_chanel_t chanel, const char * name) {
    char sp_name_buf[128];

    if (chanel->m_outgoing_recv_at != NULL) {
        dp_rsp_free(chanel->m_outgoing_recv_at);
        chanel->m_outgoing_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.outgoing-recv.sp", bpg_use_pkg_chanel_name(chanel));
    chanel->m_outgoing_recv_at = dp_rsp_create(gd_app_dp_mgr(chanel->m_app), sp_name_buf);
    if (chanel->m_outgoing_recv_at == NULL) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_use_pkg_chanel_outgoing_set_recv_at: create rsp fail!",
            bpg_use_pkg_chanel_name(chanel));
        return -1;
    }
    dp_rsp_set_processor(chanel->m_outgoing_recv_at, bpg_use_pkg_chanel_outgoing_rsp, chanel);

    if (dp_rsp_bind_string(chanel->m_outgoing_recv_at, name, chanel->m_em) != 0) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_use_pkg_chanel_outgoing_set_recv_at: bind rsp to %s fail!",
            bpg_use_pkg_chanel_name(chanel), name);
        dp_rsp_free(chanel->m_outgoing_recv_at);
        chanel->m_outgoing_recv_at = NULL;
        return -1;
    }

    return 0;
}

int bpg_use_pkg_chanel_incoming_set_send_to(bpg_use_pkg_chanel_t chanel, cfg_t cfg) {
    if (chanel->m_incoming_send_to != NULL) {
        bpg_pkg_dsp_free(chanel->m_incoming_send_to);
        chanel->m_incoming_send_to = NULL;
    }

    chanel->m_incoming_send_to = bpg_pkg_dsp_create(chanel->m_alloc);
    if (chanel->m_incoming_send_to == NULL) return -1;

    if (bpg_pkg_dsp_load(chanel->m_incoming_send_to, cfg, chanel->m_em) != 0) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_use_pkg_chanel_outgoing_set_send_to: load dsp fail!",
            bpg_use_pkg_chanel_name(chanel));
        bpg_pkg_dsp_free(chanel->m_incoming_send_to);
        chanel->m_incoming_send_to = NULL;
        return -1;
    }

    return 0;
}


int bpg_use_pkg_chanel_incoming_set_recv_at(bpg_use_pkg_chanel_t chanel, const char * name) {
    char sp_name_buf[128];

    if (chanel->m_incoming_recv_at != NULL) {
        dp_rsp_free(chanel->m_incoming_recv_at);
        chanel->m_incoming_recv_at = NULL;
    }

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.incoming-recv.sp", bpg_use_pkg_chanel_name(chanel));
    chanel->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(chanel->m_app), sp_name_buf);
    if (chanel->m_incoming_recv_at == NULL) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_use_pkg_chanel_incoming_set_recv_at: create rsp fail!",
            bpg_use_pkg_chanel_name(chanel));
        return -1;
    }
    dp_rsp_set_processor(chanel->m_incoming_recv_at, bpg_use_pkg_chanel_incoming_rsp, chanel);

    if (dp_rsp_bind_string(chanel->m_incoming_recv_at, name, chanel->m_em) != 0) {
        CPE_ERROR(
            chanel->m_em, "%s: bpg_use_pkg_chanel_incoming_set_recv_at: bind rsp to %s fail!",
            bpg_use_pkg_chanel_name(chanel), name);
        dp_rsp_free(chanel->m_incoming_recv_at);
        chanel->m_incoming_recv_at = NULL;
        return -1;
    }

    return 0;
}

bpg_pkg_manage_t bpg_use_pkg_chanel_pkg_manage(bpg_use_pkg_chanel_t chanel) {
    return chanel->m_pkg_manage;
}

/* bpg_pkg_t */
/* bpg_use_pkg_chanel_incoming_buf(bpg_use_pkg_chanel_t chanel) { */
/*     if (chanel->m_incoming_buf) { */
/*         if (bpg_pkg_pkg_data_capacity(chanel->m_incoming_buf) < chanel->m_send_pkg_max_size) { */
/*             bpg_pkg_free(chanel->m_send_pkg_buf); */
/*             chanel->m_send_pkg_buf = NULL; */
/*         } */
/*     } */

/*     if (chanel->m_send_pkg_buf == NULL) { */
/*         chanel->m_send_pkg_buf = bpg_pkg_create(chanel->m_pkg_manage, chanel->m_send_pkg_max_size, NULL, 0); */
/*     } */

/*     return chanel->m_send_pkg_buf; */
/* } */

int bpg_use_pkg_chanel_incoming_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    struct bpg_use_pkg_chanel * chanel;
    dr_cvt_result_t r;
    size_t buf_size;

    chanel = (bpg_use_pkg_chanel_t)ctx;

    if (chanel->m_incoming_buf == NULL) {
        chanel->m_incoming_buf = bpg_pkg_create_with_body(chanel->m_pkg_manage, 2048);
        if (chanel->m_incoming_buf == NULL) {
            CPE_ERROR(chanel->m_em, "%s: incomint: create buf fail!", bpg_use_pkg_chanel_name(chanel));
            return -1;
        }
    }

INCOMING_TRY_AGAIN:
    buf_size = dp_req_size(req);
    r = bpg_pkg_decode(chanel->m_incoming_buf, dp_req_data(req), &buf_size, chanel->m_em, chanel->m_debug);
    if (r != dr_cvt_result_success) {
        if (r == dr_cvt_result_not_enough_output) {
            buf_size = dp_req_capacity(chanel->m_incoming_buf);
            buf_size *= 2;

            if (buf_size > 20480000) {
                CPE_ERROR(chanel->m_em, "%s: incoming: decode fail, not enough buf!", bpg_use_pkg_chanel_name(chanel));
                return -1;
            }

            dp_req_free(chanel->m_incoming_buf);
            chanel->m_incoming_buf = bpg_pkg_create_with_body(chanel->m_pkg_manage, buf_size);
            if (chanel->m_incoming_buf == NULL) {
                CPE_ERROR(chanel->m_em, "%s: incoming: create buf fail, size=%d!", bpg_use_pkg_chanel_name(chanel), (int)buf_size);
                return -1;
            }

            goto INCOMING_TRY_AGAIN;
        }
        else {
            CPE_ERROR(chanel->m_em, "%s: incoming: decode fail, error=%d!", bpg_use_pkg_chanel_name(chanel), r);
            return -1;
        }
    }

    if (bpg_pkg_dsp_dispatch(chanel->m_incoming_send_to, chanel->m_incoming_buf, chanel->m_em) != 0) {
        CPE_ERROR(chanel->m_em, "%s: outgoing: dispatch fail!", bpg_use_pkg_chanel_name(chanel));
        return -1;
    }
    
    return 0;
}

int bpg_use_pkg_chanel_outgoing_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_use_pkg_chanel_t chanel;
    dr_cvt_result_t r;
    size_t buf_size;

    chanel = (bpg_use_pkg_chanel_t)ctx;

    if (chanel->m_outgoing_buf == NULL) {
        chanel->m_outgoing_buf = dp_req_create(gd_app_dp_mgr(chanel->m_app), 2048);
        if (chanel->m_outgoing_buf == NULL) {
            CPE_ERROR(chanel->m_em, "%s: outgoing: create buf fail!", bpg_use_pkg_chanel_name(chanel));
            return -1;
        }
    }

OUTGOING_TRY_AGAIN:
    buf_size = dp_req_capacity(chanel->m_outgoing_buf);
    r = bpg_pkg_encode(req, dp_req_data(chanel->m_outgoing_buf), &buf_size,  chanel->m_em, chanel->m_debug);
    if (r != dr_cvt_result_success) {
        if (r == dr_cvt_result_not_enough_output) {
            buf_size = dp_req_capacity(chanel->m_outgoing_buf);
            buf_size *= 2;

            if (buf_size > 20480000) {
                CPE_ERROR(chanel->m_em, "%s: outgoing: encode fail, not enough buf!", bpg_use_pkg_chanel_name(chanel));
                return -1;
            }

            dp_req_free(chanel->m_outgoing_buf);
            chanel->m_outgoing_buf = dp_req_create(gd_app_dp_mgr(chanel->m_app), buf_size);
            if (chanel->m_outgoing_buf == NULL) {
                CPE_ERROR(chanel->m_em, "%s: outgoing: create buf fail, size=%d!", bpg_use_pkg_chanel_name(chanel), (int)buf_size);
                return -1;
            }

            goto OUTGOING_TRY_AGAIN;
        }
        else {
            CPE_ERROR(chanel->m_em, "%s: outgoing: encode fail, error=%d!", bpg_use_pkg_chanel_name(chanel), r);
            return -1;
        }
    }
    dp_req_set_size(chanel->m_outgoing_buf, buf_size);
    
    if (dp_dispatch_by_string(chanel->m_outgoing_send_to, dp_req_mgr(chanel->m_outgoing_buf), chanel->m_outgoing_buf, chanel->m_em) != 0) {
        CPE_ERROR(chanel->m_em, "%s: outgoing: dispatch to %s fail!", bpg_use_pkg_chanel_name(chanel), cpe_hs_data(chanel->m_outgoing_send_to));
        return -1;
    }

    return 0;
}


EXPORT_DIRECTIVE
int bpg_use_pkg_chanel_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    struct bpg_use_pkg_chanel * bpg_use_pkg_chanel;
    bpg_pkg_manage_t pkg_manage;
    const char * incoming_recv_at;
    cfg_t incoming_send_to;
    const char * outgoing_recv_at;
    const char * outgoing_send_to;

    pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pkg-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "pkg-manage", "default"));
        return -1;
    }

    bpg_use_pkg_chanel =
        bpg_use_pkg_chanel_create(
            app, 
            gd_app_module_name(module),
            pkg_manage,
            gd_app_alloc(app),
            gd_app_em(app));
    if (bpg_use_pkg_chanel == NULL) return -1;

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    if (incoming_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-recv-at not configured!", gd_app_module_name(module));
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }
    if (bpg_use_pkg_chanel_incoming_set_recv_at(bpg_use_pkg_chanel, incoming_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set incoming-recv-at %s fail!", gd_app_module_name(module), incoming_recv_at);
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }

    incoming_send_to = cfg_find_cfg(cfg, "incoming-send-to");
    if (incoming_send_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-send-to not configured!", gd_app_module_name(module));
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }
    if (bpg_use_pkg_chanel_incoming_set_send_to(bpg_use_pkg_chanel, incoming_send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set incoming-send-to fail!", gd_app_module_name(module));
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }

    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);
    if (outgoing_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-recv-at not configured!", gd_app_module_name(module));
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }
    if (bpg_use_pkg_chanel_outgoing_set_recv_at(bpg_use_pkg_chanel, outgoing_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set outgoing-recv-at %s fail!", gd_app_module_name(module), outgoing_recv_at);
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }

    outgoing_send_to = cfg_get_string(cfg, "outgoing-send-to", NULL);
    if (outgoing_send_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-send-to not configured!", gd_app_module_name(module));
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }
    if (bpg_use_pkg_chanel_outgoing_set_send_to(bpg_use_pkg_chanel, outgoing_send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set outgoing-send-to %s fail!", gd_app_module_name(module), outgoing_send_to);
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
        return -1;
    }

    bpg_use_pkg_chanel->m_debug = cfg_get_int32(cfg, "debug", 0);

    return 0;
}

EXPORT_DIRECTIVE
void bpg_use_pkg_chanel_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_use_pkg_chanel_t bpg_use_pkg_chanel;

    bpg_use_pkg_chanel = bpg_use_pkg_chanel_find_nc(app, gd_app_module_name(module));
    if (bpg_use_pkg_chanel) {
        bpg_use_pkg_chanel_free(bpg_use_pkg_chanel);
    }
}
