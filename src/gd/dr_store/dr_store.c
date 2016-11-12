#include <assert.h>
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "dr_store_internal_ops.h"

dr_store_t
dr_store_create(dr_store_manage_t mgr, const char * name) {
    char * buf;
    dr_store_t dr_store;
    size_t name_len;

    name_len = strlen(name) + 1;

    buf = mem_alloc(mgr->m_alloc, sizeof(struct dr_store) + name_len);
    if (buf == NULL) return NULL;

    memcpy(buf, name, name_len);

    dr_store = (dr_store_t)(buf + name_len);

    dr_store->m_mgr = mgr;
    dr_store->m_name = buf;
    dr_store->m_lib = NULL;
    dr_store->m_free_fun = NULL;
    dr_store->m_free_ctx = NULL;
    dr_store->m_ref_count = 1;

    cpe_hash_entry_init(&dr_store->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_stores, dr_store) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em,
            "%s: dr_store %s create success",
            dr_store_manage_name(mgr), name);
    }

    return dr_store;
}

void dr_store_free(dr_store_t dr_store) {
    dr_store_manage_t mgr;

    assert(dr_store);

    mgr = dr_store->m_mgr;
    assert(mgr);

    dr_store_reset_lib(dr_store, NULL, NULL, NULL);

    --dr_store->m_ref_count;
    if (dr_store->m_ref_count > 0) {
        if (mgr->m_debug) {
            CPE_INFO(
                mgr->m_em, "%s: dr_store %s free delay, %d ref remain!",
                dr_store_manage_name(mgr), dr_store->m_name, dr_store->m_ref_count);
        }
    }
    else {
        if (mgr->m_debug) {
            CPE_INFO(
                mgr->m_em, "%s: dr_store %s free success!",
                dr_store_manage_name(mgr), dr_store->m_name);
        }

        cpe_hash_table_remove_by_ins(&mgr->m_stores, dr_store);
        mem_free(mgr->m_alloc, (void*)dr_store->m_name);
    }
}

void dr_store_free_all(dr_store_manage_t mgr) {
    struct cpe_hash_it dr_store_it;
    dr_store_t dr_store;

    cpe_hash_it_init(&dr_store_it, &mgr->m_stores);

    dr_store = cpe_hash_it_next(&dr_store_it);
    while (dr_store) {
        dr_store_t next = cpe_hash_it_next(&dr_store_it);
        dr_store_free(dr_store);
        dr_store = next;
    }
}

void dr_store_add_ref(dr_store_t dr_store) {
    dr_store_manage_t mgr;

    assert(dr_store);

    mgr = dr_store->m_mgr;
    assert(mgr);

    dr_store->m_ref_count++;

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: dr_store %s add ref, ref-count=%d!",
            dr_store_manage_name(mgr), dr_store->m_name, dr_store->m_ref_count);
    }
}

void dr_store_remove_ref(dr_store_t dr_store) {
    dr_store_manage_t mgr;

    assert(dr_store);

    mgr = dr_store->m_mgr;
    assert(mgr);

    --dr_store->m_ref_count;

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: dr_store %s remove ref, ref-count=%d!",
            dr_store_manage_name(mgr), dr_store->m_name, dr_store->m_ref_count);
    }

    if (dr_store->m_ref_count <= 0) {
        assert(dr_store->m_lib == NULL);

        if (mgr->m_debug) {
            CPE_INFO(
                mgr->m_em, "%s: dr_store %s free delay complete!",
                dr_store_manage_name(mgr), dr_store->m_name);
        }

        cpe_hash_table_remove_by_ins(&mgr->m_stores, dr_store);
        mem_free(mgr->m_alloc, (void*)dr_store->m_name);
    }
}

dr_store_t
dr_store_find(dr_store_manage_t mgr, const char * name) {
    struct dr_store key;

    key.m_name = name;
    return (dr_store_t)cpe_hash_table_find(&mgr->m_stores, &key);
}

dr_store_t
dr_store_find_or_create(dr_store_manage_t mgr, const char * name) {
    dr_store_t r = dr_store_find(mgr, name);
    if (r == NULL) r = dr_store_create(mgr, name);
    return r;
}

LPDRMETALIB dr_store_lib(dr_store_t dr_store) {
    return dr_store->m_lib;
}

int dr_store_set_lib(dr_store_t dr_store, LPDRMETALIB lib, dr_lib_free_fun_t free_fun, void * free_ctx) {
    if (dr_store->m_lib) return -1;

    dr_store->m_lib = lib;
    dr_store->m_free_fun = free_fun;
    dr_store->m_free_ctx = free_ctx;

    return 0;
}

void dr_store_reset_lib(dr_store_t dr_store, LPDRMETALIB lib, dr_lib_free_fun_t free_fun, void * free_ctx) {
    if (dr_store->m_lib && dr_store->m_free_fun) {
        dr_store->m_free_fun(dr_store->m_lib, dr_store->m_free_ctx);
    }

    dr_store->m_lib = lib;
    dr_store->m_free_fun = free_fun;
    dr_store->m_free_ctx = free_ctx;
}

int dr_store_add_lib(
    dr_store_manage_t mgr, const char * name,
    LPDRMETALIB lib, dr_lib_free_fun_t free_fun, void * free_ctx)
{
    dr_store_t store;
    store = dr_store_find_or_create(mgr, name);
    if (store == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: create meta lib %s: create fail!",
            dr_store_manage_name(mgr), name);
        return -1;
    } 
    
    if (dr_store_set_lib(store, lib, free_fun, free_ctx) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: create meta lib %s: meta lib already loaded!",
            dr_store_manage_name(mgr), name);
        return -1;
    }

    return 0;
}
