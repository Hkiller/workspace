#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_dlfcn.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_file.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/net/net_connector.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_use/bpg_use_sp.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "usf/bpg_net/bpg_net_client.h"

struct arg_file * a_input;
struct arg_str * a_ip;
struct arg_int * a_port;
struct arg_lit * a_help;
struct arg_int * a_app_debug;
struct arg_file * a_metalib;
struct arg_str * a_cmd_meta;
struct arg_str * a_base_cvt;
struct arg_str * a_data_cvt;
struct arg_end * a_end;

int debug_level(void) { return a_app_debug->count > 0 ? a_app_debug->ival[0] : 0; }

int recv_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    gd_app_context_t app;

    app = (gd_app_context_t)ctx;
    gd_app_notify_stop(app);

    return 0;
}

int create_recv_rsp(gd_app_context_t app) {
    dp_rsp_t rsp = dp_rsp_create(gd_app_dp_mgr(app), "recv-rsp");
    if (rsp == NULL) {
        CPE_ERROR(gd_app_em(app), "create recv rsp fail!");
        return -1;
    }

    if (dp_rsp_bind_string(rsp, "recv-from-server", NULL) != 0) {
        CPE_ERROR(gd_app_em(app), "bind recv rsp fail!");
        dp_rsp_free(rsp);
        return -1;
    }

    dp_rsp_set_processor(rsp, recv_rsp, app);
    return 0;
}

cfg_t read_pkg_cfg(gd_app_context_t app) {
    struct read_stream_file stream;
    FILE * fp;
    const char * file_name;
    cfg_t cfg;
    int r;

    file_name = a_input->count > 0 ? a_input->filename[0] : "???";
    fp = file_stream_open(file_name, "r", NULL);
    if (fp == NULL) {
        CPE_ERROR(gd_app_em(app), "open input file %s fail!", file_name);
        return NULL;
    }

    read_stream_file_init(&stream, fp, NULL);

    cfg = cfg_create(NULL);
    if (cfg == NULL) {
        CPE_ERROR(gd_app_em(app), "read input, create cfg fail!");
        file_stream_close(fp, NULL);
        return NULL;
    }

    CPE_ERROR_SET_FILE(gd_app_em(app), file_name);
    r = cfg_read(cfg, (read_stream_t)&stream, cfg_replace, gd_app_em(app));
    CPE_ERROR_SET_FILE(gd_app_em(app), NULL);
    if (r != 0) {
        CPE_ERROR(gd_app_em(app), "read input, read cfg fail!");
        cfg_free(cfg);
        file_stream_close(fp, NULL);
        return NULL;
    }

    file_stream_close(fp, NULL);
    return cfg;
}

int send_request(gd_app_context_t app) {
    bpg_use_sp_t sp;
    dp_req_t pkg;
    cfg_t pkg_cfg;
    bpg_pkg_manage_t pkg_manage;

    pkg_manage = bpg_pkg_manage_find_nc(app, "bpg_pkg_manage");
    if (pkg_manage == NULL) {
        CPE_ERROR(gd_app_em(app), "find bpg_pkg_manage fail!");
    }

    sp = bpg_use_sp_create(app, "client", pkg_manage, gd_app_alloc(app), gd_app_em(app));
    if (sp == NULL) {
        CPE_ERROR(gd_app_em(app), "create sp fail!");
        return -1;
    }

    pkg = bpg_use_sp_pkg_buf(sp, 2048);
    assert(pkg);

    pkg_cfg = read_pkg_cfg(app);
    if (pkg_cfg == NULL) {
        bpg_use_sp_free(sp);
        return -1;
    }

    if (bpg_pkg_build_from_cfg(pkg, pkg_cfg, gd_app_em(app)) != 0) {
        CPE_ERROR(gd_app_em(app), "build pkg fail!");
        cfg_free(pkg_cfg);
        bpg_use_sp_free(sp);
        return -1;
    }
    cfg_free(pkg_cfg);

    if (bpg_use_sp_send(sp, pkg) != 0) {
        CPE_ERROR(gd_app_em(app), "send pkg fail!");
        bpg_use_sp_free(sp);
        return -1;
    }

    bpg_use_sp_free(sp);
    return 0;
}

void net_connector_state_monitor(net_connector_t connector, void * ctx) {
    gd_app_context_t app;

    app = (gd_app_context_t)ctx;
    
    switch(net_connector_state(connector)) {
    case net_connector_state_connected:
        if (send_request(app) != 0) {
            net_connector_disable(connector);
        }
        else {
            if (create_recv_rsp(app) != 0) {
                net_connector_disable(connector);
            }
        }
        break;
    case net_connector_state_connecting:
        break;
    default:
        gd_app_notify_stop(app);
    }
}

int prepaire_dr_load_cfg(gd_app_context_t app, cfg_t cfg, struct gd_app_module_def * module_def, void * ctx) {
    if (a_metalib->count == 1 && strcmp(a_metalib->extension[0], ".bin") == 0) {
        cfg_struct_add_string(cfg, "load-from-bin", a_metalib->filename[0], cfg_replace);
    }
    else {
        cfg_t seq = cfg_struct_add_seq(cfg, "load-from-file", cfg_replace);
        int i = 0;
        for(; i < a_metalib->count; ++i) {
            cfg_seq_add_string(seq, a_metalib->filename[i]);
        }
    }

    cfg_struct_add_int32(cfg, "debug", debug_level(), cfg_replace);

    return 0;
}

