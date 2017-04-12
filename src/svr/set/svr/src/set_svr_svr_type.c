#include <assert.h>
#include "cpe/cfg/cfg_read.h" 
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "svr/center/agent/center_agent.h"
#include "svr/center/agent/center_agent_svr_type.h"
#include "set_svr_svr_type.h"
#include "set_svr_svr_ins.h"

set_svr_svr_type_t
set_svr_svr_type_create(set_svr_t svr, const char * svr_type_name, cfg_t svr_cfg) {
    uint16_t svr_type_id;
    set_svr_svr_type_t type;
    size_t name_len = strlen(svr_type_name) + 1;
    const char * str_pkg_meta;
    LPDRMETA pkg_meta = NULL;
    const char * str_scope;
    set_svr_scope_t scope;

    str_scope = cfg_get_string(svr_cfg, "scope", NULL);
    if (str_scope == NULL) {
        CPE_ERROR(svr->m_em, "%s: create svr type %s: scope not configured!", set_svr_name(svr), svr_type_name);
        return NULL;
    }

    if (strcmp(str_scope, "global") == 0) {
        scope = set_svr_scope_global;
    }
    else if (strcmp(str_scope, "region") == 0) {
        scope = set_svr_scope_region;
    }
    else if (strcmp(str_scope, "set") == 0) {
        scope = set_svr_scope_set;
    }
    else {
        CPE_ERROR(svr->m_em, "%s: create svr type %s: scope %s not exist!", set_svr_name(svr), svr_type_name, str_scope);
        return NULL;
    }

    if (cfg_try_get_uint16(svr_cfg, "id", &svr_type_id) != 0) {
        CPE_ERROR(svr->m_em, "%s: create svr type %s: id not configured!", set_svr_name(svr), svr_type_name);
        return NULL;
    }

    if ((str_pkg_meta = cfg_get_string(svr_cfg, "pkg-meta", NULL))) {
        dr_store_manage_t store_mgr;
        dr_store_t store;
        char const * sep;
        char lib_name[64];

        store_mgr = dr_store_manage_find(svr->m_app, NULL);
        if (store_mgr == NULL) {
            CPE_ERROR(svr->m_em, "%s: create svr type %s: default store_mgr not exist!", set_svr_name(svr), svr_type_name);
            return NULL;
        }

        sep = strchr(str_pkg_meta, '.');
        if (sep == NULL || (sep - str_pkg_meta) > (sizeof(lib_name) - 1)) {
            CPE_ERROR(svr->m_em, "%s: create svr type %s: pkg-meta %s format error or overflow!", set_svr_name(svr), svr_type_name, str_pkg_meta);
            return NULL;
        }
        memcpy(lib_name, str_pkg_meta, sep - str_pkg_meta);
        lib_name[sep - str_pkg_meta] = 0;

        store = dr_store_find(store_mgr, lib_name);
        if (store == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create svr type %s: metalib %s not exist in %s!",
                set_svr_name(svr), svr_type_name, lib_name, dr_store_manage_name(store_mgr));
            return NULL;
        }

        pkg_meta = dr_lib_find_meta_by_name(dr_store_lib(store), sep + 1);
        if (pkg_meta == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create svr type %s: metalib %s have no meta %s!",
                set_svr_name(svr), svr_type_name, svr_type_name, sep + 1);
            return NULL;
        }
    }

    type = mem_alloc(svr->m_alloc, sizeof(struct set_svr_svr_type) + name_len);
    if (type == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: create type of svr type %d: malloc fail!",
            set_svr_name(svr), svr_type_id);
        return NULL;
    }

    type->m_svr = svr;
    type->m_svr_type_id = svr_type_id;
    type->m_svr_scope = scope;
    type->m_svr_type_name = (char*)(type + 1);
    type->m_pkg_meta = pkg_meta;

    TAILQ_INIT(&type->m_runing_inses);

    memcpy(type->m_svr_type_name, svr_type_name, name_len);

    cpe_hash_entry_init(&type->m_hh_by_id);
    if (cpe_hash_table_insert_unique(&svr->m_svr_types_by_id, type) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create svr type %s(%d): id duplicate!",
            set_svr_name(svr), type->m_svr_type_name, type->m_svr_type_id);
        mem_free(svr->m_alloc, type);
        return NULL;
    }

    cpe_hash_entry_init(&type->m_hh_by_name);
    if (cpe_hash_table_insert_unique(&svr->m_svr_types_by_name, type) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create svr type %s(%d): name duplicate!",
            set_svr_name(svr), type->m_svr_type_name, type->m_svr_type_id);
        cpe_hash_table_remove_by_ins(&svr->m_svr_types_by_id, type);
        mem_free(svr->m_alloc, type);
        return NULL;
    }

    return type;
}

void set_svr_svr_type_free(set_svr_svr_type_t type) {
    set_svr_t svr = type->m_svr;

    while(!TAILQ_EMPTY(&type->m_runing_inses)) {
        set_svr_svr_ins_free(TAILQ_FIRST(&type->m_runing_inses));
    }

    cpe_hash_table_remove_by_ins(&svr->m_svr_types_by_id, type);
    cpe_hash_table_remove_by_ins(&svr->m_svr_types_by_name, type);

    mem_free(svr->m_alloc, type);
}

void set_svr_svr_type_free_all(set_svr_t svr) {
    struct cpe_hash_it type_it;
    set_svr_svr_type_t type;

    cpe_hash_it_init(&type_it, &svr->m_svr_types_by_id);

    type = cpe_hash_it_next(&type_it);
    while(type) {
        set_svr_svr_type_t next = cpe_hash_it_next(&type_it);
        set_svr_svr_type_free(type);
        type = next;
    }
}

set_svr_svr_type_t set_svr_svr_type_find_by_id(set_svr_t svr, uint16_t svr_type_id) {
    struct set_svr_svr_type key;
    key.m_svr_type_id = svr_type_id;
    return cpe_hash_table_find(&svr->m_svr_types_by_id, &key);
}

set_svr_svr_type_t set_svr_svr_type_find_by_name(set_svr_t svr, const char * svr_type_name) {
    struct set_svr_svr_type key;
    key.m_svr_type_name = (char *)svr_type_name;
    return cpe_hash_table_find(&svr->m_svr_types_by_name, &key);
}

uint32_t set_svr_svr_type_hash_by_id(set_svr_svr_type_t o) {
    return o->m_svr_type_id;
}

int set_svr_svr_type_eq_by_id(set_svr_svr_type_t l, set_svr_svr_type_t r) {
    return l->m_svr_type_id == r->m_svr_type_id;
}

uint32_t set_svr_svr_type_hash_by_name(set_svr_svr_type_t o) {
    return cpe_hash_str(o->m_svr_type_name, strlen(o->m_svr_type_name));
}

int set_svr_svr_type_eq_by_name(set_svr_svr_type_t l, set_svr_svr_type_t r) {
    return strcmp(l->m_svr_type_name, r->m_svr_type_name) == 0;
}
