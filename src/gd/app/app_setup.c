#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stat.h"
#include "cpe/pal/pal_external.h"
#include "cpe/utils/memory_debug.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/file.h"
#include "cpe/net/net_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_tl.h"
#include "app_internal_ops.h"

static int app_setup_build_tickers(gd_app_context_t app, cfg_t cfg) {
    struct cfg_it cfgs;
    cfg_t ticker_cfg;

    cfg_it_init(&cfgs, cfg);

    while((ticker_cfg = cfg_it_next(&cfgs))) {
        const char * ticker_name = NULL;
        cfg_t ticker_args = NULL;

        if (cfg_type(ticker_cfg) == CPE_CFG_TYPE_STRING) {
            ticker_name = cfg_as_string(ticker_cfg, NULL);
        }
        else if (cfg_type(ticker_cfg) == CPE_CFG_TYPE_STRUCT) {
            struct cfg_it childs;

            if (cfg_child_count(ticker_cfg) != 1) {
                APP_CTX_ERROR(app, "app_setup: read ticker: struct cfg too many child(%d)", cfg_child_count(ticker_cfg));
                return -1;
            }

            cfg_it_init(&childs, ticker_cfg);
            ticker_cfg = cfg_it_next(&childs);
            assert(ticker_cfg);

            ticker_name = cfg_name(ticker_cfg);
            ticker_args = ticker_cfg;
        }
        else {
            APP_CTX_ERROR(app, "app_setup: read ticker: not support cfg type %d", cfg_type(ticker_cfg));
            return -1;
        }

        assert(ticker_name);
        (void)ticker_args;

        if (strcmp(ticker_name, "tl") == 0) {
            const char * tl_name = cfg_get_string(ticker_cfg, "name", NULL);
            int32_t process_count = cfg_get_int32(ticker_cfg, "process-count", 500);

            tl_manage_t tl_mgr = app_tl_manage_find(app, tl_name);
            if (tl_mgr == NULL) {
                APP_CTX_ERROR(app, "app_setup: read ticker: add tl ticker: %s not exist!", tl_name ? tl_name : "default");
                return -1;
            }

            if (gd_app_tick_add(app, (gd_app_tick_fun)tl_manage_tick, tl_mgr, process_count) != 0) {
                APP_CTX_ERROR(app, "app_setup: read ticker: add tl ticker for %s fail!", tl_name ? tl_name : "default");
                return -1;
            }
            else {
                if (gd_app_debug(app)) {
                    APP_CTX_INFO(app, "app_setup: read ticker: add tl ticker %s success, process-count=%d!", tl_name ? tl_name : "default", process_count);
                }
            }
        }
        else if (strcmp(ticker_name, "net") == 0) {
            if (gd_app_tick_add(
                    app,
                    (gd_app_tick_fun)net_mgr_tick,
                    app->m_net_mgr,
                    0) != 0)
            {
                APP_CTX_ERROR(app, "app_setup: read ticker: add net ticker fail!");
                return -1;
            }
            else {
                if (gd_app_debug(app)) {
                    APP_CTX_INFO(app, "app_setup: read ticker: add net ticker success!");
                }
            }
        }
        else {
            APP_CTX_ERROR(app, "app_setup: read ticker: unknown ticker type %s!", ticker_name);
            return -1;
        }
    }

    return 0;
}

static int app_setup_build_tls(gd_app_context_t app, cfg_t cfg) {
    struct cfg_it childs;
    cfg_t child;

    cfg_it_init(&childs, cfg);

    while((child = cfg_it_next(&childs))) {
        cfg_t arg_cfg;
        const char * tl_name;

        if (cfg_type(child) == CPE_CFG_TYPE_STRING) {
            tl_name = cfg_as_string(child, NULL);
            if (tl_name == NULL) {
                APP_CTX_ERROR(app, "app_setup: build tls: get tl name fail!");
                return -1;
            }

            arg_cfg = NULL;
        }
        else if (cfg_type(child) == CPE_CFG_TYPE_STRUCT) {
            arg_cfg = cfg_child_only(child);
            if (arg_cfg == NULL) {
                APP_CTX_ERROR(app, "app_setup: build tls: config format error!");
                return -1;
            }

            tl_name = cfg_name(arg_cfg);
        }
        else {
            APP_CTX_ERROR(app, "app_setup: build tls: not support cfg type %d", cfg_type(child));
            return -1;
        }

        if (app_tl_manage_create(app, tl_name, app->m_alloc) == NULL) {
            APP_CTX_ERROR(app, "app_setup: build tls: create tl %s fail!", tl_name);
            return -1;
        }
        else {
            if (gd_app_debug(app)) {
                APP_CTX_INFO(app, "app_setup: build tls: create tl %s success!", tl_name);
            }
        }
    }

    return 0;
}

