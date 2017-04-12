#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "dblog_svr_i.h"
#include "dblog_svr_meta.h"

extern char g_metalib_svr_dblog_pro[];
static int dblog_svr_load_metas(dblog_svr_t svr, cfg_t cfg);
static int dblog_svr_load_listen(dblog_svr_t svr, const char * value);

EXPORT_DIRECTIVE
int dblog_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    dblog_svr_t dblog_svr;
    mongo_driver_t db;    
    const char * str_listen;
    const char * str_db_ns;

    str_listen = gd_app_arg_find(app, "--listen");
    if (str_listen == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: listen not configured", gd_app_module_name(module));
        return -1;
    }

    str_db_ns = gd_app_arg_find(app, "--db-ns");
    if (str_db_ns == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: db-ns not configured", gd_app_module_name(module));
        return -1;
    }
    
    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    db = mongo_driver_find_nc(app, cfg_get_string(cfg, "db", NULL));
    if (db == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: db %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "db", "default"));
        return -1;
    }

    dblog_svr = dblog_svr_create(app, gd_app_module_name(module), stub, db, gd_app_alloc(app), gd_app_em(app));
    if (dblog_svr == NULL) return -1;

    cpe_str_dup(dblog_svr->m_db_ns, sizeof(dblog_svr->m_db_ns), str_db_ns);
    dblog_svr->m_debug = cfg_get_int8(cfg, "debug", dblog_svr->m_debug);

    if (dblog_svr_load_metas(dblog_svr, cfg_find_cfg(gd_app_cfg(app), "metalibs")) != 0
        || dblog_svr_load_listen(dblog_svr, str_listen) != 0)
    {
        dblog_svr_free(dblog_svr);
        return -1;
    }

    if (dblog_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done, listen at %s:%d.", gd_app_module_name(module), dblog_svr->m_ip, dblog_svr->m_port);
    }

    return 0;
}

EXPORT_DIRECTIVE
void dblog_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dblog_svr_t dblog_svr;

    dblog_svr = dblog_svr_find_nc(app, gd_app_module_name(module));
    if (dblog_svr) {
        dblog_svr_free(dblog_svr);
    }
}

static cfg_t dblog_svr_info_find_by_type(dblog_svr_t svr, const char * svr_type) {
    struct cfg_it child_it;
    cfg_t svr_cfg;

    cfg_it_init(&child_it, cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types"));
    while((svr_cfg = cfg_it_next(&child_it))) {
        if (strcmp(cfg_name(svr_cfg), svr_type) == 0) return svr_cfg;
    }

    return NULL;
}

static int dblog_svr_load_meta(
    dblog_svr_t svr, dr_store_manage_t store_mgr,
    const char * svr_type_name, uint16_t svr_type, const char * metalib_path)
{
    dr_store_t store;
    LPDRMETALIB metalib;
    int i;
    
    if (dr_store_manage_load_from_bin(store_mgr, svr_type_name, metalib_path) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create: load meta: load metalib from %s fail!",
            dblog_svr_name(svr), metalib_path);
        return -1;
    }
    
    store = dr_store_find_or_create(store_mgr, svr_type_name);
    assert(store);

    metalib = dr_store_lib(store);
    assert(metalib);

    for(i = 0; i < dr_lib_meta_num(metalib); ++i) {
        LPDRMETA meta = dr_lib_meta_at(metalib, i);

        if (dr_meta_id(meta)  == 0) continue;

        if (dblog_svr_meta_create(svr, svr_type, svr_type_name, meta) == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: load meta %s: create fail!", dblog_svr_name(svr), dr_meta_name(meta));
            return -1;
        }
    }

    return 0;
}
    
static int dblog_svr_load_metas(dblog_svr_t svr, cfg_t cfg) {
    struct cfg_it child_it;
    cfg_t meta_cfg;
    dr_store_manage_t store_mgr;

    store_mgr = dr_store_manage_find_nc(svr->m_app, NULL);
    if (store_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: create: load metas: store mgr not exist!", dblog_svr_name(svr));
        return -1;
    }

    cfg_it_init(&child_it, cfg);
    while((meta_cfg = cfg_it_next(&child_it))) {
        const char * svr_type = cfg_get_string(meta_cfg, "svr-type", NULL);
        const char * metalib_path = cfg_get_string(meta_cfg, "metalib", NULL);
        cfg_t svr_info ;

        if (svr_type == NULL || metalib_path == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: load metas: meta def format error!", dblog_svr_name(svr));
            return -1;
        }

        svr_info = dblog_svr_info_find_by_type(svr, svr_type);
        if (svr_info == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: load metas: svr type %s unknown!", dblog_svr_name(svr), svr_type);
            return -1;
        }

        if (dblog_svr_load_meta(svr, store_mgr, svr_type, cfg_get_uint16(svr_info, "id", (uint16_t)0), metalib_path) != 0) {
            return -1;
        }
    }
    
    return 0;
}

static int dblog_svr_load_listen(dblog_svr_t svr, const char * value) {
    const char * sep;
    const char * host;
    int port;
    char host_buf[64];
    
    if ((sep = strchr(value, ':'))) {
        host = cpe_str_dup_range(host_buf, sizeof(host_buf), value, sep);
        if (host == NULL) {
            CPE_ERROR(svr->m_em, "%s: create: target %s host too long!", dblog_svr_name(svr), value);
            return -1;
        }
        port = atoi(sep + 1);
    }
    else {
        host = "";
        port = atoi(value);
    }
    
    if (dblog_svr_listen(svr, host, port) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: set target %s:%d fail!", dblog_svr_name(svr), host, port);
        return -1;
    }

    return 0;
}
