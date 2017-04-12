#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_store_build_obj_normal(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void const * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;
    void * buf;

    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    buf  = pom_grp_obj_normal_check_or_create_ex(obj_mgr, obj, entry->m_entry_meta);
    if (buf == NULL) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: normal entry %s: get buf fail!",
            entry->m_entry_meta->m_name);
        return -1;
    }

    if (dr_entry_data_start_pos(data_entry, 0) + dr_entry_element_size(data_entry) > data_size) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: notrmal entry %s: %d + %d overflow, data_size=%d!",
            entry->m_entry_meta->m_name,
            (int)dr_entry_data_start_pos(data_entry, 0),
            (int)dr_entry_element_size(data_entry),
            (int)data_size);
        return -1;
    }

    if (dr_meta_copy_same_entry(
            buf, pom_grp_entry_meta_normal_capacity(entry->m_entry_meta), pom_grp_entry_meta_normal_meta(entry->m_entry_meta),
            (const char *)data_buf + dr_entry_data_start_pos(data_entry, 0),
            dr_entry_element_size(data_entry), dr_entry_ref_meta(data_entry),
            0, obj_mgr->m_em) < 0)
    {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: notrmal entry %s: copy_same_entry fail!",
            entry->m_entry_meta->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_store_build_obj_list(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void const * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;
    LPDRMETAENTRY count_entry;

    LPDRMETA src_element_meta;
    size_t src_element_size;

    LPDRMETA des_element_meta;
    char * des_element_buf;
    size_t des_element_size;

    char count_entry_name[128];
    uint32_t i, count;
    
    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    snprintf(count_entry_name, sizeof(count_entry_name), "%sCount", entry->m_name);
    count_entry = dr_meta_find_entry_by_name(data_meta, count_entry_name);
    if (count_entry == NULL) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: list entry %s: find count entry %s fail!",
            entry->m_entry_meta->m_name, count_entry_name);
        return -1;
    }

    if (dr_entry_try_read_uint32(&count, ((const char *)data_buf) + dr_entry_data_start_pos(count_entry, 0), count_entry, obj_mgr->m_em) != 0) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: list entry %s: read count!",
            entry->m_entry_meta->m_name);
        return -1;
    }

    src_element_meta = dr_entry_ref_meta(data_entry);
    src_element_size = dr_entry_element_size(data_entry);

    des_element_meta = pom_grp_entry_meta_list_meta(entry->m_entry_meta);
    des_element_size = dr_meta_size(des_element_meta);

    for(i = 0; i < count; ++i, des_element_buf += des_element_size) {
        if (pom_grp_obj_list_append_ex(obj_mgr, obj, entry->m_entry_meta, NULL) != 0) {
            CPE_ERROR(
                obj_mgr->m_em, "pom_grp_store_build_obj: list entry %s: %d: append entry fail!",
                entry->m_entry_meta->m_name, i);
            return -1;
        }

        des_element_buf = pom_grp_obj_list_at_ex(obj_mgr, obj, entry->m_entry_meta, i);
        assert(des_element_buf);

        if (dr_meta_copy_same_entry(
                des_element_buf, des_element_size, des_element_meta,
                ((char const *)data_buf) + dr_entry_data_start_pos(data_entry, i),
                src_element_size, src_element_meta,
                0, obj_mgr->m_em) < 0)
        {
            CPE_ERROR(
                obj_mgr->m_em, "pom_grp_store_build_obj: list entry %s: %d: copy data fail!",
                entry->m_entry_meta->m_name, (int)dr_entry_size(data_entry));
            return -1;
        }
    }

    return 0;
}

static int pom_grp_store_build_obj_ba(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void const * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;

    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    if (pom_grp_obj_ba_set_binary_ex(
            obj_mgr, obj, entry->m_entry_meta,
            ((char const *)data_buf) + dr_entry_data_start_pos(data_entry, 0),
            (uint32_t)dr_entry_size(data_entry)) != 0)
    {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: ba entry %s: set data fail, size=%d!",
            entry->m_entry_meta->m_name, (int)dr_entry_size(data_entry));
        return -1;
    }

    return 0;
}

static int pom_grp_store_build_obj_binary(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void const * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;

    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    if (pom_grp_obj_binary_set_ex(
            obj_mgr, obj, entry->m_entry_meta,
            ((char const *)data_buf) + dr_entry_data_start_pos(data_entry, 0),
            dr_entry_size(data_entry)) != 0)
    {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_build_obj: binary entry %s: set data fail, size=%d!",
            entry->m_entry_meta->m_name, (int)dr_entry_size(data_entry));
        return -1;
    }

    return 0;
}


