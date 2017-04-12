#include <assert.h>
#include "cpe/pal/pal_signal.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/vfs/vfs_manage.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "app_internal_types.h"

void gd_app_set_main(gd_app_context_t context, gd_app_fn_t fn_main, gd_app_fn_t fn_stop, void * fn_ctx) {
    if (fn_main) {
        if (context->m_main != NULL) {
            CPE_ERROR(context->m_em, "gd_app_set_main: already have main!");
            return;
        }
    }
    else {
        if (context->m_main == NULL) {
            CPE_ERROR(context->m_em, "gd_app_set_main: no main!");
            return;
        }
    }
    
    context->m_main = fn_main;
    context->m_stop = fn_stop;
    context->m_fun_ctx = fn_ctx;
}

void gd_app_set_em(gd_app_context_t context, error_monitor_t em) {
    context->m_em = em;
}

error_monitor_t gd_app_print_em(gd_app_context_t context) {
    return &context->m_em_print;
}

error_monitor_t gd_app_em(gd_app_context_t context) {
    return context->m_em;
}

mem_allocrator_t gd_app_alloc(gd_app_context_t context) {
    return context->m_alloc;
}

void gd_app_set_alloc(gd_app_context_t context, mem_allocrator_t alloc) {
    context->m_alloc = alloc;
}

cfg_t gd_app_cfg(gd_app_context_t context) {
    return context->m_cfg;
}

tl_manage_t gd_app_tl_mgr(gd_app_context_t context) {
    return TAILQ_EMPTY(&context->m_tls)
        ? NULL
        : TAILQ_FIRST(&context->m_tls)->m_tl_mgr;
}

dp_mgr_t gd_app_dp_mgr(gd_app_context_t context) {
    return context->m_dp_mgr;
}

nm_mgr_t gd_app_nm_mgr(gd_app_context_t context) {
    return context->m_nm_mgr;
}

net_mgr_t gd_app_net_mgr(gd_app_context_t context) {
    return context->m_net_mgr;
}

vfs_mgr_t gd_app_vfs_mgr(gd_app_context_t context) {
    return context->m_vfs_mgr;
}

int gd_app_argc(gd_app_context_t context) {
    return context->m_argc;
}

char ** gd_app_argv(gd_app_context_t context) {
    return context->m_argv;
}

int gd_app_add_arg(gd_app_context_t context, char * arg) {
    if (context->m_argc + 1 < GD_APP_MAX_ARGV) {
        context->m_argv[context->m_argc++] = cpe_str_mem_dup(context->m_alloc, arg);
        return 0;
    }
    else {
        return -1;
    }
}

int gd_app_context_argc(gd_app_context_t context) {
    return context->m_argc;
}

char ** gd_app_context_argv(gd_app_context_t context) {
    return context->m_argv;
}

int gd_app_arg_is_set(gd_app_context_t context, const char * arg_name) {
    int i;
    int arg_name_len = (int)strlen(arg_name);

    for(i = 1; i < context->m_argc; ++i) {
        char * p;

        p = strstr(context->m_argv[i], arg_name);
        if (p != context->m_argv[i]) continue;

        if (p[arg_name_len] == 0) {
            return 1;
        }
        else if (p[arg_name_len] == '=') {
            return 1;
        }
    }

    return 0;
}

const char * gd_app_arg_find(gd_app_context_t context, const char * arg_name) {
    int i;
    int arg_name_len = (int)strlen(arg_name);

    for(i = 1; i < context->m_argc; ++i) {
        char * p;

        p = strstr(context->m_argv[i], arg_name);
        if (p != context->m_argv[i]) continue;

        if (p[arg_name_len] == 0) {
            if ((i + 1) < context->m_argc) {
                return context->m_argv[i + 1];
            }
            else {
                return NULL;
            }
        }
        else if (p[arg_name_len] == '=') {
            return p + arg_name_len + 1;
        }
    }

    return NULL;
}