struct app_setup_info {
    gd_app_context_t m_app;
    mem_allocrator_t m_debug_alloc;
};

static void app_setup_info_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_app_setup_info = {
    "app_setup_info",
    app_setup_info_clear
};

static
struct app_setup_info *
app_setup_info_create(gd_app_context_t app, const char * name) {
    struct app_setup_info * setup_info;
    nm_node_t setup_info_node;

    setup_info_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct app_setup_info));
    if (setup_info_node == NULL) return NULL;

    setup_info = (struct app_setup_info *)nm_node_data(setup_info_node);
    setup_info->m_app = app;
    setup_info->m_debug_alloc = NULL;

    nm_node_set_type(setup_info_node, &s_nm_node_type_app_setup_info);
    return setup_info;
}

static void app_setup_info_clear(nm_node_t node) {
    struct app_setup_info * setup_info;
    setup_info = (struct app_setup_info *)nm_node_data(node);

    if (setup_info->m_debug_alloc) {
        struct write_stream_file stream = CPE_WRITE_STREAM_FILE_INITIALIZER(stderr, NULL);

        mem_allocrator_debug_dump((write_stream_t)&stream, 4, NULL);
        mem_allocrator_debug_free(setup_info->m_debug_alloc);
        setup_info->m_debug_alloc = NULL;
    }
}

EXPORT_DIRECTIVE
int app_setup_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    struct app_setup_info * setup_info;
    const char * cfg_dup_path;

    gd_app_set_debug(app, cfg_get_int16(cfg, "debug-app", 0));
    net_mgr_set_debug(gd_app_net_mgr(app), cfg_get_int16(cfg, "debug-net", 0));

    if (app_setup_build_tls(app, cfg_find_cfg(cfg, "tl")) != 0) return -1;
    if (app_setup_build_tickers(app, cfg_find_cfg(cfg, "tickers")) != 0) return -1;

    setup_info = app_setup_info_create(app, gd_app_module_name(module));
    if (setup_info == NULL) return -1;

    if (cfg_get_int16(cfg, "debug-mem", 0)) {
        setup_info->m_debug_alloc = 
            mem_allocrator_debug_create(
                NULL,
                NULL,
                20,
                gd_app_em(app));
        gd_app_set_alloc(app, setup_info->m_debug_alloc);
    }

    if ((cfg_dup_path = cfg_get_string(cfg, "cfg-dump", NULL))) {
        FILE * f;
        struct mem_buffer buf;
        const char * dump_dir;
        mem_buffer_init(&buf, gd_app_alloc(app));

        if ((dump_dir = dir_name(cfg_dup_path, &buf))) {
            if (dir_mk_recursion(dump_dir, S_IRWXU, gd_app_em(app), gd_app_alloc(app)) != 0) {
                APP_CTX_ERROR(
                    app, "%s: create: cfg-dump: create dir %s fail!", 
                    gd_app_module_name(module), dump_dir);
            }
        }
        mem_buffer_clear(&buf);

        f = file_stream_open(cfg_dup_path, "w", gd_app_em(app));
        if (f == NULL) {
            APP_CTX_ERROR(
                app, "%s: create: cfg-dump: open file %s fail!", 
                gd_app_module_name(module), cfg_dup_path);
        }
        else {
            struct write_stream_file stream = CPE_WRITE_STREAM_FILE_INITIALIZER(f, gd_app_em(app));
            if (cfg_yaml_write((write_stream_t)&stream, gd_app_cfg(app), gd_app_em(app)) != 0) {
                APP_CTX_ERROR(
                    app, "%s: create: cfg-dump: dump cfg fail!", 
                    gd_app_module_name(module));
            }
            file_stream_close(f, gd_app_em(app));
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
void app_setup_app_fini(gd_app_context_t app, gd_app_module_t module) {
    nm_node_t setup_info_node;

    setup_info_node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), gd_app_module_name(module));
    if (setup_info_node) {
        nm_node_free(setup_info_node);
    }
}

