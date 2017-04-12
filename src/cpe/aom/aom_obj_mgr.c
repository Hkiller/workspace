#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/range_bitarry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "aom_internal_types.h"
#include "aom_data.h"

static void aom_obj_mgr_init_objs(aom_obj_mgr_t, void * data, uint32_t data_size);
static int aom_obj_is_free(LPDRMETA meta, char const * data);
static void aom_obj_set_free(LPDRMETA meta, char * data);

aom_obj_mgr_t
aom_obj_mgr_create(
    mem_allocrator_t alloc,
    void * data,
    size_t data_capacity,
    error_monitor_t em)
{
    aom_obj_mgr_t obj_mgr;
    struct aom_obj_control_data * control;

    if (data_capacity < sizeof(struct aom_obj_control_data)) {
        CPE_ERROR(
            em, "aom_obj_mgr_create: not enouth size, size="FMT_SIZE_T", control_data_size="FMT_SIZE_T"!",
            data_capacity, sizeof(struct aom_obj_control_data));
        return NULL;
    }

    control = (struct aom_obj_control_data *)data;

    if(control->m_magic != OM_GRP_OBJ_CONTROL_MAGIC) {
        CPE_ERROR(
            em, "aom_obj_mgr_create: matic mismatch!, %d and %d",
            control->m_magic, OM_GRP_OBJ_CONTROL_MAGIC);
        return NULL;
    }

    if(control->m_head_version != 1) {
        CPE_ERROR(
            em, "aom_obj_mgr_create: not support version %d",
            control->m_head_version);
        return NULL;
    }

    if (data_capacity < (control->m_metalib_start + control->m_metalib_size)) {
        CPE_ERROR(
            em, "aom_obj_mgr_create: not enouth size, size="FMT_SIZE_T", metalib end at %u!",
            data_capacity, control->m_metalib_start + control->m_metalib_size)
        return NULL;
    }

    if (data_capacity < (control->m_data_start + control->m_data_size)) {
        CPE_ERROR(
            em, "aom_obj_mgr_create: not enouth size, size="FMT_SIZE_T", data end at %u!",
            data_capacity, control->m_data_start + control->m_data_size)
        return NULL;
    }

    obj_mgr = (aom_obj_mgr_t)mem_alloc(alloc, sizeof(struct aom_obj_mgr));
    if (obj_mgr == NULL) {
        CPE_ERROR(em, "aom_obj_mgr_create: create fail!");
        return NULL;
    }

    obj_mgr->m_alloc = alloc;
    obj_mgr->m_em = em;
    obj_mgr->m_full_base = (char *)data;
    obj_mgr->m_full_capacity = data_capacity;
    obj_mgr->m_obj_base = obj_mgr->m_full_base + control->m_data_start;
    obj_mgr->m_obj_capacity = control->m_data_size;

    obj_mgr->m_metalib = (LPDRMETALIB)(obj_mgr->m_full_base + control->m_metalib_start);

    obj_mgr->m_meta = dr_lib_find_meta_by_name(obj_mgr->m_metalib, control->m_meta_name);
    if (obj_mgr->m_meta == NULL) {
        CPE_ERROR(em, "aom_obj_mgr_create: meta %s not exist!", control->m_meta_name);
        mem_free(alloc, obj_mgr);
        return NULL;
    }
    obj_mgr->m_record_size = dr_meta_size(obj_mgr->m_meta);

    if (dr_meta_key_entry_num(obj_mgr->m_meta) == 0) {
        CPE_ERROR(em, "aom_obj_mgr_create: meta %s have no key!", control->m_meta_name);
        mem_free(alloc, obj_mgr);
        return NULL;
    }

    obj_mgr->m_allocked_obj_count = 0;
    cpe_range_mgr_init(&obj_mgr->m_allocked_objs, alloc);
    obj_mgr->m_free_obj_count = 0;
    cpe_range_mgr_init(&obj_mgr->m_free_objs, alloc);

    aom_obj_mgr_init_objs(obj_mgr, obj_mgr->m_obj_base, obj_mgr->m_obj_capacity);

    return obj_mgr;
}

void aom_obj_mgr_free(aom_obj_mgr_t mgr) {
    cpe_range_mgr_fini(&mgr->m_allocked_objs);
    cpe_range_mgr_fini(&mgr->m_free_objs);

    mem_free(mgr->m_alloc, mgr);
}

void * aom_obj_mgr_data(aom_obj_mgr_t mgr) {
    return mgr->m_full_base;
}

size_t aom_obj_mgr_data_capacity(aom_obj_mgr_t mgr) {
    return mgr->m_full_capacity;
}

LPDRMETA aom_obj_mgr_meta(aom_obj_mgr_t mgr) {
    return mgr->m_meta;
}

uint32_t aom_obj_mgr_free_obj_count(aom_obj_mgr_t mgr) {
    return mgr->m_free_obj_count;
}

uint32_t aom_obj_mgr_allocked_obj_count(aom_obj_mgr_t mgr) {
    return mgr->m_allocked_obj_count;
}

void * aom_obj_alloc(aom_obj_mgr_t mgr) {
    ptr_int_t idx = cpe_range_get_one(&mgr->m_free_objs);
    char * obj;

    if (idx <0) return NULL;

    cpe_range_put_one(&mgr->m_allocked_objs, idx);

    obj = mgr->m_obj_base + mgr->m_record_size * idx;

    assert(aom_obj_is_free(mgr->m_meta, obj));

    --mgr->m_free_obj_count;
    ++mgr->m_allocked_obj_count;

    return obj;
}

