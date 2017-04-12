#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_data_dyn.h"

static int logic_data_calc_dyn_capacity(LPDRMETA meta, size_t record_capacity, dr_meta_dyn_info_t dyn_info, error_monitor_t em);

int logic_data_record_is_dyn(logic_data_t data) {
    struct dr_meta_dyn_info dyn_info;
    return 
        dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0
        ? 1
        : 0;
}

LPDRMETA logic_data_record_meta(logic_data_t data) {
    LPDRMETA meta;
    struct dr_meta_dyn_info dyn_info;

    meta = logic_data_meta(data);

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        return dr_entry_ref_meta(dyn_info.m_data.m_array.m_array_entry);
    }
    else {
        return meta;
    }
}

static int logic_data_record_count_i(logic_data_t data, dr_meta_dyn_info_t dyn_info ) {
    if (dyn_info->m_data.m_array.m_refer_entry == NULL) {
        return dr_entry_array_count(dyn_info->m_data.m_array.m_array_entry);
    }
    else {
        char * buf = logic_data_data(data);
        uint32_t count;
        error_monitor_t em = logic_manage_em(logic_data_mgr(data));
        if (dr_entry_try_read_uint32(&count, buf + dyn_info->m_data.m_array.m_refer_start, dyn_info->m_data.m_array.m_refer_entry, em) != 0) {
            CPE_ERROR(
                em, "logic_data_record_count: read count fail, start-pos=%d!",
                dyn_info->m_data.m_array.m_refer_start);
            return -1;
        }
        else {
            return count;
        }
    }
}

int logic_data_record_count(logic_data_t data) {
    struct dr_meta_dyn_info dyn_info;

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);
        return logic_data_record_count_i(data, &dyn_info);
    }
    else {
        return 1;
    }
}

static size_t logic_data_record_capacity_i(logic_data_t data, dr_meta_dyn_info_t dyn_info) {
    size_t element_size = dr_entry_element_size(dyn_info->m_data.m_array.m_array_entry);

    return dr_entry_array_calc_ele_num(
        dyn_info->m_data.m_array.m_array_entry,
        logic_data_capacity(data) - (dr_meta_size(logic_data_meta(data)) - element_size));
}

size_t logic_data_record_capacity(logic_data_t data) {
    struct dr_meta_dyn_info dyn_info;

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);
        return logic_data_record_capacity_i(data, &dyn_info);
    }
    else {
        return 1;
    }
}

void * logic_data_record_at(logic_data_t data, int pos) {
    struct dr_meta_dyn_info dyn_info;

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        size_t record_capacity;

        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        record_capacity = logic_data_record_capacity_i(data, &dyn_info);
        if (pos >= (int)record_capacity) {
            CPE_ERROR(
                logic_manage_em(logic_data_mgr(data)),
                "logic_data_record_at: pos %d overflow, capacity = %d",
                pos, (int)record_capacity);
            return NULL;
        }

        return ((char *)logic_data_data(data))
            + dyn_info.m_data.m_array.m_array_start
            + (dr_entry_data_start_pos(dyn_info.m_data.m_array.m_array_entry, pos) 
               - dr_entry_data_start_pos(dyn_info.m_data.m_array.m_array_entry, 0));
    }
    else {
        if (pos != 0) {
            CPE_ERROR(
                logic_manage_em(logic_data_mgr(data)),
                "logic_data_record_at: pos %d overflow, fix meta, capacity = 1",
                pos);
            return NULL;
        }
        else {
            return logic_data_data(data);
        }
    }
}

size_t logic_data_record_size(logic_data_t data) {
    LPDRMETA meta;
    struct dr_meta_dyn_info dyn_info;

    meta = logic_data_meta(data);

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);
        return dr_entry_element_size(dyn_info.m_data.m_array.m_array_entry);
    }
    else {
        return dr_meta_size(meta);
    }
}

