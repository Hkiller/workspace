#include <assert.h>
#include <string.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "gd/app/app.h"
#include "gd/app/app_context.h"
#include "gd/app/app_rsp.h"
#include "app_internal_ops.h"

struct gd_app_rsp_create_ctx {
    gd_app_context_t m_context;
    gd_app_module_t m_module;
    dp_mgr_t m_dpm;
    struct gd_app_rsp_mgr * m_rspMgr;
    struct mem_buffer m_buffer;
};

struct gd_app_rsp_proxy {
    SIMPLEQ_ENTRY(gd_app_rsp_proxy) m_next;
};

struct gd_app_rsp_mgr {
    gd_app_context_t m_context;
    SIMPLEQ_HEAD(gd_app_rsp_proxy_list, gd_app_rsp_proxy) m_rsps;
    struct mem_buffer m_buffer;
};

static int gd_app_rsp_mgr_add_rsp(struct gd_app_rsp_mgr * rspMgr, const char * rspName) {
    size_t rspNameLen = strlen(rspName);
    struct gd_app_rsp_proxy * proxy;

    proxy = (struct gd_app_rsp_proxy * )
        mem_buffer_alloc(&rspMgr->m_buffer, sizeof(struct gd_app_rsp_proxy) + rspNameLen + 1);
    if (proxy == NULL) return -1;

    memcpy(proxy + 1, rspName, rspNameLen + 1);
    SIMPLEQ_INSERT_TAIL(&rspMgr->m_rsps, proxy, m_next);
    return 0;
}

static void gd_app_rsp_mgr_init(nm_node_t node, gd_app_context_t context) {
    struct gd_app_rsp_mgr * rspMgr =
        (struct gd_app_rsp_mgr *)nm_node_data(node);

    rspMgr->m_context = context;
    SIMPLEQ_INIT(&rspMgr->m_rsps);
    mem_buffer_init(&rspMgr->m_buffer, gd_app_alloc(context));
}

static void gd_app_rsp_mgr_free(nm_node_t node) {
    struct gd_app_rsp_proxy * rspProxy;
    dp_mgr_t dpm;

    struct gd_app_rsp_mgr * rspMgr =
        (struct gd_app_rsp_mgr *)nm_node_data(node);

    dpm = gd_app_dp_mgr(rspMgr->m_context);

    SIMPLEQ_FOREACH(rspProxy, &rspMgr->m_rsps, m_next) {
        dp_rsp_t rsp =
            dp_rsp_find_by_name(dpm, (char *)(rspProxy + 1));
        if (rsp) {
            dp_rsp_free(rsp);
        }
    }

    mem_buffer_clear(&rspMgr->m_buffer);
}

static struct nm_node_type g_module_rsp_mgr = {
    "app_module_rsp_mgr",
    gd_app_rsp_mgr_free
};

static
int gd_app_rsp_bind(
    struct gd_app_rsp_create_ctx * ctx,
    dp_rsp_t rsp,
    cfg_t responsToCfg)
{
    int rv = 0;

    assert(responsToCfg);

    switch(cfg_type(responsToCfg)) {
    case CPE_CFG_TYPE_SEQUENCE: {
        struct cfg_it cfgIt;
        cfg_t subResponsToCfg;
        cfg_it_init(&cfgIt, responsToCfg);
        while((subResponsToCfg = cfg_it_next(&cfgIt))) {
            if (gd_app_rsp_bind(ctx, rsp, subResponsToCfg) != 0) {
                rv = -1;
            }
        }
        break;
    }
    case CPE_CFG_TYPE_STRING: {
        const char * cmd = cfg_as_string(responsToCfg, NULL);
        if (cmd == NULL) {
            APP_CTX_ERROR(
                ctx->m_context, "%s reading rsp: not support bind to str cmd NULL!",
                gd_app_module_name(ctx->m_module));
            return -1;
        }

        if (dp_rsp_bind_string(rsp, cmd, gd_app_em(ctx->m_context)) != 0) {
            APP_CTX_ERROR(
                ctx->m_context, "%s reading rsp: bind to str cmd %s fail!",
                gd_app_module_name(ctx->m_module), cmd);
            return -1;
        }

        break;
    }
    case CPE_CFG_TYPE_INT8:
    case CPE_CFG_TYPE_UINT8:
    case CPE_CFG_TYPE_INT16:
    case CPE_CFG_TYPE_UINT16:
    case CPE_CFG_TYPE_INT32:
    case CPE_CFG_TYPE_UINT32:
    case CPE_CFG_TYPE_INT64:
    case CPE_CFG_TYPE_UINT64:
    {
        int32_t cmd = cfg_as_int32(responsToCfg, -1);
        if (cmd == -1) {
            APP_CTX_ERROR(
                ctx->m_context, "%s reading rsp: not support bind to numeric cmd %d!",
                gd_app_module_name(ctx->m_module), cmd);
            return -1;
        }

        if (dp_rsp_bind_numeric(rsp, cmd, gd_app_em(ctx->m_context)) != 0) {
            APP_CTX_ERROR(
                ctx->m_context, "%s reading rsp: bind to numeric cmd %d fail!",
                gd_app_module_name(ctx->m_module), cmd);
            return -1;
        }

        break;
    }
    default:
        APP_CTX_ERROR(
            ctx->m_context, "%s reading rsp: not support bind to type %d!",
            gd_app_module_name(ctx->m_module), cfg_type(responsToCfg));
        return -1;
    }

    return rv;
}

