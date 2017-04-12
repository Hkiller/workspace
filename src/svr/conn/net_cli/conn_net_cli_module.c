#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "svr/conn/net_cli/conn_net_cli_svr_stub.h"
#include "conn_net_cli_internal_ops.h"

static int conn_net_cli_load_svr_stub_error_info(
    conn_net_cli_t cli, conn_net_cli_svr_stub_t svr_info, const char * svr_type_name, const char * str_error_pkg);

static int conn_net_cli_app_load_svr_stubs(conn_net_cli_t conn_net_cli, cfg_t cfg) {
    struct cfg_it childs;
    cfg_t child_cfg;
    dr_store_manage_t store_mgr;

    store_mgr = dr_store_manage_find(conn_net_cli->m_app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(conn_net_cli->m_em, "%s: load svr stubs: default store_mgr not exist!", conn_net_cli_name(conn_net_cli));
        return -1;
    }

    cfg_it_init(&childs, cfg);

    while((child_cfg = cfg_it_next(&childs))) {
        conn_net_cli_svr_stub_t svr_stub;
        const char * svr_type_name;
        uint16_t svr_type_id;
        const char * response_send_to;
        const char * notify_send_to;
        const char * outgoing_recv_at;
        const char * str_pkg_meta;
        const char * svr_pkg_data_entry;
        const char * svr_pkg_error_info;
        dr_store_t store;
        char const * sep;
        char lib_name[64];

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(conn_net_cli->m_em, "%s: create: load svrs: config format error!", conn_net_cli_name(conn_net_cli));
            return -1;
        }

        svr_type_name = cfg_name(child_cfg);
        if (svr_type_name == NULL) {
            CPE_ERROR(conn_net_cli->m_em, "%s: create: load svrs: config format error(no name)!", conn_net_cli_name(conn_net_cli));
            return -1;
        }

        if (cfg_try_get_uint16(child_cfg, "id", &svr_type_id) != 0) {
            CPE_ERROR(
                conn_net_cli->m_em, "%s: create: load svrs: svr %s: id not configured!",
                conn_net_cli_name(conn_net_cli), svr_type_name);
            return -1;
        }

        str_pkg_meta = cfg_get_string(child_cfg, "pkg-meta", NULL);
        if (str_pkg_meta == NULL) {
            CPE_ERROR(
                conn_net_cli->m_em, "%s: create: load svrs: svr %s: pkg-meta not configured!",
                conn_net_cli_name(conn_net_cli), svr_type_name);
            return -1;
        }

        svr_stub = conn_net_cli_svr_stub_create(conn_net_cli, svr_type_name, svr_type_id);
        if (svr_stub == NULL) {
            CPE_ERROR(
                conn_net_cli->m_em, "%s: create: load svrs: svr %s: create svr_stub fail!",
                conn_net_cli_name(conn_net_cli), svr_type_name);
            return -1;
        }

        sep = strchr(str_pkg_meta, '.');
        if (sep == NULL || (sep - str_pkg_meta) > (sizeof(lib_name) - 1)) {
            CPE_ERROR(
                conn_net_cli->m_em, "%s: create svr type %s: pkg-meta %s format error or overflow!",
                conn_net_cli_name(conn_net_cli), svr_type_name, str_pkg_meta);
            return -1;
        }
        memcpy(lib_name, str_pkg_meta, sep - str_pkg_meta);
        lib_name[sep - str_pkg_meta] = 0;

        store = dr_store_find(store_mgr, lib_name);
        if (store == NULL) {
            CPE_ERROR(
                conn_net_cli->m_em, "%s: create svr type %s: metalib %s not exist in %s!",
                conn_net_cli_name(conn_net_cli), svr_type_name, lib_name, dr_store_manage_name(store_mgr));
            return -1;
        }

        svr_stub->m_pkg_meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
        if (svr_stub->m_pkg_meta == NULL) {
            CPE_ERROR(
                conn_net_cli->m_em, "%s: create svr type %s: metalib %s have no meta %s!",
                conn_net_cli_name(conn_net_cli), svr_type_name, svr_type_name, sep + 1);
            return -1;
        }

        svr_pkg_data_entry = cfg_get_string(child_cfg, "pkg-meta-data", NULL);
        if (svr_pkg_data_entry) {
            LPDRMETAENTRY pkg_data_entry;
            LPDRMETA pkg_data_meta;
            LPDRMETAENTRY pkg_cmd_entry;
            int i;

            pkg_data_entry = dr_meta_find_entry_by_name(svr_stub->m_pkg_meta, svr_pkg_data_entry);
            if (pkg_data_entry == NULL) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: %s: pkg-meta %s no data entry %s!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, str_pkg_meta, svr_pkg_data_entry);
                return -1;
            }

            pkg_data_meta = dr_entry_ref_meta(pkg_data_entry);
            if (pkg_data_meta == NULL || dr_meta_type(pkg_data_meta) != CPE_DR_TYPE_UNION) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: %s: data_entry %s.%s not union!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, str_pkg_meta, svr_pkg_data_entry);
                return -1;
            }

            pkg_cmd_entry = dr_entry_select_entry(pkg_data_entry);
            if (pkg_cmd_entry == NULL) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: %s: pkg-meta %s data entry %s no select entry!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, str_pkg_meta, svr_pkg_data_entry);
                return -1;
            }

            svr_stub->m_pkg_cmd_entry = pkg_cmd_entry;
            svr_stub->m_pkg_data_entry = pkg_data_entry;

            for(i = 0; i < dr_meta_entry_num(pkg_data_meta); ++i) {
                LPDRMETAENTRY cmd_entry = dr_meta_entry_at(pkg_data_meta, i);
                if (conn_net_cli_cmd_info_create(svr_stub, cmd_entry) == NULL) {
                    CPE_ERROR(
                        conn_net_cli->m_em, "%s: %s: add cmd %s fail!",
                        conn_net_cli_name(conn_net_cli), svr_type_name, dr_entry_name(cmd_entry));
                    return -1;
                }
            }
        }

        if ((svr_pkg_error_info = cfg_get_string(child_cfg, "pkg-meta-error", NULL))) {
            if (conn_net_cli_load_svr_stub_error_info(conn_net_cli, svr_stub, svr_type_name, svr_pkg_error_info) != 0) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: create: load svrs: svr %s: load pkg-meta-error %s fail!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, svr_pkg_error_info);
                return -1;
            }
        }

        if ((response_send_to = cfg_get_string(child_cfg, "response-send-to", NULL))) {
            if (conn_net_cli_svr_stub_set_response_dispatch_to(svr_stub, response_send_to) != 0) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: create: load svrs: svr %s: set response send to %s fail!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, response_send_to);
                return -1;
            }
        }

        if ((notify_send_to = cfg_get_string(child_cfg, "notify-send-to", NULL))) {
            if (conn_net_cli_svr_stub_set_notify_dispatch_to(svr_stub, notify_send_to) != 0) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: create: load svrs: svr %s: set notify send to %s fail!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, notify_send_to);
                return -1;
            }
        }

        if ((outgoing_recv_at = cfg_get_string(child_cfg, "outgoing-recv-at", NULL))) {
            if (conn_net_cli_svr_stub_set_outgoing_recv_at(svr_stub, outgoing_recv_at) != 0) {
                CPE_ERROR(
                    conn_net_cli->m_em, "%s: create: load svrs: svr %s: set outgoing_recv_at %s fail!",
                    conn_net_cli_name(conn_net_cli), svr_type_name, outgoing_recv_at);
                return -1;
            }
        }

    }
    
    return 0;
}

