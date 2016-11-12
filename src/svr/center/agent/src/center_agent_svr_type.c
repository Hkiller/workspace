#include <assert.h> 
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_store/dr_store.h"
#include "gd/app/app_context.h"
#include "svr/center/agent/center_agent.h"
#include "svr/center/agent/center_agent_svr_type.h"
#include "center_agent_internal_ops.h"

center_agent_svr_type_t
center_agent_svr_type_create(center_agent_t agent, const char * svr_type_name) {
    cfg_t svr_cfg;
    uint16_t svr_type_id;
    center_agent_svr_type_t type;
    size_t name_len = strlen(svr_type_name) + 1;
    const char * str_pkg_meta;
    LPDRMETA pkg_meta = NULL;

    svr_cfg = cfg_find_cfg(cfg_find_cfg(gd_app_cfg(agent->m_app), "svr_types"), svr_type_name);
    if (svr_cfg == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: create type of svr type %s: no config!",
            center_agent_name(agent), svr_type_name);
    }

    if (cfg_try_get_uint16(svr_cfg, "id", &svr_type_id) != 0) {
        CPE_ERROR(agent->m_em, "%s: create svr type %s: id not configured!", center_agent_name(agent), svr_type_name);
        return NULL;
    }

    if ((str_pkg_meta = cfg_get_string(svr_cfg, "pkg-meta", NULL))) {
        dr_store_manage_t store_mgr;
        dr_store_t store;
        char const * sep;
        char lib_name[64];

        store_mgr = dr_store_manage_find(center_agent_app(agent), NULL);
        if (store_mgr == NULL) {
            CPE_ERROR(agent->m_em, "%s: create svr type %s: default store_mgr not exist!", center_agent_name(agent), svr_type_name);
            return NULL;
        }

        sep = strchr(str_pkg_meta, '.');
        if (sep == NULL || (sep - str_pkg_meta) > (sizeof(lib_name) - 1)) {
            CPE_ERROR(agent->m_em, "%s: create svr type %s: pkg-meta %s format error or overflow!", center_agent_name(agent), svr_type_name, str_pkg_meta);
            return NULL;
        }
        memcpy(lib_name, str_pkg_meta, sep - str_pkg_meta);
        lib_name[sep - str_pkg_meta] = 0;

        store = dr_store_find(store_mgr, lib_name);
        if (store == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: create svr type %s: metalib %s not exist in %s!",
                center_agent_name(agent), svr_type_name, lib_name, dr_store_manage_name(store_mgr));
            return NULL;
        }

        pkg_meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
        if (pkg_meta == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: create svr type %s: metalib %s have no meta %s!",
                center_agent_name(agent), svr_type_name, svr_type_name, sep + 1);
            return NULL;
        }
    }

    type = mem_alloc(agent->m_alloc, sizeof(struct center_agent_svr_type) + name_len);
    if (type == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: create type of svr type %d: malloc fail!",
            center_agent_name(agent), svr_type_id);
        return NULL;
    }

    type->m_agent = agent;
    type->m_svr_type_id = svr_type_id;
    type->m_svr_type_name = (char*)(type + 1);
    type->m_pkg_meta = pkg_meta;

    memcpy(type->m_svr_type_name, svr_type_name, name_len);

    cpe_hash_entry_init(&type->m_hh);
    if (cpe_hash_table_insert_unique(&agent->m_svr_types, type) != 0) {
        CPE_ERROR(
            agent->m_em, "%s: create type of svr type %d: insert fail!",
            center_agent_name(agent), svr_type_id);
        mem_free(agent->m_alloc, type);
        return NULL;
    }

    return type;
}

void center_agent_svr_type_free(center_agent_svr_type_t type) {
    center_agent_t agent = type->m_agent;

    assert(agent);

    cpe_hash_table_remove_by_ins(&agent->m_svr_types, type);

    mem_free(agent->m_alloc, type);
}

void center_agent_svr_type_free_all(center_agent_t agent) {
    struct cpe_hash_it type_it;
    center_agent_svr_type_t type;

    cpe_hash_it_init(&type_it, &agent->m_svr_types);

    type = cpe_hash_it_next(&type_it);
    while(type) {
        center_agent_svr_type_t next = cpe_hash_it_next(&type_it);
        center_agent_svr_type_free(type);
        type = next;
    }
}

const char * center_agent_svr_type_name(center_agent_svr_type_t svr_type) {
    return svr_type->m_svr_type_name;
}

uint16_t center_agent_svr_type_id(center_agent_svr_type_t svr_type) {
    return svr_type->m_svr_type_id;
}

center_agent_svr_type_t
center_agent_svr_type_find(center_agent_t agent, uint16_t svr_type) {
    struct center_agent_svr_type key;

    key.m_svr_type_id = svr_type;
    return cpe_hash_table_find(&agent->m_svr_types, &key);
}

center_agent_svr_type_t
center_agent_svr_type_lsearch_by_name(center_agent_t agent, const char * svr_type_name) {
    struct cpe_hash_it type_it;
    center_agent_svr_type_t type;

    cpe_hash_it_init(&type_it, &agent->m_svr_types);

    while((type = cpe_hash_it_next(&type_it))) {
        if (strcmp(type->m_svr_type_name, svr_type_name) ==0) return type; 
    }

    return NULL;
}

LPDRMETA center_agent_svr_type_pkg_meta(center_agent_svr_type_t type) {
    return type->m_pkg_meta;
}

center_agent_svr_type_t center_agent_svr_type_check_create(center_agent_t agent, const char * svr_type_name) {
    center_agent_svr_type_t svr_type;

    svr_type = center_agent_svr_type_lsearch_by_name(agent, svr_type_name);
    if (svr_type == NULL) { 
        svr_type = center_agent_svr_type_create(agent, svr_type_name);
    }

    return svr_type;
}

cfg_t center_agent_svr_type_cfg(center_agent_svr_type_t svr_type) {
    return cfg_find_cfg(cfg_find_cfg(gd_app_cfg(svr_type->m_agent->m_app), "svr_types"), svr_type->m_svr_type_name);
}

uint32_t center_agent_svr_type_hash(center_agent_svr_type_t type) {
    return type->m_svr_type_id;
}

int center_agent_svr_type_eq(center_agent_svr_type_t l, center_agent_svr_type_t r) {
    return l->m_svr_type_id == r->m_svr_type_id;
}
