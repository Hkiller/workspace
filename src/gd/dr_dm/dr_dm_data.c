#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/utils/id_generator.h"
#include "gd/dr_dm/dr_dm_data.h"
#include "gd/dr_dm/dr_dm_manage.h"
#include "dr_dm_internal_ops.h"

dr_dm_data_t
dr_dm_data_create(dr_dm_manage_t mgr, const void * data, size_t data_size, const char ** duplicate_index) {
    char * buf;
    dr_dm_data_t role;
    size_t data_capacity;
    dr_dm_data_id_t role_id;
    int generate_role_id;
    size_t index_count;
    struct cpe_hash_it index_it;
    struct dr_dm_data_index * index;
    size_t i;

    index_count = cpe_hash_table_count(&mgr->m_indexes);

    if (duplicate_index) *duplicate_index = NULL;

    if (mgr->m_role_meta == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: dr_dm_data_create: role meta not exist!",
            dr_dm_manage_name(mgr));
        return NULL;
    }

    if (mgr->m_id_index == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: dr_dm_data_create: role id entry not exist!",
            dr_dm_manage_name(mgr));
        return NULL;
    }

    data_capacity = dr_meta_size(mgr->m_role_meta);
    if (data_size > data_capacity) {
        CPE_ERROR(
            mgr->m_em, "%s: dr_dm_data_create: data too long, data_size=%d, data_capacity=%d!",
            dr_dm_manage_name(mgr), (int)data_size, (int)data_capacity);
        return NULL;
    }

    generate_role_id = 0;
    role_id = dr_entry_read_int64(data, mgr->m_id_index->m_entry);
    if (role_id == 0) {
        if (mgr->m_id_generate) {
            if (gd_id_generator_generate(&role_id, mgr->m_id_generate, dr_dm_manage_name(mgr)) != 0) {
                CPE_ERROR(
                    mgr->m_em, "%s: dr_dm_data_create: generate id from %s fail!",
                    dr_dm_manage_name(mgr), gd_id_generator_name(mgr->m_id_generate));
                return NULL;
            }
            else {
                generate_role_id = 1;
            }
        }
    }

    buf = (char *)mem_alloc(
        mgr->m_alloc,
        sizeof(struct cpe_hash_entry) * index_count
        + sizeof(struct dr_dm_data)
        + data_capacity);
    if (buf == NULL) return NULL;

    role = (dr_dm_data_t)(buf + sizeof(struct cpe_hash_entry) * index_count);

    role->m_mgr = mgr;

    memcpy(dr_dm_data_data(role), data, data_capacity);

    if (generate_role_id) {
        if (dr_entry_set_from_int64(dr_dm_data_data(role), role_id, mgr->m_id_index->m_entry, NULL) != 0) {
            CPE_ERROR(
                mgr->m_em, "%s: dr_dm_data_create: set generated id to data fail!",
                dr_dm_manage_name(mgr));
            mem_free(mgr->m_alloc, buf);
            return NULL;
        }
    }

    for(i = 0; i < index_count; ++i) {
        cpe_hash_entry_init(((struct cpe_hash_entry*)buf) + i);
    }

    cpe_hash_it_init(&index_it, &mgr->m_indexes);
    while((index = cpe_hash_it_next(&index_it))) {
        if (dr_dm_data_index_add(index, role) != 0) {
            struct dr_dm_data_index * index_fall_back;

            CPE_ERROR(
                mgr->m_em, "%s: dr_dm_data_create: add to index %s: duplicate!",
                dr_dm_manage_name(mgr), dr_entry_name(index->m_entry));

            if (duplicate_index) *duplicate_index = dr_dm_data_index_name(index);

            cpe_hash_it_init(&index_it, &mgr->m_indexes);
            while((index_fall_back = cpe_hash_it_next(&index_it)) && index_fall_back != index) {
                dr_dm_data_index_remove(index_fall_back, role);
            }
            
            mem_free(mgr->m_alloc, buf);
            return NULL;
        }
    }

    return role;
}

void dr_dm_data_free(dr_dm_data_t dr_dm_data) {
    struct cpe_hash_it index_it;
    struct dr_dm_data_index * index;
    size_t index_count;

    index_count = cpe_hash_table_count(&dr_dm_data->m_mgr->m_indexes);

    cpe_hash_it_init(&index_it, &dr_dm_data->m_mgr->m_indexes);
    while((index = cpe_hash_it_next(&index_it))) {
        dr_dm_data_index_remove(index, dr_dm_data);
    }

    mem_free(dr_dm_data->m_mgr->m_alloc, ((char*)dr_dm_data) - sizeof(struct cpe_hash_entry) * index_count);
}