static
int gd_app_rsp_init(
    struct gd_app_rsp_create_ctx * ctx,
    dp_rsp_t rsp,
    cfg_t cfg)
{
    gd_app_rsp_init_fun_t init;
    dp_rsp_process_fun_t processor;
    const char * processorSymName;
    const char * initSymName;

    processorSymName = cfg_get_string(cfg, "processor", NULL);
    initSymName = cfg_get_string(cfg, "init", NULL);

    if (processorSymName && initSymName) {
        APP_CTX_ERROR(
            ctx->m_context,
            "%s reading rsp %s: can`t config processor and init, please select one!",
            gd_app_module_name(ctx->m_module), dp_rsp_name(rsp));
        return -1;
    }

    if (initSymName) {
        int rv;

        init = (gd_app_rsp_init_fun_t)gd_app_lib_sym(gd_app_module_lib(ctx->m_module), initSymName, NULL);
        if (init == NULL) {
            APP_CTX_ERROR(
                ctx->m_context,
                "%s reading rsp %s: can`t find init function %s!",
                gd_app_module_name(ctx->m_module), dp_rsp_name(rsp), initSymName);
            return -1;
        }

        rv = init(rsp, ctx->m_context, ctx->m_module, cfg);
        if (rv == 0) {
            if(dp_rsp_processor(rsp) == NULL) {
                rv = -1;
            }
        }
        return rv;
    }

    if (processorSymName) {
        processor = (dp_rsp_process_fun_t)gd_app_lib_sym(gd_app_module_lib(ctx->m_module), processorSymName, NULL);
        if (processor == NULL) {
            APP_CTX_ERROR(
                ctx->m_context,
                "%s reading rsp %s: can`t find processor function %s!",
                gd_app_module_name(ctx->m_module), dp_rsp_name(rsp), processorSymName);
            return -1;
        }

        dp_rsp_set_processor(rsp, processor, ctx->m_context);
        return 0;
    }

    assert(processorSymName == NULL && initSymName == NULL);

    mem_buffer_set_size(&ctx->m_buffer, 0);
    mem_buffer_strcat(&ctx->m_buffer, "rsp_");
    mem_buffer_strcat(&ctx->m_buffer, dp_rsp_name(rsp));
    mem_buffer_strcat(&ctx->m_buffer, "_init");

    init = (gd_app_rsp_init_fun_t)gd_app_lib_sym(
        gd_app_module_lib(ctx->m_module),
        (char*)mem_buffer_make_continuous(&ctx->m_buffer, 0),
        NULL);
    if (init) {
        int rv = init(rsp, ctx->m_context, ctx->m_module, cfg);
        if (rv == 0) {
            if(dp_rsp_processor(rsp) == 0) {
                rv = -1;
            }
        }
        return rv;
    }

    /*remove _init post fix*/
    mem_buffer_set_size(&ctx->m_buffer, mem_buffer_size(&ctx->m_buffer) - 6);
    mem_buffer_append_char(&ctx->m_buffer, 0);
    processorSymName = (char*)mem_buffer_make_continuous(&ctx->m_buffer, 0);

    processor = (dp_rsp_process_fun_t)
        gd_app_lib_sym(gd_app_module_lib(ctx->m_module), processorSymName, NULL);
    if (processor) {
        dp_rsp_set_processor(rsp, processor, ctx->m_context);
        return 0;
    }

    APP_CTX_ERROR(
        ctx->m_context,
        "%s reading rsp %s: no default init or processor function found!",
        gd_app_module_name(ctx->m_module), dp_rsp_name(rsp));

    return -1;
}

