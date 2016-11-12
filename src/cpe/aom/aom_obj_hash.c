#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/aom/aom_obj_hash.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "aom_internal_types.h"

#define AOM_OBJ_HS_MAGIC ((uint16_t)0x32412f1u)

struct aom_obj_hs_buff_head {
    uint16_t m_magic;
    uint16_t m_head_version;
    uint32_t m_buf_capacity;
    uint32_t m_entry_start_pos;
    uint32_t m_entry_capacity;
    uint32_t m_bucket_start_pos;
    uint32_t m_bucket_capacity;
    uint32_t m_record_size;
};

struct aom_obj_hs_entry {
    uint32_t m_next_pos;
};

struct aom_obj_hash_table {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    aom_obj_mgr_t m_obj_mgr;
    aom_obj_hash_fun_t m_hash_fun;
    aom_obj_cmp_fun_t m_cmp_fun;
    uint32_t * m_buckets;
    uint32_t m_bucket_capacity;
    struct aom_obj_hs_entry * m_entries;
    uint32_t m_entry_capacity;
    size_t m_buff_capacity;
};

aom_obj_hash_table_t
aom_obj_hash_table_create(
    mem_allocrator_t alloc, error_monitor_t em,
    aom_obj_mgr_t mgr, aom_obj_hash_fun_t hash_fun, aom_obj_cmp_fun_t cmp_fun,
    void * buff, size_t buff_capacity)
{
    struct aom_obj_hs_buff_head * head;
    aom_obj_hash_table_t hs_table;

    head = buff;
    if (head->m_magic != AOM_OBJ_HS_MAGIC) {
        CPE_ERROR(em, "aom_obj_hash_table_create: buf head magic mismatch!");
        return NULL;
    }

    if (head->m_head_version != 1) {
        CPE_ERROR(em, "aom_obj_hash_table_create: buf head version %d not support!", head->m_head_version);
        return NULL;
    }

    if (head->m_buf_capacity > buff_capacity) {
        CPE_ERROR(
            em, "aom_obj_hash_table_create: buff capacity error, at least %d, but only %d!",
            head->m_buf_capacity, (int)buff_capacity);
        return NULL;
    }

    hs_table = mem_alloc(alloc, sizeof(struct aom_obj_hash_table));
    if (hs_table == NULL) {
        CPE_ERROR(em, "aom_obj_hash_table_create: alloc fail!");
        return NULL;
    }

    hs_table->m_alloc = alloc;
    hs_table->m_em = em;
    hs_table->m_obj_mgr = mgr;
    hs_table->m_hash_fun = hash_fun;
    hs_table->m_cmp_fun = cmp_fun;
    hs_table->m_buckets = (uint32_t*)(((char *)buff) + head->m_bucket_start_pos);
    hs_table->m_bucket_capacity = head->m_bucket_capacity;

    hs_table->m_entries = (struct aom_obj_hs_entry *)(((char *)buff) + head->m_entry_start_pos);
    hs_table->m_entry_capacity = head->m_entry_capacity;
    hs_table->m_buff_capacity = buff_capacity;
    
    return hs_table;
}

void aom_obj_hash_table_free(aom_obj_hash_table_t hs_table) {
    mem_free(hs_table->m_alloc, hs_table);
}

size_t aom_obj_hash_table_buff_capacity(aom_obj_hash_table_t hs_table) {
    return hs_table->m_buff_capacity;
}

void * aom_obj_hash_table_find(aom_obj_hash_table_t hs_table, void const * key) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket;

    assert(key);

    hs_value = hs_table->m_hash_fun(key, hs_table->m_obj_mgr->m_meta);
    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket = hs_table->m_buckets + bucket_pos;

    while(*bucket) {
        void * check_record;

        check_record = aom_obj_get(hs_table->m_obj_mgr, *bucket);
        assert(check_record);

        if (hs_table->m_cmp_fun(check_record, key, hs_table->m_obj_mgr->m_meta) == 0) {
            return check_record;
        }
        else {
            bucket = &(hs_table->m_entries + (*bucket))->m_next_pos;
        }
    }

    return NULL;
}