void aom_obj_free(aom_obj_mgr_t mgr, void * obj) {
    ptr_int_t idx;
    size_t obj_size = mgr->m_record_size;

    assert((char*)obj >= mgr->m_obj_base);
    assert((char*)obj + obj_size <= mgr->m_obj_base + mgr->m_obj_capacity);

    idx = (((char*)obj) - mgr->m_obj_base) / obj_size;

    assert(obj == (mgr->m_obj_base + idx * obj_size));

    aom_obj_set_free(mgr->m_meta, obj);
    assert(aom_obj_is_free(mgr->m_meta, obj));

    cpe_range_remove_one(&mgr->m_allocked_objs, idx);
    cpe_range_put_one(&mgr->m_free_objs, idx);

    ++mgr->m_free_obj_count;
    --mgr->m_allocked_obj_count;
}

void aom_obj_free_by_idx(aom_obj_mgr_t mgr, ptr_int_t idx) {
    size_t obj_size = mgr->m_record_size;
    void * obj;

    obj = (mgr->m_obj_base + idx * obj_size);

    aom_obj_set_free(mgr->m_meta, obj);
    assert(aom_obj_is_free(mgr->m_meta, obj));

    cpe_range_remove_one(&mgr->m_allocked_objs, idx);
    cpe_range_put_one(&mgr->m_free_objs, idx);

    ++mgr->m_free_obj_count;
    --mgr->m_allocked_obj_count;
}

void * aom_obj_get(aom_obj_mgr_t mgr, ptr_int_t idx) {
    char * obj;

    assert(idx >= 0 && idx < (mgr->m_allocked_obj_count + mgr->m_free_obj_count));

    obj = mgr->m_obj_base + mgr->m_record_size * idx;

    return aom_obj_is_free(mgr->m_meta, obj) ? NULL : obj;
}


ptr_int_t aom_obj_index(aom_obj_mgr_t mgr, void const * obj) {
    ptr_int_t idx;
    size_t obj_size = mgr->m_record_size;

    assert((char const *)obj >= mgr->m_obj_base);
    assert((char const *)obj + obj_size <= mgr->m_obj_base + mgr->m_obj_capacity);

    idx = (((char const *)obj) - mgr->m_obj_base) / obj_size;

    assert(obj == (mgr->m_obj_base + idx * obj_size));

    return idx;
}

struct aom_objs_it_data {
    aom_obj_mgr_t m_mgr;
    struct cpe_range_it m_range_it;
    struct cpe_range m_range;
};

static void * aom_objs_next(struct aom_obj_it * it) {
    struct aom_objs_it_data * data = (struct aom_objs_it_data *)it->m_data;
    void * obj;

    if (cpe_range_size(data->m_range) <= 0) {
        data->m_range = cpe_range_it_next(&data->m_range_it);

        if (cpe_range_size(data->m_range) <= 0) return NULL;
    }

    obj = data->m_mgr->m_obj_base + data->m_mgr->m_record_size * data->m_range.m_start;

    ++data->m_range.m_start;

    return obj;
}

int aom_obj_it_check_size[sizeof(struct aom_objs_it_data) <= (sizeof(struct aom_obj_it) - sizeof(void*)) ? 1 : -1];

void aom_objs(aom_obj_mgr_t mgr, aom_obj_it_t it) {
    struct aom_objs_it_data * data = (struct aom_objs_it_data *)it->m_data;

    data->m_mgr = mgr;
    cpe_range_mgr_ranges(&data->m_range_it, &mgr->m_allocked_objs);
    data->m_range = cpe_range_it_next(&data->m_range_it);

    it->next = aom_objs_next;
}

static int aom_obj_is_free(LPDRMETA meta, char const * data) {
    int num = dr_meta_key_entry_num(meta);
    int i;

    for(i = 0; i < num; ++i) {
        LPDRMETAENTRY key_entry = dr_meta_key_entry_at(meta, i);
        int len = dr_entry_element_size(key_entry);
        char const * b = data + dr_entry_data_start_pos(key_entry, 0);

        for(; len > 0; --len) {
            if (b[len - 1] != 0) return 0;
        }
    }

    return 1;
}

static void aom_obj_set_free(LPDRMETA meta, char * data) {
    int num = dr_meta_key_entry_num(meta);
    int i;

    for(i = 0; i < num; ++i) {
        LPDRMETAENTRY key_entry = dr_meta_key_entry_at(meta, i);
        bzero(
            data + dr_entry_data_start_pos(key_entry, 0),
            dr_entry_element_size(key_entry));
    }
}

static void aom_obj_mgr_init_objs(aom_obj_mgr_t mgr, void * data, uint32_t data_size) {
    char * obj;
    size_t obj_size = mgr->m_record_size;
    ptr_int_t idx = 0;

    for(obj = data; data_size >= obj_size; obj += obj_size, data_size -= obj_size, ++idx) {
        if (aom_obj_is_free(mgr->m_meta, obj)) {
            ++mgr->m_free_obj_count;
            cpe_range_put_one(&mgr->m_free_objs, idx);
        }
        else {
            ++mgr->m_allocked_obj_count;
            cpe_range_put_one(&mgr->m_allocked_objs, idx);
        }
    }
}

CPE_HS_DEF_VAR(aom_control_class_name, "aom_control");
