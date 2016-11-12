#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "protocol/svr/conn/svr_conn_pro.h"
#include "version_svr.h"
#include "version_svr_version.h"
#include "version_svr_package.h"

static int version_svr_version_load_package(version_svr_t svr, version_svr_version_t version, cfg_t package_cfg);

EXPORT_DIRECTIVE
int version_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    version_svr_t version_svr;
    const char * send_to;
    const char * recv_at;
    struct cfg_it version_it;
    cfg_t version_cfg;
    
    if ((send_to = cfg_get_string(cfg, "send-to", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: send-to not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((recv_at = cfg_get_string(cfg, "recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    version_svr = version_svr_create(app, gd_app_module_name(module), stub, gd_app_alloc(app), gd_app_em(app));
    if (version_svr == NULL) return -1;

    version_svr->m_debug = cfg_get_int8(cfg, "debug", version_svr->m_debug);

    if (version_svr_set_send_to(version_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        return -1;
    }

    if (version_svr_set_recv_at(version_svr, recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set recv-at %s fail!", gd_app_module_name(module), recv_at);
        return -1;
    }

    cfg_it_init(&version_it, cfg_find_cfg(gd_app_cfg(app), "versions"));
    while((version_cfg = cfg_it_next(&version_it))) {
        const char * version_name;
        version_svr_version_t version;
        const char * str_update_strategy;
        uint8_t update_strategy;
        struct cfg_it package_it;
        cfg_t package_cfg;

        if ((version_name = cfg_get_string(version_cfg, "version", NULL)) == NULL) {
            APP_CTX_ERROR(app, "%s: load versions: version config format error!", gd_app_module_name(module));
            version_svr_free(version_svr);
            return -1;
        }

        if ((str_update_strategy = cfg_get_string(version_cfg, "strategy", NULL)) == NULL) {
            APP_CTX_ERROR(
                app, "%s: load versions: version %s strategy not configured!",
                gd_app_module_name(module), version_name);
            version_svr_free(version_svr);
            return -1;
        }
        
        if (strcmp(str_update_strategy, "advise") == 0) { update_strategy = SVR_VERSION_UPDATE_STRATEGY_ADVISE; }
        else if (strcmp(str_update_strategy, "force") == 0) { update_strategy = SVR_VERSION_UPDATE_STRATEGY_FORCE; }
        else if (strcmp(str_update_strategy, "silence") == 0) { update_strategy = SVR_VERSION_UPDATE_STRATEGY_SILENCE; }
        else if (strcmp(str_update_strategy, "hide") == 0) { update_strategy = SVR_VERSION_UPDATE_STRATEGY_NO; }
        else {
            APP_CTX_ERROR(
                app, "%s: load version_svr: version %s strategy %s unknown, support(advise,force,silence,hide)!",
                gd_app_module_name(module), version_name, str_update_strategy);
            version_svr_free(version_svr);
            return -1;
        }
        
        version = version_svr_version_create(version_svr, version_name, update_strategy);
        if (version == NULL) {
            APP_CTX_ERROR(app, "%s: load versions: create version %s fail!", gd_app_module_name(module), version_name);
            version_svr_free(version_svr);
            return -1;
        }

        
        cfg_it_init(&package_it, cfg_find_cfg(version_cfg, "packages"));
        while((package_cfg = cfg_it_next(&package_it))) {
            if (version_svr_version_load_package(version_svr, version, package_cfg) != 0) {
                version_svr_free(version_svr);
                return -1;
            }
        }

        APP_CTX_INFO(
            app, "%s: version %s!",
            gd_app_module_name(module), version->m_name);
    }

    if (version_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void version_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    version_svr_t version_svr;

    version_svr = version_svr_find_nc(app, gd_app_module_name(module));
    if (version_svr) {
        version_svr_free(version_svr);
    }
}

static int version_svr_version_load_package(version_svr_t svr, version_svr_version_t version, cfg_t package_cfg) {
    version_svr_package_t package;
    const char * chanel;
    const char * str_device_category;
    uint8_t device_category;
    SVR_VERSION_PACKAGE package_data;
    cfg_t type_cfg;
    
    if ((chanel = cfg_get_string(package_cfg, "chanel", NULL)) == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: load versions: version %s package chanel not configured",
            version_svr_name(svr), version->m_name);
        return -1;
    }

    if ((str_device_category = cfg_get_string(package_cfg, "device-category", NULL)) == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: load versions: version %s package device-category not configured",
            version_svr_name(svr), version->m_name);
        return -1;
    }

    if (strcmp(str_device_category, "windows") == 0) {
        device_category = conn_svr_device_windows;
    }
    else if (strcmp(str_device_category, "ios") == 0) {
        device_category = conn_svr_device_ios;
    }
    else if (strcmp(str_device_category, "android") == 0) {
        device_category = conn_svr_device_android;
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: load versions: versions %s device category %s unknown!",
            version_svr_name(svr), version->m_name, str_device_category);
        return -1;
    }
            
    package = version_svr_package_create(version, chanel, device_category, &package_data);
    if (package == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: load versions: version %s package for chanel %s device category %s create fail!",
            version_svr_name(svr), version->m_name, chanel, str_device_category);
        return -1;
    }

    if ((type_cfg = cfg_find_cfg(package_cfg, "full"))) {
        package->m_data.type = svr_version_package_type_full;
        
        cpe_str_dup(package->m_data.data.full.md5, sizeof(package->m_data.data.full.md5), cfg_get_string(type_cfg, "md5", ""));
        if (package->m_data.data.full.md5[0] == 0) {
            CPE_ERROR(
                svr->m_em, "%s: load versions: version %s package for chanel %s device category %s md5 not configured!",
                version_svr_name(svr), version->m_name, chanel, str_device_category);
            return -1;
        }
    
        cpe_str_dup(package->m_data.data.full.url, sizeof(package->m_data.data.full.url), cfg_get_string(type_cfg, "url", ""));
        if (package->m_data.data.full.url[0] == 0) {
            CPE_ERROR(
                svr->m_em, "%s: load versions: version %s package for chanel %s device category %s url not configured!",
                version_svr_name(svr), version->m_name, chanel, str_device_category);
            return -1;
        }
    
        package->m_data.data.full.size = cfg_get_uint32(type_cfg, "size", 0);
        if (package->m_data.data.full.size == 0) {
            CPE_ERROR(
                svr->m_em, "%s: load versions: version %s package for chanel %s device category %s size not configured!",
                version_svr_name(svr), version->m_name, chanel, str_device_category);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            svr->m_em, "%s: load versions: version %s package for chanel %s device category %s unknown type cfg!",
            version_svr_name(svr), version->m_name, chanel, str_device_category);
        return -1;
    }
    
    return 0;
}
