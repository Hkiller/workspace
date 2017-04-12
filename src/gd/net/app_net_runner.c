#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "app_net_internal_types.h"

static void app_net_runner_clear(nm_node_t node);

static cpe_hash_string_buf s_app_net_runner_default_name = CPE_HS_BUF_MAKE("app_net_runner");

struct nm_node_type s_nm_node_type_app_net_runner = {
    "app_net_runner",
    app_net_runner_clear
};

static int app_net_run_main(gd_app_context_t ctx, void * user_ctx) {
    struct app_net_runner * runner;

    runner = (struct app_net_runner *)user_ctx;

    return net_mgr_run(gd_app_net_mgr(ctx), runner->m_tick_span, (net_run_tick_fun_t)gd_app_tick, ctx);
}

static int app_net_run_stop(gd_app_context_t ctx, void * user_ctx) {
    net_mgr_stop(gd_app_net_mgr(ctx));
    return 0;
}

struct app_net_runner *
app_net_runner_create(gd_app_context_t app, const char * name) {
    struct app_net_runner * runner;
    nm_node_t runner_node;

    if (name == 0) name = cpe_hs_data((cpe_hash_string_t)&s_app_net_runner_default_name);

    runner_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct app_net_runner));
    if (runner_node == NULL) return NULL;

    runner = (struct app_net_runner *)nm_node_data(runner_node);
    runner->m_app = app;
    runner->m_tick_span = 10;

    gd_app_set_main(app, app_net_run_main, app_net_run_stop, runner);

    nm_node_set_type(runner_node, &s_nm_node_type_app_net_runner);
    return runner;
}

static void app_net_runner_clear(nm_node_t node) {
    struct app_net_runner * runner;
    runner = (struct app_net_runner *)nm_node_data(node);

    gd_app_set_main(runner->m_app, NULL, NULL, NULL);
}

void app_net_runner_free(struct app_net_runner * runner) {
    nm_node_t runner_node;
    assert(runner);

    runner_node = nm_node_from_data(runner);
    if (nm_node_type(runner_node) != &s_nm_node_type_app_net_runner) return;
    nm_node_free(runner_node);
}

struct app_net_runner *
app_net_runner_find(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    assert(name);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_app_net_runner) return NULL;
    return (struct app_net_runner *)nm_node_data(node);
}

EXPORT_DIRECTIVE
int app_net_runner_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    struct app_net_runner * runner;

    runner = app_net_runner_create(app, gd_app_module_name(module));
    if (runner == NULL) return -1;

    runner->m_tick_span = cfg_get_int64(cfg, "tick-span", 1000);
    if (runner->m_tick_span <= 0) {
        APP_CTX_ERROR(app, "%s: tick-span (%d) error!", gd_app_module_name(module), (int)runner->m_tick_span);
        app_net_runner_free(runner);
        return -1;
    }

    return 0;
}

EXPORT_DIRECTIVE
void app_net_runner_app_fini(gd_app_context_t app, gd_app_module_t module) {
    struct app_net_runner * runner;

    runner = app_net_runner_find(app, gd_app_module_name(module));
    if (runner) {
        app_net_runner_free(runner);
    }
}
