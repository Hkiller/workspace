#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/memory.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "app_internal_ops.h"

struct gd_app_module {
    mem_allocrator_t m_alloc;
    struct gd_app_module_type * m_type;

    TAILQ_ENTRY(gd_app_module) m_qh_for_app;
    TAILQ_ENTRY(gd_app_module) m_qh_for_runing;
}; 

static
struct gd_app_module * 
gd_app_module_create_i(
    gd_app_context_t context,
    const char * module_name,
    const char * type_name,
    const char * libName,
    cfg_t moduleCfg)
{
    struct gd_app_module_type * module;
    struct gd_app_module * runing_module;
    nm_node_t moduleDataGroup;
    size_t name_len;

    assert(module_name);

    if (type_name == NULL) type_name = module_name;

    module = gd_app_module_type_find(type_name);
    if (module == NULL) {
        module = gd_app_module_type_create_from_lib(type_name, libName, context->m_em);
        if (module == NULL) return NULL;
    }

    name_len = strlen(module_name) + 1;

    runing_module = (struct gd_app_module *)
        mem_alloc(context->m_alloc, sizeof(struct gd_app_module) + name_len);
    if (runing_module == NULL) {
        APP_CTX_ERROR(context, "%s: alloc runing module fail!", module_name);
        if (TAILQ_EMPTY(&module->m_runing_modules)) gd_app_module_type_free(module, context->m_em);
        return NULL;
    }

    memcpy(runing_module + 1, module_name, name_len);

    runing_module->m_alloc = context->m_alloc;
    runing_module->m_type = module;

    moduleDataGroup = gd_app_module_data_load(context, module_name);
    if (moduleDataGroup == NULL) {
        if (TAILQ_EMPTY(&module->m_runing_modules)) gd_app_module_type_free(module, context->m_em);
        mem_free(context->m_alloc, runing_module);
        return NULL;
    }

    if (module->m_app_init) {
        if (module->m_app_init(context, runing_module, moduleCfg) != 0) {
            APP_CTX_ERROR(context, "%s: app init fail!", module_name);
            gd_app_module_data_free(context, module_name);
            if (TAILQ_EMPTY(&module->m_runing_modules)) gd_app_module_type_free(module, context->m_em);
            mem_free(context->m_alloc, runing_module);
            return NULL;
        }
    }

    TAILQ_INSERT_TAIL(&context->m_runing_modules, runing_module, m_qh_for_app);
    TAILQ_INSERT_TAIL(&module->m_runing_modules, runing_module, m_qh_for_runing);

    if (context->m_debug) {
        APP_CTX_INFO(context, "%s: module create success!", module_name);
    }

    return runing_module;
}

static int gd_app_module_create(gd_app_context_t context, const char * module_name, cfg_t cfg) {
    assert(context);

    return gd_app_module_create_i(
        context,
        module_name,
        cfg_get_string(cfg, "type", NULL),
        cfg_get_string(cfg, "library", NULL),
        cfg) == NULL
        ? -1
        : 0;
}

static void gd_app_module_free(
    struct gd_app_module * module,
    gd_app_context_t context)
{
    struct gd_app_module_type * type;

    assert(module);

    type = module->m_type;

    if (type->m_app_fini) {
        type->m_app_fini(context, module);
    }

    TAILQ_REMOVE(&context->m_runing_modules, module, m_qh_for_app);
    TAILQ_REMOVE(&type->m_runing_modules, module, m_qh_for_runing);

    gd_app_module_data_free(context, gd_app_module_name(module));

    if (context->m_debug) {
        APP_CTX_INFO(context, "%s: module free success!", gd_app_module_name(module));
    }

    mem_free(module->m_alloc, module);

    if (TAILQ_EMPTY(&type->m_runing_modules)) {
        gd_app_module_type_free(type, context->m_em);
    }
}

static int gd_app_modules_load_i(gd_app_context_t context, cfg_t moduleListCfg, mem_buffer_t buffer) {
    int rv;
    cfg_t moduleCfg;
    struct cfg_it cfgIt;

    if (cfg_type(moduleListCfg) != CPE_CFG_TYPE_SEQUENCE) {
        APP_CTX_ERROR(
            context, "app: load module [%s]: config type error!",
            cfg_path(buffer, moduleListCfg, 0));
        return -1;
    }

    rv = 0;
    cfg_it_init(&cfgIt, moduleListCfg);
    while(rv == 0 && (moduleCfg = cfg_it_next(&cfgIt))) {
        const char * buf;

        buf = cfg_get_string(moduleCfg, "name", NULL);
        if (buf) {
            if (gd_app_module_create(context, buf, moduleCfg) != 0) {
                rv = -1;
            }
            continue;
        }

        buf = cfg_get_string(moduleCfg, "include", NULL);
        if (buf) {
            cfg_t includeCfg =
                cfg_find_cfg(
                    cfg_find_cfg(cfg_parent(moduleListCfg), buf),
                    "load");
            if (includeCfg == NULL) {
                APP_CTX_ERROR(
                    context, "app: load module [%s]: config type error!",
                    cfg_path(buffer, moduleListCfg, 0));
                rv = -1;
            }
            else {
                if (gd_app_modules_load_i(context, includeCfg, buffer) != 0) {
                    rv = -1;
                }
            }

            continue;
        }

        APP_CTX_ERROR(
            context, "app: load module [%s]: no name or include configured!",
            cfg_path(buffer, moduleCfg, 0));

        return -1;
    }

    if (rv != 0) {
        gd_app_modules_unload(context);
    }

    return rv;
}