void * aom_obj_hash_table_find_next(aom_obj_hash_table_t hs_table, void const * obj) {
    uint32_t * bucket;
    int32_t idx;

    assert(obj);

    idx = aom_obj_index(hs_table->m_obj_mgr, obj);
    if (idx < 0) {
        CPE_ERROR(hs_table->m_em, "aom_obj_hash_table_find_next: record to idx fail");
        return NULL;
    }

    bucket = &(hs_table->m_entries + idx)->m_next_pos;

    while(*bucket) {
        void * check_record;

        check_record = aom_obj_get(hs_table->m_obj_mgr, *bucket);
        assert(check_record);

        if (hs_table->m_cmp_fun(check_record, obj, hs_table->m_obj_mgr->m_meta) == 0) {
            return check_record;
        }
        else {
            bucket = &(hs_table->m_entries + (*bucket))->m_next_pos;
        }
    }

    return NULL;
}

int aom_obj_hash_table_insert_unique(aom_obj_hash_table_t hs_table, void const * data, ptr_int_t * record_id) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket_insert;
    void * new_record;
    ptr_int_t new_record_idx;
    struct aom_obj_hs_entry * new_entry;

    assert(data);

    hs_value = hs_table->m_hash_fun(data, hs_table->m_obj_mgr->m_meta);
    assert(hs_value != 0);

    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket_insert = hs_table->m_buckets + bucket_pos;

    while(*bucket_insert) {
        void const * check_record;

        check_record = aom_obj_get(hs_table->m_obj_mgr, *bucket_insert);
        assert(check_record);

        if (hs_table->m_cmp_fun(check_record, data, hs_table->m_obj_mgr->m_meta) == 0) {
            return aom_obj_hash_table_error_duplicate;
        }

        bucket_insert = &hs_table->m_entries[*bucket_insert].m_next_pos;
    }

    do {
        new_record = aom_obj_alloc(hs_table->m_obj_mgr);
        if (new_record == NULL) {
            CPE_ERROR(hs_table->m_em, "aom_obj_hash_table_insert_unique: alloc new record fail");
            return aom_obj_hash_table_error_no_memory;
        }
        new_record_idx = aom_obj_index(hs_table->m_obj_mgr, new_record);
    } while(new_record_idx == 0);

    memcpy(new_record, data, hs_table->m_obj_mgr->m_record_size);

    assert(new_record_idx < hs_table->m_entry_capacity);
    new_entry = hs_table->m_entries + new_record_idx;

    assert(new_entry->m_next_pos == 0);

    *bucket_insert = new_record_idx;
    if (record_id) *record_id = new_record_idx;

    return 0;
}

int aom_obj_hash_table_insert(aom_obj_hash_table_t hs_table, void const * data, ptr_int_t * record_id) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket_insert;
    void * new_record;
    int new_record_idx;
    struct aom_obj_hs_entry * new_entry;

    assert(data);

    hs_value = hs_table->m_hash_fun(data, hs_table->m_obj_mgr->m_meta);
    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket_insert = hs_table->m_buckets + bucket_pos;

    while(*bucket_insert) {
        bucket_insert = &hs_table->m_entries[*bucket_insert].m_next_pos;
    }

    do {
        new_record = aom_obj_alloc(hs_table->m_obj_mgr);
        if (new_record == NULL) {
            CPE_ERROR(hs_table->m_em, "aom_obj_hash_table_insert: alloc new record fail");
            return aom_obj_hash_table_error_no_memory;
        }
        new_record_idx = aom_obj_index(hs_table->m_obj_mgr, new_record);
    } while(new_record_idx == 0);

    memcpy(new_record, data, hs_table->m_obj_mgr->m_record_size);

    assert(new_record_idx < hs_table->m_entry_capacity);
    new_entry = hs_table->m_entries + new_record_idx;

    assert(new_entry->m_next_pos == 0);

    *bucket_insert = new_record_idx;
    if (record_id) *record_id = new_record_idx;

    return 0;
}

