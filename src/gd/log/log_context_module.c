#include <assert.h>
#include "log4c.h"
#include "log4c/rollingpolicy_type_sizewin.h"
#include "log4c/appender_type_rollingfile.h"
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "log_internal_ops.h"

static int log_context_app_init_cfg(struct log_context * ctx, cfg_t cfg);
static int log_context_app_init_rollingpolicies(struct log_context * ctx, cfg_t cfg);
static int log_context_app_init_layouts(struct log_context * ctx, cfg_t cfg);
static int log_context_app_init_appenders(struct log_context * ctx, cfg_t cfg);
static int log_context_app_init_categories(struct log_context * ctx, cfg_t cfg);

EXPORT_DIRECTIVE
int log_context_app_init(gd_app_context_t context, gd_app_module_t module, cfg_t cfg) {
    struct log_context * ctx;
    error_monitor_t em;
    
    assert(context);
    em = gd_app_em(context);

    ctx = log_context_create(context, em, gd_app_alloc(context));
    if (ctx == NULL) return -1;

    ctx->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (log_context_app_init_cfg(ctx, cfg) != 0
        || log_context_app_init_layouts(ctx, cfg) != 0
        || log_context_app_init_rollingpolicies(ctx, cfg) != 0
        || log_context_app_init_appenders(ctx, cfg) != 0
        || log_context_app_init_categories(ctx, cfg) != 0)
    {
        CPE_ERROR(ctx->m_em, "%s: init fail!", gd_app_module_name(module));
        log_context_free(ctx);
        return -1;
    }

    return 0;
}

EXPORT_DIRECTIVE
void log_context_app_fini(gd_app_context_t context) {
    struct log_context * ctx;
    ctx = log_context_find(context);

    if (ctx) {
        log_context_free(ctx);
    }
}

EXPORT_DIRECTIVE
int log_context_global_init(void) {
    return log4c_init() == 0 ? 0 : -1;
}

EXPORT_DIRECTIVE
void log_context_global_fini() {
    log4c_fini();
}

static int log_context_app_init_rollingpolicies(struct log_context * ctx, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "rollingpolicies"));
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * rollingpolicy_name;
        const char * rollingpolicy_type_name;
        log4c_rollingpolicy_t * rollingpolicy = NULL;

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log rollingpolicy: format error!", log_context_name(ctx));
            return -1;
        }

        rollingpolicy_name = cfg_name(child_cfg);

        rollingpolicy = log4c_rollingpolicy_get(rollingpolicy_name);
        if (rollingpolicy == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log rollingpolicy %s: get fail!", log_context_name(ctx), rollingpolicy_name);
            return -1;
        }

        if ((rollingpolicy_type_name = cfg_get_string(child_cfg, "type", NULL))) {
            const log4c_rollingpolicy_type_t * rollingpolicy_type = log4c_rollingpolicy_type_get(rollingpolicy_type_name);
            if (rollingpolicy_type == NULL) {
                CPE_ERROR(
                    ctx->m_em, "%s: create log rollingpolicy %s: get type %s fail!",
                    log_context_name(ctx), rollingpolicy_name, rollingpolicy_type_name);
                return -1;
            }
            log4c_rollingpolicy_set_type(rollingpolicy, rollingpolicy_type);
        }

        if (strcmp(rollingpolicy_type_name, "sizewin") == 0) {
            rollingpolicy_sizewin_udata_t * sizewin_udatap = NULL;

            if ((sizewin_udatap = log4c_rollingpolicy_get_udata(rollingpolicy))) {
                sizewin_udata_set_file_maxsize(
                    sizewin_udatap,
                    cpe_str_parse_byte_size_with_dft(cfg_get_string(child_cfg, "maxsize", NULL), ROLLINGPOLICY_SIZE_DEFAULT_MAX_FILE_SIZE));

                sizewin_udata_set_max_num_files(sizewin_udatap, cfg_get_int32(child_cfg, "maxnum", ROLLINGPOLICY_SIZE_DEFAULT_MAX_NUM_FILES));

                log4c_rollingpolicy_init(rollingpolicy, log4c_rollingpolicy_get_rfudata(rollingpolicy));
            }
            else {
                sizewin_udatap = sizewin_make_udata();

                log4c_rollingpolicy_set_udata(rollingpolicy, sizewin_udatap);   

                sizewin_udata_set_file_maxsize(
                    sizewin_udatap,
                    cpe_str_parse_byte_size_with_dft(cfg_get_string(child_cfg, "maxsize", NULL), ROLLINGPOLICY_SIZE_DEFAULT_MAX_FILE_SIZE));

                sizewin_udata_set_max_num_files(sizewin_udatap, cfg_get_int32(child_cfg, "maxnum", ROLLINGPOLICY_SIZE_DEFAULT_MAX_NUM_FILES));
            }
        }
    }

    return 0;
}

