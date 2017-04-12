#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "set_bpg_internal_ops.h"

EXPORT_DIRECTIVE
int set_bpg_chanel_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_bpg_chanel_t sp;
    bpg_pkg_manage_t bpg_pkg_manage;
    const char * incoming_dispatch_to;
    const char * incoming_recv_at;
    const char * outgoing_dispatch_to;
    const char * outgoing_recv_at;
    const char * str_pkg_max_size;

    bpg_pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
    if (bpg_pkg_manage == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: pkg-manage %s not exist!",
            gd_app_module_name(module),
            cfg_get_string(cfg, "pkg-manage", NULL));
        return -1;
    }

    incoming_dispatch_to = cfg_get_string(cfg, "incoming-send-to", NULL);
    if (incoming_dispatch_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    if (incoming_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: incoming-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);
    if (outgoing_recv_at == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    outgoing_dispatch_to = cfg_get_string(cfg, "outgoing-send-to", NULL);
    if (outgoing_dispatch_to == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: outgoing-send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    sp = set_bpg_chanel_create(
        app, gd_app_module_name(module),
        bpg_pkg_manage, gd_app_alloc(app), gd_app_em(app));
    if (sp == NULL) return -1;

    sp->m_debug = cfg_get_int8(cfg, "debug", sp->m_debug);

    if (set_bpg_chanel_set_incoming_recv_at(sp, incoming_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-recv-at %s fail!",
            gd_app_module_name(module), incoming_recv_at);
        set_bpg_chanel_free(sp);
        return -1;
    }

    if (set_bpg_chanel_set_incoming_dispatch_to(sp, incoming_dispatch_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-send-to %s fail!",
            gd_app_module_name(module), incoming_dispatch_to);
        set_bpg_chanel_free(sp);
        return -1;
    }

    if (set_bpg_chanel_set_outgoing_recv_at(sp, outgoing_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-recv-at %s fail!",
            gd_app_module_name(module), outgoing_recv_at);
        set_bpg_chanel_free(sp);
        return -1;
    }

    if (set_bpg_chanel_set_outgoing_dispatch_to(sp, outgoing_dispatch_to) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-send-to %s fail!",
            gd_app_module_name(module), outgoing_dispatch_to);
        set_bpg_chanel_free(sp);
        return -1;
    }

    if ((str_pkg_max_size = cfg_get_string(cfg, "pkg-max-size", NULL))) {
        uint64_t result;
        if (cpe_str_parse_byte_size(&result, str_pkg_max_size) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read pkg-max-size %s fail!",
                gd_app_module_name(module), str_pkg_max_size);
            set_bpg_chanel_free(sp);
            return -1;
        }

        sp->m_pkg_max_size = (uint32_t)result;
    }

    if (sp->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. pkg-max-size=%d", gd_app_module_name(module), sp->m_pkg_max_size);
    }

    return 0;
}

EXPORT_DIRECTIVE
void set_bpg_chanel_app_fini(gd_app_context_t app, gd_app_module_t module) {
    set_bpg_chanel_t set_bpg_chanel;

    set_bpg_chanel = set_bpg_chanel_find_nc(app, gd_app_module_name(module));
    if (set_bpg_chanel) {
        set_bpg_chanel_free(set_bpg_chanel);
    }
}
