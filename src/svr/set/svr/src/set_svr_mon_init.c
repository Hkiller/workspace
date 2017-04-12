#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/share/set_repository.h"
#include "svr/set/share/set_utils.h"
#include "set_svr_mon_app.h"
#include "set_svr_svr_ins.h"

static int set_svr_all_root(const char * app_bin, char * buf, size_t buf_capacity, error_monitor_t em);
static int set_svr_mon_app_create_svr_ins(set_svr_mon_app_t mon_app, uint16_t svr_id);
static int set_svr_app_init_load_local_env(set_svr_mon_t mon, cfg_t g_env);
static int set_svr_app_init_create_app(set_svr_mon_t mon, const char * all_root, cfg_t g_env, const char * app_name, cfg_t app_args);

int set_svr_app_init_mon(set_svr_mon_t mon, cfg_t cfg_set_root) {
    set_svr_t svr = mon->m_svr;
    struct cfg_it mon_app_it;
    cfg_t mon_app_cfg;
    char all_root[256];
    cfg_t cfg_tmp;
    cfg_t g_env = NULL;

    if (set_svr_all_root(gd_app_argv(svr->m_app)[0], all_root, sizeof(all_root), svr->m_em) != 0) goto INIT_MON_FAIL;

    /*读入全局的参数配置 */
    g_env = cfg_create(svr->m_alloc);
    if (g_env == NULL) {
        CPE_ERROR(svr->m_em, "%s: load apps: create local env cfg fail", set_svr_name(svr));
        goto INIT_MON_FAIL;
    }

    /*      从set的配置中读取 */
    if ((cfg_tmp = cfg_find_cfg(cfg_set_root, "env"))) {
        if (cfg_merge(g_env, cfg_tmp, cfg_merge_use_new, svr->m_em) != 0) {
            CPE_ERROR(svr->m_em, "%s: load apps: merge set env fail", set_svr_name(svr));
            goto INIT_MON_FAIL;
        }
    }

    /*      从本机的运行环境中读取 */
    if (set_svr_app_init_load_local_env(mon, g_env) != 0) {
        goto INIT_MON_FAIL;
    }

    /*遍历应用配置，加载app */
    cfg_it_init(&mon_app_it, cfg_find_cfg(cfg_set_root, "apps"));
    while((mon_app_cfg = cfg_it_next(&mon_app_it))) {
        cfg_t app_args;
        const char * app_name;

        if (cfg_type(mon_app_cfg) == CPE_CFG_TYPE_STRING) {
            app_name = cfg_as_string(mon_app_cfg, NULL);
            assert(app_name);
            app_args = NULL;
        }
        else if (cfg_type(mon_app_cfg) == CPE_CFG_TYPE_STRUCT) {
            mon_app_cfg = cfg_child_only(mon_app_cfg);
            if (mon_app_cfg == NULL) {
                CPE_ERROR(
                    svr->m_em, "%s: load apps: app cfg type error, struct have not only child!",
                    set_svr_name(svr));
                goto INIT_MON_FAIL;
            }

            app_name = cfg_name(mon_app_cfg);
            assert(app_name);
            app_args = mon_app_cfg;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: load apps: app cfg type error, type=%d!",
                set_svr_name(svr), cfg_type(mon_app_cfg));
            goto INIT_MON_FAIL;
        }

        if (set_svr_app_init_create_app(mon, all_root, g_env, app_name, app_args) != 0) {
            goto INIT_MON_FAIL;
        }
    }

    return 0;

INIT_MON_FAIL:
    if (g_env) cfg_free(g_env);
    return -1;
}

