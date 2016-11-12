#ifndef GD_APP_SVR_CONTEXT_H
#define GD_APP_SVR_CONTEXT_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"

#ifdef GD_APP_MULTI_THREAD
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GD_APP_MAX_ARGV 128

struct gd_app_lib;
typedef TAILQ_HEAD(gd_app_lib_list, gd_app_lib) gd_app_lib_list_t;

struct gd_app_module_type;
typedef TAILQ_HEAD(gd_app_module_type_list, gd_app_module_type) gd_app_module_type_list_t;

struct gd_app_module;
typedef TAILQ_HEAD(gd_app_module_list, gd_app_module) gd_app_module_list_t;

struct gd_app_ticker {
    gd_app_tick_fun m_tick;
    void * m_ctx;
    ptr_int_t m_arg;
    TAILQ_ENTRY(gd_app_ticker) m_next;
};
typedef TAILQ_HEAD(gd_app_ticker_list, gd_app_ticker) gd_app_ticker_list_t;

struct gd_app_tl {
    gd_app_context_t m_app;
    const char * m_name;
    tl_manage_t m_tl_mgr;
    TAILQ_ENTRY(gd_app_tl) m_next;
};
typedef TAILQ_HEAD(gd_app_tl_list, gd_app_tl) gd_app_tl_list_t;

struct gd_app_em {
    const char * m_name;
    error_monitor_t m_em;
    struct cpe_hash_entry m_hh;
};

typedef TAILQ_HEAD(gd_app_child_context_list, gd_app_child_context) gd_app_child_context_list_t;

typedef enum gd_app_child_type_t {
    gd_app_child_inline,
    gd_app_child_follow
} gd_app_child_type_t;

struct gd_app_child_context {
    gd_app_child_type_t m_child_type;
    void * m_child;
    gd_app_context_t m_parent;
    uint8_t m_lib_handler_owner;
    void * m_lib_handler;
#ifdef GD_APP_MULTI_THREAD
    pthread_t m_thread;
#endif

    gd_app_status_t (*m_state)(void * context);
    int (*m_run)(void * context);
    int (*m_tick)(void * context, float delta_s);
    void (*m_free)(void * context);
    int (*m_stop)(void * context);
    int (*m_notify_stop)(void * context);

    TAILQ_ENTRY(gd_app_child_context) m_next;
};

struct gd_app_context {
    gd_app_context_t m_from;
    gd_app_status_t m_state;
    int m_argc;
    char * m_argv[GD_APP_MAX_ARGV];
    mem_allocrator_t m_alloc;
    size_t m_capacity;
    uint32_t m_flags;
    uint32_t m_debug;
    void * m_lib_handler;
    struct mem_buffer m_tmp_buffer[8];

    error_monitor_t m_em;
    struct error_monitor m_em_print;
    struct cpe_hash_table m_named_ems;

    gd_app_ticker_list_t m_tick_chain;

    char * m_root;
    cfg_t m_cfg;
    gd_app_tl_list_t m_tls;
    dp_mgr_t m_dp_mgr;
    nm_mgr_t m_nm_mgr;
    net_mgr_t m_net_mgr;
    vfs_mgr_t m_vfs_mgr;
    
    uint8_t m_notify_stop;
    gd_app_fn_t m_main;
    gd_app_fn_t m_stop;
    void * m_fun_ctx;

    gd_app_module_list_t m_runing_modules;

    struct gd_app_child_context * m_to_parent;
    gd_app_child_context_list_t m_inline_childs;

#ifdef GD_APP_MULTI_THREAD
    gd_app_child_context_list_t m_follow_childs;
#endif

};

struct app_tick_runner {
    gd_app_context_t m_app;
    int m_is_runing;
    int64_t m_tick_span;
};

struct gd_app_module_type {
    cpe_hash_string_t m_name;
    struct gd_app_lib * m_lib;
    gd_app_module_global_init m_global_init;
    gd_app_module_global_fini m_global_fini;
    gd_app_module_app_init m_app_init;
    gd_app_module_app_fini m_app_fini;

    TAILQ_ENTRY(gd_app_module_type) m_next;
    TAILQ_ENTRY(gd_app_module_type) m_qh_for_lib;

    gd_app_module_list_t m_runing_modules;
};

typedef struct gd_app_module_type * gd_app_module_type_t;

extern cpe_hash_string_t gd_app_module_type_root_group_name;

#ifdef __cplusplus
}
#endif

#endif
