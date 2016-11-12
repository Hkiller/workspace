#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "conn_http_svr_ops.h"

static int conn_http_svr_app_init_load_services(conn_http_svr_t svr, cfg_t cfg);

EXPORT_DIRECTIVE
int conn_http_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    conn_http_svr_t conn_http_svr;
    const char * request_recv_at;
    const char * response_recv_at;
    const char * str_port;
    uint16_t port;
    const char * str_buf_size;
    uint64_t buf_size;

    str_port = gd_app_arg_find(app, "--port");
    if (str_port == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: port not set in args!", gd_app_module_name(module));
        return -1;
    }

    port = atoi(str_port);;

    str_buf_size = gd_app_arg_find(app, "--buf-size");
    if (str_buf_size == NULL) str_buf_size = cfg_get_string(cfg, "buf-size", NULL);
    if (str_buf_size == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: buf-size not configured in args or config!", gd_app_module_name(module));
        return -1;
    }
    if (cpe_str_parse_byte_size(&buf_size, str_buf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: buf-size %s format error!", gd_app_module_name(module), str_buf_size);
        return -1;
    }

    if ((request_recv_at = cfg_get_string(cfg, "request-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: request-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    if ((response_recv_at = cfg_get_string(cfg, "response-recv-at", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: response-recv-at not configured!", gd_app_module_name(module));
        return -1;
    }

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    conn_http_svr =
        conn_http_svr_create(
            app, gd_app_module_name(module),
            stub, port, gd_app_alloc(app), gd_app_em(app));
    if (conn_http_svr == NULL) return -1;

    conn_http_svr->m_debug = cfg_get_int8(cfg, "debug", conn_http_svr->m_debug);

    if (conn_http_svr_set_request_recv_at(conn_http_svr, request_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set request-recv-at %s fail!", gd_app_module_name(module), request_recv_at);
        conn_http_svr_free(conn_http_svr);
        return -1;
    }

    if (conn_http_svr_set_response_recv_at(conn_http_svr, response_recv_at) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set response-recv-at %s fail!", gd_app_module_name(module), response_recv_at);
        conn_http_svr_free(conn_http_svr);
        return -1;
    }

    if (conn_http_svr_set_ringbuf_size(conn_http_svr, buf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set buf size to %d fail!",
            conn_http_svr_name(conn_http_svr), (int)buf_size);
        return -1;
    }

    if (conn_http_svr_app_init_load_services(conn_http_svr, cfg_find_cfg(gd_app_cfg(app), "services")) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: load services fail!", gd_app_module_name(module));
        conn_http_svr_free(conn_http_svr);
        return -1;
    }

    if (conn_http_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. listen at %d", gd_app_module_name(module), port);
    }

    return 0;
}

EXPORT_DIRECTIVE
void conn_http_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    conn_http_svr_t conn_http_svr;

    conn_http_svr = conn_http_svr_find_nc(app, gd_app_module_name(module));
    if (conn_http_svr) {
        conn_http_svr_free(conn_http_svr);
    }
}

static int conn_http_svr_app_init_load_service_cmds_by_prefix(conn_http_svr_t svr, conn_http_service_t service, const char * prefix) {
    LPDRMETAENTRY data_entry = set_svr_svr_info_pkg_data_entry(service->m_dispatch_to);
    LPDRMETA data_meta = dr_entry_ref_meta(data_entry);
    size_t i;

    for(i = 0; i < dr_meta_entry_num(data_meta); i++) {
        conn_http_cmd_t cmd;
        LPDRMETAENTRY entry = dr_meta_entry_at(data_meta, i);
        int id;
        LPDRMETA cmd_meta;
        const char * path;

        cmd_meta = dr_entry_ref_meta(entry);
        path = dr_meta_name(cmd_meta);
        id = dr_entry_id(entry);

        if (strstr(path, prefix) != path) continue;

        cmd = conn_http_cmd_create(service, path, id, cmd_meta);
        if (cmd == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: cmd %s: create cmd fail!",
                conn_http_svr_name(svr), service->m_path, path);
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: create: load service %s: cmd %s: id=%d, meta=%s!",
                conn_http_svr_name(svr), service->m_path, path, id, cmd_meta ? dr_meta_name(cmd_meta) : "none");
        }
    }

    return 0;
}

static int conn_http_svr_app_init_load_service_cmds(conn_http_svr_t svr, conn_http_service_t service, cfg_t cfg) {
    struct cfg_it cmd_it;
    cfg_t cmd_cfg;
    LPDRMETALIB metalib = dr_meta_owner_lib(set_svr_svr_info_pkg_meta(service->m_dispatch_to));

    assert(metalib);

    cfg_it_init(&cmd_it, cfg);

    while((cmd_cfg = cfg_it_next(&cmd_it))) {
        conn_http_cmd_t cmd;
        const char * path = cfg_get_string(cmd_cfg, "path", "");
        const char * str_id = cfg_get_string(cmd_cfg, "id", NULL);
        int id;
        LPDRMETA cmd_meta;

        if (str_id == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: cmd %s: id not configured",
                conn_http_svr_name(svr), service->m_path, path);
            return -1;
        }

        if (dr_lib_find_macro_value(&id, metalib, str_id) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: cmd %s: id %s not exist",
                conn_http_svr_name(svr), service->m_path, path, str_id);
            return -1;
        }

        cmd_meta = set_svr_svr_info_find_data_meta_by_cmd(service->m_dispatch_to, id);
        cmd = conn_http_cmd_create(service, path, id, cmd_meta);
        if (cmd == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: cmd %s: create cmd fail!",
                conn_http_svr_name(svr), service->m_path, path);
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: create: load service %s: cmd %s: id=%d, meta=%s!",
                conn_http_svr_name(svr), service->m_path, path, id, cmd_meta ? dr_meta_name(cmd_meta) : "none");
        }
    }

    return 0;
}