EXPORT_DIRECTIVE
int conn_net_cli_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    conn_net_cli_t conn_net_cli;
    const char * ip;
    short port;
    const char * str_ringbuf_size;
    const char * str_read_block_size;
    uint64_t ringbuf_size;
    uint64_t read_block_size;

    ip = cfg_get_string(cfg, "ip", NULL);
    port = cfg_get_int16(cfg, "port", 0);

    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }
    
    conn_net_cli =
        conn_net_cli_create(
            app, gd_app_module_name(module),
            gd_app_alloc(app), gd_app_em(app));
    if (conn_net_cli == NULL) return -1;

    conn_net_cli->m_debug = cfg_get_int8(cfg, "debug", conn_net_cli->m_debug);
    conn_net_cli->m_reconnect_span_ms = cfg_get_uint32(cfg, "reconnect-span-ms", conn_net_cli->m_reconnect_span_ms);

    if ((str_read_block_size = cfg_get_string(cfg, "read-block-size", NULL))) {
        if (cpe_str_parse_byte_size(&read_block_size, str_read_block_size) != 0) {
            CPE_ERROR(
                gd_app_em(app), "%s: create: read read-block-size %s fail!",
                gd_app_module_name(module), str_read_block_size);
            conn_net_cli_free(conn_net_cli);
            return -1;
        }

        conn_net_cli->m_read_block_size = (uint32_t)read_block_size;
    }

    if (ip && port > 0) {
        if (conn_net_cli_set_svr(conn_net_cli, ip, port) != 0) {
            CPE_ERROR(gd_app_em(app), "%s: create: set svr %s:%d fail fail!", gd_app_module_name(module), ip, port);
            conn_net_cli_free(conn_net_cli);
            return -1;
        }
    }

    if (conn_net_cli_set_ringbuf_size(conn_net_cli, (size_t)ringbuf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ringbuf-size %d fail!", gd_app_module_name(module), (int)ringbuf_size);
        conn_net_cli_free(conn_net_cli);
        return -1;
    }

    if (conn_net_cli_app_load_svr_stubs(conn_net_cli, cfg_find_cfg(cfg, "svrs")) != 0) {
        conn_net_cli_free(conn_net_cli);
        return -1;
    }

    conn_net_cli->m_auto_reconnect = cfg_get_uint8(cfg, "auto-reconnect", conn_net_cli->m_auto_reconnect);
    
    if (conn_net_cli->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done. svr at %s:%d", gd_app_module_name(module), ip, port);
    }

    if (cfg_get_int8(cfg, "auto-enable", 1)) {
        if (ip == NULL) {
            CPE_ERROR(gd_app_em(app), "%s: create: ip not configured!", gd_app_module_name(module));
            conn_net_cli_free(conn_net_cli);
            return -1;
        }

        if (port == 0) {
            CPE_ERROR(gd_app_em(app), "%s: create: port not configured!", gd_app_module_name(module));
            conn_net_cli_free(conn_net_cli);
            return -1;
        }

        conn_net_cli_enable(conn_net_cli);
    }

    return 0;
}

