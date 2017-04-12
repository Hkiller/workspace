#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_dm/dr_dm_manage.h"
#include "dr_dm_internal_ops.h"

struct nm_node_type s_nm_node_type_dr_dm_manage;
cpe_hash_string_t s_dr_dm_manage_default_name;

dr_dm_manage_t
dr_dm_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    dr_dm_manage_t mgr;
    nm_node_t mgr_node;

    assert(app);
    assert(name);

    if (em == NULL) em = gd_app_em(app);

    mgr_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct dr_dm_manage));
    if (mgr_node == NULL) return NULL;

    mgr = (dr_dm_manage_t)nm_node_data(mgr_node);
    mgr->m_alloc = alloc;
    mgr->m_app = app;
    mgr->m_em = em;
    mgr->m_key_buf = NULL;
    mgr->m_debug = 0;

    mgr->m_metalib = NULL;
    mgr->m_role_meta = NULL;
    mgr->m_id_generate = NULL;
    mgr->m_id_index = NULL;

    if (cpe_hash_table_init(
            &mgr->m_indexes,
            mgr->m_alloc,
            (cpe_hash_fun_t) dr_dm_data_index_hash,
            (cpe_hash_eq_t) dr_dm_data_index_cmp,
            CPE_HASH_OBJ2ENTRY(dr_dm_data_index, m_hh),
            -1) != 0)
    {
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_dr_dm_manage);

    return mgr;
}

static void dr_dm_manage_clear(nm_node_t node) {
    dr_dm_manage_t mgr;
    mgr = (dr_dm_manage_t)nm_node_data(node);

    if (mgr->m_key_buf) mem_free(mgr->m_alloc, mgr->m_key_buf);

    dr_dm_data_index_free_all(mgr);
    cpe_hash_table_fini(&mgr->m_indexes);
    mgr->m_id_index = NULL;
}

void dr_dm_manage_free(dr_dm_manage_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_dr_dm_manage) return;
    nm_node_free(mgr_node);
}

dr_dm_manage_t
dr_dm_manage_find(gd_app_context_t app, cpe_hash_string_t name) {
	nm_node_t node;

    if (name == NULL) name = s_dr_dm_manage_default_name;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dr_dm_manage) return NULL;
    return (dr_dm_manage_t)nm_node_data(node);
}

dr_dm_manage_t
dr_dm_manage_find_nc(gd_app_context_t app, const char * name) {
	nm_node_t node;

    if (name == NULL) return dr_dm_manage_default(app);

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dr_dm_manage) return NULL;
    return (dr_dm_manage_t)nm_node_data(node);
}

dr_dm_manage_t
dr_dm_manage_default(gd_app_context_t app) {
    return dr_dm_manage_find(app, s_dr_dm_manage_default_name);
}

gd_app_context_t dr_dm_manage_app(dr_dm_manage_t mgr) {
    return mgr->m_app;
}

const char * dr_dm_manage_name(dr_dm_manage_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
dr_dm_manage_name_hs(dr_dm_manage_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

LPDRMETA dr_dm_manage_meta(dr_dm_manage_t mgr) {
    return mgr->m_role_meta;
}

int dr_dm_manage_set_meta(dr_dm_manage_t mgr, LPDRMETA meta, dr_ref_t metalib) {
    if (!dr_dm_manage_is_empty(mgr)) {
        CPE_ERROR(
            mgr->m_em, "%s: set_meta: already have data!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    if (mgr->m_metalib) dr_ref_free(mgr->m_metalib);

    mgr->m_metalib = metalib;
    mgr->m_role_meta = meta;

    if (mgr->m_id_index) dr_dm_data_index_free(mgr, mgr->m_id_index);
    assert(mgr->m_id_index == NULL);

    if (mgr->m_key_buf) {
        mem_free(mgr->m_alloc, mgr->m_key_buf);
        mgr->m_key_buf = NULL;
    }

    return 0;
}

int dr_dm_manage_set_id_attr(dr_dm_manage_t mgr, const char * id_attr_name) {
    LPDRMETAENTRY entry;

    if (!dr_dm_manage_is_empty(mgr)) {
        CPE_ERROR(
            mgr->m_em, "%s: set_id_attr: already have data!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    if (mgr->m_key_buf) {
        mem_free(mgr->m_alloc, mgr->m_key_buf);
        mgr->m_key_buf = NULL;
    }

    if (id_attr_name) {
        if (mgr->m_role_meta == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s: set_id_attr: meta not exist!",
                dr_dm_manage_name(mgr));
            return -1;
        }

        entry = dr_meta_find_entry_by_name(mgr->m_role_meta, id_attr_name);
        if (entry == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s: set_id_attr: meta %s have no entry %s!",
                dr_dm_manage_name(mgr), dr_meta_name(mgr->m_role_meta), id_attr_name);
            return -1;
        }

        if (mgr->m_id_index) dr_dm_data_index_free(mgr, mgr->m_id_index);

        mgr->m_id_index = dr_dm_data_index_create(mgr, entry, 1);
        if (mgr->m_id_index == NULL) {
            CPE_ERROR(
                mgr->m_em, "%s: set_id_attr: create id_index fail!",
                dr_dm_manage_name(mgr));
            return -1;
        }

        return 0;
    }
    else {
        if (mgr->m_id_index) dr_dm_data_index_free(mgr, mgr->m_id_index);
        assert(mgr->m_id_index == NULL);
        return 0;
    }
}

LPDRMETAENTRY dr_dm_manage_id_attr(dr_dm_manage_t mgr) {
    return mgr->m_id_index ? mgr->m_id_index->m_entry : NULL;
}

void dr_dm_manage_set_id_generate(dr_dm_manage_t mgr, gd_id_generator_t id_generate) {
    mgr->m_id_generate = id_generate; 
}

gd_id_generator_t dr_dm_manage_id_generate(dr_dm_manage_t mgr) {
    return mgr->m_id_generate;
}

int dr_dm_manage_create_index(dr_dm_manage_t mgr, const char * name, int is_uniqure) {
    LPDRMETAENTRY entry;

    if (!dr_dm_manage_is_empty(mgr)) {
        CPE_ERROR(
            mgr->m_em, "%s: create_index: already have data!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    if (mgr->m_role_meta == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create_index: meta not exist!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    entry = dr_meta_find_entry_by_name(mgr->m_role_meta, name);
    if (entry == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create_index: meta %s have no entry %s!",
            dr_dm_manage_name(mgr), dr_meta_name(mgr->m_role_meta), name);
        return -1;
    }

    if (dr_dm_data_index_create(mgr, entry, is_uniqure) == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create_index: create index fail!",
            dr_dm_manage_name(mgr));
        return -1;
    }

    if (mgr->m_key_buf) {
        mem_free(mgr->m_alloc, mgr->m_key_buf);
        mgr->m_key_buf = NULL;
    }

    return 0;
}

dr_dm_data_t dr_dm_manage_key_buf(dr_dm_manage_t mgr) {
    if (mgr->m_key_buf == NULL) {
        mgr->m_key_buf = (dr_dm_data_t)mem_alloc(
            mgr->m_alloc,
            sizeof(struct dr_dm_data) + dr_meta_size(mgr->m_role_meta));
    }

    return mgr->m_key_buf;
}

CPE_HS_DEF_VAR(s_dr_dm_manage_default_name, "dr_dm_manage");

struct nm_node_type s_nm_node_type_dr_dm_manage = {
    "usf_dr_dm_manage",
    dr_dm_manage_clear
};