int logic_data_record_set_count(logic_data_t data, size_t record_count) {
    struct dr_meta_dyn_info dyn_info;

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        size_t record_capacity;
        char * buf;
        error_monitor_t em = logic_manage_em(logic_data_mgr(data));

        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        if (dyn_info.m_data.m_array.m_refer_entry == NULL) {
            CPE_ERROR(em, "logic_data_record_set_count: can`t set_size to not refer array!");
            return -1;
        }

        record_capacity = logic_data_record_capacity_i(data, &dyn_info);

        if (record_count > record_capacity) {
            CPE_ERROR(
                em, "logic_data_record_set_count: size overflow, size=%d, capacity=%d",
                (int)record_count, (int)record_capacity);
            return -1;
        }

        buf = logic_data_data(data);
        if (dr_entry_set_from_uint32(buf + dyn_info.m_data.m_array.m_refer_start, record_count, dyn_info.m_data.m_array.m_refer_entry, em) != 0){
            CPE_ERROR(em, "logic_data_record_set_count: set count %d fail!", (int)record_count);
            return -1;
        }

        return 0;
    }
    else {
        CPE_ERROR(
            logic_manage_em(logic_data_mgr(data)),
            "logic_data_record_set_count: can`t set_size to fix meta data!");
        return -1;
    }
}

static void * logic_data_record_append_i(logic_data_t * r, int auto_inc) {
    struct dr_meta_dyn_info dyn_info;
    logic_data_t data = *r;

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        int record_count;
        int record_capacity;
        char * buf;
        error_monitor_t em = logic_manage_em(logic_data_mgr(data));

        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        if (dyn_info.m_data.m_array.m_refer_entry == NULL) {
            CPE_ERROR(em, "logic_data_record_append: can`t append to not refer array!");
            return NULL;
        }

        record_capacity = logic_data_record_capacity_i(data, &dyn_info);
        record_count = logic_data_record_count_i(data, &dyn_info);
        if (record_count < 0) return NULL;

        if (record_count + 1 > record_capacity) {
            if (auto_inc) {
                size_t new_capacity  = record_capacity < 16 ? 16 : record_capacity * 2;
                data = logic_data_record_reserve(data, new_capacity);
                if (r) *r = data;
                if (data == NULL) {
                    CPE_ERROR(em, "logic_data_record_append: auto inc size to %d fail!", (int)new_capacity);
                    return NULL;
                }
            }
            else {
                CPE_ERROR(
                    em, "logic_data_record_append: size overflow, size=%d, capacity=%d",
                    record_count, record_capacity);
                return NULL;
            }
        }

        buf = logic_data_data(data);
        if (dr_entry_set_from_uint32(buf + dyn_info.m_data.m_array.m_refer_start, record_count + 1, dyn_info.m_data.m_array.m_refer_entry, em) != 0){
            CPE_ERROR(em, "logic_data_record_append: set count %d fail!", record_count);
            return NULL;
        }

        return ((char *)logic_data_data(data))
            + dyn_info.m_data.m_array.m_array_start
            + (dr_entry_data_start_pos(dyn_info.m_data.m_array.m_array_entry, record_count)
               - dr_entry_data_start_pos(dyn_info.m_data.m_array.m_array_entry, 0));
    }
    else {
        return logic_data_data(data);
    }
}

void * logic_data_record_append(logic_data_t data) {
    return logic_data_record_append_i(&data, 0);
}

void * logic_data_record_append_auto_inc(logic_data_t * data) {
    return logic_data_record_append_i(data, 1);
}

