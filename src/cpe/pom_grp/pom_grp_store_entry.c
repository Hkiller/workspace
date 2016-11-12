#include <assert.h>
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

pom_grp_store_entry_t 
pom_grp_store_entry_create(pom_grp_store_table_t table, pom_grp_entry_meta_t entry_meta) {
    pom_grp_store_entry_t entry;

    entry = (pom_grp_store_entry_t)mem_alloc(table->m_store->m_alloc, sizeof(struct pom_grp_store_entry));
    if (entry == NULL) return NULL;

    entry->m_table = table;
    entry->m_name = entry_meta->m_name;
    entry->m_entry_meta = entry_meta;

    cpe_hash_entry_init(&entry->m_hh);

    if (cpe_hash_table_insert_unique(&table->m_store->m_entries, entry) != 0) {
        mem_free(table->m_store->m_alloc, entry);
        return NULL; 
    }
    
    TAILQ_INSERT_TAIL(&table->m_entries, entry, m_next);

    return entry;
}

void pom_grp_store_entry_free(struct pom_grp_store_entry * store_entry) {
    cpe_hash_table_remove_by_ins(&store_entry->m_table->m_store->m_entries, store_entry);
    TAILQ_REMOVE(&store_entry->m_table->m_entries, store_entry, m_next);
    mem_free(store_entry->m_table->m_store->m_alloc, store_entry);
}

uint32_t pom_grp_store_entry_hash(const struct pom_grp_store_entry * store_entry) {
    return cpe_hash_str(store_entry->m_name, strlen(store_entry->m_name));
}

int pom_grp_store_entry_cmp(const struct pom_grp_store_entry * l, const struct pom_grp_store_entry * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void pom_grp_store_entry_free_all(pom_grp_store_t store) {
    struct cpe_hash_it stroe_entry_it;
    struct pom_grp_store_entry * stroe_entry;

    cpe_hash_it_init(&stroe_entry_it, &store->m_entries);

    stroe_entry = cpe_hash_it_next(&stroe_entry_it);
    while(stroe_entry) {
        struct pom_grp_store_entry * next = (struct pom_grp_store_entry *)cpe_hash_it_next(&stroe_entry_it);
        pom_grp_store_entry_free(stroe_entry);
        stroe_entry = next;
    }
}

static pom_grp_store_entry_t pom_grp_table_entry_next(struct pom_grp_store_entry_it * it) {
    pom_grp_store_entry_t * pentry;
    pom_grp_store_entry_t r;

    pentry = (pom_grp_store_entry_t *)it->m_data;

    r = *pentry;
    if (*pentry) *pentry = TAILQ_NEXT(*pentry, m_next);

    return r;
}

void pom_grp_table_entries(pom_grp_store_table_t table, pom_grp_store_entry_it_t it) {
    pom_grp_store_entry_t * pentry;
    pentry = (pom_grp_store_entry_t *)it->m_data;

    *pentry = TAILQ_FIRST(&table->m_entries);
    it->next = pom_grp_table_entry_next;
}

pom_grp_store_entry_t pom_grp_table_entry_find(pom_grp_store_table_t table, const char * name) {
    pom_grp_store_entry_t entry;

    TAILQ_FOREACH(entry, &table->m_entries, m_next) {
        if (strcmp(entry->m_name, name) == 0) return entry;
    }

    return NULL;
}

static pom_grp_store_entry_t pom_grp_store_entry_next(struct pom_grp_store_entry_it * it) {
    struct cpe_hash_it * stroe_entry_it;

    stroe_entry_it = (struct cpe_hash_it *)(it->m_data);

    return (struct pom_grp_store_entry *)cpe_hash_it_next(stroe_entry_it);
}

void pom_grp_store_entrys(pom_grp_store_t store, pom_grp_store_entry_it_t it) {
    struct cpe_hash_it * stroe_entry_it;

    assert(sizeof(struct cpe_hash_it) < sizeof(it->m_data));

    stroe_entry_it = (struct cpe_hash_it *)(it->m_data);

    cpe_hash_it_init(stroe_entry_it, &store->m_entries);
    it->next = pom_grp_store_entry_next;
}

pom_grp_store_entry_t pom_grp_store_entry_find(pom_grp_store_t store, const char * name) {
    struct pom_grp_store_entry key;
    key.m_name = name;
    return (pom_grp_store_entry_t)cpe_hash_table_find(&store->m_entries, &key);
}

const char * pom_grp_store_entry_name(pom_grp_store_entry_t entry) {
    return entry->m_name;
}

pom_grp_entry_meta_t pom_grp_store_entry_meta(pom_grp_store_entry_t entry) {
    return entry->m_entry_meta;
}
