#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "dblog_svr_meta.h"

dblog_svr_meta_t
dblog_svr_meta_create(dblog_svr_t svr, uint16_t svr_type, const char * svr_type_name, LPDRMETA dr_meta) {
    dblog_svr_meta_t meta;

    meta = mem_alloc(svr->m_alloc, sizeof(struct dblog_svr_meta));
    if (meta == NULL) {
        CPE_ERROR(svr->m_em, "dblog svr meta alloc fail!");
        return NULL;
    }

    meta->m_svr = svr;
    meta->m_svr_type = svr_type;
    meta->m_meta_id = dr_meta_id(dr_meta);
    meta->m_meta = dr_meta;
    snprintf(meta->m_collection, sizeof(meta->m_collection), "%s-%s", svr_type_name, dr_meta_name(dr_meta));

    cpe_hash_entry_init(&meta->m_hh);
    if (cpe_hash_table_insert(&svr->m_metas, meta) != 0) {
        CPE_ERROR(svr->m_em, "dblog svr meta %s insert fail!", dr_meta_name(dr_meta));
        mem_free(svr->m_alloc, meta);
        return NULL;
    }

    return meta;
}

void dblog_svr_meta_free(dblog_svr_meta_t meta) {
    dblog_svr_t svr = meta->m_svr;

    cpe_hash_table_remove_by_ins(&svr->m_metas, meta);

    mem_free(svr->m_alloc, meta);
}

dblog_svr_meta_t
dblog_svr_meta_find(dblog_svr_t svr, uint16_t svr_type, uint16_t meta_id) {
    struct dblog_svr_meta key;
    key.m_svr_type = svr_type;
    key.m_meta_id = meta_id;
    return cpe_hash_table_find(&svr->m_metas, &key);
}

void dblog_svr_meta_free_all(dblog_svr_t svr) {
    struct cpe_hash_it meta_it;
    dblog_svr_meta_t meta;

    cpe_hash_it_init(&meta_it, &svr->m_metas);

    meta = cpe_hash_it_next(&meta_it);
    while (meta) {
        dblog_svr_meta_t next = cpe_hash_it_next(&meta_it);
        dblog_svr_meta_free(meta);
        meta = next;
    }
}

uint32_t dblog_svr_meta_hash(dblog_svr_meta_t meta) {
    return ((uint32_t) meta->m_svr_type) << 16 | ((uint32_t)(meta->m_meta_id));
}

int dblog_svr_meta_eq(dblog_svr_meta_t l, dblog_svr_meta_t r) {
    return l->m_svr_type == r->m_svr_type && l->m_meta_id == r->m_meta_id ? 1 : 0;
}