const char * gd_app_arg_find_ex(gd_app_context_t context, const char * arg_name, int * from_pos) {
    int i;
    int arg_name_len = (int)strlen(arg_name);

    for(i = *from_pos; i < context->m_argc; ++i) {
        char * p;

        p = strstr(context->m_argv[i], arg_name);
        if (p != context->m_argv[i]) continue;

        if (p[arg_name_len] == 0) {
            if ((i + 1) < context->m_argc) {
                *from_pos = i + 1;
                return context->m_argv[i + 1];
            }
            else {
                *from_pos = context->m_argc;
                return NULL;
            }
        }
        else if (p[arg_name_len] == '=') {
            *from_pos = i;
            return p + arg_name_len + 1;
        }
    }

    *from_pos = context->m_argc;
    return NULL;
}

void * gd_app_context_user_data(gd_app_context_t context) {
    return (void*)(context + 1);
}

int gd_app_set_root(gd_app_context_t context, const char * path) {
    char * buf;

    if (path == NULL || context == NULL) return -1;

    buf = strdup(path);
    if (buf == NULL) return -1;

    if (context->m_root) {
        free(context->m_root);
    }
    context->m_root = buf;
    return 0;
}

const char * gd_app_root(gd_app_context_t context) {
    return context->m_root;
}

gd_app_context_t gd_app_context_from(gd_app_context_t app) {
    return app->m_from;
}

nm_node_t gd_app_context_find_node(gd_app_context_t app, cpe_hash_string_t name) {
    while(app) {
        nm_node_t r = nm_mgr_find_node(app->m_nm_mgr, name);
        if (r) return r;
        app = app->m_from;
    }

    return NULL;
}

nm_node_t gd_app_context_find_node_nc(gd_app_context_t app, const char * name) {
    while(app) {
        nm_node_t r = nm_mgr_find_node_nc(app->m_nm_mgr, name);
        if (r) return r;
        app = app->m_from;
    }

    return NULL;
}    

int gd_app_cfg_reload(gd_app_context_t context) {
    int rv;

    mem_buffer_clear_data(&context->m_tmp_buffer[0]);

    if (context->m_root) {
        mem_buffer_strcat(&context->m_tmp_buffer[0], context->m_root);
        if (strcmp(context->m_root, "/") != 0) mem_buffer_strcat(&context->m_tmp_buffer[0], "/");
    }
    mem_buffer_strcat(&context->m_tmp_buffer[0], "etc.bc");
    if (vfs_file_exist(context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0))) {
        rv = cfg_bin_read_file(context->m_cfg, context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0), context->m_em);
        goto READ_COMPLETE;
    }

    mem_buffer_clear_data(&context->m_tmp_buffer[0]);
    if (context->m_root) {
        mem_buffer_strcat(&context->m_tmp_buffer[0], context->m_root);
        if (strcmp(context->m_root, "/") != 0) mem_buffer_strcat(&context->m_tmp_buffer[0], "/");
    }
    mem_buffer_strcat(&context->m_tmp_buffer[0], "etc.yml");
    if (vfs_file_exist(context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0))) {
        rv = cfg_yaml_read_file(context->m_cfg, context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0), cfg_merge_use_new, context->m_em);
        goto READ_COMPLETE;
    }

    mem_buffer_clear_data(&context->m_tmp_buffer[0]);
    if (context->m_root) {
        mem_buffer_strcat(&context->m_tmp_buffer[0], context->m_root);
        if (strcmp(context->m_root, "/") != 0) mem_buffer_strcat(&context->m_tmp_buffer[0], "/");
    }
    mem_buffer_strcat(&context->m_tmp_buffer[0], "etc");
    
    rv = cfg_read_dir(context->m_cfg, context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0), cfg_merge_use_new, context->m_em, context->m_alloc);