int aom_obj_hash_table_insert_or_update(aom_obj_hash_table_t hs_table, void const * data, ptr_int_t * record_id) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket_insert;
    void * new_record;
    ptr_int_t new_record_idx;
    struct aom_obj_hs_entry * new_entry;

    assert(data);

    hs_value = hs_table->m_hash_fun(data, hs_table->m_obj_mgr->m_meta);
    assert(hs_value != 0);

    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket_insert = hs_table->m_buckets + bucket_pos;

    while(*bucket_insert) {
        void * check_record;

        check_record = aom_obj_get(hs_table->m_obj_mgr, *bucket_insert);
        assert(check_record);

        if (hs_table->m_cmp_fun(check_record, data, hs_table->m_obj_mgr->m_meta) == 0) {
            memcpy(check_record, data, hs_table->m_obj_mgr->m_record_size);
            if (record_id) *record_id = *bucket_insert;
            return 0;
        }

        bucket_insert = &hs_table->m_entries[*bucket_insert].m_next_pos;
    }

    do {
        new_record = aom_obj_alloc(hs_table->m_obj_mgr);
        if (new_record == NULL) {
            CPE_ERROR(hs_table->m_em, "aom_obj_hash_table_insert_or_update: alloc new record fail");
            return aom_obj_hash_table_error_no_memory;
        }
        new_record_idx = aom_obj_index(hs_table->m_obj_mgr, new_record);
    } while(new_record_idx == 0);

    memcpy(new_record, data, hs_table->m_obj_mgr->m_record_size);

    assert(new_record_idx < hs_table->m_entry_capacity);
    new_entry = hs_table->m_entries + new_record_idx;

    assert(new_entry->m_next_pos == 0);

    *bucket_insert = new_record_idx;
    if (record_id) *record_id = new_record_idx;

    return 0;
}

int aom_obj_hash_table_remove_by_ins(aom_obj_hash_table_t hs_table, void * data) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket_remove;
    struct aom_obj_hs_entry * remove_entry;
    int32_t idx;

    assert(data);

    hs_value = hs_table->m_hash_fun(data, hs_table->m_obj_mgr->m_meta);
    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket_remove = hs_table->m_buckets + bucket_pos;

    idx = aom_obj_index(hs_table->m_obj_mgr, data);
    if (idx < 0) {
        CPE_ERROR(hs_table->m_em, "aom_obj_hash_table_remove_by_ins: record to idx fail");
        return aom_obj_hash_table_error_obj_not_managed;
    }

    while(*bucket_remove && *bucket_remove != (uint32_t)idx) {
        bucket_remove = &(hs_table->m_entries + (*bucket_remove))->m_next_pos;
    }

    if (*bucket_remove == 0) {
        return aom_obj_hash_table_error_not_exist;
    }

    assert(*bucket_remove < hs_table->m_entry_capacity);
    remove_entry = hs_table->m_entries + (*bucket_remove);

    *bucket_remove = remove_entry->m_next_pos;
    remove_entry->m_next_pos = 0;

    aom_obj_free_by_idx(hs_table->m_obj_mgr, idx);

    return 0;
}

int aom_obj_hash_table_remove_by_key(aom_obj_hash_table_t hs_table, void const * key) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket_remove;
    uint32_t remove_idx;
    struct aom_obj_hs_entry * remove_entry;

    assert(key);

    hs_value = hs_table->m_hash_fun(key, hs_table->m_obj_mgr->m_meta);
    assert(hs_value != 0);

    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket_remove = hs_table->m_buckets + bucket_pos;

    while(*bucket_remove) {
        void const * check_record;

        check_record = aom_obj_get(hs_table->m_obj_mgr, *bucket_remove);
        assert(check_record);

        if (hs_table->m_cmp_fun(check_record, key, hs_table->m_obj_mgr->m_meta) == 0) {
            break;
        }
        else {
            bucket_remove = &(hs_table->m_entries + (*bucket_remove))->m_next_pos;
        }
    }

    if (*bucket_remove == 0) {
        return aom_obj_hash_table_error_not_exist;
    }

    remove_idx = *bucket_remove;
    assert(remove_idx < hs_table->m_entry_capacity);
    remove_entry = hs_table->m_entries + remove_idx;

    *bucket_remove = remove_entry->m_next_pos;
    remove_entry->m_next_pos = 0;

    aom_obj_free_by_idx(hs_table->m_obj_mgr, remove_idx);

    return 0;
}