int logic_data_record_remove_by_pos(logic_data_t data, size_t pos) {
    struct dr_meta_dyn_info dyn_info;
    int record_count;
    size_t record_size;
    char * buf;
    error_monitor_t em = logic_manage_em(logic_data_mgr(data));

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) != 0) {
        CPE_ERROR(em, "logic_data_record_remove: can`t remove from fix data!");
        return -1;
    }

    if (dyn_info.m_data.m_array.m_refer_entry == NULL) {
        CPE_ERROR(em, "logic_data_record_remove: can`t remove not refer array!");
        return -1;
    }

    record_count = logic_data_record_count_i(data, &dyn_info);
    record_size = dr_entry_element_size(dyn_info.m_data.m_array.m_array_entry);

    if (record_count < 0) {
        CPE_ERROR(em, "logic_data_record_remove: record cout %d error!", record_count);
        return -1;
    }

    if (pos >= record_count) {
        CPE_ERROR(em, "logic_data_record_remove: remove pos %d overflow, count %d!", (int)pos, record_count);
        return -1;
    }

    buf = logic_data_data(data);
    if (pos + 1 < record_count) {
        memmove(
            buf + dyn_info.m_data.m_array.m_refer_start + record_size * pos,
            buf + dyn_info.m_data.m_array.m_refer_start + record_size * (pos + 1),
            record_size * (record_count - pos - 1)); 
    }

    if (dr_entry_set_from_uint32(buf + dyn_info.m_data.m_array.m_refer_start, record_count - 1, dyn_info.m_data.m_array.m_refer_entry, em) != 0){
        CPE_ERROR(em, "logic_data_record_remove: set count %d fail!", record_count -1);
        return -1;
    }

    return 0;
}

int logic_data_record_remove_by_ins(logic_data_t data, void * p) {
    struct dr_meta_dyn_info dyn_info;
    int record_count;
    size_t record_size;
    char * buf;
    int pos;
    error_monitor_t em = logic_manage_em(logic_data_mgr(data));

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) != 0) {
        CPE_ERROR(em, "logic_data_record_remove: can`t remove from fix data!");
        return -1;
    }

    if (dyn_info.m_data.m_array.m_refer_entry == NULL) {
        CPE_ERROR(em, "logic_data_record_remove: can`t remove not refer array!");
        return -1;
    }

    record_count = logic_data_record_count_i(data, &dyn_info);
    record_size = dr_entry_element_size(dyn_info.m_data.m_array.m_array_entry);

    if (record_count < 0) {
        CPE_ERROR(em, "logic_data_record_remove: record cout %d error!", record_count);
        return -1;
    }

    buf = logic_data_data(data);

    if ((char *)p < buf || (char *)p >= (buf + record_count * record_size)) {
        CPE_ERROR(em, "logic_data_record_remove: remove %p address not in range!", p);
        return -1;
    }

    if ((((char *)p - buf) - dyn_info.m_data.m_array.m_array_start) % record_size) {
        CPE_ERROR(em, "logic_data_record_remove: remove %p address not regular!", p);
        return -1;
    }

    pos = ((((char *)p - buf) - dyn_info.m_data.m_array.m_array_start) % record_size);
    assert(pos >= 0 && pos < record_count);

    if (pos + 1 < record_count) {
        memmove(
            buf + record_size * pos,
            buf + record_size * (pos + 1),
            record_size * (record_count - pos - 1)); 
    }

    if (dr_entry_set_from_uint32(buf + dyn_info.m_data.m_array.m_refer_start, record_count - 1, dyn_info.m_data.m_array.m_refer_entry, em) != 0){
        CPE_ERROR(em, "logic_data_record_remove: set count %d fail!", record_count -1);
        return -1;
    }

    return 0;
}

static int logic_data_calc_dyn_capacity(LPDRMETA meta, size_t record_capacity, dr_meta_dyn_info_t dyn_info, error_monitor_t em) {
    assert(dyn_info->m_data.m_array.m_array_entry);

    if (dr_entry_array_count(dyn_info->m_data.m_array.m_array_entry) > 1) {
        
        if (dr_entry_array_count(dyn_info->m_data.m_array.m_array_entry) < (int)record_capacity) {
            CPE_ERROR(
                em,
                "logic_data_calc_capacity: record_capacity %d overflow, max count is %d!",
                (int)record_capacity,
                dr_entry_array_count(dyn_info->m_data.m_array.m_array_entry));
            return -1;
        }
        else {
            return dr_meta_size(meta);
        }
    }
    else {
        size_t record_size = dr_entry_element_size(dyn_info->m_data.m_array.m_array_entry);
        return dr_meta_size(meta) + (record_capacity > 1 ? record_size * (record_capacity - 1) : 0);
    }
}

