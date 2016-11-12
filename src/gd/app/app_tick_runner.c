#include <assert.h>
#ifndef _MSC_VER
#include <sched.h>
#endif
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_socket.h" /*for select (sleep)*/
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/time_utils.h"
#include "cpe/net/net_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_log.h"
#include "app_internal_ops.h"

static void app_tick_runner_clear(nm_node_t node);

static cpe_hash_string_buf s_app_tick_runner_default_name = CPE_HS_BUF_MAKE("app_tick_runner");

struct nm_node_type s_nm_node_type_app_tick_runner = {
    "app_tick_runner",
    app_tick_runner_clear
};

static int app_tick_run_main(gd_app_context_t ctx, void * user_ctx) {
    struct app_tick_runner * runner;
    int r;
    int64_t last_update_time;

    runner = (struct app_tick_runner *)user_ctx;

    runner->m_is_runing = 1;
    last_update_time = cur_time_ms();
    while(runner->m_is_runing) {
        int64_t cur_time = cur_time_ms();
        float delta_s = cur_time > last_update_time ? ((float)(cur_time - last_update_time)) / 1000.0f : 0.0f;

        r = gd_app_tick(ctx, delta_s);
        if (r > 0) continue;

        if (runner->m_tick_span > 0) {
            fd_set rfds;
            struct timeval tv;
            int fd = 1;

            FD_ZERO (&rfds);
            FD_SET(fd, &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = runner->m_tick_span * 1000;
            if (select(0, NULL, NULL, NULL, &tv) == -1) {
                APP_CTX_INFO(
                    ctx, "%s: sleep %d ms, select error, error=%d (%s)!",
                    "app_tick_runner", (int)runner->m_tick_span, errno, strerror(errno));
            }
        }
        else {
#ifndef _MSC_VER
            sched_yield();
#endif
        }
    };

    return -1;
}

static int app_tick_run_stop(gd_app_context_t ctx, void * user_ctx) {
    struct app_tick_runner * runner;

    runner = (struct app_tick_runner *)user_ctx;

    runner->m_is_runing = 0;

    net_mgr_stop(gd_app_net_mgr(ctx));

    APP_CTX_INFO(ctx, "app_tick_runner: stop!");

    return 0;
}

struct app_tick_runner *
app_tick_runner_create(gd_app_context_t app, const char * name) {
    struct app_tick_runner * runner;
    nm_node_t runner_node;

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_app_tick_runner_default_name);

    runner_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct app_tick_runner));
    if (runner_node == NULL) return NULL;

    runner = (struct app_tick_runner *)nm_node_data(runner_node);
    runner->m_app = app;
    runner->m_tick_span = 10;
    runner->m_is_runing = 0;

    gd_app_set_main(app, app_tick_run_main, app_tick_run_stop, runner);

    nm_node_set_type(runner_node, &s_nm_node_type_app_tick_runner);
    return runner;
}

static void app_tick_runner_clear(nm_node_t node) {
    struct app_tick_runner * runner;
    runner = (struct app_tick_runner *)nm_node_data(node);

    gd_app_set_main(runner->m_app, NULL, NULL, NULL);
}

void app_tick_runner_free(struct app_tick_runner * runner) {
    nm_node_t runner_node;
    assert(runner);

    runner_node = nm_node_from_data(runner);
    if (nm_node_type(runner_node) != &s_nm_node_type_app_tick_runner) return;
    nm_node_free(runner_node);
}

struct app_tick_runner *
app_tick_runner_find(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    assert(name);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_tick_runner) return NULL;
    return (struct app_tick_runner *)nm_node_data(node);
}

EXPORT_DIRECTIVE
int app_tick_runner_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    struct app_tick_runner * runner;

    runner = app_tick_runner_create(app, gd_app_module_name(module));
    if (runner == NULL) return -1;

    runner->m_tick_span = cfg_get_int64(cfg, "tick-span", runner->m_tick_span);
    if (runner->m_tick_span <= 0) {
        APP_CTX_ERROR(app, "%s: tick-span (%d) error!", gd_app_module_name(module), (int)runner->m_tick_span);
        app_tick_runner_free(runner);
        return -1;
    }

    return 0;
}

EXPORT_DIRECTIVE
void app_tick_runner_app_fini(gd_app_context_t app, gd_app_module_t module) {
    struct app_tick_runner * runner;

    runner = app_tick_runner_find(app, gd_app_module_name(module));
    if (runner) {
        app_tick_runner_free(runner);
    }
}


