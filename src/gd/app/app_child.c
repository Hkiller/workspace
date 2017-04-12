#include <assert.h>
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_dlfcn.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/cfg/cfg.h"
#include "gd/app/app_child.h"
#include "gd/app/app_log.h"
#include "app_internal_ops.h"

gd_app_child_context_t 
gd_app_child_context_create(
    gd_app_context_t from, gd_app_child_type_t child_type, const char * lib_name,
    mem_allocrator_t alloc, size_t capacity, int argc, char ** argv)
{
    struct gd_app_child_context * child_context;
    void * (*create_fun)(gd_app_context_t from, mem_allocrator_t alloc, size_t capacity, void * lib_handler, int argc, char * argv[]);

    if (from == NULL) return NULL;

    child_context = mem_alloc(from->m_alloc, sizeof(struct gd_app_child_context));
    if (child_context == NULL) {
        APP_CTX_ERROR(from, "gd_app_context_create_follow: malloc child context fail1");
        return NULL;
    }

#ifdef GD_APP_MULTI_THREAD
    child_context->m_thread = 0;
#endif

    child_context->m_child_type = child_type;
    child_context->m_parent = from;

    if (lib_name) {
        child_context->m_lib_handler = dlopen(lib_name, RTLD_NOW | RTLD_LOCAL);
        if (child_context->m_lib_handler == NULL) {
            APP_CTX_ERROR(from, "gd_app_context_create_follow: load lib %s fail", lib_name);
            mem_free(from->m_alloc, child_context);
            return NULL;
        }
        child_context->m_lib_handler_owner = 1;
    }
    else {
        child_context->m_lib_handler = from->m_lib_handler;
        child_context->m_lib_handler_owner = 0;
    }

    create_fun = dlsym(child_context->m_lib_handler, "gd_app_context_create_i");
    child_context->m_state = dlsym(child_context->m_lib_handler, "gd_app_state");
    child_context->m_run = dlsym(child_context->m_lib_handler, "gd_app_run");
    child_context->m_tick = dlsym(child_context->m_lib_handler, "gd_app_tick");
    child_context->m_free = dlsym(child_context->m_lib_handler, "gd_app_context_free");
    child_context->m_stop = dlsym(child_context->m_lib_handler, "gd_app_stop");
    child_context->m_notify_stop = dlsym(child_context->m_lib_handler, "gd_app_notify_stop");

    if (create_fun == NULL
        || child_context->m_state == NULL
        || child_context->m_run == NULL
        || child_context->m_tick == NULL
        || child_context->m_free == NULL
        || child_context->m_stop == NULL
        || child_context->m_notify_stop == NULL)
    {
        APP_CTX_ERROR(from, "gd_app_context_create_follow: find symbols from lib %s fail", lib_name);
        if (child_context->m_lib_handler_owner) dlclose(child_context->m_lib_handler);
        mem_free(from->m_alloc, child_context);
        return NULL;
    }

    child_context->m_child = create_fun(from, from->m_alloc, 0, child_context->m_lib_handler, argc, argv);
    if (child_context->m_child == NULL) {
        APP_CTX_ERROR(from, "gd_app_context_create_follow: crate context fail");
        if (child_context->m_lib_handler_owner) dlclose(child_context->m_lib_handler);
        mem_free(from->m_alloc, child_context);
        return NULL;
    }

    switch(child_type) {
    case gd_app_child_inline:
        TAILQ_INSERT_TAIL(&from->m_inline_childs, child_context, m_next);
        break;
#ifdef GD_APP_MULTI_THREAD
    case gd_app_child_follow:
        TAILQ_INSERT_TAIL(&from->m_follow_childs, child_context, m_next);
        break;
#endif
    default:
        APP_CTX_ERROR(from, "gd_app_context_create_follow: not support app type %d", child_type);
        child_context->m_free(child_context->m_child);
        mem_free(from->m_alloc, child_context);
        return NULL;
    }

    return child_context;
}

gd_app_child_context_t 
gd_app_child_context_create_follow(
    gd_app_context_t from, const char * lib_name,
    mem_allocrator_t alloc, size_t capacity, int argc, char ** argv)
{
    return gd_app_child_context_create(from, gd_app_child_follow, lib_name, alloc, capacity, argc, argv);
}

