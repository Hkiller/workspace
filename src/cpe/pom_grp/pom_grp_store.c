#include <assert.h>
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

pom_grp_store_t
pom_grp_store_create(
    mem_allocrator_t alloc,
    pom_grp_meta_t meta,
    LPDRMETA dr_meta,
    error_monitor_t em)
{
    pom_grp_store_t store;

    store = mem_alloc(alloc, sizeof(struct pom_grp_store));
    if (store == NULL) {
        CPE_ERROR(em, "pom_grp_store_create: alloc pom_grp_store fail!");
        return NULL;
    }

    store->m_alloc = alloc;
    store->m_em = em;
    store->m_meta = meta;
    store->m_main_table = NULL;

    if (cpe_hash_table_init(
            &store->m_tables,
            alloc,
            (cpe_hash_fun_t) pom_grp_store_table_hash,
            (cpe_hash_eq_t) pom_grp_store_table_cmp,
            CPE_HASH_OBJ2ENTRY(pom_grp_store_table, m_hh),
            -1) != 0)
    {
        mem_free(alloc, store);
        return NULL;
    }

    if (cpe_hash_table_init(
            &store->m_entries,
            alloc,
            (cpe_hash_fun_t) pom_grp_store_entry_hash,
            (cpe_hash_eq_t) pom_grp_store_entry_cmp,
            CPE_HASH_OBJ2ENTRY(pom_grp_store_entry, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&store->m_tables);
        mem_free(alloc, store);
        return NULL;
    }

    if (pom_grp_store_table_build(store, dr_meta) != 0) {
        cpe_hash_table_fini(&store->m_entries);
        cpe_hash_table_fini(&store->m_tables);
        mem_free(alloc, store);
        return NULL;
    }

    return store;
}

void pom_grp_store_free(pom_grp_store_t store) {
    pom_grp_store_table_free_all(store);
    pom_grp_store_entry_free_all(store);
    cpe_hash_table_fini(&store->m_tables);
    cpe_hash_table_fini(&store->m_entries);
    mem_free(store->m_alloc, store);
}

pom_grp_meta_t pom_grp_store_meta(pom_grp_store_t store) {
    return store->m_meta;
}