static int conn_http_svr_app_init_load_services(conn_http_svr_t svr, cfg_t cfg) {
    struct cfg_it service_it;
    cfg_t service_cfg;

    cfg_it_init(&service_it, cfg);

    while((service_cfg = cfg_it_next(&service_it))) {
        conn_http_service_t service;
        const char * svr_name = cfg_name(service_cfg);
        const char * svr_path = cfg_get_string(service_cfg, "service-path", NULL);
        const char * str_dispatch_to = cfg_get_string(service_cfg, "dispatch-to", NULL);
        const char * str_formator = cfg_get_string(service_cfg, "formator", NULL);
        set_svr_svr_info_t dispatch_to;
        conn_http_formator_t formator;
        cfg_t cmd_prefixes_cfg;

        if (svr_path == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: service-path not configured",
                conn_http_svr_name(svr), svr_name);
            return -1;
        }

        if (str_dispatch_to == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: dispatch-to not configured",
                conn_http_svr_name(svr), svr_name);
            return -1;
        }

        if (str_formator == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: formator not configured",
                conn_http_svr_name(svr), svr_name);
            return -1;
        }

        dispatch_to = set_svr_svr_info_find_by_name(svr->m_stub, str_dispatch_to);
        if (dispatch_to == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: dispatch-to service %s not exist",
                conn_http_svr_name(svr), svr_name, str_dispatch_to);
            return -1;
        }

        if (strcmp(str_formator, "json") == 0) {
            formator = &g_conn_http_formator_json;
        }
        else if (strcmp(str_formator, "yaml") == 0) {
            formator = &g_conn_http_formator_yaml;
        }
        else if (strcmp(str_formator, "xml") == 0) {
            formator = &g_conn_http_formator_xml;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: formator %s not exist",
                conn_http_svr_name(svr), svr_name, str_formator);
            return -1;
        }

        service = conn_http_service_create(svr, svr_path, dispatch_to, formator);
        if (service == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: create service fail!",
                conn_http_svr_name(svr), svr_name);
            return -1;
        }

        if ((cmd_prefixes_cfg = cfg_find_cfg(service_cfg, "cmd-prefix"))) {
            const char * cmd_prefix;

            if ((cmd_prefix = cfg_as_string(cmd_prefixes_cfg, NULL))) {
                if (conn_http_svr_app_init_load_service_cmds_by_prefix(svr, service, cmd_prefix) != 0) {
                    CPE_ERROR(
                        svr->m_em, "%s: create: load service %s: load command of prefix %s fail!",
                        conn_http_svr_name(svr), svr_name, cmd_prefix);
                    conn_http_service_free(service);
                    return -1;
                }
            }
            else if (cfg_type(cmd_prefixes_cfg) == CPE_CFG_TYPE_SEQUENCE) {
                struct cfg_it child_it;
                cfg_t child_cfg;
                cfg_it_init(&child_it, cmd_prefixes_cfg);
                while((child_cfg = cfg_it_next(&child_it))) {
                    const char * cmd_prefixe = cfg_as_string(child_cfg, NULL);
                    if (cmd_prefixe == NULL) {
                        CPE_ERROR(
                            svr->m_em, "%s: create: load service %s: cmd-prefix format error!",
                            conn_http_svr_name(svr), svr_name);
                        conn_http_service_free(service);
                        return -1;
                    }

                    if (conn_http_svr_app_init_load_service_cmds_by_prefix(svr, service, cmd_prefix) != 0) {
                        CPE_ERROR(
                            svr->m_em, "%s: create: load service %s: load command of prefix %s fail!",
                            conn_http_svr_name(svr), svr_name, cmd_prefix);
                        conn_http_service_free(service);
                        return -1;
                    }
                }
            }
            else {
                CPE_ERROR(
                    svr->m_em, "%s: create: load service %s: cmd-prefix format error!",
                    conn_http_svr_name(svr), svr_name);
                conn_http_service_free(service);
                return -1;
            }
        }

        if (conn_http_svr_app_init_load_service_cmds(svr, service, cfg_find_cfg(service_cfg, "cmds")) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create: load service %s: load cmds fail!",
                conn_http_svr_name(svr), svr_name);
                conn_http_service_free(service);
            return -1;
        }

        CPE_INFO(
            svr->m_em, "%s: create: load service %s: service-path=%s, dispatch-to=%s",
            conn_http_svr_name(svr), svr_name, service->m_path, set_svr_svr_info_svr_type_name(service->m_dispatch_to));
    }

    return 0;
}