gd_app_child_context_t 
gd_app_child_context_create_inline(
    gd_app_context_t from, const char * lib_name,
    mem_allocrator_t alloc, size_t capacity, int argc, char ** argv)
{
    return gd_app_child_context_create(from, gd_app_child_inline, lib_name, alloc, capacity, argc, argv);
}

void gd_app_child_context_free(gd_app_child_context_t child_context) {
#ifdef GD_APP_MULTI_THREAD
    if (child_context->m_thread != 0 && child_context->m_thread != pthread_self()) {
        child_context->m_notify_stop(child_context->m_child);
        pthread_join(child_context->m_thread, NULL);
        child_context->m_thread = 0;
    }
#endif

    switch(child_context->m_child_type) {
    case gd_app_child_inline:
        TAILQ_REMOVE(&child_context->m_parent->m_inline_childs, child_context, m_next);
        break;
#ifdef GD_APP_MULTI_THREAD
    case gd_app_child_follow: {
        TAILQ_REMOVE(&child_context->m_parent->m_follow_childs, child_context, m_next);
        break;
    }
#endif
    default:
        assert(0 && "not support sub type!");
        break;
    }

    child_context->m_free(child_context->m_child);

    if (child_context->m_lib_handler_owner) dlclose(child_context->m_lib_handler);

    mem_free(child_context->m_parent->m_alloc, child_context);
}

void gd_app_child_context_free_all(gd_app_context_t context) {
    while(!TAILQ_EMPTY(&context->m_inline_childs)) {
        gd_app_child_context_free(TAILQ_FIRST(&context->m_inline_childs));
    }

#ifdef GD_APP_MULTI_THREAD
    while(!TAILQ_EMPTY(&context->m_follow_childs)) {
        gd_app_child_context_free(TAILQ_FIRST(&context->m_follow_childs));
    }
#endif
}

void * gd_app_context_context_child(gd_app_child_context_t child_context) {
    return child_context->m_child;
}

int gd_app_child_context_set_root(gd_app_child_context_t child_context, const char * root) {
    int (*fun)(void * context, const char * root);

    fun = dlsym(child_context->m_lib_handler, "gd_app_set_root");
    if (fun == NULL) return -1;

    return fun(child_context->m_child, root);
}

int gd_app_child_context_run(gd_app_child_context_t child_context) {
    return child_context->m_run(child_context->m_child);
}

void gd_app_child_context_cancel_all(gd_app_context_t context) {
    gd_app_child_context_t child_context;

    TAILQ_FOREACH(child_context, &context->m_inline_childs, m_next) {
        child_context->m_stop(child_context->m_child);
    }

#ifdef GD_APP_MULTI_THREAD
    TAILQ_FOREACH(child_context, &context->m_follow_childs, m_next) {
        child_context->m_notify_stop(child_context->m_child);
    }
#endif
}

void gd_app_child_context_wait_all(gd_app_context_t context) {
#ifdef GD_APP_MULTI_THREAD
    gd_app_child_context_t child_context;

    TAILQ_FOREACH(child_context, &context->m_follow_childs, m_next) {
        if (child_context->m_child_type != gd_app_child_follow) continue;
        if (child_context->m_thread == 0) continue;

        pthread_join(child_context->m_thread, NULL);
        child_context->m_thread = 0;
    }
#endif
}

#ifdef GD_APP_MULTI_THREAD
static void * gd_app_thread_run(void * ctx) {
    gd_app_child_context_t child_context = (gd_app_child_context_t)ctx;

    child_context->m_run(child_context->m_child);

    return NULL;
}
#endif

int gd_app_child_context_start(gd_app_child_context_t child_context) {
    switch(child_context->m_child_type) {
    case gd_app_child_inline: {
        int (*fun)(void * context);
        fun = dlsym(child_context->m_lib_handler, "gd_app_start_inline");
        if (fun == NULL) return -1;

#ifdef GD_APP_MULTI_THREAD
        child_context->m_thread = pthread_self();
#endif
        return fun(child_context->m_child);
    }
#ifdef GD_APP_MULTI_THREAD
    case gd_app_child_follow: {
        if (pthread_create(&child_context->m_thread, NULL, gd_app_thread_run, child_context) != 0) {
            APP_CTX_ERROR(child_context->m_parent, "gd_app_child_context_start: create thread fail!");
            return -1;
        }
        return 0;
    }
#endif
    default:
        APP_CTX_ERROR(child_context->m_parent, "gd_app_child_context_start: not support app child type %d!", child_context->m_child_type);
        return -1;
    }
}