static int log_context_app_init_layouts(struct log_context * ctx, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "layouts"));

    while((child_cfg = cfg_it_next(&child_it))) {
        const char * layout_name;
        const char * layout_type_name;
        log4c_layout_t * layout = NULL;

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log layout: format error!", log_context_name(ctx));
            return -1;
        }

        layout_name = cfg_name(child_cfg);

        layout = log4c_layout_get(layout_name);
        if (layout == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log layout %s: get fail!", log_context_name(ctx), layout_name);
            return -1;
        }

        if ((layout_type_name = cfg_get_string(child_cfg, "type", NULL))) {
            const log4c_layout_type_t * layout_type = log4c_layout_type_get(layout_type_name);
            if (layout_type == NULL) {
                CPE_ERROR(
                    ctx->m_em, "%s: create log layout %s: get type %s fail!",
                    log_context_name(ctx), layout_name, layout_type_name);
                return -1;
            }

            log4c_layout_set_type(layout, layout_type);
        }
    }

    return 0;
}

static int log_context_app_init_categories(struct log_context * ctx, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "categories"));

    while((child_cfg = cfg_it_next(&child_it))) {
        const char * category_name;
        const char * appender_name;
        log4c_category_t * category;

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log em: format error!", log_context_name(ctx));
            return -1;
        }

        category_name = cfg_name(child_cfg);

        category = log4c_category_get(category_name);
        log4c_category_set_additivity(category, cfg_get_int32(cfg, "additivity", 0));

        appender_name = cfg_get_string(child_cfg, "appender", NULL);
        if (appender_name) {
            log4c_category_set_appender(category, log4c_appender_get(appender_name));
        }

        if (log4c_em_create(ctx, category_name, category) != 0) {
            CPE_ERROR(ctx->m_em, "%s: create log em %s: fail!", log_context_name(ctx), category_name);
            return -1;
        }

        if (ctx->m_debug) {
            CPE_INFO(
                ctx->m_em, "%s: create log em %s: success, appender=%s!",
                log_context_name(ctx), category_name,
                appender_name ? appender_name : "???");
        }
    }

    return 0;
}