int gd_app_modules_load(gd_app_context_t context) {
    cfg_t moduleListCfg;
    struct mem_buffer buffer;
    int r;

    moduleListCfg = cfg_find_cfg(context->m_cfg, "modules.load");

    if (moduleListCfg == NULL) {
        if (context->m_debug) {
            APP_CTX_INFO(context, "app: no modules need to load!");
        }
        return -1;
    }

    mem_buffer_init(&buffer, context->m_alloc);

    r = gd_app_modules_load_i(context, moduleListCfg, &buffer);

    mem_buffer_clear(&buffer);

    return r;
}

void gd_app_modules_unload(gd_app_context_t context) {
    while (!TAILQ_EMPTY(&context->m_runing_modules)) {
        gd_app_module_free(
            TAILQ_LAST(&context->m_runing_modules, gd_app_module_list),
            context);
    }
}

const char * gd_app_module_name(gd_app_module_t module) {
    return (const char *)(module + 1);
}

const char * gd_app_module_type_name(gd_app_module_t module) {
    return cpe_hs_data(module->m_type->m_name);
}

gd_app_lib_t gd_app_module_lib(gd_app_module_t module) {
    return module->m_type->m_lib;
}

gd_app_module_t
gd_app_install_module(
    gd_app_context_t context,
    const char * name,
    const char * type_name,
    const char * libName,
    cfg_t moduleCfg)
{
    gd_app_module_t module;

    module = 
        gd_app_module_create_i(context, name, type_name, libName, moduleCfg);

    return module;
}

gd_app_module_t
gd_app_find_module(
    gd_app_context_t context,
    const char * name)
{
    gd_app_module_t runint_module;

    TAILQ_FOREACH(runint_module, &context->m_runing_modules, m_qh_for_app) {
        if (strcmp(name, gd_app_module_name(runint_module)) == 0) {
            return runint_module;
        }
    }

    return NULL;
}

int gd_app_uninstall_module(
    gd_app_context_t context,
    const char * name)
{
    struct gd_app_module * runint_module;

    TAILQ_FOREACH(runint_module, &context->m_runing_modules, m_qh_for_app) {
        if (strcmp(name, gd_app_module_name(runint_module)) == 0) {
            gd_app_module_free(runint_module, context);
            return 0;
        }
    }

    return -1;
}

int gd_app_bulk_install_module(
    gd_app_context_t context,
    gd_app_module_def_t module_defs,
    int module_def_count,
    void * ctx)
{
    cfg_t cfg = NULL;
    int rv = 0;
    int i;

    for(i = 0; rv == 0 && i < module_def_count; ++i) {
        gd_app_module_def_t module_info = module_defs + i;

        if (cfg) { cfg_free(cfg); cfg = NULL; }

        if (module_info->static_cfg) {
            struct read_stream_mem stream = 
                CPE_READ_STREAM_MEM_INITIALIZER(module_info->static_cfg, strlen(module_info->static_cfg));

            cfg = cfg_create(NULL);
            if (cfg == NULL) {
                APP_CTX_ERROR(
                    context, "app: bulk install module: [%d] %s: create cfg fail!",
                    i, module_info->name);
                rv = -1;
                break;
            }

            if (cfg_yaml_read(cfg, (read_stream_t)&stream, cfg_merge_use_new, gd_app_em(context)) != 0) {
                APP_CTX_ERROR(
                    context, "app: bulk install module: [%d] %s: read cfg error!",
                    i, module_info->name);
                rv = -1;
                break;
            }
        }

        if (module_info->dynamic_cfg) {
            if (cfg == NULL) {
                cfg = cfg_create(NULL);
                if (cfg == NULL) {
                    APP_CTX_ERROR(
                        context, "app: bulk install module: [%d] %s: create cfg fail!",
                        i, module_info->name);
                    rv = -1;
                    break;
                }
            }

            if (module_info->dynamic_cfg(context, cfg, module_info, ctx) != 0) {
                APP_CTX_ERROR(
                    context, "app: bulk install module: [%d] %s: process dynamic cfg fail!",
                    i, module_info->name);
                rv = -1;
                break;
            }
        }

        if (gd_app_install_module(context, module_info->name, module_info->type, module_info->lib, cfg) == NULL) {
            APP_CTX_ERROR(
                context, "app: bulk install module: [%d] %s: install fail!",
                i, module_info->name);
            rv = -1;
            break;
        }
    }

    if (cfg) { cfg_free(cfg); cfg = NULL; }

    return rv;
}
