#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_types.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/pom_mgr/pom_mgr_manage.h"
#include "pom_mgr_internal_ops.h"

static
LPDRMETALIB
pom_mgr_load_metalib(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    const char * metalib_name;
    dr_store_manage_t dr_store_mgr;
    dr_ref_t metalib_ref;
    LPDRMETALIB metalib;

    dr_store_mgr = dr_store_manage_default(app);
    if (dr_store_mgr == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load base meta info: dr_store_manage_default not exist!",
            gd_app_module_name(module));
        return NULL;
    }

    metalib_name = cfg_get_string(cfg, "meta-lib", NULL);
    if (metalib_name == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: meta-lib not configured!",
            gd_app_module_name(module));
        return NULL;
    }

    metalib_ref = dr_ref_create(dr_store_manage_default(app), metalib_name);
    if (metalib_ref == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: metalib %s not exist!", 
            gd_app_module_name(module), metalib_name);
        return NULL;
    }

    metalib = dr_ref_lib(metalib_ref);
    if (metalib == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: metalib %s no data!", 
            gd_app_module_name(module), metalib_name);
        return NULL;
    }

    dr_ref_free(metalib_ref);

    return metalib;
}

static pom_grp_meta_t
pom_manage_meta_load_from_file(
    gd_app_context_t app, gd_app_module_t module, const char * file)
{
    if (file == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: pom-meta: file !",
            gd_app_module_name(module));
        return NULL;
    }

    return NULL;
}

static pom_grp_meta_t
pom_manage_meta_load_from_symbol(
    gd_app_context_t app, gd_app_module_t module, const char * symbol)
{
    return NULL;
}

EXPORT_DIRECTIVE
int pom_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    pom_grp_meta_t pom_grp_meta;
    pom_manage_t pom_manage;
    LPDRMETALIB metalib;
    cfg_t meta_cfg;
    cfg_t child_cfg;

    metalib = pom_mgr_load_metalib(app, module, cfg);
    if (metalib == NULL) return -1;

    meta_cfg = cfg_find_cfg(cfg, "pom-meta");
    if (meta_cfg == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: pom-meta not configured!",
            gd_app_module_name(module));
        return -1;
    }

    if ((child_cfg = cfg_find_cfg(meta_cfg, "load-from-file"))) {
        const char * file = cfg_as_string(child_cfg, NULL);
        if (file == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: pom-meta: load-from-file: format error!",
                gd_app_module_name(module));
            return -1;
        }

        pom_grp_meta = pom_manage_meta_load_from_file(app, module, file);
    }
    else if ((child_cfg = cfg_find_cfg(meta_cfg, "load-from-symbol"))) {
        const char * symbol = cfg_as_string(child_cfg, NULL);
        if (symbol == NULL) {
            CPE_ERROR(
                gd_app_em(app), "%s: pom-meta: load-from-symbol: format error!",
                gd_app_module_name(module));
            return -1;
        }

        pom_grp_meta = pom_manage_meta_load_from_symbol(app, module, symbol);
    }
    else {
        CPE_ERROR(
            gd_app_em(app), "%s: pom-meta: no way to load meta!",
            gd_app_module_name(module));
        return -1;
    }

    if (pom_grp_meta == NULL) return -1;

    pom_manage =
        pom_manage_create(
            app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (pom_manage == NULL) {
        pom_grp_meta_free(pom_grp_meta);
        return -1;
    }

    pom_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    //load_cfg = cfg_find_cfg(cfg, "load-from-memory", 

    pom_grp_meta_free(pom_grp_meta);

    return 0;
}

EXPORT_DIRECTIVE
void pom_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    pom_manage_t pom_manage;

    pom_manage = pom_manage_find_nc(app, gd_app_module_name(module));
    if (pom_manage) {
        pom_manage_free(pom_manage);
    }
}

