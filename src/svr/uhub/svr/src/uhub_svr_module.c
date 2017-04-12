#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "uhub_svr_ops.h"

static int uhub_svr_load_def(uhub_svr_t svr, cfg_t cfg);

EXPORT_DIRECTIVE
int uhub_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    uhub_svr_t uhub_svr;
    mongo_cli_proxy_t db;
    cfg_t uhub_def;

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    db = mongo_cli_proxy_find_nc(app, cfg_get_string(cfg, "db", NULL));
    if (db == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: db %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "db", "default"));
        return -1;
    }

    uhub_svr =
        uhub_svr_create(
            app, gd_app_module_name(module),
            stub, db,
            gd_app_alloc(app), gd_app_em(app));
    if (uhub_svr == NULL) return -1;

    uhub_svr->m_debug = cfg_get_int8(cfg, "debug", uhub_svr->m_debug);

    uhub_def = cfg_find_cfg(gd_app_cfg(app), "uhub-svr-def");
    if (uhub_def == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: uhub-svr-def not configured.", gd_app_module_name(module));
        uhub_svr_free(uhub_svr);
        return -1;
    }

    if (uhub_svr_load_def(uhub_svr, uhub_def) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load uhub-svr-def fail.", gd_app_module_name(module));
        uhub_svr_free(uhub_svr);
        return -1;
    }

    if (uhub_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void uhub_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    uhub_svr_t uhub_svr;

    uhub_svr = uhub_svr_find_nc(app, gd_app_module_name(module));
    if (uhub_svr) {
        uhub_svr_free(uhub_svr);
    }
}

static int uhub_svr_load_def(uhub_svr_t svr, cfg_t cfg) {
    struct cfg_it svr_type_it;
    cfg_t svr_type_cfg;

    cfg_it_init(&svr_type_it, cfg_find_cfg(cfg, "svr-types"));

    while((svr_type_cfg = cfg_it_next(&svr_type_it))) {
        uhub_svr_info_t svr_info;
        const char * svr_name = cfg_name(svr_type_cfg);
        const char * to_uid_entry = cfg_get_string(svr_type_cfg, "to-uid-entry", NULL);

        if (to_uid_entry == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load svr %s: to-uid-entry not configured.",
                uhub_svr_name(svr), svr_name);
            return -1;
        }

        svr_info = uhub_svr_info_create(svr, svr_name, to_uid_entry);
        if (svr_info == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: load svr %s: create fail", uhub_svr_name(svr), svr_name);
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: create: load svr %s to-uid-entry %s success", uhub_svr_name(svr), svr_name, to_uid_entry);
        }
    }

    return 0;
}
