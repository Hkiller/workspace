#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/dr_dm/dr_dm_data.h"
#include "dr_dm_internal_ops.h"

static uint32_t dr_dm_data_hash_fun(struct dr_dm_data * obj, struct dr_dm_data_index * index) {
    return dr_entry_hash(dr_dm_data_data(obj), index->m_entry);
}

static int dr_dm_data_hash_cmp(struct dr_dm_data * obj_l, struct dr_dm_data * obj_r, struct dr_dm_data_index * index) {
    return dr_entry_cmp(dr_dm_data_data(obj_l), dr_dm_data_data(obj_r) , index->m_entry) == 0;
}

struct dr_dm_data_index *
dr_dm_data_index_create(dr_dm_manage_t mgr, LPDRMETAENTRY entry, int is_unique) {
    struct dr_dm_data_index * index;

    assert(entry);

    index = (struct dr_dm_data_index*)mem_alloc(mgr->m_alloc, sizeof(struct dr_dm_data_index));
    if (index == NULL) return NULL;

    index->m_id = cpe_hash_table_count(&mgr->m_indexes);
    index->m_name = dr_entry_name(entry);
    index->m_entry = entry;
    index->m_insert_fun = is_unique ? cpe_hash_table_insert_unique : cpe_hash_table_insert;

    if (cpe_hash_table_init(
            &index->m_roles,
            mgr->m_alloc,
            (cpe_hash_fun_t) dr_dm_data_hash_fun,
            (cpe_hash_eq_t) dr_dm_data_hash_cmp,
            - (int)(sizeof(struct cpe_hash_entry) * (index->m_id + 1)),
            -1) != 0)
    {
        mem_free(mgr->m_alloc, index);
        return NULL;
    }
    cpe_hash_table_set_user_data(&index->m_roles, index);

    cpe_hash_entry_init(&index->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_indexes, index) != 0) {
        cpe_hash_table_fini(&index->m_roles);
        mem_free(mgr->m_alloc, index);
        return NULL;
    }

    return index;
}

void dr_dm_data_index_free(dr_dm_manage_t mgr, struct dr_dm_data_index * index) {
    if (index == mgr->m_id_index) mgr->m_id_index = NULL;
    cpe_hash_table_remove_by_ins(&mgr->m_indexes, index);
    cpe_hash_table_fini(&index->m_roles);
    mem_free(mgr->m_alloc, index);
}

const char * dr_dm_data_index_name(struct dr_dm_data_index * index) {
    return dr_entry_name(index->m_entry);
}

int dr_dm_data_index_add(struct dr_dm_data_index * index,  dr_dm_data_t role) {
    return index->m_insert_fun(&index->m_roles, role);
}

void dr_dm_data_index_remove(struct dr_dm_data_index * index,  dr_dm_data_t role) {
    cpe_hash_table_remove_by_ins(&index->m_roles, role);
}

uint32_t dr_dm_data_index_hash(const struct dr_dm_data_index * idx) {
    return cpe_hash_str(idx->m_name, strlen(idx->m_name));
}

int dr_dm_data_index_cmp(const struct dr_dm_data_index * l, const struct dr_dm_data_index * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void dr_dm_data_index_free_all(dr_dm_manage_t mgr) {
    struct cpe_hash_it index_it;
    struct dr_dm_data_index * index;

    cpe_hash_it_init(&index_it, &mgr->m_indexes);

    index = cpe_hash_it_next(&index_it);
    while (index) {
        struct dr_dm_data_index * next = cpe_hash_it_next(&index_it);
        dr_dm_data_index_free(mgr, index);
        index = next;
    }
}