int aom_obj_hash_table_remove_all_by_key(aom_obj_hash_table_t hs_table, void const * key) {
    uint32_t hs_value;
    int32_t bucket_pos;
    uint32_t * bucket_remove;
    uint32_t remove_idx;
    struct aom_obj_hs_entry * remove_entry;

    assert(key);

    hs_value = hs_table->m_hash_fun(key, hs_table->m_obj_mgr->m_meta);
    assert(hs_value != 0);

    bucket_pos = hs_value % hs_table->m_bucket_capacity;

    bucket_remove = hs_table->m_buckets + bucket_pos;

    while(*bucket_remove) {
        void const * check_record;

        check_record = aom_obj_get(hs_table->m_obj_mgr, *bucket_remove);
        assert(check_record);

        if (hs_table->m_cmp_fun(check_record, key, hs_table->m_obj_mgr->m_meta) == 0) {
            remove_idx = *bucket_remove;
            assert(remove_idx < hs_table->m_entry_capacity);
            remove_entry = hs_table->m_entries + remove_idx;

            *bucket_remove = remove_entry->m_next_pos;
            remove_entry->m_next_pos = 0;

            aom_obj_free_by_idx(hs_table->m_obj_mgr, remove_idx);
        }
        else {
            bucket_remove = &(hs_table->m_entries + (*bucket_remove))->m_next_pos;
        }
    }

    return 0;
}

size_t aom_obj_hash_table_buf_calc_capacity(aom_obj_mgr_t mgr, float bucket_ratio) {
    uint32_t entry_capacity;
    uint32_t bucket_capacity;
    uint32_t total_size;

    assert(bucket_ratio > 1.0);

    entry_capacity = mgr->m_free_obj_count + mgr->m_allocked_obj_count;
    bucket_capacity = (uint32_t)(entry_capacity * bucket_ratio);

    total_size = sizeof(struct aom_obj_hs_buff_head);
    CPE_PAL_ALIGN_DFT(total_size);

    total_size += entry_capacity * sizeof(struct aom_obj_hs_entry);
    CPE_PAL_ALIGN_DFT(total_size);

    total_size += bucket_capacity * sizeof(uint32_t);
    CPE_PAL_ALIGN_DFT(total_size);
    
    return total_size;
}

int aom_obj_hash_table_buf_init(
    aom_obj_mgr_t mgr, float bucket_ratio, aom_obj_hash_fun_t hash_fun,
    void * buf, size_t buf_capacity, error_monitor_t em)
{
    struct aom_obj_hs_buff_head * head;
    uint32_t * buckets;
    struct aom_obj_hs_entry * entries;
    struct aom_obj_it obj_it;
    void * obj;

    if (buf_capacity < aom_obj_hash_table_buf_calc_capacity(mgr, bucket_ratio)) {
        CPE_ERROR(
            em, "aom_obj_hash_table_buf_init: buf_capacity not enouth, input=%d, require=%d",
            (int)buf_capacity, (int)aom_obj_hash_table_buf_calc_capacity(mgr, bucket_ratio));
        return -1;
    }

    bzero(buf, buf_capacity);

    head = buf;

    head->m_magic = AOM_OBJ_HS_MAGIC;
    head->m_head_version = 1;
    head->m_buf_capacity = (uint32_t)buf_capacity;

    head->m_entry_start_pos = sizeof(struct aom_obj_hs_buff_head);
    CPE_PAL_ALIGN_DFT(head->m_entry_start_pos);
    head->m_entry_capacity = mgr->m_free_obj_count + mgr->m_allocked_obj_count;

    head->m_bucket_start_pos = head->m_entry_start_pos + head->m_entry_capacity * sizeof(struct aom_obj_hs_entry);
    CPE_PAL_ALIGN_DFT(head->m_bucket_start_pos);
    head->m_bucket_capacity = (uint32_t)(head->m_entry_capacity * bucket_ratio);

    head->m_record_size = mgr->m_record_size;

    buckets = (uint32_t*)(((char *)buf) + head->m_bucket_start_pos);
    entries = (struct aom_obj_hs_entry *)(((char *)buf) + head->m_entry_start_pos);

    aom_objs(mgr, &obj_it);

    while((obj = aom_obj_it_next(&obj_it))) {
        uint32_t hs_value;
        int32_t bucket_pos;
        uint32_t * bucket_insert;
        int idx;
        struct aom_obj_hs_entry * entry;

        idx = aom_obj_index(mgr, obj);
        assert(idx >= 0);
        if (idx == 0) continue;

        hs_value = hash_fun(obj, mgr->m_meta);
        bucket_pos = hs_value % head->m_bucket_capacity;
        bucket_insert = buckets + bucket_pos;

        while(*bucket_insert) {
            bucket_insert = &entries[*bucket_insert].m_next_pos;
        }

        entry = entries + idx;

        assert(entry->m_next_pos == 0);

        *bucket_insert = idx;
    }

    return 0;
}