dr_dm_data_id_t dr_dm_data_id(dr_dm_data_t dr_dm_data) {
    assert(dr_dm_data->m_mgr->m_id_index);

    return dr_entry_read_int64(
        dr_dm_data + 1,
        dr_dm_data->m_mgr->m_id_index->m_entry);
}

void * dr_dm_data_data(dr_dm_data_t dr_dm_data) {
    return dr_dm_data + 1;
}

size_t dr_dm_data_data_capacity(dr_dm_data_t dr_dm_data) {
    assert(dr_dm_data->m_mgr->m_role_meta);
    return dr_meta_size(dr_dm_data->m_mgr->m_role_meta);
}

LPDRMETA dr_dm_data_meta(dr_dm_data_t dr_dm_data) {
    assert(dr_dm_data->m_mgr->m_role_meta);

    return dr_dm_data->m_mgr->m_role_meta;
}

dr_dm_data_t dr_dm_data_find_by_id(dr_dm_manage_t mgr, dr_dm_data_id_t id) {
    dr_dm_data_t key;

    if (mgr->m_id_index == NULL) return NULL;

    key = dr_dm_manage_key_buf(mgr);

    if (dr_entry_set_from_uint64(
            (char*)dr_dm_data_data(key) + dr_entry_data_start_pos(mgr->m_id_index->m_entry, 0)
            , id, mgr->m_id_index->m_entry, NULL) != 0)
        return NULL;

    return (dr_dm_data_t)cpe_hash_table_find(&mgr->m_id_index->m_roles, key);
}

#define DEF_DM_ROLE_FIND_BY_INDEX_FUN(__type_name, __type)              \
    dr_dm_data_t dr_dm_data_find_by_index_ ## __type_name(                    \
        dr_dm_manage_t mgr, const char * idx_name, __type input)      \
    {                                                                   \
        struct dr_dm_data_index index_key;                                 \
        struct dr_dm_data_index * index;                                   \
        dr_dm_data_t key;                                                  \
                                                                        \
        index_key.m_name = idx_name;                                    \
        index = (struct dr_dm_data_index *)                                \
            cpe_hash_table_find(&mgr->m_indexes, &index_key);           \
        if (index == NULL) return NULL;                                 \
                                                                        \
        key = dr_dm_manage_key_buf(mgr);                              \
        if (dr_entry_set_from_ ## __type_name(                          \
                (char *)dr_dm_data_data(key)                            \
                + dr_entry_data_start_pos(index->m_entry, 0),              \
                input, index->m_entry, NULL) != 0)                      \
        {                                                               \
            return NULL;                                                \
        }                                                               \
                                                                        \
        return (dr_dm_data_t)cpe_hash_table_find(&index->m_roles, key);    \
    }

DEF_DM_ROLE_FIND_BY_INDEX_FUN(int8, int8_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(uint8, uint8_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(int16, int16_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(uint16, uint16_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(int32, int32_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(uint32, uint32_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(int64, int64_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(uint64, uint64_t)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(float, float)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(double, double)
DEF_DM_ROLE_FIND_BY_INDEX_FUN(string, const char *)

dr_dm_data_t
dr_dm_data_find_by_index_ctype(dr_dm_manage_t mgr, const char * idx_name, const void * input, int input_type) {
    struct dr_dm_data_index index_key;
    struct dr_dm_data_index * index;
    dr_dm_data_t key;

    index_key.m_name = idx_name;
    index = (struct dr_dm_data_index *)
        cpe_hash_table_find(&mgr->m_indexes, &index_key);
    if (index == NULL) return NULL;
    key = dr_dm_manage_key_buf(mgr);
    if (dr_entry_set_from_ctype(
            dr_dm_data_data(key), input, input_type, index->m_entry, NULL) != 0)
    {
        return NULL;
    }
    return (dr_dm_data_t)cpe_hash_table_find(&index->m_roles, key);
}

dr_dm_data_t dr_dm_data_do_next(struct dr_dm_data_it * it) {
    char _check_data_size[sizeof(it->m_data) < sizeof(struct cpe_hash_it) ? -1 : 1];
    (void)_check_data_size;

    return (dr_dm_data_t)cpe_hash_it_next((struct cpe_hash_it *)(it->m_data));
}

void dr_dm_data_it_init(struct dr_dm_data_it * it, dr_dm_manage_t mgr) {
    it->next = dr_dm_data_do_next;
    cpe_hash_it_init((struct cpe_hash_it *)it->m_data, &mgr->m_id_index->m_roles);
}