static int gd_app_rsp_create(struct gd_app_rsp_create_ctx * ctx, cfg_t cfgNode) {
    const char * rspName;
    dp_rsp_t rsp;
    cfg_t responsToCfg;
 
    rspName = cfg_get_string(cfgNode, "name", NULL);
    if (rspName == NULL) {
        APP_CTX_ERROR(
            ctx->m_context,
            "%s reading rsp: no module node!", gd_app_module_name(ctx->m_module));
        return -1;
    }

    rsp = dp_rsp_create(ctx->m_dpm, rspName);
    if (rsp == NULL) {
        APP_CTX_ERROR(
            ctx->m_context,
            "%s reading rsp %s: create rsp fail!", gd_app_module_name(ctx->m_module), rspName);
        return -1;
    }

    if (gd_app_rsp_init(ctx, rsp, cfgNode) != 0) {
        dp_rsp_free(rsp);
        return -1;
    }

    responsToCfg = cfg_find_cfg(cfgNode, "respons-to");
    if (responsToCfg == NULL) {
        APP_CTX_ERROR(
            ctx->m_context,
            "%s reading rsp %s: no respons-to cfg!", gd_app_module_name(ctx->m_module), rspName);
        dp_rsp_free(rsp);
        return -1;
    }

    if (gd_app_rsp_bind(ctx, rsp, responsToCfg) != 0) {
        dp_rsp_free(rsp);
        return -1;
    }

    if (gd_app_rsp_mgr_add_rsp(ctx->m_rspMgr, rspName) != 0) {
        APP_CTX_ERROR(
            ctx->m_context,
            "%s reading rsp %s: add to rsp mgr fail, no memory!",
            gd_app_module_name(ctx->m_module), rspName);
        dp_rsp_free(rsp);
        return -1;
    }

    return 0;
}

static nm_node_t
gd_app_rsp_create_group_node(gd_app_context_t context, const char * moduleName) {
    nm_node_t moduleNode;
    nm_node_t rspNode;

    moduleNode = gd_app_module_data(context, moduleName);
    if (moduleNode == NULL) {
        APP_CTX_ERROR(context, "%s reading rsp: no module node!", moduleName);
        return NULL;
    }

    rspNode = nm_instance_create(gd_app_nm_mgr(context), "rsps", sizeof(struct gd_app_rsp_mgr));
    if (rspNode == NULL) {
        APP_CTX_ERROR(context, "%s reading rsp: create rsps node fail!", moduleName);
        return NULL;
    }
    gd_app_rsp_mgr_init(rspNode, context);
    nm_node_set_type(rspNode, &g_module_rsp_mgr);

    if (nm_group_add_member(moduleNode, rspNode) != 0) {
        APP_CTX_ERROR(context, "create module %s: rsp node to module fail!", moduleName);
        nm_node_free(rspNode);
        return NULL;
    }

    return rspNode;
}

int gd_app_rsp_load_i(
    struct gd_app_rsp_create_ctx  * ctx,
    cfg_t rspsCfg)
{
    if (rspsCfg == NULL) return 0;

    if (cfg_type(rspsCfg) == CPE_CFG_TYPE_STRUCT) {
        cfg_t subRspsCfg;
        struct cfg_it cfgIt;
        int rv = 0;
        cfg_it_init(&cfgIt, rspsCfg);
        while((subRspsCfg = cfg_it_next(&cfgIt))) {
            if (gd_app_rsp_load_i(ctx, subRspsCfg) != 0) {
                rv = -1;
            }
        }

        return rv;
    }
    else if (cfg_type(rspsCfg) == CPE_CFG_TYPE_SEQUENCE) {
        cfg_t rspCfg;
        struct cfg_it cfgIt;
        int rv = 0;
        cfg_it_init(&cfgIt, rspsCfg);
        while((rspCfg = cfg_it_next(&cfgIt))) {
            if (gd_app_rsp_create(ctx, rspCfg) != 0) {
                rv = -1;
            }
        }

        return rv;
    }
    else {
        APP_CTX_ERROR(
            ctx->m_context,
            "%s reading rsp: config node type error!",
            gd_app_module_name(ctx->m_module));
        return -1;
    }
}

int gd_app_rsp_load(
    gd_app_context_t context,
    gd_app_module_t module,
    cfg_t moduleCfg)
{
    struct gd_app_rsp_create_ctx ctx;
    int rv;
    nm_node_t rspNode;

    assert(context);
    assert(module);

    rspNode = gd_app_rsp_create_group_node(context, gd_app_module_name(module));
    if (rspNode == NULL) return -1;

    ctx.m_context = context;
    ctx.m_module = module;
    ctx.m_rspMgr = (struct gd_app_rsp_mgr*)(nm_node_data(rspNode));
    ctx.m_dpm = gd_app_dp_mgr(context);
    mem_buffer_init(&ctx.m_buffer, gd_app_alloc(context));

    rv = gd_app_rsp_load_i(&ctx, cfg_find_cfg(moduleCfg, "rsps"));

    mem_buffer_clear(&ctx.m_buffer);

    if (rv != 0) {
        nm_node_free(rspNode);
    }

    return rv;
}