int logic_data_calc_capacity(LPDRMETA meta, size_t record_capacity, error_monitor_t em) {
    struct dr_meta_dyn_info dyn_info;

    if (dr_meta_find_dyn_info(meta, &dyn_info) == 0) {
        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);
        return logic_data_calc_dyn_capacity(meta, record_capacity, &dyn_info, em);
    }
    else {
        if (record_capacity > 1) {
            CPE_ERROR(
                em,
                "logic_data_calc_capacity: record_capacity %d error, %s is not dynmiac, only support up to capacity 1!",
                (int)record_capacity, dr_meta_name(meta));
            return -1;
        }
        else {
            return dr_meta_size(meta);
        }
    }
}

logic_data_t logic_data_record_reserve(logic_data_t data, size_t new_record_capacity) {
    LPDRMETA meta;
    struct dr_meta_dyn_info dyn_info;

    meta = logic_data_meta(data);

    if (dr_meta_find_dyn_info(logic_data_meta(data), &dyn_info) == 0) {
        size_t record_capacity;
        int new_data_capacity;
        error_monitor_t em = logic_manage_em(logic_data_mgr(data));

        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        record_capacity = logic_data_record_capacity_i(data, &dyn_info);
        if (new_record_capacity <= record_capacity) return data;


        if (dyn_info.m_data.m_array.m_refer_entry == NULL) {
            CPE_ERROR(em, "logic_data_record_reserve: can`t reserve to not refer array!");
            return NULL;
        }

        new_data_capacity = logic_data_calc_dyn_capacity(meta, new_record_capacity, &dyn_info, em);
        if (new_data_capacity < 0) {
            CPE_ERROR(
                em, "logic_data_record_reserve: not enough data, calc new data size fail, record_capacity=%d!",
                (int)new_record_capacity);
            return NULL;
        }

        data = logic_data_resize(data, new_data_capacity);
        if (data == NULL) {
            CPE_ERROR(
                em, "logic_data_record_reserve: not enough data, resize fail, record_capacity=%d, data_capacity=%d!",
                (int)new_record_capacity, new_data_capacity);
            return NULL;
        }

        return data;
    }
    else {
        CPE_ERROR(
            logic_manage_em(logic_data_mgr(data)),
            "logic_data_record_reserve: can`t reserve to fix meta data!");
        return NULL;
    }
}

void logic_data_record_sort(logic_data_t data, int(*cmp)(const void *, const void *)) {
    LPDRMETA meta;
    struct dr_meta_dyn_info dyn_info;

    meta = logic_data_meta(data);

    if (dr_meta_find_dyn_info(meta, &dyn_info) == 0) {
        size_t record_size;
        int record_count;

        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        record_size = dr_entry_element_size(dyn_info.m_data.m_array.m_array_entry);
        record_count = logic_data_record_count_i(data, &dyn_info);
        if (record_count < 0) {
            CPE_ERROR(
                logic_manage_em(logic_data_mgr(data)),
                "logic_data_record_sort: get count fail, count = %d",
                record_count);
            return;
        }

        qsort(
            ((char *)logic_data_data(data)) + dyn_info.m_data.m_array.m_array_start,
            record_count, record_size, cmp);
    }
}

void * logic_data_record_find(logic_data_t data, void const * key, int(*cmp)(const void *, const void *)) {
    LPDRMETA meta;
    struct dr_meta_dyn_info dyn_info;

    meta = logic_data_meta(data);

    if (dr_meta_find_dyn_info(meta, &dyn_info) == 0) {
        size_t record_size;
        int record_count;

        assert(dyn_info.m_type == dr_meta_dyn_info_type_array);

        record_size = dr_entry_element_size(dyn_info.m_data.m_array.m_array_entry);
        record_count = logic_data_record_count_i(data, &dyn_info);
        if (record_count < 0) {
            CPE_ERROR(
                logic_manage_em(logic_data_mgr(data)),
                "logic_data_record_find: get count fail, count = %d",
                record_count);
            return NULL;
        }

        return bsearch(
            key,
            ((char *)logic_data_data(data)) + dyn_info.m_data.m_array.m_array_start,
            record_count, record_size, cmp);
    }
    else {
        void * data_p = logic_data_data(data);
        return cmp(data_p, key) == 0
            ? data_p
            : NULL;
    }
}
