#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "bpg_pkg_internal_types.h"

static int bpg_pkg_manage_app_validate_meta(gd_app_context_t app, gd_app_module_t module, bpg_pkg_manage_t mgr) {
    if (bpg_pkg_manage_data_metalib(mgr) == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: validate meta info: meta lib %s not exist!",
            bpg_pkg_manage_name(mgr), bpg_pkg_manage_data_metalib_name(mgr));
        return -1;
    }

    return 0;
}

static int bpg_pkg_manage_app_load_meta(gd_app_context_t app, gd_app_module_t module, bpg_pkg_manage_t mgr, cfg_t cfg) {
    const char * arg;
    cfg_t cmd_meta_cfg;
    LPDRMETALIB metalib;

    if ((arg = cfg_get_string(cfg, "lib-name", NULL))) {
        if (bpg_pkg_manage_set_data_metalib(mgr, arg) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: load meta info: set meta ref %s fail!",
                bpg_pkg_manage_name(mgr), arg);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: lib-name not configured!",
            bpg_pkg_manage_name(mgr));
        return -1;
    }

    metalib = bpg_pkg_manage_data_metalib(mgr);
    if (metalib == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: no meta lib!",
            gd_app_module_name(module));
        return -1;
    }

    if ((cmd_meta_cfg = cfg_find_cfg(cfg, "cmd-meta-name"))) {
        if (cfg_type(cmd_meta_cfg) == CPE_CFG_TYPE_STRING) {
            if (bpg_pkg_manage_add_cmd_by_meta(mgr, cfg_as_string(cmd_meta_cfg, NULL)) != 0) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load meta info: add cmd-meta-name %s fail!",
                    gd_app_module_name(module), cfg_as_string(cmd_meta_cfg, NULL));
                return -1;
            }
        }
        else if (cfg_type(cmd_meta_cfg) == CPE_CFG_TYPE_SEQUENCE) {
            struct cfg_it it;
            cfg_t child_cfg;

            cfg_it_init(&it, cmd_meta_cfg);

            while((child_cfg = cfg_it_next(&it))) {
                const char * arg = cfg_as_string(child_cfg, NULL);
                if (arg == NULL) {
                    CPE_ERROR(
                        gd_app_em(app), "%s: load meta info: add cmd-meta-name(sub) type error!",
                        gd_app_module_name(module));
                    return -1;
                }

                if (bpg_pkg_manage_add_cmd_by_meta(mgr, arg)) {
                    CPE_ERROR(
                        gd_app_em(app), "%s: load meta info: add cmd-meta-name %s fail!",
                        gd_app_module_name(module), arg);
                    return -1;
                }
            }
        }
        else {
            CPE_ERROR(
                gd_app_em(app), "%s: load meta info: cmd-meta-name type error!",
                gd_app_module_name(module));
            return -1;
        }
    }

    if ((cmd_meta_cfg = cfg_find_cfg(cfg, "cmds"))) {
        struct cfg_it it;
        cfg_t child_cfg;

        cfg_it_init(&it, cmd_meta_cfg);

        while((child_cfg = cfg_it_next(&it))) {
            int cmd;

            child_cfg = cfg_child_only(child_cfg);
            if (child_cfg == NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load meta info: add cmd: config type error!",
                    gd_app_module_name(module));
                return -1;
            }

            cmd = cfg_as_uint32(child_cfg, 0);
            if (cmd == 0) {
                const char * cmd_name = cfg_as_string(child_cfg, NULL);
                if (cmd_name == NULL) {
                    CPE_ERROR(
                        gd_app_em(app), "%s: load meta info: add cmd: read cmd fail, not int or string!",
                        gd_app_module_name(module));
                    return -1;
                }

                if (dr_lib_find_macro_value(&cmd, metalib, cmd_name) != 0) {
                    CPE_ERROR(
                        gd_app_em(app), "%s: load meta info: add cmd: read value of cmd %s fail!",
                        gd_app_module_name(module), cmd_name);
                    return -1;
                }
            }

            if (bpg_pkg_manage_add_cmd(mgr, cmd, cfg_name(child_cfg))) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load meta info: add cmd %d ==> %s fail!",
                    gd_app_module_name(module), cmd, cfg_name(child_cfg));
                return -1;
            }
        }
    }

    if (cpe_hash_table_count(&mgr->m_cmd_info_by_cmd) == 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: load meta info: no cmd meta info!",
            gd_app_module_name(module));
        return -1;
    }

    return 0;
}

static bpg_pkg_debug_level_t
bpg_pkg_manage_app_load_calc_debug_level(gd_app_context_t app, gd_app_module_t module, const char * name, const char * value) {
    if (value == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: load pkg debug info: %s: no value!",
            gd_app_module_name(module), name);
        return -1;
    }

    if (strcmp(value, "none") == 0) return bpg_pkg_debug_none;
    if (strcmp(value, "summary") == 0) return bpg_pkg_debug_summary;
    if (strcmp(value, "detail") == 0) return bpg_pkg_debug_detail;
    if (strcmp(value, "progress") == 0) return bpg_pkg_debug_progress;

    CPE_ERROR(
        gd_app_em(app), "%s: load pkg debug info: %s: unknown value %s!",
        gd_app_module_name(module), name, value);

    return -1;
}

