#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_types.h"

EXPORT_DIRECTIVE
int bpg_bind_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_bind_manage_t bpg_bind_manage;
    bpg_pkg_manage_t pkg_manage;
    const char * incoming_recv_at;
    cfg_t incoming_send_to;
    const char * outgoing_recv_at;
    cfg_t outgoing_send_to;
    const char * cmd_kickoff;

	pkg_manage = bpg_pkg_manage_find_nc(app, cfg_get_string(cfg, "pkg-manage", NULL));
	if (pkg_manage == NULL) {
		CPE_ERROR(
			gd_app_em(app), "%s: create: pkg-manage %s not exist!",
			gd_app_module_name(module),
			cfg_get_string(cfg, "pkg-manage", "default"));
		return -1;
	}

    incoming_recv_at = cfg_get_string(cfg, "incoming-recv-at", NULL);
    incoming_send_to = cfg_find_cfg(cfg, "incoming-send-to");
    outgoing_recv_at = cfg_get_string(cfg, "outgoing-recv-at", NULL);
    outgoing_send_to = cfg_find_cfg(cfg, "outgoing-send-to");

    if (incoming_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: incoming-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (outgoing_recv_at == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: outgoing-recv-at not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if (outgoing_send_to == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: outgoing-send-to not configured!",
            gd_app_module_name(module));
        return -1;
    }

    bpg_bind_manage =
        bpg_bind_manage_create(
            app,
            gd_app_alloc(app),
            gd_app_module_name(module),
            pkg_manage,
            gd_app_em(app));
    if (bpg_bind_manage == NULL) return -1;

    if (bpg_bind_manage_set_incoming_recv_at(bpg_bind_manage, incoming_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set incoming-recv-at %s fail!",
            gd_app_module_name(module),
            incoming_recv_at);
        bpg_bind_manage_free(bpg_bind_manage);
        return -1;
    }

	if (bpg_bind_manage_set_incoming_send_to(bpg_bind_manage, incoming_send_to) != 0) {
		CPE_ERROR(
			gd_app_em(app), "%s: create: set incoming_send_to fail!",
			gd_app_module_name(module));
		bpg_bind_manage_free(bpg_bind_manage);
		return -1;
	}

    if (bpg_bind_manage_set_outgoing_recv_at(bpg_bind_manage, outgoing_recv_at) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set outgoing-recv-at %s fail!",
            gd_app_module_name(module),
            outgoing_recv_at);
        bpg_bind_manage_free(bpg_bind_manage);
        return -1;
    }

	if (bpg_bind_manage_set_outgoing_send_to(bpg_bind_manage, outgoing_send_to) != 0) {
		CPE_ERROR(
			gd_app_em(app), "%s: create: set outgoing_send_to fail!",
			gd_app_module_name(module));
		bpg_bind_manage_free(bpg_bind_manage);
		return -1;
	}

    if ((cmd_kickoff = cfg_get_string(cfg, "cmd-kickoff", NULL))) {
        int value = 0;
        if (dr_lib_find_macro_value(&value, bpg_pkg_manage_data_metalib(pkg_manage), cmd_kickoff) == 0) {
            bpg_bind_manage->m_cmd_kickoff = value;
        }
        else {
            char *endptr = NULL;
            value = (int)strtol(cmd_kickoff, &endptr, 10);
            if (endptr == NULL || *endptr != 0) {
                CPE_ERROR(gd_app_em(app), "%s: create: cmd-kickof format error!", gd_app_module_name(module));
                bpg_bind_manage_free(bpg_bind_manage);
                return -1;
            }
            else {
                bpg_bind_manage->m_cmd_kickoff = value;
            }
        }
    }

    bpg_bind_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (bpg_bind_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app),
            "%s: create: done., rsp-manage=%s",
            gd_app_module_name(module),
            bpg_pkg_manage_name(pkg_manage));
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_bind_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_bind_manage_t bpg_bind_manage;

    bpg_bind_manage = bpg_bind_manage_find_nc(app, gd_app_module_name(module));
    if (bpg_bind_manage) {
        bpg_bind_manage_free(bpg_bind_manage);
    }
}