EXPORT_DIRECTIVE
void conn_net_cli_app_fini(gd_app_context_t app, gd_app_module_t module) {
    conn_net_cli_t conn_net_cli;

    conn_net_cli = conn_net_cli_find_nc(app, gd_app_module_name(module));
    if (conn_net_cli) {
        conn_net_cli_free(conn_net_cli);
    }
}

static int conn_net_cli_load_svr_stub_error_info(
    conn_net_cli_t cli, conn_net_cli_svr_stub_t svr_info, const char * svr_type_name, const char * str_error_pkg)
{
    LPDRMETA data_meta;
    char err_pkg_meta[64];
    char * err_entry_name;
    int i;

    if (svr_info->m_pkg_data_entry == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: %s: pkg-meta-error configured, but no data entry!!",
            conn_net_cli_name(cli), svr_type_name);
        return -1;
    }

    data_meta = dr_entry_ref_meta(svr_info->m_pkg_data_entry);
    if (data_meta == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: %s: pkg-meta-error configured, but no data entry!!",
            conn_net_cli_name(cli), svr_type_name);
        return -1;
    }

    err_entry_name = strchr(str_error_pkg, '.');
    if (err_entry_name == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: %s: pkg-meta-error %s format error or overflow!",
            conn_net_cli_name(cli), svr_type_name, str_error_pkg);
        return -1;
    }
    memcpy(err_pkg_meta, str_error_pkg, err_entry_name - str_error_pkg);
    err_pkg_meta[err_entry_name - str_error_pkg] = 0;
    err_entry_name += 1;

    for(i = 0; i < dr_meta_entry_num(data_meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(data_meta, i);
        LPDRMETA entry_meta = dr_entry_ref_meta(entry);
        if (entry_meta && strcmp(dr_meta_name(entry_meta), err_pkg_meta) == 0) {
            svr_info->m_error_pkg_meta = entry_meta;
            svr_info->m_error_pkg_cmd = dr_entry_id(entry);
            break;
        }
    }

    if (svr_info->m_error_pkg_meta == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: %s: pkg-meta-error %s not entry of %s",
            conn_net_cli_name(cli), svr_type_name, err_pkg_meta, dr_meta_name(data_meta));
        return -1;
    }

    svr_info->m_error_pkg_error_entry = dr_meta_find_entry_by_name(svr_info->m_error_pkg_meta, err_entry_name);
    if (svr_info->m_error_pkg_error_entry == NULL) {
        CPE_ERROR(
            cli->m_em, "%s: %s: pkg-meta-error %s no error entry %s",
            conn_net_cli_name(cli), svr_type_name, err_pkg_meta, err_entry_name);
        return -1;
    }

    return 0; 
}

