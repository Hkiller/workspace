#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "fdb_table_i.h"

fdb_table_t fdb_table_create(fdb_repo_t repo, LPDRMETA meta) {
    fdb_table_t table;

    table = mem_alloc(repo->m_alloc, sizeof(struct fdb_table));
    if (table == NULL) {
        CPE_ERROR(repo->m_em, "fdb_table_create: alloc fdb_table fail!");
        return NULL;
    }

    table->m_repo = repo;
    table->m_name = dr_meta_name(meta);
    table->m_meta = meta;
    table->m_records = NULL;
    table->m_record_count = 0;
    table->m_record_capacity = 0;

    cpe_hash_entry_init(&table->m_hh);
    if (cpe_hash_table_insert(&repo->m_tables, table) != 0) {
        CPE_ERROR(repo->m_em, "fdb_table_create: table %s insert fail!", dr_meta_name(meta));
        mem_free(repo->m_alloc, table);
        return NULL;
    }
    
    return table;
}

void fdb_table_free(fdb_table_t table) {
    fdb_repo_t repo = table->m_repo;

    if (table->m_records) {
        mem_free(table->m_repo->m_alloc, table->m_records);
        table->m_records = NULL;
        table->m_record_count = 0;
        table->m_record_capacity = 0;
    }

    cpe_hash_table_remove_by_ins(&repo->m_tables, table);

    mem_free(repo->m_alloc, table);
}

void fdb_table_free_all(fdb_repo_t repo) {
    struct cpe_hash_it table_it;
    fdb_table_t table;

    cpe_hash_it_init(&table_it, &repo->m_tables);

    table = cpe_hash_it_next(&table_it);
    while (table) {
        fdb_table_t next = cpe_hash_it_next(&table_it);
        fdb_table_free(table);
        table = next;
    }
}

fdb_table_t fdb_table_find_by_name(fdb_repo_t db, const char * name) {
    struct fdb_table key;
    key.m_name = name;
    return cpe_hash_table_find(&db->m_tables, &key);
}

LPDRMETA fdb_table_meta(fdb_table_t table) {
    return table->m_meta;
}

static int fdb_table_reserve_record(fdb_table_t table, uint32_t n) {
    if (table->m_record_count + n <= table->m_record_capacity) return 0;

    return 0;
}

void * fdb_table_record_insert(fdb_table_t table, const void * data, const char ** duplicate_record) {
    if (dr_meta_key_entry_num(table->m_meta) == 0) {
        if (fdb_table_reserve_record(table, 1) != 0) {
            if (duplicate_record) *duplicate_record = NULL;
            return NULL;
        }

        return NULL;
    }
    else {
        return NULL;
    }
}

int fdb_table_record_remove(fdb_table_t table, const void * record) {
    return 0;
}

uint32_t fdb_table_hash(fdb_table_t table) {
    return cpe_hash_str(table->m_name, strlen(table->m_name));
}

int fdb_table_eq(fdb_table_t l, fdb_table_t r) {
    return strcmp(l->m_name, r->m_name) == 0 ? 1 : 0;
}

