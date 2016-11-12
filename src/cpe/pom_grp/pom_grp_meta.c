#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"
#include "pom_grp_data.h"

pom_grp_meta_t
pom_grp_meta_create(
    mem_allocrator_t alloc,
    const char * name,
    uint32_t omm_page_size)
{
    char * buf;
    size_t name_len;
    pom_grp_meta_t meta;

    name_len = strlen(name) + 1;
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(alloc, name_len + sizeof(struct pom_grp_meta));
    if (buf == NULL) return NULL;

    cpe_str_dup(buf, name_len, name);

    meta = (pom_grp_meta_t)(buf + name_len);
    meta->m_alloc = alloc;
    meta->m_name = buf;
    meta->m_omm_page_size = omm_page_size;
    meta->m_control_class_id = 1;
    meta->m_control_obj_size = 0;
    meta->m_page_count = 0;
    meta->m_size_buf_start = 0;
    meta->m_size_buf_count = 0;

    meta->m_entry_count = 0;
    meta->m_entry_capacity = 0;
    meta->m_entry_buf = NULL; 
    meta->m_main_entry = NULL;

    if (cpe_hash_table_init(
            &meta->m_entry_ht,
            alloc,
            (cpe_hash_fun_t) pom_grp_entry_meta_hash,
            (cpe_hash_eq_t) pom_grp_entry_meta_cmp,
            CPE_HASH_OBJ2ENTRY(pom_grp_entry_meta, m_hh),
            -1) != 0)
    {
        mem_free(alloc, buf);
        return NULL;
    }

    return meta;
}

void pom_grp_meta_free(pom_grp_meta_t meta) {
    pom_grp_entry_meta_free_all(meta);

    if (meta->m_entry_buf) {
        mem_free(meta->m_alloc, meta->m_entry_buf);
    }

    cpe_hash_table_fini(&meta->m_entry_ht);

    mem_free(meta->m_alloc, (void*)meta->m_name);
}

uint16_t pom_grp_meta_entry_count(pom_grp_meta_t meta) {
    return meta->m_entry_count;
}

pom_grp_entry_meta_t pom_grp_meta_entry_at(pom_grp_meta_t meta, uint16_t pos) {
    assert(pos < meta->m_entry_count);
    return meta->m_entry_buf[pos];
}

const char * pom_grp_meta_name(pom_grp_meta_t meta) {
    return meta->m_name;
}

uint32_t pom_grp_omm_page_size(pom_grp_meta_t meta) {
    return meta->m_omm_page_size;
}

int pom_grp_meta_set_main_entry(pom_grp_meta_t meta, const char * entry_name) {
    if (meta->m_main_entry) return -1;

    meta->m_main_entry = pom_grp_entry_meta_find(meta, entry_name);
    if (meta->m_main_entry == NULL) return -1;

    return 0;
}

pom_grp_entry_meta_t pom_grp_meta_main_entry(pom_grp_meta_t meta) {
    return meta->m_main_entry;
}

pom_grp_entry_meta_t
pom_grp_entry_meta_find(pom_grp_meta_t meta, const char * name) {
    struct pom_grp_entry_meta key;
    key.m_name = name;

    return (pom_grp_entry_meta_t)cpe_hash_table_find(&meta->m_entry_ht, &key);
}

pom_grp_entry_meta_t
pom_grp_entry_meta_at(pom_grp_meta_t meta, uint16_t pos) {
    if (pos >= meta->m_entry_count) return NULL;

    return meta->m_entry_buf[pos];
}

static pom_grp_entry_meta_t pom_grp_entry_meta_it_next(struct pom_grp_entry_meta_it * it) {
    pom_grp_entry_meta_t r;
    if (it->m_data == NULL) return NULL;

    r = it->m_data;

    it->m_data =
        (r->m_index + 1) < r->m_meta->m_entry_count
        ? r->m_meta->m_entry_buf[r->m_index + 1]
        : NULL;

    return r;
}

void pom_grp_entry_meta_it_init(pom_grp_meta_t meta, pom_grp_entry_meta_it_t it) {
    it->m_data = meta->m_entry_count > 0 ? meta->m_entry_buf[0] : NULL;
    it->next = pom_grp_entry_meta_it_next;
}

