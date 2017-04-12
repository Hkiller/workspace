#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_error.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/evt/evt_manage.h"
#include "gd/evt/evt_read.h"
#include "evt_internal_ops.h"

static int gd_evt_mgr_app_init_register_metalib(gd_evt_mgr_t mgr, const char * libname) {
    dr_store_manage_t dr_store_mgr;
    dr_store_t dr_store;
    LPDRMETALIB metalib;

    dr_store_mgr = dr_store_manage_default(mgr->m_app);
    if (dr_store_mgr == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create: register metalib %s: default dr_store_manage not exist!",
            gd_evt_mgr_name(mgr), libname);
        return -1;
    }

    dr_store = dr_store_find(dr_store_mgr, libname);
    if (dr_store == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create: register metalib %s: lib not exist!",
            gd_evt_mgr_name(mgr), libname);
        return -1;
    }

    metalib = dr_store_lib(dr_store);
    if (metalib == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create: register metalib %s: lib no metalib!",
            gd_evt_mgr_name(mgr), libname);
        return -1;
    }

    return gd_evt_mgr_register_evt_in_metalib(mgr, metalib);
}

static int gd_evt_mgr_app_init_load_metalibs(gd_evt_mgr_t mgr, cfg_t cfg) {
    cfg_t meta_cfg;

    meta_cfg = cfg_find_cfg(cfg, "meta");
    if (meta_cfg == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create: meta not configured!",
            gd_evt_mgr_name(mgr));
        return -1;
    }

    if (cfg_type(meta_cfg) == CPE_DR_TYPE_STRING) {
        return gd_evt_mgr_app_init_register_metalib(mgr, cfg_as_string(meta_cfg, NULL));
    }
    else {
        struct cfg_it it;
        cfg_t child_cfg;

        cfg_it_init(&it, meta_cfg);
        while((child_cfg = cfg_it_next(&it))) {
            if (gd_evt_mgr_app_init_register_metalib(mgr, cfg_as_string(child_cfg, "no-libname")) != 0) {
                return -1;
            }
        }

        return 0;
    }

}

EXPORT_DIRECTIVE
int gd_evt_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    gd_evt_mgr_t gd_evt_mgr;

    gd_evt_mgr =
        gd_evt_mgr_create(
            app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (gd_evt_mgr == NULL) return -1;

    if (gd_evt_mgr_app_init_load_metalibs(gd_evt_mgr, cfg) != 0) {
        gd_evt_mgr_free(gd_evt_mgr);
        return -1;
    }

    gd_evt_mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (gd_evt_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            gd_evt_mgr_name(gd_evt_mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void gd_evt_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    gd_evt_mgr_t gd_evt_mgr;

    gd_evt_mgr = gd_evt_mgr_find_nc(app, gd_app_module_name(module));
    if (gd_evt_mgr) {
        gd_evt_mgr_free(gd_evt_mgr);
    }
}

