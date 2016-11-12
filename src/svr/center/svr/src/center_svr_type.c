#include <assert.h> 
#include "cpe/pal/pal_string.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_type.h"

center_svr_type_t
center_svr_type_create(center_svr_t svr, const char * svr_type_name, uint16_t svr_type_id) {
    center_svr_type_t svr_type;
    size_t name_len = strlen(svr_type_name) + 1;

    svr_type = mem_alloc(svr->m_alloc, sizeof(struct center_svr_type) + name_len);
    if (svr_type == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: create svr_type of svr type %d: malloc fail!",
            center_svr_name(svr), svr_type_id);
        return NULL;
    }

    svr_type->m_svr = svr;
    svr_type->m_svr_type_name = (char *)(svr_type + 1);
    svr_type->m_svr_type = svr_type_id;
    svr_type->m_svr_count = 0;
    TAILQ_INIT(&svr_type->m_ins_proxies);

    memcpy(svr_type->m_svr_type_name, svr_type_name, name_len);

    cpe_hash_entry_init(&svr_type->m_hh);
    cpe_hash_table_insert_unique(&svr->m_svr_types, svr_type);

    return svr_type;
}

center_svr_type_t center_svr_type_find(center_svr_t svr, uint16_t svr_type) {
    struct center_svr_type key;
    key.m_svr_type = svr_type;
    return cpe_hash_table_find(&svr->m_svr_types, &key);
}

void center_svr_type_free(center_svr_type_t svr_type) {
    center_svr_t svr = svr_type->m_svr;

    assert(svr);
    assert(svr_type->m_svr_count == 0);
    assert(TAILQ_EMPTY(&svr_type->m_ins_proxies));

    cpe_hash_table_remove_by_ins(&svr->m_svr_types, svr_type);

    mem_free(svr->m_alloc, svr_type);
}

void center_svr_type_free_all(center_svr_t svr) {
    struct cpe_hash_it svr_type_it;
    center_svr_type_t svr_type;

    cpe_hash_it_init(&svr_type_it, &svr->m_svr_types);

    svr_type = cpe_hash_it_next(&svr_type_it);
    while(svr_type) {
        center_svr_type_t next = cpe_hash_it_next(&svr_type_it);
        center_svr_type_free(svr_type);
        svr_type = next;
    }
}

center_svr_type_t center_svr_type_lsearch_by_name(center_svr_t svr, const char * svr_type_name) {
    struct cpe_hash_it svr_type_it;
    center_svr_type_t svr_type;

    cpe_hash_it_init(&svr_type_it, &svr->m_svr_types);

    while((svr_type = cpe_hash_it_next(&svr_type_it))) {
        if (strcmp(svr_type->m_svr_type_name, svr_type_name) == 0) return svr_type;
    }

    return NULL;
}

uint32_t center_svr_type_hash(center_svr_type_t svr_type) {
    return svr_type->m_svr_type;
}

int center_svr_type_eq(center_svr_type_t l, center_svr_type_t r) {
    return l->m_svr_type == r->m_svr_type;
}
