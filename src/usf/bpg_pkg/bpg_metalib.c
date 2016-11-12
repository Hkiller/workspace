#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_library.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "usf/bpg_pkg/bpg_pkg_types.h"

extern char g_metalib_base_package[];

EXPORT_DIRECTIVE
int bpg_metalib_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    dr_store_manage_t dr_store_mgr;
    dr_store_t dr_store;
    LPDRMETALIB metalib;
    int debug;

    debug = cfg_get_int32(cfg, "debug", 0);

    dr_store_mgr = dr_store_manage_default(app);
    if (dr_store_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load base meta info: dr_store_manage_default not exist!",
            gd_app_module_name(module));
        return -1;
    }

    dr_store = dr_store_find_or_create(dr_store_mgr, BPG_BASEPKG_LIB_NAME);
    if (dr_store == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load base meta info: dr_store %s create fail!",
            gd_app_module_name(module), BPG_BASEPKG_LIB_NAME);
        return -1;
    }

    if (dr_store_lib(dr_store)) {
        if (debug) {
            CPE_ERROR(
                gd_app_em(app), "%s: load base meta info: use extern metalib",
                gd_app_module_name(module));
        }

        return 0;
    }

    metalib = (LPDRMETALIB)g_metalib_base_package;
    if (dr_store_set_lib(dr_store, metalib, NULL, NULL) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: set default metalib fail!",
            gd_app_module_name(module));
        return -1;
    }

    if (debug) {
        CPE_ERROR(
            gd_app_em(app), "%s: load base meta info: use self metalib",
            gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_metalib_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dr_store_manage_t dr_store_mgr;
    dr_store_t dr_store;
    LPDRMETALIB metalib;

    dr_store_mgr = dr_store_manage_default(app);
    if (dr_store_mgr == NULL) return;

    dr_store = dr_store_find_or_create(dr_store_mgr, BPG_BASEPKG_LIB_NAME);
    if (dr_store == NULL) return;

    if (dr_store_lib(dr_store) == NULL) return;

    metalib = (LPDRMETALIB)g_metalib_base_package;
    if (dr_store_lib(dr_store) == metalib) {
        dr_store_reset_lib(dr_store, NULL, NULL, NULL);
    }
}
