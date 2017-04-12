#include <assert.h>
#include "cpe/cfg/cfg_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store.h"
#include "dr_store_internal_ops.h"

dr_ref_t
dr_ref_create(dr_store_manage_t mgr, const char * name) {
    dr_ref_t dr_ref;
    dr_store_t dr_store;

    assert(mgr);
    if (mgr == NULL) return NULL;

    dr_store = dr_store_find_or_create(mgr, name);
    if (dr_store == NULL) return NULL;

    dr_ref = (dr_ref_t)mem_alloc(mgr->m_alloc, sizeof(struct dr_ref));
    if (dr_ref == NULL) return NULL;

    dr_ref->m_mgr = mgr;
    dr_ref->m_store = dr_store;

    dr_store_add_ref(dr_store);

    TAILQ_INSERT_TAIL(&mgr->m_refs, dr_ref, m_next);

    return dr_ref;
}

void dr_ref_free(dr_ref_t dr_ref) {
    assert(dr_ref);

    TAILQ_REMOVE(&dr_ref->m_mgr->m_refs, dr_ref, m_next);

    dr_store_remove_ref(dr_ref->m_store);

    mem_free(dr_ref->m_mgr->m_alloc, dr_ref);
}

const char * dr_ref_lib_name(dr_ref_t dr_ref) {
    return dr_ref->m_store->m_name;
}

LPDRMETALIB dr_ref_lib(dr_ref_t dr_ref) {
    assert(dr_ref);
    assert(dr_ref->m_store);
    return dr_ref->m_store->m_lib;
}

void dr_ref_free_all(dr_store_manage_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_refs)) {
        dr_ref_t ref = TAILQ_FIRST(&mgr->m_refs);

        dr_ref_free(ref);
    }
}
