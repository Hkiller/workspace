#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

pom_grp_store_table_t
pom_grp_store_table_create(pom_grp_store_t store, LPDRMETA meta) {
    pom_grp_store_table_t table;

    table = (pom_grp_store_table_t)mem_alloc(store->m_alloc, sizeof(struct pom_grp_store_table));
    if (table == NULL) return NULL;

    table->m_store = store;
    table->m_name = dr_meta_name(meta);
    table->m_meta = meta;
    TAILQ_INIT(&table->m_entries);

    cpe_hash_entry_init(&table->m_hh);
    if (cpe_hash_table_insert_unique(&store->m_tables, table) != 0) {
        mem_free(store->m_alloc, table);
        return NULL; 
    }

    return table;
}

void pom_grp_store_table_free(pom_grp_store_table_t store_table) {
    while(!TAILQ_EMPTY(&store_table->m_entries)) {
        pom_grp_store_entry_free(TAILQ_FIRST(&store_table->m_entries));
    }

    cpe_hash_table_remove_by_ins(&store_table->m_store->m_tables, store_table);
    mem_free(store_table->m_store->m_alloc, store_table);
}

const char * pom_grp_store_table_name(pom_grp_store_table_t table) {
    return table->m_name;
}

LPDRMETA pom_grp_store_table_meta(pom_grp_store_table_t table) {
    return table->m_meta;
}

uint32_t pom_grp_store_table_hash(const struct pom_grp_store_table * store_table) {
    return cpe_hash_str(store_table->m_name, strlen(store_table->m_name));
}

int pom_grp_store_table_cmp(const struct pom_grp_store_table * l, const struct pom_grp_store_table * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void pom_grp_store_table_free_all(pom_grp_store_t store) {
    struct cpe_hash_it stroe_table_it;
    struct pom_grp_store_table * stroe_table;

    cpe_hash_it_init(&stroe_table_it, &store->m_tables);

    stroe_table = cpe_hash_it_next(&stroe_table_it);
    while(stroe_table) {
        struct pom_grp_store_table * next = (struct pom_grp_store_table *)cpe_hash_it_next(&stroe_table_it);
        pom_grp_store_table_free(stroe_table);
        stroe_table = next;
    }
}

static int pom_grp_store_table_build_for_entry_normal(
    pom_grp_store_t store,
    pom_grp_store_table_t main_table,
    pom_grp_entry_meta_t entry)
{
    if (pom_grp_store_entry_create(main_table, entry) == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: create entry %s in table %s fail!",
            main_table->m_name, entry->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_store_table_build_for_entry_ba(
    pom_grp_store_t store,
    pom_grp_store_table_t main_table,
    pom_grp_entry_meta_t entry)
{
    if (pom_grp_store_entry_create(main_table, entry) == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: create entry %s in table %s fail!",
            main_table->m_name, entry->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_store_table_build_for_entry_binary(
    pom_grp_store_t store,
    pom_grp_store_table_t main_table,
    pom_grp_entry_meta_t entry)
{
    if (pom_grp_store_entry_create(main_table, entry) == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: create entry %s in table %s fail!",
            main_table->m_name, entry->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_store_table_build_for_entry_list(
    pom_grp_store_t store,
    pom_grp_store_table_t main_table,
    pom_grp_entry_meta_t entry)
{
    if (pom_grp_store_entry_create(main_table, entry) == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: create entry %s in table %s fail!",
            main_table->m_name, entry->m_name);
        return -1;
    }

    return 0;
}

int pom_grp_store_table_build(pom_grp_store_t store, LPDRMETA meta) {
    pom_grp_store_table_t main_table;
    uint16_t i, count;

    if (store->m_meta->m_main_entry == NULL) {
        CPE_ERROR(store->m_em, "pom_grp_store_table_build: main entry not exist!");
        return -1;
    }

    if (store->m_meta->m_main_entry->m_type != pom_grp_entry_type_normal) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: main entry %s is not type normal!",
            store->m_meta->m_main_entry->m_name);
        return -1;
    }

    main_table = pom_grp_store_table_create(store, meta);
    if (main_table == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: create main table fail!");
        return -1;
    }

    if (pom_grp_store_entry_create(main_table, store->m_meta->m_main_entry) == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_table_build: create main entry in table fail!");
        return -1;
    }

    count = pom_grp_meta_entry_count(store->m_meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta = pom_grp_meta_entry_at(store->m_meta, i);
        if (entry_meta == store->m_meta->m_main_entry) continue;

        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            if (pom_grp_store_table_build_for_entry_normal(store, main_table, entry_meta) != 0) return -1;
            break;
        case pom_grp_entry_type_list:
            if (pom_grp_store_table_build_for_entry_list(store, main_table, entry_meta) != 0) return -1;
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_store_table_build_for_entry_ba(store, main_table, entry_meta) != 0) return -1;
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_store_table_build_for_entry_binary(store, main_table, entry_meta) != 0) return -1;
            break;
        }
    }

    return 0;
}

static pom_grp_store_table_t pom_grp_store_table_next(struct pom_grp_store_table_it * it) {
    struct cpe_hash_it * store_table_it;

    store_table_it = (struct cpe_hash_it *)(it->m_data);

    return (struct pom_grp_store_table *)cpe_hash_it_next(store_table_it);
}

uint32_t pom_grp_store_table_count(pom_grp_store_t store) {
    return cpe_hash_table_count(&store->m_tables);
}

void pom_grp_store_tables(pom_grp_store_t store, pom_grp_store_table_it_t it) {
    struct cpe_hash_it * store_table_it;

    assert(sizeof(struct cpe_hash_it) >= sizeof(it->m_data));

    store_table_it = (struct cpe_hash_it *)(it->m_data);

    cpe_hash_it_init(store_table_it, &store->m_tables);
    it->next = pom_grp_store_table_next;
}

pom_grp_store_table_t pom_grp_store_table_find(pom_grp_store_t store, const char * name) {
    struct pom_grp_store_table key;
    key.m_name = name;
    return (pom_grp_store_table_t)cpe_hash_table_find(&store->m_tables, &key);
}
