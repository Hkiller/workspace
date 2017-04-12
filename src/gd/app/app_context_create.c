#include <assert.h>
#include "cpe/pal/pal_signal.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/net/net_manage.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "gd/app/app_tl.h"
#include "app_internal_ops.h"

static gd_app_context_t g_main_app_context = NULL;

static int gd_app_parse_args(gd_app_context_t context, int argc, char * argv[]) {
    int i;
    const char * value;

    for(i = 0; i < argc; ++i) {
        if (gd_app_add_arg(context, argv[i]) != 0) return -1;
    }

    if ((value = gd_app_arg_find(context, "--root"))) {
        context->m_root = cpe_str_mem_dup(context->m_alloc, value);
    }

    return 0;
}

gd_app_context_t
gd_app_context_create_i(gd_app_context_t from, mem_allocrator_t alloc, size_t capacity, void * lib_handler, int argc, char * argv[]) {
    gd_app_context_t context;
    size_t allocSize;
    uint8_t i;

    allocSize = sizeof(struct gd_app_context) + capacity;
    context = (gd_app_context_t)mem_alloc(alloc, allocSize);
    if (context == NULL) {
        return NULL;
    }
    
    bzero(context, allocSize);

    context->m_from = from;
    context->m_alloc = alloc;
    context->m_capacity = capacity;
    context->m_state = gd_app_init;
    context->m_notify_stop = 0;
    context->m_lib_handler = lib_handler;

    TAILQ_INIT(&context->m_runing_modules);
    TAILQ_INIT(&context->m_tick_chain);
    TAILQ_INIT(&context->m_tls);
    TAILQ_INIT(&context->m_inline_childs);

#ifdef GD_APP_MULTI_THREAD
    TAILQ_INIT(&context->m_follow_childs);
#endif
    
    cpe_error_monitor_init(&context->m_em_print, cpe_error_log_to_consol, NULL);

    context->m_em = &context->m_em_print;

    if (cpe_hash_table_init(
            &context->m_named_ems,
            alloc,
            (cpe_hash_fun_t) gd_app_em_hash,
            (cpe_hash_eq_t) gd_app_em_cmp,
            CPE_HASH_OBJ2ENTRY(gd_app_em, m_hh),
            -1) != 0)
    {
        mem_free(context->m_alloc, context);
        return NULL;
    }

    if (gd_app_parse_args(context, argc, argv) != 0) {
        gd_app_context_free(context);
        return NULL;
    }

    context->m_cfg = cfg_create(alloc);
    if (context->m_cfg == NULL) {
        gd_app_context_free(context);
        return NULL;
    }

    app_tl_manage_create(context, "default", alloc);
    if (TAILQ_EMPTY(&context->m_tls)) {
        gd_app_context_free(context);
        return NULL;
    }

    context->m_nm_mgr = nm_mgr_create(alloc);
    if (context->m_nm_mgr == NULL) {
        gd_app_context_free(context);
        return NULL;
    }

    context->m_dp_mgr = dp_mgr_create(alloc);
    if (context->m_dp_mgr == NULL) {
        gd_app_context_free(context);
        return NULL;
    }

    context->m_net_mgr = net_mgr_create(alloc, context->m_em);
    if (context->m_net_mgr == NULL) {
        gd_app_context_free(context);
        return NULL;
    }

    context->m_vfs_mgr = vfs_mgr_create(alloc, context->m_em);
    if (context->m_vfs_mgr == NULL) {
        gd_app_context_free(context);
        return NULL;
    }

    for(i = 0; i < CPE_ARRAY_SIZE(context->m_tmp_buffer); ++i) {
        mem_buffer_init(&context->m_tmp_buffer[i], alloc);
    }
    
    return context;
}

void gd_app_context_free(gd_app_context_t context) {
    uint8_t i;
    
    if (context == NULL) return;

    gd_app_child_context_cancel_all(context);

    gd_app_modules_unload(context);

    gd_app_child_context_wait_all(context);

    gd_app_child_context_free_all(context);

    gd_app_tick_chain_free(context);

    if (context->m_nm_mgr) {
        nm_mgr_free(context->m_nm_mgr);
        context->m_nm_mgr = NULL;
    }
    
    if (context->m_vfs_mgr) {
        vfs_mgr_free(context->m_vfs_mgr);
        context->m_vfs_mgr = NULL;
    }

    if (context->m_net_mgr) {
        net_mgr_free(context->m_net_mgr);
        context->m_net_mgr = NULL;
    }
    
    if (context->m_dp_mgr) {
        dp_mgr_free(context->m_dp_mgr);
        context->m_dp_mgr = NULL;
    }

    while(!TAILQ_EMPTY(&context->m_tls)) {
        gd_app_tl_free(TAILQ_FIRST(&context->m_tls));
    }

    if (context->m_cfg) {
        cfg_free(context->m_cfg);
        context->m_cfg = NULL;
    }

    while(context->m_argc > 0) {
        mem_free(context->m_alloc, context->m_argv[--context->m_argc]);
    }

    if (context->m_root) {
        mem_free(context->m_alloc, context->m_root);
        context->m_root = NULL;
    }

    gd_app_em_free_all(context);
    cpe_hash_table_fini(&context->m_named_ems);

    if (gd_app_ins() == context) {
        gd_app_ins_set(NULL);
    }

    if (g_main_app_context == context) {
#ifdef GD_APP_MULTI_THREAD
        gd_app_ms_global_fini();
#endif
        g_main_app_context = NULL;
    }

    for(i = 0; i < CPE_ARRAY_SIZE(context->m_tmp_buffer); ++i) {
        mem_buffer_clear(&context->m_tmp_buffer[i]);
    }
    
    mem_free(context->m_alloc, context);
}

extern void * gd_app_default_lib_handler;

gd_app_context_t
gd_app_context_create_main(mem_allocrator_t alloc, size_t capacity, int argc, char * argv[]) {
    if (g_main_app_context != NULL) return NULL;

#ifdef GD_APP_MULTI_THREAD
    if (gd_app_ms_global_init() != 0) return NULL;
#endif
    
    g_main_app_context = gd_app_context_create_i(NULL, alloc, capacity, gd_app_default_lib_handler, argc, argv);

#ifdef GD_APP_MULTI_THREAD
    if (g_main_app_context == NULL) {
        gd_app_ms_global_fini();
    }
#endif

    return g_main_app_context;
}

static void gd_global_stop_sig_handler(int sig) {
    if (g_main_app_context == NULL) return;
    g_main_app_context->m_notify_stop = 1;
}

void gd_stop_on_signal(int sig) {
    signal(sig, gd_global_stop_sig_handler);
}