/**************************************************/
/*load*/
struct gd_app_load_childs_ctx {
    gd_app_context_t m_context;
    struct mem_buffer m_tbuffer;
    int m_error_count;
};

static int gd_app_do_load_child(gd_app_context_t context, const char * root_dir, const char * name, cfg_t cfg) {
    const char * str_app_type;
    gd_app_child_context_t child_context;

    str_app_type = cfg_get_string(cfg, "app-type", NULL);
    if (str_app_type == NULL) {
        APP_CTX_ERROR(context, "load sub app: %s: app-type not configured!", name);
        return -1;
    }

    if (strcmp(str_app_type, "inline") == 0) {
        child_context = gd_app_child_context_create_inline(
            context, cfg_get_string(cfg, "lib-name", NULL), context->m_alloc, 0, 0, NULL);
    }
    else if (strcmp(str_app_type, "follow") == 0) {
        child_context = gd_app_child_context_create_follow(
            context, cfg_get_string(cfg, "lib-name", NULL), context->m_alloc, 0, 0, NULL);
    }
    else {
        APP_CTX_ERROR(context, "load sub app: %s: app-type %s not support!", name, str_app_type);
        return -1;
    }

    if (child_context == NULL) {
        APP_CTX_ERROR(context, "load sub app: %s: create follow context fail!", name);
        return -1;
    }

    if (gd_app_child_context_set_root(child_context, root_dir) != 0) {
        APP_CTX_ERROR(context, "load sub app: %s: set root %s!", name, root_dir);
        gd_app_child_context_free(child_context);
        return -1;
    }

    if (gd_app_child_context_run(child_context) != 0) {
        APP_CTX_ERROR(context, "load sub app: %s: run fail!", name);
        gd_app_child_context_free(child_context);
        return -1;
    }

    return 0;
}

static dir_visit_next_op_t gd_app_load_childs_dir_on_enter(const char * full, const char * base, void * i) {
    const char * cfg_file;
    cfg_t svr_cfg;
    struct gd_app_load_childs_ctx * ctx = (struct gd_app_load_childs_ctx *)i;
    assert(ctx);

    mem_buffer_clear_data(&ctx->m_tbuffer);
    mem_buffer_strcat(&ctx->m_tbuffer, full);
    mem_buffer_strcat(&ctx->m_tbuffer, "/subapp.yml");
    cfg_file = (const char *)mem_buffer_make_continuous(&ctx->m_tbuffer, 0);

    if (!file_exist(cfg_file, NULL)) return dir_visit_next_go;

    if (ctx->m_context->m_debug) {
        APP_CTX_INFO(ctx->m_context, "load sub app: %s: found", base);
    }

    svr_cfg = cfg_create(ctx->m_context->m_alloc);
    if (svr_cfg == NULL) {
        APP_CTX_ERROR(ctx->m_context, "load sub app: %s: create cfg fail", base);
        return dir_visit_next_go;
    }

    if (cfg_yaml_read_file(svr_cfg, ctx->m_context->m_vfs_mgr, cfg_file, cfg_replace, NULL) != 0) {
        APP_CTX_ERROR(ctx->m_context, "load sub app: %s: load cfg from %s fail", base, cfg_file);
        cfg_free(svr_cfg);
        return dir_visit_next_go;
    }

    gd_app_do_load_child(ctx->m_context, full, base, svr_cfg);

    cfg_free(svr_cfg);

    if (ctx->m_context->m_debug) {
        APP_CTX_INFO(ctx->m_context, "load sub app: load %s success", base);
    }

    return dir_visit_next_go;
}

int gd_app_load_childs(gd_app_context_t context) {
    struct gd_app_load_childs_ctx ctx;
    struct dir_visitor dirVisitor;

    if (context->m_root == NULL) return 0;

    ctx.m_context = context;
    ctx.m_error_count = 0;

    dirVisitor.on_dir_enter = gd_app_load_childs_dir_on_enter;
    dirVisitor.on_dir_leave = NULL;
    dirVisitor.on_file = NULL;

    mem_buffer_init(&ctx.m_tbuffer, context->m_alloc);
    dir_search(&dirVisitor, &ctx, context->m_root, 1, context->m_em, NULL);
    mem_buffer_clear(&ctx.m_tbuffer);

    return 0;
}