static int set_svr_mon_app_create_svr_ins(set_svr_mon_app_t mon_app, uint16_t svr_id) {
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    int shmid;
    uint8_t i;

    char shm_tag = 'a';
    for(i = 0; i < mon_app->m_svr_type_count; ++i, ++shm_tag) {
        set_svr_svr_type_t svr_type = mon_app->m_svr_types[i];
        set_svr_svr_ins_t svr_ins;
        cfg_t svr_cfg;

        assert(svr_type);

        svr_cfg = cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types");
        svr_cfg = cfg_find_cfg(svr_cfg, svr_type->m_svr_type_name);
        if (svr_cfg == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: on find svr %s(%d): config of svr_type %s not exist!",
                set_svr_name(svr), mon_app->m_name, svr_id, svr_type->m_svr_type_name);
            return -1;
        }

        svr_ins = set_svr_svr_ins_create(svr_type, svr_id, NULL);
        if (svr_ins == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: on find svr %s(%d): create ins of svr ty pe %s fail!",
                set_svr_name(svr), mon_app->m_name, svr_id, svr_type->m_svr_type_name);
            set_svr_svr_ins_free(svr_ins);
            return -1;
        }

        shmid = set_shm_key_get(svr_type->m_svr_type_id, svr_id, shm_tag);
        if (shmid == -1) {
            CPE_ERROR(
                svr->m_em, "%s: on find svr %s(%d): get shmid at %s(%c) fail!",
                set_svr_name(svr), mon_app->m_name, svr_id, mon_app->m_pidfile, shm_tag);
            set_svr_svr_ins_free(svr_ins);
            return -1;
        }

        svr_ins->m_chanel = set_chanel_shm_init(shmid, mon_app->m_wq_size, mon_app->m_rq_size, svr->m_em);
        if (svr_ins->m_chanel == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: on find svr %s(%d): svr type %s: attach chanel fail!",
                set_svr_name(svr), mon_app->m_name, svr_id, svr_type->m_svr_type_name);
            set_svr_svr_ins_free(svr_ins);
            return -1;
        }
    }

    return 0;
}

static int set_svr_all_root(const char * app_bin, char * buf, size_t buf_capacity, error_monitor_t em) {
    const char * dir_end_1 = NULL;
    const char * dir_end_2 = NULL;
    const char * dir_end_3 = NULL;
    const char * p;
    int len;

    if (app_bin == NULL) {
        CPE_ERROR(em, "set_svr: get project root fail!");
        return -1;
    }

    for(p = strchr(app_bin, '/'); p; p = strchr(p + 1, '/')) {
        dir_end_1 = dir_end_2;
        dir_end_2 = dir_end_3;
        dir_end_3 = p;
    }

    if (dir_end_1 == NULL) {
        CPE_ERROR(em, "set_svr: get project root fail, gd_app_root=%s", app_bin);
        return -1;
    }

    len = dir_end_1 - app_bin;
    if (len > buf_capacity) {
        CPE_ERROR(em, "set_svr: get project root overflow!, buf_capacity=%d", (int)buf_capacity);
        return -1;
    }

    memcpy(buf, app_bin, len);
    buf[len] = 0;

    return 0;
}

static const char * set_svr_app_read_value(set_svr_t svr, cfg_t g_env, cfg_t app_env, const char * input, char * buf, size_t buf_capacity) {
    const char * value;

    if (input == NULL) return input;

    if (input[0] != '$') return input;

    if (app_env && (value = cfg_get_string_cvt(app_env, input + 1, NULL, buf, buf_capacity))) return value;

    if (g_env && (value = cfg_get_string_cvt(g_env, input + 1, NULL, buf, buf_capacity))) return value;

    return NULL;
}