int pom_grp_store_build_obj(
    pom_grp_store_t store, pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj,
    void const * data_buf, size_t data_size, LPDRMETA data_meta)
{
    pom_grp_store_table_t store_table;
    struct pom_grp_store_entry_it entry_it;
    pom_grp_store_entry_t entry;
    int rv;

    store_table = pom_grp_store_table_find(store, dr_meta_name(data_meta));
    if (store_table == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_build_obj: table %s not exist in store",
            dr_meta_name(data_meta));
        return -1;
    }

    pom_grp_table_entries(store_table, &entry_it);

    rv = 0;
    while((entry = pom_grp_store_entry_it_next(&entry_it))) {
        switch(entry->m_entry_meta->m_type) {
        case pom_grp_entry_type_normal: {
            if (pom_grp_store_build_obj_normal(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        }
        case pom_grp_entry_type_list:
            if (pom_grp_store_build_obj_list(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_store_build_obj_ba(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_store_build_obj_binary(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        }
    }

    return rv;
}

static int pom_grp_store_write_obj_normal(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;
    void const * buf;

    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    buf  = pom_grp_obj_normal_ex(obj_mgr, obj, entry->m_entry_meta);
    if (buf == NULL) return 0;

    if (dr_entry_data_start_pos(data_entry, 0) + dr_entry_element_size(data_entry) > data_size) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: notrmal entry %s: %d + %d overflow, data_size=%d!",
            entry->m_entry_meta->m_name,
            (int)dr_entry_data_start_pos(data_entry, 0),
            (int)dr_entry_element_size(data_entry),
            (int)data_size);
        return -1;
    }

    if (dr_meta_copy_same_entry(
            (char *)data_buf + dr_entry_data_start_pos(data_entry, 0), dr_entry_element_size(data_entry), dr_entry_ref_meta(data_entry),
            buf, pom_grp_entry_meta_normal_capacity(entry->m_entry_meta), pom_grp_entry_meta_normal_meta(entry->m_entry_meta),
            0, obj_mgr->m_em) < 0)
    {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: notrmal entry %s: copy_same_entry fail!",
            entry->m_entry_meta->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_store_write_obj_list(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;
    LPDRMETAENTRY count_entry;

    LPDRMETA des_element_meta;
    size_t des_element_size;

    LPDRMETA src_element_meta;
    const char * src_element_buf;
    size_t src_element_size;

    uint32_t i, count;
    
    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    count_entry = dr_entry_array_refer_entry(data_entry);
    if (count_entry == NULL) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: list entry %s: find count entry fail!",
            entry->m_entry_meta->m_name);
        return -1;
    }

    count = pom_grp_obj_list_count_ex(obj_mgr, obj, entry->m_entry_meta);
    if (count > dr_entry_array_count(data_entry)) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: list entry %s: count %d overflow, output-count=%d!",
            entry->m_entry_meta->m_name, count, dr_entry_array_count(data_entry));
        return -1;
    }

    if (dr_entry_set_from_uint32(((char *)data_buf) + dr_entry_data_start_pos(count_entry, 0), count, count_entry, obj_mgr->m_em) != 0) {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: list entry %s: set count error!",
            entry->m_entry_meta->m_name);
        return -1;
    }

    des_element_meta = dr_entry_ref_meta(data_entry);
    des_element_size = dr_entry_element_size(data_entry);

    src_element_meta = pom_grp_entry_meta_list_meta(entry->m_entry_meta);
    src_element_size = dr_meta_size(des_element_meta);

    for(i = 0; i < count; ++i) {
        src_element_buf = pom_grp_obj_list_at_ex(obj_mgr, obj, entry->m_entry_meta, i);
        assert(src_element_buf);

        if (dr_meta_copy_same_entry(
                ((char *)data_buf) + dr_entry_data_start_pos(data_entry, i),
                des_element_size, des_element_meta,
                src_element_buf, src_element_size, src_element_meta,
                0, obj_mgr->m_em) < 0)
        {
            CPE_ERROR(
                obj_mgr->m_em, "pom_grp_store_write_obj: list entry %s: %d: copy data fail!",
                entry->m_entry_meta->m_name, (int)dr_entry_size(data_entry));
            return -1;
        }
    }

    return 0;
}

static int pom_grp_store_write_obj_ba(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;

    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    if (pom_grp_obj_ba_get_binary_ex(
            obj_mgr, obj, entry->m_entry_meta,
            ((char *)data_buf) + dr_entry_data_start_pos(data_entry, 0),
            (uint32_t)dr_entry_size(data_entry)) != 0)
    {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: binary entry %s: set data fail, size=%d!",
            entry->m_entry_meta->m_name, (int)dr_entry_size(data_entry));
        return -1;
    }

    return 0;
}

static int pom_grp_store_write_obj_binary(
    pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, pom_grp_store_entry_t entry,
    void * data_buf, size_t data_size, LPDRMETA data_meta)
{
    LPDRMETAENTRY data_entry;

    data_entry = dr_meta_find_entry_by_name(data_meta, entry->m_name);
    if (data_entry == NULL) return 0;

    if (pom_grp_obj_binary_get_ex(
            obj_mgr, obj, entry->m_entry_meta,
            ((char *)data_buf) + dr_entry_data_start_pos(data_entry, 0),
            dr_entry_size(data_entry)) != 0)
    {
        CPE_ERROR(
            obj_mgr->m_em, "pom_grp_store_write_obj: binary entry %s: set data fail, size=%d!",
            entry->m_entry_meta->m_name, (int)dr_entry_size(data_entry));
        return -1;
    }

    return 0;
}


int pom_grp_store_write_obj(
    pom_grp_store_t store, pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj,
    void * data_buf, size_t data_size, LPDRMETA data_meta)
{
    pom_grp_store_table_t store_table;
    struct pom_grp_store_entry_it entry_it;
    pom_grp_store_entry_t entry;
    int rv;

    bzero(data_buf, data_size);

    store_table = pom_grp_store_table_find(store, dr_meta_name(data_meta));
    if (store_table == NULL) {
        CPE_ERROR(
            store->m_em, "pom_grp_store_write_obj: table %s not exist in store",
            dr_meta_name(data_meta));
        return -1;
    }

    pom_grp_table_entries(store_table, &entry_it);

    rv = 0;
    while((entry = pom_grp_store_entry_it_next(&entry_it))) {
        switch(entry->m_entry_meta->m_type) {
        case pom_grp_entry_type_normal: {
            if (pom_grp_store_write_obj_normal(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        }
        case pom_grp_entry_type_list:
            if (pom_grp_store_write_obj_list(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_store_write_obj_ba(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_store_write_obj_binary(obj_mgr, obj, entry,  data_buf, data_size, data_meta) != 0) {
                rv = -1;
            }
            break;
        }
    }

    return rv;
}
