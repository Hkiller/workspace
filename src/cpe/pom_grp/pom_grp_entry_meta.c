#include <assert.h>
#include "cpe/utils/bitarry.h"
#include "cpe/utils/error.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static pom_grp_entry_meta_t
pom_grp_entry_meta_create_i(
    pom_grp_meta_t meta,
    const char * entry_name,
    pom_grp_entry_type_t type,
    uint16_t page_count,
    uint16_t obj_size,
    uint16_t obj_align,
    error_monitor_t em)
{
    char * buf;
    size_t name_len;
    pom_grp_entry_meta_t entry_meta;
    pom_grp_entry_meta_t pre_entry_meta;

    if (meta->m_entry_count >= meta->m_entry_capacity) {
        uint16_t new_capacity = meta->m_entry_capacity < 16 ? 16 : meta->m_entry_capacity * 2;
        pom_grp_entry_meta_t * new_buffer = 
            (pom_grp_entry_meta_t *)mem_alloc(meta->m_alloc, new_capacity * sizeof(pom_grp_entry_meta_t));
        if (new_buffer == NULL) {
            CPE_ERROR(em, "alloc new pom_grp_entry_meta array fail!");
            return NULL;
        }

        if (meta->m_entry_count) {
            assert(meta->m_entry_buf);
            memcpy(new_buffer, meta->m_entry_buf, sizeof(pom_grp_entry_meta_t) * meta->m_entry_count);
        }

        if (meta->m_entry_buf) mem_free(meta->m_alloc, meta->m_entry_buf);
        meta->m_entry_buf = new_buffer;
        meta->m_entry_capacity = new_capacity;
    }

    assert(meta->m_entry_count < meta->m_entry_capacity);

    name_len = cpe_hs_len_to_binary_len(strlen(entry_name));
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(meta->m_alloc, name_len + sizeof(struct pom_grp_entry_meta));
    if (buf == NULL) return NULL;

    cpe_hs_init((cpe_hash_string_t)buf, name_len, entry_name);

    pre_entry_meta = meta->m_entry_count > 0 ? meta->m_entry_buf[meta->m_entry_count - 1] : NULL;

    entry_meta = (pom_grp_entry_meta_t)(buf + name_len);
    entry_meta->m_meta = meta;
    entry_meta->m_name = cpe_hs_data((cpe_hash_string_t)buf);
    entry_meta->m_index = meta->m_entry_count;
    entry_meta->m_type = type;
    entry_meta->m_page_size = obj_size;
    entry_meta->m_obj_align = obj_align;

    entry_meta->m_page_begin = 
        pre_entry_meta
        ? pre_entry_meta->m_page_begin + pre_entry_meta->m_page_count
        : 0;

    entry_meta->m_page_count = page_count;

    entry_meta->m_class_id = 
        pre_entry_meta
        ? pre_entry_meta->m_class_id + 1
        : meta->m_control_class_id + 1;

    cpe_hash_entry_init(&entry_meta->m_hh);
    if (cpe_hash_table_insert_unique(&meta->m_entry_ht, entry_meta) != 0) {
        mem_free(meta->m_alloc, buf);
        return NULL;
    }

    meta->m_entry_buf[meta->m_entry_count] = entry_meta;
    ++(meta->m_entry_count);

    meta->m_page_count += page_count;
    meta->m_control_obj_size += page_count * sizeof(pom_oid_t);
    meta->m_size_buf_start += page_count * sizeof(pom_oid_t);

    return entry_meta;
}

pom_grp_entry_meta_t
pom_grp_entry_meta_normal_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta,
    error_monitor_t em)
{
    pom_grp_entry_meta_t r =
        pom_grp_entry_meta_create_i(
            meta,
            entry_name,
            pom_grp_entry_type_normal,
            1,
            dr_meta_size(entry_meta),
            dr_meta_align(entry_meta), em);
    if (r) r->m_data.m_normal.m_data_meta = entry_meta;
    return r;
}

pom_grp_entry_meta_t
pom_grp_entry_meta_list_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta, uint32_t count_per_page, uint32_t capacity, int standalone,
    error_monitor_t em)
{
    pom_grp_entry_meta_t r;
    
    uint16_t page_count = capacity / count_per_page;
    if (capacity % count_per_page) page_count += 1;

    r =
        pom_grp_entry_meta_create_i(
            meta, entry_name, pom_grp_entry_type_list,
            page_count,
            count_per_page * dr_meta_size(entry_meta), dr_meta_align(entry_meta),
            em);

    if (r) {
        r->m_data.m_list.m_data_meta = entry_meta;
        r->m_data.m_list.m_capacity = capacity;
        r->m_data.m_list.m_size_idx = meta->m_size_buf_count;
        r->m_data.m_list.m_standalone = standalone;
        meta->m_size_buf_count += 1;
        meta->m_control_obj_size += sizeof(uint16_t);
    }

    return r;
}