int prepaire_bpg_net_client_cfg(gd_app_context_t app, cfg_t cfg, struct gd_app_module_def * module_def, void * ctx) {
    cfg_struct_add_string(cfg, "ip", a_ip->count > 0 ? a_ip->sval[0] : "", cfg_replace);
    cfg_struct_add_int32(cfg, "port", a_port->count > 0 ? a_port->ival[0] : 0, cfg_replace);
    cfg_struct_add_int32(cfg, "debug", debug_level(), cfg_replace);

    cfg_struct_add_string(cfg, "req-recv-at", "send-to-server", cfg_replace);
    cfg_struct_add_string(cfg, "rsp-send-to", "recv-from-server", cfg_replace);

    return 0;
}

int prepaire_bpg_pkg_manage_cfg(gd_app_context_t app, cfg_t cfg, struct gd_app_module_def * module_def, void * ctx) {
    const char * data_cvt;
    cfg_t meta_cfg;

    data_cvt = a_data_cvt->count > 0 ? a_data_cvt->sval[0] : "copy";
    cfg_struct_add_string(cfg, "base-cvt", a_base_cvt->count > 0 ? a_base_cvt->sval[0] : data_cvt, cfg_replace);
    cfg_struct_add_string(cfg, "data-cvt", data_cvt, cfg_replace);

    meta_cfg = cfg_struct_add_struct(cfg, "meta", cfg_replace);
    cfg_struct_add_string(meta_cfg, "lib-name", "metalib-data", cfg_replace);
    cfg_struct_add_string(meta_cfg, "cmd-meta-name", a_cmd_meta->count > 0 ? a_cmd_meta->sval[0] : "not-input", cfg_replace);

    return 0;
}

static struct gd_app_module_def g_init_modules[] = {
    { "app_net_runner", NULL, NULL, NULL, NULL}
    , { "dr_store_manage", NULL, NULL, NULL, NULL }
    , { "basepkg", "bpg_metalib", NULL, NULL, NULL }
    , { "metalib-data", "dr_store_loader", NULL, NULL, prepaire_dr_load_cfg }
    , { "logic_manage", NULL, NULL, NULL, NULL }
    , { "bpg_pkg_manage", NULL, NULL, NULL, prepaire_bpg_pkg_manage_cfg }
    , { "bpg_net_client", NULL, NULL, NULL, prepaire_bpg_net_client_cfg }
};

int tools_main(gd_app_context_t app) {
    net_connector_t connector;

    gd_set_default_library(dlopen(NULL, RTLD_NOW));
    gd_app_flag_enable(app, gd_app_flag_no_auto_load);

    if (gd_app_bulk_install_module(app, g_init_modules, sizeof(g_init_modules) / sizeof(g_init_modules[0]), NULL) != 0) {
        return -1;
    }

    connector = bpg_net_client_connector(
        bpg_net_client_find_nc(app, "bpg_net_client"));
    assert(connector);

    net_connector_add_monitor(connector, net_connector_state_monitor, app);

    net_connector_enable(connector);

    return gd_app_run(app);
}

int main(int argc, char * argv[]) {
    int rv = 0;
    gd_app_context_t app = NULL;
    void* argtable[] = {
                 a_ip = arg_str1(   NULL,      "ip",              "<string>",     "target ip")
         ,    a_port = arg_int1(   "p",   "port",              "<string>",     "input file")
         ,    a_input = arg_file1(   "i",   "input",              "<string>",   "input file")
         ,    a_metalib = arg_filen(   NULL,   "meta-lib",              "<string>",   1, 100,  "meta lib name")
         , a_cmd_meta = arg_str1(   NULL,      "cmd-meta",              "<string>",     "cmd meta name")
         , a_base_cvt = arg_str0(   NULL,      "base-cvt",              "<string>",     "pkg base cvt name")
         , a_data_cvt = arg_str1(   NULL,      "data-cvt",              "<string>",     "pkg base cvt name")
         ,    a_help = arg_lit0(   NULL,  "help",                                   "print this help and exit")
                 , a_app_debug = arg_int0(   NULL,  "app-debug",         "<debug-level>", "debug app framework")
         ,    a_end = arg_end(20)
    };
    int nerrors;

    if (arg_nullcheck(argtable) != 0) {
        printf("init arg table fail!");
        goto exit;
    }

    nerrors = arg_parse(argc, argv, argtable);

    if (a_help->count > 0) {
        printf("Usage: %s", argv[0]);
        arg_print_syntax(stdout,argtable,"\n");
        rv = 0;
        goto exit;
    }

    if (nerrors > 0) {
        arg_print_errors(stdout, a_end, argv[0]);
        printf("Try '%s --help' for more information.\n", argv[0]);
        goto exit;
    }

    app = gd_app_context_create_main(NULL, 0, argc, argv);
    if (app == NULL) {
        printf("create app context fail!");
        return -1;
    }

    if (a_app_debug->count > 0) {
        gd_app_set_debug(app, debug_level());
    }

    rv = tools_main(app);

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (app) { gd_app_context_free(app); app = NULL; }

    return rv;
}
