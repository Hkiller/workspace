#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_types.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "dr_store_internal_ops.h"

cpe_hash_string_t s_dr_store_manage_default_name;
struct nm_node_type s_nm_node_type_dr_store_manage;

dr_store_manage_t
dr_store_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    dr_store_manage_t mgr;
    nm_node_t mgr_node;

    if (name == 0) name = cpe_hs_data(s_dr_store_manage_default_name);

    if (em == NULL) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct dr_store_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (dr_store_manage_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;

    TAILQ_INIT(&mgr->m_refs);

    if (cpe_hash_table_init(
            &mgr->m_stores,
            alloc,
            (cpe_hash_fun_t) dr_store_hash,
            (cpe_hash_eq_t) dr_store_cmp,
            CPE_HASH_OBJ2ENTRY(dr_store, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_dr_store_manage);

    return mgr;
}

static void dr_store_manage_clear(nm_node_t node) {
    dr_store_manage_t mgr;
    mgr = (dr_store_manage_t)nm_node_data(node);

    dr_store_free_all(mgr);

    cpe_hash_table_fini(&mgr->m_stores);
}

void dr_store_manage_free(dr_store_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_dr_store_manage) return;
    nm_node_free(mgr_node);
}

dr_store_manage_t
dr_store_manage_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;

    if (name == NULL) name = s_dr_store_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dr_store_manage) return NULL;
    return (dr_store_manage_t)nm_node_data(node);
}

dr_store_manage_t
dr_store_manage_find_nc(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    if (name == NULL) return dr_store_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dr_store_manage) return NULL;
    return (dr_store_manage_t)nm_node_data(node);
}

dr_store_manage_t
dr_store_manage_default(gd_app_context_t app) {
    return dr_store_manage_find(app, s_dr_store_manage_default_name);
}

gd_app_context_t dr_store_manage_app(dr_store_manage_t mgr) {
    return mgr->m_app;
}

const char * dr_store_manage_name(dr_store_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
dr_store_manage_name_hs(dr_store_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

uint32_t dr_store_hash(const struct dr_store * store) {
    return cpe_hash_str(store->m_name, strlen(store->m_name));
}

int dr_store_cmp(const struct dr_store * l, const struct dr_store * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

LPDRMETA dr_store_manage_find_meta(dr_store_manage_t mgr, const char * lib_and_name) {
    dr_store_t store;
    const char * sep;
    char lib_name[64];
    LPDRMETA meta;
    
    sep = strchr(lib_and_name, '.');
    if (sep == NULL) {
        CPE_ERROR(mgr->m_em, "dr_store_manage_find_meta: name %s format error", lib_and_name);
        return NULL;
    }

    cpe_str_dup_range(lib_name, sizeof(lib_name), lib_and_name, sep);
    
    store = dr_store_find(mgr, lib_name);
    if (store == NULL) {
        CPE_ERROR(mgr->m_em, "dr_store_manage_find_meta: lib %s not exist", lib_name);
        return NULL;
    }

    meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
    if (meta == NULL) {
        CPE_ERROR(mgr->m_em, "dr_store_manage_find_meta: lib %s no meta %s", lib_name, sep + 1);
        return NULL;
    }

    return meta;
}

CPE_HS_DEF_VAR(s_dr_store_manage_default_name, "dr_store_manage");

struct nm_node_type s_nm_node_type_dr_store_manage = {
    "gd_dr_store_manage",
    dr_store_manage_clear
};

EXPORT_DIRECTIVE
int dr_store_manage_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    dr_store_manage_t dr_store_manage;

    dr_store_manage =
        dr_store_manage_create(
            app, gd_app_module_name(module), gd_app_alloc(app), gd_app_em(app));
    if (dr_store_manage == NULL) return -1;

    dr_store_manage->m_debug = cfg_get_int32(cfg, "debug", 0);

    return 0;
}

EXPORT_DIRECTIVE
void dr_store_manage_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dr_store_manage_t dr_store_manage;

    dr_store_manage = dr_store_manage_find_nc(app, gd_app_module_name(module));
    if (dr_store_manage) {
        dr_store_manage_free(dr_store_manage);
    }
}