pom_grp_entry_meta_t
pom_grp_entry_meta_ba_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    uint16_t byte_per_page, uint16_t bit_capacity,
    error_monitor_t em)
{
    uint16_t byte_capacity = cpe_ba_bytes_from_bits(bit_capacity);
    pom_grp_entry_meta_t r;
    uint16_t page_count;

    page_count = byte_capacity / byte_per_page;
    if (byte_capacity % byte_per_page) page_count += 1;

    r =
        pom_grp_entry_meta_create_i(
            meta, entry_name, pom_grp_entry_type_ba,
            page_count, byte_per_page, 1, em);

    if (r) {
        r->m_data.m_ba.m_bit_capacity = bit_capacity;
    }

    return r;
}

pom_grp_entry_meta_t
pom_grp_entry_meta_binary_create(
    pom_grp_meta_t meta,
    const char * entry_name,
    uint32_t capacity,
    error_monitor_t em)
{
    pom_grp_entry_meta_t r;

    r = pom_grp_entry_meta_create_i(
        meta, entry_name, pom_grp_entry_type_binary, 1, capacity, 1, em);

    if (r) r->m_data.m_binary.m_capacity = capacity;

    return r;
}

void pom_grp_entry_meta_free(pom_grp_entry_meta_t entry_meta) {
    uint16_t i;
    pom_grp_meta_t meta = entry_meta->m_meta;

    assert(meta->m_entry_buf);
    assert(entry_meta->m_index < meta->m_entry_count);
    assert(meta->m_entry_buf[entry_meta->m_index] == entry_meta);

    if (entry_meta->m_index + 1 < meta->m_entry_count) {
        memmove(
            meta->m_entry_buf + entry_meta->m_index,
            meta->m_entry_buf + entry_meta->m_index + 1,
            sizeof(pom_grp_entry_meta_t) * meta->m_entry_count - entry_meta->m_index - 1);
    }        
    -- meta->m_entry_count;

    for(i = entry_meta->m_index; i < meta->m_entry_count; ++i) {
        assert(meta->m_entry_buf[i]);
        -- meta->m_entry_buf[i]->m_index;
    }

    cpe_hash_table_remove_by_ins(&meta->m_entry_ht, entry_meta);

    mem_free(meta->m_alloc, (void*)cpe_hs_from_str(entry_meta->m_name));
}

const char * pom_grp_entry_meta_name(pom_grp_entry_meta_t entry_meta) {
    return entry_meta->m_name;
}

cpe_hash_string_t pom_grp_entry_meta_name_hs(pom_grp_entry_meta_t entry_meta) {
    return cpe_hs_from_str(entry_meta->m_name);
}

pom_grp_entry_type_t pom_grp_entry_meta_type(pom_grp_entry_meta_t entry_meta) {
    return entry_meta->m_type;
}

uint16_t pom_grp_entry_meta_index(pom_grp_entry_meta_t entry_meta) {
    return entry_meta->m_index;
}

uint16_t pom_grp_entry_meta_page_count(pom_grp_entry_meta_t entry_meta) {
    return entry_meta->m_page_count;
}

uint16_t pom_grp_entry_meta_page_size(pom_grp_entry_meta_t entry_meta) {
    return entry_meta->m_page_size;
}

LPDRMETA pom_grp_entry_meta_normal_meta(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_normal);
    return entry_meta->m_data.m_normal.m_data_meta;
}

uint16_t pom_grp_entry_meta_normal_capacity(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_normal);
    return entry_meta->m_page_size;
}

LPDRMETA pom_grp_entry_meta_list_meta(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_list);
    return entry_meta->m_data.m_list.m_data_meta;
}

uint16_t pom_grp_entry_meta_list_capacity(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_list);
    return entry_meta->m_data.m_list.m_capacity;
}

uint32_t pom_grp_entry_meta_ba_bits(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_ba);
    return entry_meta->m_data.m_ba.m_bit_capacity;
}

uint32_t pom_grp_entry_meta_ba_bytes(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_ba);
    return (uint32_t)cpe_ba_bytes_from_bits(entry_meta->m_data.m_ba.m_bit_capacity);
}

uint16_t pom_grp_entry_meta_binary_capacity(pom_grp_entry_meta_t entry_meta) {
    assert(entry_meta->m_type == pom_grp_entry_type_binary);
    return entry_meta->m_data.m_binary.m_capacity;
}

uint32_t pom_grp_entry_meta_hash(const struct pom_grp_entry_meta * entry_meta) {
    return cpe_hash_str(entry_meta->m_name, strlen(entry_meta->m_name));
}

int pom_grp_entry_meta_cmp(const struct pom_grp_entry_meta * l, const struct pom_grp_entry_meta * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void pom_grp_entry_meta_free_all(pom_grp_meta_t meta) {
    while(meta->m_entry_count > 0) {
        pom_grp_entry_meta_free(meta->m_entry_buf[0]);
    }
}
