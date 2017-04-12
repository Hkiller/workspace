#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "protocol/svr/conn/svr_conn_pro.h"
#include "dir_svr.h"
#include "dir_svr_region.h"
#include "dir_svr_server.h"

static int dir_svr_load_regions(dir_svr_t svr);

EXPORT_DIRECTIVE
int dir_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    dir_svr_t dir_svr;
    const char * send_to;
    const char * recv_at;

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

    dir_svr = dir_svr_create(app, gd_app_module_name(module), stub, gd_app_alloc(app), gd_app_em(app));
    if (dir_svr == NULL) return -1;

    dir_svr->m_debug = cfg_get_int8(cfg, "debug", dir_svr->m_debug);

    if (dir_svr_set_send_to(dir_svr, send_to) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set send-to %s fail!", gd_app_module_name(module), send_to);
        return -1;
    }

    if (dir_svr_set_recv_at(dir_svr, recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set recv-at %s fail!", gd_app_module_name(module), recv_at);
        return -1;
    }

    if (dir_svr_load_regions(dir_svr) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load regions fail!", gd_app_module_name(module));
        return -1;
    }

    if (dir_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done.", gd_app_module_name(module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void dir_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dir_svr_t dir_svr;

    dir_svr = dir_svr_find_nc(app, gd_app_module_name(module));
    if (dir_svr) {
        dir_svr_free(dir_svr);
    }
}

static int dir_svr_region_load_servers(dir_svr_region_t region, cfg_t region_cfg) {
    dir_svr_t svr = region->m_svr;
    struct cfg_it server_it;
    cfg_t server_cfg;

    cfg_it_init(&server_it, cfg_find_cfg(region_cfg, "servers"));
    while((server_cfg = cfg_it_next(&server_it))) {
        const char * ip;
        uint16_t port;
        dir_svr_server_t server;

        ip = cfg_get_string(server_cfg, "ip", NULL);
        if (ip == NULL) {
            CPE_ERROR(svr->m_em, "%s: load regions: read ip fail!", dir_svr_name(svr));
            return -1;
        }

        if (cfg_try_get_uint16(server_cfg, "port", &port) != 0) {
            CPE_ERROR(svr->m_em, "%s: load regions: read port fail!", dir_svr_name(svr));
            return -1;
        }

        server = dir_svr_server_create(region, ip, port);
        if (server == NULL) {
            CPE_ERROR(svr->m_em, "%s: load regions: create server fail!", dir_svr_name(svr));
            return -1;
        }
    }

    return 0;
}

static int dir_svr_load_regions(dir_svr_t svr) {
    struct cfg_it region_it;
    cfg_t region_cfg;

    cfg_it_init(&region_it, cfg_find_cfg(gd_app_cfg(svr->m_app), "regions"));
    while((region_cfg = cfg_it_next(&region_it))) {
        uint16_t region_id;
        const char * region_name;
        const char * str_region_state;
        uint8_t region_state;
        const char * str_region_type;
        uint8_t region_type;
        dir_svr_region_t region;
        struct cfg_it child_it;
        cfg_t child_cfg;

        if (cfg_try_get_uint16(region_cfg, "id", &region_id) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load regions: %s: read id fail!",
                dir_svr_name(svr), cfg_name(region_cfg));
            return -1;
        }

        if ((region_name = cfg_get_string(region_cfg, "name", NULL)) == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: load regions: %s: read name fail!",
                dir_svr_name(svr), cfg_name(region_cfg));
            return -1;
        }

        if ((str_region_state = cfg_get_string(region_cfg, "state", NULL)) == NULL) {
            region_state = SVR_DIR_REGION_NORMAL;
        }
        else {
            if (strcmp(str_region_state, "normal") == 0) {
                region_state = SVR_DIR_REGION_NORMAL;
            }
            else if (strcmp(str_region_state, "maintenance") == 0) {
                region_state = SVR_DIR_REGION_MAINTENANCE;
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: load regions: %s: read state %s fail, should be normal or maintenance!",
                    dir_svr_name(svr), cfg_name(region_cfg), str_region_state);
                return -1;
            }
        }

        str_region_type = cfg_get_string(region_cfg, "type", NULL);
        if (str_region_type == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: load regions: %s: type not configured!",
                dir_svr_name(svr), cfg_name(region_cfg));
            return -1;
        }

        if (strcmp(str_region_type, "public") == 0) {
            region_type = SVR_DIR_REGION_PUBLIC;
        }
        else if (strcmp(str_region_type, "test") == 0) {
            region_type = SVR_DIR_REGION_TESTING;
        }
        else if (strcmp(str_region_type, "internal") == 0) {
            region_type = SVR_DIR_REGION_INTERNAL;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: load regions: %s: type %s unknown!",
                dir_svr_name(svr), cfg_name(region_cfg), str_region_type);
            return -1;
        }

        region = dir_svr_region_create(svr, region_id, region_name, region_state, region_type);
        if (region == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: load regions: %s: name id fail!",
                dir_svr_name(svr), cfg_name(region_cfg));
            return -1;
        }

        cfg_it_init(&child_it, cfg_find_cfg(region_cfg, "support-chanels"));
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * chanel = cfg_as_string(child_cfg, NULL);
            if (chanel == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: load regions: %s: support-chanels format error!",
                    dir_svr_name(svr), region_name);
                return -1;
            }

            if (dir_svr_region_add_support_chanel(region, chanel) != 0) {
                CPE_ERROR(
                    svr->m_em, "%s: load regions: %s: support-chanels add chanel %s fail!",
                    dir_svr_name(svr), region_name, chanel);
                return -1;
            }
        }

        cfg_it_init(&child_it, cfg_find_cfg(region_cfg, "support-device-categories"));
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * str_category;
            uint8_t category;

            str_category = cfg_as_string(child_cfg, NULL);
            if (str_category == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: load regions: %s: support-device-categories format error!",
                    dir_svr_name(svr), region_name);
                return -1;
            }

            if (strcmp(str_category, "windows") == 0) {
                category = conn_svr_device_windows;
            }
            else if (strcmp(str_category, "ios") == 0) {
                category = conn_svr_device_ios;
            }
            else if (strcmp(str_category, "android") == 0) {
                category = conn_svr_device_android;
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: load regions: %s: device category %s unknown!",
                    dir_svr_name(svr), region_name, str_category);
                return -1;
            }
            
            if (dir_svr_region_add_support_category(region, category) != 0) {
                CPE_ERROR(
                    svr->m_em, "%s: load regions: %s: support-device-categories add category %d fail!",
                    dir_svr_name(svr), region_name, category);
                return -1;
            }
        }
        
        if (dir_svr_region_load_servers(region, region_cfg) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load regions: %s: load servers fail!",
                dir_svr_name(svr), cfg_name(region_cfg));
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: load regions: %s: load region s[id=%d, name=%s, state=%s, type=%s]!",
                dir_svr_name(svr), cfg_name(region_cfg), region->m_region_id, region->m_region_name,
                region->m_region_state == SVR_DIR_REGION_NORMAL ? "normal" : "maintenance",
                str_region_type);
        }
    }

    return 0;
}
