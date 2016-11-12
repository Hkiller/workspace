#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_log.h"
#include "set_svr_mon_app.h"
#include "set_svr_svr_type.h"

static int set_svr_app_init_calc_repository(
    gd_app_context_t app, const char * set_name,
    error_monitor_t em, char * buf, size_t buf_capacity);

int set_svr_app_init_center(set_svr_center_t center, cfg_t cfg, const char * center_addr);
int set_svr_app_init_listener(struct set_svr_listener * listener, cfg_t cfg, const char * listener_addr);
int set_svr_app_init_set(set_svr_t svr, cfg_t cfg);

static int set_svr_app_init_load_svr_types(set_svr_t svr) {
    struct cfg_it svr_it;
    cfg_t svr_cfg;

    cfg_it_init(&svr_it, cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types"));
    while((svr_cfg = cfg_it_next(&svr_it))) {
        set_svr_svr_type_t set_svr_type;
        
        set_svr_type = set_svr_svr_type_create(svr, cfg_name(svr_cfg), svr_cfg);
        if (set_svr_type == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create: load_svr_types: check_create %s fail!",
                set_svr_name(svr), cfg_name(svr_cfg));
            return -1;
        }
        else {
            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: create: load_svr_types: create svr type %s(%d), scope=%d!",
                    set_svr_name(svr), set_svr_type->m_svr_type_name, set_svr_type->m_svr_type_id,
                    set_svr_type->m_svr_scope);
            }
        }
    }

    return 0;
}

EXPORT_DIRECTIVE
int set_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_t set_svr;
    const char * set_name;
    uint16_t set_id;
    uint16_t region;
    const char * value;
    const char * str_ringbuf_size;
    uint64_t ringbuf_size;
    char repository_root_buf[128];
    const char * center_addr;
    const char * listener_addr;
    cfg_t set_cfg;

    if ((set_name = gd_app_arg_find(app, "--set-name")) == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: --set-name not configured in command!", gd_app_module_name(module));
        return -1;
    }

    set_cfg = cfg_find_cfg(cfg_find_cfg(gd_app_cfg(app), "sets"), set_name);
    if (set_cfg == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: set %s cfg not exist!", gd_app_module_name(module), set_name);
        return -1;
    }

    if ((set_id = cfg_get_uint16(set_cfg, "id", 0)) == 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set %s id not configured!", gd_app_module_name(module), set_name);
        return -1;
    }

    region = cfg_get_uint16(set_cfg, "region", 0); 
    center_addr = cfg_get_string(set_cfg, "center", NULL);
    listener_addr = cfg_get_string(set_cfg, "listener", NULL);

    value = gd_app_arg_find(app, "--repository");
    if (value) {
        snprintf(repository_root_buf, sizeof(repository_root_buf), "%s", value);
    }
    else {
        if (set_svr_app_init_calc_repository(app, set_name, gd_app_em(app), repository_root_buf, sizeof(repository_root_buf)) != 0) {
            APP_CTX_ERROR(app, "%s: create: --repository get fail!", gd_app_module_name(module));
            return -1;
        }
    }

    if (dir_mk_recursion(repository_root_buf, DIR_DEFAULT_MODE, gd_app_em(app), gd_app_alloc(app)) != 0) {
        APP_CTX_ERROR(app, "%s: create: repository_root %s create fail!", gd_app_module_name(module), repository_root_buf);
        return -1;
    }
    
    if ((str_ringbuf_size = cfg_get_string(cfg, "ringbuf-size", NULL)) == NULL) {
        APP_CTX_ERROR(app, "%s: create: ringbuf-size not configured!", gd_app_module_name(module));
        return -1;
    }

    if (cpe_str_parse_byte_size(&ringbuf_size, str_ringbuf_size) != 0) {
        APP_CTX_ERROR(
            app, "%s: create: read ringbuf-size %s fail!",
            gd_app_module_name(module), str_ringbuf_size);
        return -1;
    }

    set_svr =
        set_svr_create(
            app, gd_app_module_name(module), repository_root_buf, set_id, region,
            gd_app_alloc(app), gd_app_em(app));
    if (set_svr == NULL) return -1;

    set_svr->m_debug = cfg_get_int8(cfg, "debug", set_svr->m_debug);
    set_svr->m_mon->m_debug = cfg_get_int32(cfg, "mon.debug", 0);

    if (set_svr_set_ringbuf_size(set_svr, ringbuf_size) != 0) {
        CPE_ERROR(gd_app_em(app), "%s: create: set ringbuf-size %d fail!", gd_app_module_name(module), (int)ringbuf_size);
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr_app_init_listener(set_svr->m_listener, cfg_find_cfg(cfg, "listener"), listener_addr) != 0) {
        set_svr_free(set_svr);
        return -1;
    }
    
    if (set_svr_app_init_center(set_svr->m_center, cfg_find_cfg(cfg, "center"), center_addr) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr_app_init_set(set_svr, cfg_find_cfg(cfg, "set")) != 0) {
        set_svr_free(set_svr);
        return -1;
    }
    
    if (set_svr_app_init_load_svr_types(set_svr) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    set_svr->m_mon->m_restart_wait_ms = (tl_time_span_t)cfg_get_uint32(cfg, "mon.restart-wait-ms", 1000);
    if (set_svr_app_init_mon(set_svr->m_mon, set_cfg) != 0) {
        set_svr_free(set_svr);
        return -1;
    }

    if (set_svr->m_debug) {
        CPE_INFO(
            set_svr->m_em, "%s: creat: success, repository_root=%s, set-name=%s, center=%s, ringbuf-size=%fmb",
            set_svr_name(set_svr), repository_root_buf, set_name, center_addr, ((float)ringbuf_size) / 1024.0f / 1024.0f);
    }

    set_svr_mon_app_start_all(set_svr->m_mon);

    return 0;
}

EXPORT_DIRECTIVE
void set_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    set_svr_t set_svr;

    set_svr = set_svr_find_nc(app, gd_app_module_name(module));
    if (set_svr) {
        set_svr_free(set_svr);
    }
}

int set_svr_app_init_calc_repository(
    gd_app_context_t app, const char * set_name,
    error_monitor_t em, char * buf, size_t buf_capacity)
{
    struct cpe_str_buf str_buf = CPE_STR_BUF_INIT(buf, buf_capacity);
    const char * app_bin = gd_app_argv(app)[0];
    const char * dir_end_1 = NULL;
    const char * dir_end_2 = NULL;
    const char * dir_end_3 = NULL;
    const char * dir_end_4 = NULL;
    const char * p;
    const char * home;
    size_t home_len;

    for(p = strchr(app_bin, '/'); p; p = strchr(p + 1, '/')) {
        dir_end_1 = dir_end_2;
        dir_end_2 = dir_end_3;
        dir_end_3 = dir_end_4;
        dir_end_4 = p;
    }

    if (dir_end_1 == NULL || dir_end_2 == NULL) {
        CPE_ERROR(em, "set_repository_root: root %s format error!", app_bin);
        return -1;
    }

    home = getenv("HOME");
    home_len = strlen(home);

    cpe_str_buf_append(&str_buf, home, home_len);
    if (home[home_len - 1] != '/') {
        cpe_str_buf_cat(&str_buf, "/");
    }
    cpe_str_buf_cat(&str_buf, ".");
    cpe_str_buf_append(&str_buf, dir_end_1 + 1, dir_end_2 - dir_end_1 - 1);
    cpe_str_buf_cat_printf(&str_buf, "/%s", set_name);

    return cpe_str_buf_is_overflow(&str_buf) ? -1 : 0;
}