static int bpg_pkg_manage_app_load_pkg_debug_info(gd_app_context_t app, gd_app_module_t module, bpg_pkg_manage_t mgr, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child;
    int rv;

    rv = 0;

    cfg_it_init(&child_it, cfg);

    while((child = cfg_it_next(&child_it))) {
        const char * name;
        bpg_pkg_debug_level_t level;
        uint32_t cmd;

        name = cfg_name(child);
        level = bpg_pkg_manage_app_load_calc_debug_level(app, module, name, cfg_as_string(child, NULL));

        if (strcmp(name, "default") == 0) {
            mgr->m_pkg_debug_default_level = level;
            if (mgr->m_debug) {
                CPE_INFO(
                    gd_app_em(app), "%s: load pkg debug info: %s => %s",
                    gd_app_module_name(module), name, cfg_as_string(child, NULL));
            }
            continue;
        }

        if (dr_ctype_try_read_uint32(&cmd, name, CPE_DR_TYPE_STRING, gd_app_em(app)) != 0) {
            LPDRMETALIB metalib;
            int buf;

            metalib = bpg_pkg_manage_data_metalib(mgr);
            if (metalib == NULL) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load pkg debug info: %s: no meta lib!",
                    gd_app_module_name(module), name);
                rv = -1;
                continue;
            }

            if (dr_lib_find_macro_value(&buf, metalib, name) != 0) {
                CPE_ERROR(
                    gd_app_em(app), "%s: load pkg debug info: %s: macro not exist!",
                    gd_app_module_name(module), name);
                rv = -1;
                continue;
            }

            cmd = buf;
        }

        bpg_pkg_manage_set_debug_level(mgr, cmd, level);
        if (mgr->m_debug) {
            CPE_INFO(
                gd_app_em(app), "%s: load pkg debug info: %s => %s",
                gd_app_module_name(module), name, cfg_as_string(child, NULL));
        }
    }

    return rv;
}

EXPORT_DIRECTIVE
int bpg_pkg_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    bpg_pkg_manage_t bpg_pkg_manage;
    cfg_t child_cfg;

    bpg_pkg_manage = bpg_pkg_manage_create(app, gd_app_module_name(module), gd_app_em(app));
    if (bpg_pkg_manage == NULL) {
        return -1;
    }

    if (bpg_pkg_manage_set_base_cvt(bpg_pkg_manage, cfg_get_string(cfg, "base-cvt", NULL)) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set base-cvt %s fail",
            gd_app_module_name(module), cfg_get_string(cfg, "base-cvt", NULL));
        bpg_pkg_manage_free(bpg_pkg_manage);
        return -1;
    }

    if (bpg_pkg_manage_set_data_cvt(bpg_pkg_manage, cfg_get_string(cfg, "data-cvt", NULL)) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set data-cvt %s fail",
            gd_app_module_name(module), cfg_get_string(cfg, "data-cvt", NULL));
        bpg_pkg_manage_free(bpg_pkg_manage);
        return -1;
    }

    child_cfg = cfg_find_cfg(cfg, "meta");
    if (child_cfg) {
        if (bpg_pkg_manage_app_load_meta(app, module, bpg_pkg_manage, child_cfg) != 0) {
            bpg_pkg_manage_free(bpg_pkg_manage);
            return -1;
        }
    }

    bpg_pkg_manage_set_op_buff_capacity(
        bpg_pkg_manage,
        cfg_get_int32(cfg, "buf-size", bpg_pkg_manage->m_op_buff_capacity));

    bpg_pkg_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    bpg_pkg_manage->m_zip_size_threshold = cfg_get_int32(cfg, "zip-size-threshold", bpg_pkg_manage->m_zip_size_threshold);

    if (cfg_get_int32(child_cfg, "validate", 1) != 0
        && bpg_pkg_manage_app_validate_meta(app, module, bpg_pkg_manage) != 0)
    {
        bpg_pkg_manage_free(bpg_pkg_manage);
        return -1;
    }

    if (bpg_pkg_manage_app_load_pkg_debug_info(app, module, bpg_pkg_manage, cfg_find_cfg(cfg, "pkg-debug-infos")) != 0) {
        bpg_pkg_manage_free(bpg_pkg_manage);
        return -1;
    }

    if (bpg_pkg_manage->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done. base-cvt=%s, data-cvt=%s",
            gd_app_module_name(module),
            bpg_pkg_manage_base_cvt_name(bpg_pkg_manage),
            bpg_pkg_manage_data_cvt_name(bpg_pkg_manage));
    }

    return 0;
}

EXPORT_DIRECTIVE
void bpg_pkg_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    bpg_pkg_manage_t bpg_pkg_manage;

    bpg_pkg_manage = bpg_pkg_manage_find_nc(app, gd_app_module_name(module));
    if (bpg_pkg_manage) {
        bpg_pkg_manage_free(bpg_pkg_manage);
    }
}