void pom_grp_meta_dump(write_stream_t stream, pom_grp_meta_t meta, int ident) {
    pom_grp_entry_meta_t entry;
    uint16_t i;

    stream_putc_count(stream, ' ', ident);
    stream_printf(stream, "pom_grp_meta: name=%s", meta->m_name);

    stream_printf(
        stream, ", page-size=%d, class-id=%d, obj-size=%d, page-count=%d, size-buf-start=%d, size-buf-count=%d",
        meta->m_omm_page_size, (int)meta->m_control_class_id,
        meta->m_control_obj_size, meta->m_page_count, meta->m_size_buf_start, meta->m_size_buf_count);

    for(i = 0; i < meta->m_entry_count; ++i) {
        entry = meta->m_entry_buf[i];

        stream_printf(stream, "\n");
        stream_putc_count(stream, ' ', ident ? (ident << 2) : 4);
        stream_printf(stream, "%s: ", entry->m_name);

        switch(entry->m_type) {
        case pom_grp_entry_type_normal:
            stream_printf(
                stream, "entry-type=normal, data=type=%s",
                dr_meta_name(entry->m_data.m_normal.m_data_meta));
            break;
        case pom_grp_entry_type_list:
            stream_printf(
                stream, "entry-type=list, data=type=%s, capacity=%d, size-idx=%d",
                dr_meta_name(entry->m_data.m_list.m_data_meta),
                (int)entry->m_data.m_list.m_capacity,
                (int)entry->m_data.m_list.m_size_idx);
            break;
        case pom_grp_entry_type_ba:
            stream_printf(
                stream, "entry-type=ba, bit-capacity=%d",
                (int)entry->m_data.m_ba.m_bit_capacity);
            break;
        case pom_grp_entry_type_binary:
            stream_printf(
                stream, "entry-type=binary, capacity=%d",
                (int)entry->m_data.m_binary.m_capacity);
            break;
        default:
            stream_printf(stream, "entry-type=unknown");
            break;
        }

        stream_printf(
            stream, ", page-begin=%d, page-count=%d, class-id=%d, page-size=%d, obj-align=%d",
            entry->m_page_begin, entry->m_page_count, entry->m_class_id, entry->m_page_size, entry->m_obj_align);
    }
}

int pom_grp_meta_compatable(pom_grp_meta_t l, pom_grp_meta_t r) {
    uint16_t i, count;

    if (l->m_omm_page_size != r->m_omm_page_size
        || l->m_control_class_id != r->m_control_class_id
        || l->m_control_obj_size != r->m_control_obj_size
        || l->m_page_count != r->m_page_count
        || l->m_size_buf_start != r->m_size_buf_start
        || l->m_size_buf_count != r->m_size_buf_count
        || l->m_entry_count != r->m_entry_count
        || l->m_entry_capacity != r->m_entry_capacity
        || strcmp(l->m_name, r->m_name) != 0)
    {
        return 0;
    }

    count = l->m_entry_count;
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t l_em = l->m_entry_buf[i];
        pom_grp_entry_meta_t r_em = r->m_entry_buf[i];

        assert(l_em);
        assert(r_em);

        if (l_em->m_index != r_em->m_index
            || l_em->m_type != r_em->m_type
            || l_em->m_page_begin != r_em->m_page_begin
            || l_em->m_page_count != r_em->m_page_count
            || l_em->m_class_id != r_em->m_class_id
            || l_em->m_page_size != r_em->m_page_size
            || l_em->m_obj_align != r_em->m_obj_align
            || strcmp(l_em->m_name, r_em->m_name) != 0)
        {
            return 0;
        }

        switch(l_em->m_type) {
        case pom_grp_entry_type_normal:
            if (!dr_meta_compatible(l_em->m_data.m_normal.m_data_meta, r_em->m_data.m_normal.m_data_meta)) {
                return 0;
            }
            break;
        case pom_grp_entry_type_list:
            if (l_em->m_data.m_list.m_capacity != r_em->m_data.m_list.m_capacity
                || l_em->m_data.m_list.m_size_idx != r_em->m_data.m_list.m_size_idx
                || l_em->m_data.m_list.m_standalone != r_em->m_data.m_list.m_standalone
                || !dr_meta_compatible(l_em->m_data.m_list.m_data_meta, r_em->m_data.m_list.m_data_meta))
            {
                return 0;
            }
            break;
        case pom_grp_entry_type_ba:
            if (l_em->m_data.m_ba.m_bit_capacity != r_em->m_data.m_ba.m_bit_capacity) {
                return 0;
            }
            break;
        case pom_grp_entry_type_binary:
            if (l_em->m_data.m_binary.m_capacity != r_em->m_data.m_binary.m_capacity) {
                return 0;
            }
            break;
        }
    }

    return 1;
}