static int set_svr_app_init_create_app(set_svr_mon_t mon, const char * all_root, cfg_t g_env, const char * app_name, cfg_t app_args) {
    set_svr_t svr = mon->m_svr;
    const char * base;
    const char * bin;
    struct cfg_it arg_it;
    const char * str_rq_size;
    uint64_t rq_size;
    const char * str_wq_size;
    uint64_t wq_size;
    cfg_t arg_cfg;
    char app_root[256];
    char app_bin[256];
    char pidfile[256];
    set_svr_mon_app_t mon_app;
    cfg_t app_cfg;
    cfg_t app_env;
    char buf[128];
    char set_id[25];
    cfg_t svr_type_cfg;
    const char * svr_type_name;
    set_svr_svr_type_t svr_type;
    const char * log_dir;

    snprintf(buf, sizeof(buf), "apps.%s", app_name);
    app_cfg = cfg_find_cfg(gd_app_cfg(svr->m_app), buf);
    if (app_cfg == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: app not configured, path=%s", set_svr_name(svr), app_name, buf);
        return -1;
    }
    app_env = cfg_find_cfg(app_args, "env");

    log_dir = gd_app_arg_find(svr->m_app, "--log-dir");
    if (log_dir == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: log-dir not configured in app", set_svr_name(svr), app_name);
        return -1;
    }

    svr_type_cfg = cfg_find_cfg(app_cfg, "svr-type");
    if (svr_type_cfg == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: svr-type not configured", set_svr_name(svr), app_name);
        return -1;
    }

    base = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "base", NULL), NULL, 0);
    if (base == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: base not configured", set_svr_name(svr), app_name);
        return -1;
    }

    bin = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "bin", NULL), NULL, 0);
    if (bin == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: bin not configured", set_svr_name(svr), app_name);
        return -1;
    }

    snprintf(app_root, sizeof(app_root), "%s/%s", all_root, base);
    if (!dir_exist(app_root, svr->m_em)) {
        CPE_ERROR(svr->m_em, "%s: load app %s: root dir %s not exist", set_svr_name(svr), app_name, app_root);
        return -1;
    }

    snprintf(app_bin, sizeof(app_bin), "%s/%s", app_root, bin);
    if (access(app_bin, X_OK) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: load app %s: check %s execute fail, error=%d (%s)",
            set_svr_name(svr), app_name, app_bin, errno, strerror(errno));
        return -1;
    }

    snprintf(pidfile, sizeof(pidfile), "%s/%s.pid", svr->m_repository_root, base);

    if ((str_rq_size = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "rq-size", NULL), buf, sizeof(buf))) == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: rq-size not configured!", set_svr_name(svr), app_name);
        return -1;
    }

    if (cpe_str_parse_byte_size(&rq_size, str_rq_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: load app %s: rq-size %s format error!", set_svr_name(svr), app_name, str_rq_size);
        return -1;
    }

    if ((str_wq_size = set_svr_app_read_value(svr, g_env, app_env, cfg_get_string(app_cfg, "wq-size", NULL), buf, sizeof(buf))) == NULL) {
        CPE_ERROR(svr->m_em, "%s: load app %s: wq-size not configured!", set_svr_name(svr), app_name);
        return -1;
    }

    if (cpe_str_parse_byte_size(&wq_size, str_wq_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: load app %s: wq-size %s format error!", set_svr_name(svr), app_name, str_wq_size);
        return -1;
    }

    mon_app = set_svr_mon_app_create(mon, app_name, app_bin, pidfile, rq_size, wq_size);
    if (mon_app == NULL) return -1;

    if ((svr_type_name = cfg_as_string(svr_type_cfg, NULL))) {
        svr_type = set_svr_svr_type_find_by_name(svr, svr_type_name);
        if (svr_type == NULL) {
            CPE_ERROR(svr->m_em, "%s: load app %s: svr-type %s not exist", set_svr_name(svr), app_name, svr_type_name);
            return -1;
        }

        if (set_svr_mon_app_add_svr_type(mon_app, svr_type) != 0) {
            CPE_ERROR(svr->m_em, "%s: load app %s: add svr-type %s fail", set_svr_name(svr), app_name, svr_type_name);
            return -1;
        }
    }
    else if (cfg_type(svr_type_cfg) == CPE_CFG_TYPE_SEQUENCE) {
        struct cfg_it child_it;
        cfg_t child_cfg;

        cfg_it_init(&child_it, svr_type_cfg);

        while((child_cfg = cfg_it_next(&child_it))) {
            svr_type_name = cfg_as_string(child_cfg, NULL);
            if (svr_type_name == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: svr-type format error", set_svr_name(svr), app_name);
                return -1;
            }

            svr_type = set_svr_svr_type_find_by_name(svr, svr_type_name);
            if (svr_type == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: svr-type %s not exist", set_svr_name(svr), app_name, svr_type_name);
                return -1;
            }

            if (set_svr_mon_app_add_svr_type(mon_app, svr_type) != 0) {
                CPE_ERROR(svr->m_em, "%s: load app %s: add svr-type %s fail", set_svr_name(svr), app_name, svr_type_name);
                return -1;
            }
        }

        if (mon_app->m_svr_type_count == 0) {
            CPE_ERROR(svr->m_em, "%s: load app %s: no svr type", set_svr_name(svr), app_name);
            return -1;
        }
    }
    else {
        CPE_ERROR(svr->m_em, "%s: load app %s: svr-type configure type format error", set_svr_name(svr), app_name);
        return -1;
    }

    if (set_svr_mon_app_add_arg(mon_app, app_bin) != 0) {
        CPE_ERROR(svr->m_em, "%s: load app: add ap_bin arg fail", set_svr_name(svr));
        return -1;
    }

    cfg_it_init(&arg_it, cfg_find_cfg(app_cfg, "args"));
    while((arg_cfg = cfg_it_next(&arg_it))) {
        const char * arg_name;
        const char * arg_value;

        switch(cfg_type(arg_cfg)) {
        case CPE_CFG_TYPE_STRING:
            arg_name = cfg_as_string(arg_cfg, NULL);
            assert(arg_name);
            arg_value = NULL;
            break;
        case CPE_CFG_TYPE_STRUCT: {
            const char * orig_arg_value;
            char value_buf[128];

            arg_cfg = cfg_child_only(arg_cfg);
            if (arg_cfg == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: arg format error!", set_svr_name(svr), app_name);
                return -1;
            }

            arg_name = cfg_name(arg_cfg);
            assert(arg_name);

            orig_arg_value = cfg_as_string_cvt(arg_cfg, NULL, value_buf, sizeof(value_buf));
            if (orig_arg_value == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: arg %s value is not string!", set_svr_name(svr), app_name, arg_name);
                return -1;
            }

            arg_value = set_svr_app_read_value(svr, g_env, app_env, orig_arg_value, buf, sizeof(buf));
            if (arg_value == NULL) {
                CPE_ERROR(svr->m_em, "%s: load app %s: arg %s read value %s fail!", set_svr_name(svr), app_name, arg_name, orig_arg_value);
                return -1;
            }

            break;
        }
        default:
            CPE_ERROR(svr->m_em, "%s: load app %s: arg type error, type=%d!", set_svr_name(svr), app_name, cfg_type(arg_cfg));
            return -1;
        }

        if (set_svr_mon_app_add_arg(mon_app, arg_name) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load app %s: add arg name %s fail",
                set_svr_name(svr), app_name, arg_name);
            return -1;
        }

        if (arg_value && set_svr_mon_app_add_arg(mon_app, arg_value) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: load app %s: add arg value %s fail",
                set_svr_name(svr), app_name, arg_value);
            return -1;
        }
    }

    snprintf(set_id, sizeof(set_id), "%d", svr->m_set_id);

    if (set_svr_mon_app_add_arg(mon_app, "--pidfile") != 0
        || set_svr_mon_app_add_arg(mon_app, pidfile) != 0
        || set_svr_mon_app_add_arg(mon_app, "--root") != 0
        || set_svr_mon_app_add_arg(mon_app, app_root) != 0
        || set_svr_mon_app_add_arg(mon_app, "--app-id") != 0
        || set_svr_mon_app_add_arg(mon_app, set_id) != 0
        || set_svr_mon_app_add_arg(mon_app, "--log-dir") != 0
        || set_svr_mon_app_add_arg(mon_app, log_dir) != 0
        ) 
    {
        CPE_ERROR(svr->m_em, "%s: load app: add common arg fail", set_svr_name(svr));
        return -1;
    }

    if (set_svr_mon_app_create_svr_ins(mon_app, svr->m_set_id) != 0) return -1;

    CPE_INFO(svr->m_em, "%s: load app %s: success", set_svr_name(svr), app_name);

    return 0;
}

static int set_svr_app_init_load_local_env(set_svr_mon_t mon, cfg_t g_env) {
    set_svr_t svr = mon->m_svr;
    char path[128];

    snprintf(path, sizeof(path), "%s/env.yml", svr->m_repository_root);

    if (!file_exist(path, svr->m_em)) {
        CPE_INFO(svr->m_em, "%s: load env from %s: skip!", set_svr_name(svr), path);
        return 0;
    }

    if (cfg_yaml_read_file(g_env, gd_app_vfs_mgr(svr->m_app), path, cfg_replace, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: load env from %s: load cfg fail", set_svr_name(svr), path);
        return -1;
    }

    CPE_INFO(svr->m_em, "%s: load env from %s: success!", set_svr_name(svr), path);
    return 0;
}