READ_COMPLETE:
    if (rv == 0) {
        if (context->m_debug) {
            CPE_INFO(context->m_em, "load config from %s success!", (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0));
        }
    }

    if (rv == 0) {
        mem_buffer_clear_data(&context->m_tmp_buffer[0]);
        if (context->m_root) {
            mem_buffer_strcat(&context->m_tmp_buffer[0], context->m_root);
            if (strcmp(context->m_root, "/") != 0) mem_buffer_strcat(&context->m_tmp_buffer[0], "/");
        }
        mem_buffer_strcat(&context->m_tmp_buffer[0], "alter");
        
        if (vfs_dir_exist(context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0))
            || vfs_file_exist(context->m_vfs_mgr, (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0)))
        {
            cfg_t alter_cfg = cfg_create(context->m_alloc);

            rv = cfg_read_dir(
                alter_cfg,
                context->m_vfs_mgr,
                (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0),
                cfg_merge_use_new,
                context->m_em,
                context->m_alloc);
            if (rv == 0) {
                struct cfg_it childs;
                cfg_t alter_node;

                cfg_it_init(&childs, alter_cfg);
                while((alter_node = cfg_it_next(&childs)) && rv == 0) {
                    rv = cfg_apply_modify_seq(context->m_cfg, alter_node, context->m_em);
                }
            }

            cfg_free(alter_cfg);

            if (context->m_debug) {
                CPE_INFO(context->m_em, "load config alter from %s success!", (char*)mem_buffer_make_continuous(&context->m_tmp_buffer[0], 0));
            }
        }
    }

    return rv;
}

uint32_t gd_app_flags(gd_app_context_t app) {
    return app->m_flags;
}

void gd_app_flags_set(gd_app_context_t app, uint32_t flag) {
    app->m_flags = flag;
}

void gd_app_flag_enable(gd_app_context_t app, gd_app_flag_t flag) {
    app->m_flags |= flag;
}

void gd_app_flag_disable(gd_app_context_t app, gd_app_flag_t flag) {
    app->m_flags &= ~((uint32_t)flag);
}

int gd_app_flag_is_enable(gd_app_context_t app, gd_app_flag_t flag) {
    return app->m_flags & flag;
}

void gd_app_set_state(gd_app_context_t context, gd_app_status_t state) {
    context->m_state = state;
}

int gd_app_debug(gd_app_context_t context) {
    return context->m_debug;
}

void gd_app_set_debug(gd_app_context_t context, int level) {
    context->m_debug = level;
}

int gd_app_add_tag(gd_app_context_t context, const char * tag) {
    cfg_t tags;

    tags = cfg_struct_add_seq(context->m_cfg, "tags", cfg_merge_use_exist);
    if (tags == NULL) {
        CPE_ERROR(context->m_em, "gd_app_add_tag: create tags fail!");
        return -1;
    }
    
    if (cfg_seq_add_string(tags, tag) == NULL) {
        CPE_ERROR(context->m_em, "gd_app_add_tag: add tag fail!");
        return -1;
    }

    return 0;
}

uint8_t gd_app_have_tag(gd_app_context_t context, const char * tag) {
    cfg_t tags;
    struct cfg_it child_it;
    cfg_t child_cfg;
    
    tags = cfg_find_cfg(context->m_cfg, "tags");
    if (tags == NULL) return 0;

    cfg_it_init(&child_it, tags);
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * value = cfg_as_string(child_cfg, NULL);
        if (value && strcmp(value, tag) == 0) return 1;
    }

    return 0;
}

gd_app_status_t gd_app_state(gd_app_context_t context) {
    return context->m_state;
}

mem_buffer_t gd_app_tmp_buffer(gd_app_context_t context) {
    return &context->m_tmp_buffer[0];
}

mem_buffer_t gd_app_tmp_buffer_at(gd_app_context_t context, uint8_t index) {
    assert(index < CPE_ARRAY_SIZE(context->m_tmp_buffer));
    return &context->m_tmp_buffer[index];
}