static int log_context_app_init_appender_rollingfile(struct log_context * context, log4c_appender_t * appender, cfg_t cfg) {
    const char * log_dir;
    const char * log_prefix = cfg_get_string(cfg, "prefix", NULL);
    const char * rollingpolicy_name = cfg_get_string(cfg, "policy", NULL);
    struct mem_buffer buffer;
    rollingfile_udata_t * udata;

    log_dir = gd_app_arg_find(context->m_app, "--log-dir");
    if (log_dir == NULL) {
        log_dir = cfg_get_string(cfg, "dir", NULL);
    }

    udata = rollingfile_make_udata();
    if (udata == NULL) {
        CPE_ERROR(context->m_em, "%s: create append rollingfile: create udata fail!", log_context_name(context));
        return -1;
    }

    log4c_appender_set_udata(appender, udata);

    mem_buffer_init(&buffer, NULL);

    /*set log dir*/
    if (log_dir == NULL || log_dir[0] != '/') {
        const char * root_dir = gd_app_root(context->m_app);
        if (root_dir) {
            mem_buffer_strcat(&buffer, root_dir);
            mem_buffer_strcat(&buffer, "/");
        }
        else {
            mem_buffer_strcat(&buffer, "/tmp/");
        }

        mem_buffer_strcat(&buffer, log_dir ? log_dir : "log");
        log_dir = (const char *)mem_buffer_make_continuous(&buffer, 0);
    }

    if (rollingfile_udata_set_logdir(udata, (char *)log_dir) != 0) {
        CPE_ERROR(context->m_em, "%s: create append rollingfile: set log dir to %s fail!", log_context_name(context), log_dir);
        mem_buffer_clear(&buffer);
        return -1;
    }

    if (dir_mk_recursion(log_dir, DIR_DEFAULT_MODE, context->m_em, NULL) != 0) {
        CPE_ERROR(context->m_em, "%s: create append rollingfile: set log dir to %s: create dir fail!", log_context_name(context), log_dir);
        mem_buffer_clear(&buffer);
        return -1;
    }

    /*set files prefix*/
    mem_buffer_clear_data(&buffer);
    if (log_prefix == NULL) {
        if (gd_app_argc(context->m_app) > 0) {
            const char * base_name = file_name_base(gd_app_argv(context->m_app)[0], &buffer);
            if (mem_buffer_size(&buffer) == 0) {
                mem_buffer_strcat(&buffer, base_name);
            }
            mem_buffer_strcat(&buffer, ".log");
            log_prefix = mem_buffer_make_continuous(&buffer, 0);
        }
        else {
            log_prefix = "app.log";
        }
    }

    if (rollingfile_udata_set_files_prefix(udata, (char *)log_prefix) != 0) {
        CPE_ERROR(context->m_em, "%s: create append rollingfile: set prefix to %s fail!", log_context_name(context), log_prefix);
        mem_buffer_clear(&buffer);
        return -1;
    }

    if (rollingpolicy_name) {
        log4c_rollingpolicy_t * rollingfilepolicy = log4c_rollingpolicy_get(rollingpolicy_name);
        rollingfile_udata_set_policy(udata, rollingfilepolicy);
        log4c_rollingpolicy_init(rollingfilepolicy, udata);
    }

    mem_buffer_clear(&buffer);
    return 0;
}

static int log_context_app_init_appenders(struct log_context * ctx, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t child_cfg;

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "appenders"));

    while((child_cfg = cfg_it_next(&child_it))) {
        const char * appender_name;
        const char * appender_type_name;
        const char * appender_layout_name;
        log4c_appender_t * appender;

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log appender: format error!", log_context_name(ctx));
            return -1;
        }

        appender_name = cfg_name(child_cfg);

        appender = log4c_appender_get(appender_name);
        if (appender == NULL) {
            CPE_ERROR(ctx->m_em, "%s: create log appender %s: get fail!", log_context_name(ctx), appender_name);
            return -1;
        }

        if ((appender_type_name = cfg_get_string(child_cfg, "type", NULL))) {
            log4c_appender_set_type(appender, log4c_appender_type_get(appender_type_name));

            if (strcmp(appender_type_name, "rollingfile") == 0) {
                if (log_context_app_init_appender_rollingfile(ctx, appender, child_cfg) != 0) {
                    return -1;
                }
            }
        }

        if ((appender_layout_name = cfg_get_string(child_cfg, "layout", NULL))) {
            log4c_appender_set_layout(appender, log4c_layout_get(appender_layout_name));
        }


        if (ctx->m_debug) {
            if (appender_type_name && strcmp(appender_type_name, "rollingfile") == 0) {
                rollingfile_udata_t * udata = log4c_appender_get_udata(appender);
                assert(udata);

                CPE_INFO(
                    ctx->m_em, "%s: create appender %s: type=%s, layout=%s, log=%s/%s!",
                    log_context_name(ctx), appender_name, appender_type_name, appender_layout_name,
                    rollingfile_udata_get_logdir(udata), rollingfile_udata_get_files_prefix(udata));
            }
            else {
                CPE_INFO(
                    ctx->m_em, "%s: create appender %s: type=%s, layout=%s!",
                    log_context_name(ctx), appender_name, appender_type_name, appender_layout_name);
            }
        }
    }

    return 0;
}


static int log_context_app_init_cfg(struct log_context * ctx, cfg_t cfg) {
    cfg = cfg_find_cfg(cfg, "config");

    log4c_rc->config.nocleanup = cfg_get_int32(cfg, "nocleanup", log4c_rc->config.nocleanup);
    log4c_rc->config.bufsize = 
        cpe_str_parse_byte_size_with_dft(cfg_get_string(cfg, "bufsize", NULL), log4c_rc->config.bufsize);
    return 0;
}
