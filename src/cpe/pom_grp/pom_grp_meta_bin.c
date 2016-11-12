#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

struct pom_grp_meta_data {
    uint16_t m_magic;
    uint16_t m_size;
    uint16_t m_name_pos;
    uint16_t m_main_entry_name_pos;
    uint16_t m_entry_count;
    pom_class_id_t m_omm_control_class_id;
    uint32_t m_omm_page_size;
};

struct pom_grp_entry_meta_data {
    uint16_t m_name_pos;
    uint16_t m_meta_name_pos;
    uint16_t m_capacity;
    uint16_t m_type;
    uint16_t m_page_size;
    uint16_t m_obj_align;
    uint16_t m_standalone;
    uint16_t m_reserve1;
    pom_class_id_t m_class_id;
};

size_t pom_grp_meta_calc_bin_size(pom_grp_meta_t meta) {
    pom_grp_entry_meta_t entry;
    uint16_t i;
    size_t size;

    size = sizeof(struct pom_grp_meta_data) 
        + cpe_hash_table_count(&meta->m_entry_ht) * sizeof(struct pom_grp_entry_meta_data);

    size += strlen(meta->m_name) + 1;

    for(i = 0; i < meta->m_entry_count; ++i) {
        entry = meta->m_entry_buf[i];

        size += strlen(entry->m_name) + 1;

        switch(entry->m_type) {
        case pom_grp_entry_type_normal:
            size += strlen(dr_meta_name(entry->m_data.m_normal.m_data_meta)) + 1;
            break;
        case pom_grp_entry_type_list:
            size += strlen(dr_meta_name(entry->m_data.m_list.m_data_meta)) + 1;
            break;
        case pom_grp_entry_type_ba:
            break;
        case pom_grp_entry_type_binary:
            break;
        }
    }

    return size;
}

pom_grp_meta_t
pom_grp_meta_build_from_bin(mem_allocrator_t alloc, void const * data, LPDRMETALIB metalib, error_monitor_t em) {
    struct pom_grp_meta_data * meta_data;
    struct pom_grp_entry_meta_data * entry_meta_data;
    pom_grp_meta_t meta;
    int i = 0;

    meta_data = (struct pom_grp_meta_data *)data;
    if (meta_data->m_magic != 5346) {
        CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: magic error\n");
        return NULL;
    }

    if (meta_data->m_size < sizeof(struct pom_grp_meta_data) + meta_data->m_entry_count * sizeof(struct pom_grp_entry_meta_data)) {
        CPE_ERROR(
            em, "pom_grp_entry_meta_build_from_bin: not enough input, size=%d, with entry len=%d\n",
            (int)meta_data->m_size, (int)(sizeof(struct pom_grp_meta_data) + meta_data->m_entry_count * sizeof(struct pom_grp_entry_meta_data)));
        return NULL;
    }

    if (meta_data->m_name_pos == 0) {
        CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: meta no name!");
        return NULL;
    }

    if (meta_data->m_name_pos > meta_data->m_size) {
        CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: meta name pos overflow!");
        return NULL;
    }

    meta = pom_grp_meta_create(alloc, ((const char *)data) + meta_data->m_name_pos, meta_data->m_omm_page_size);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: create meta fail!");
        return NULL;
    }
    assert(meta->m_omm_page_size == meta_data->m_omm_page_size);

    if (meta->m_control_class_id != meta_data->m_omm_control_class_id) {
        CPE_ERROR(
            em, "pom_grp_entry_meta_build_from_bin: entry %d class id mismatch! %d and %d", i,
            meta->m_control_class_id, meta_data->m_omm_control_class_id);
        pom_grp_meta_free(meta);
        return NULL;
    }

    entry_meta_data = (struct pom_grp_entry_meta_data *)(meta_data + 1);
    for(i = 0; i < meta_data->m_entry_count; ++i, ++entry_meta_data) {
        const char * entry_name = NULL;
        const char * data_meta_name = NULL;
        LPDRMETA data_meta = NULL;
        pom_grp_entry_meta_t entry_meta = NULL;

        if (entry_meta_data->m_name_pos == 0) {
            CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: entry %d no name!", i);
            pom_grp_meta_free(meta);
            return NULL;
        }

        if (entry_meta_data->m_name_pos > meta_data->m_size) {
            CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: entry %d name pos overflow!", i);
            pom_grp_meta_free(meta);
            return NULL;
        }
        entry_name = ((const char *)data) + entry_meta_data->m_name_pos;

        if (entry_meta_data->m_meta_name_pos) {
            if (entry_meta_data->m_meta_name_pos >= meta_data->m_size) {
                CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: entry %d(%s) meta name pos overflow!", i, entry_name);
                pom_grp_meta_free(meta);
                return NULL;
            }
            else {
                data_meta_name = ((const char *)data) + entry_meta_data->m_meta_name_pos;
            }
        }

        if (data_meta_name) {
            data_meta = dr_lib_find_meta_by_name(metalib, data_meta_name);
            if (data_meta == NULL) {
                CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: entry %d(%s): meta %s not exist!", i, entry_name, data_meta_name);
                pom_grp_meta_free(meta);
                return NULL;
            }
        }

        switch(entry_meta_data->m_type) {
        case pom_grp_entry_type_normal:
            if (data_meta == NULL) {
                CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: entry %d(%s): no meta configured!", i, entry_name);
                pom_grp_meta_free(meta);
                return NULL;
            }
            entry_meta = pom_grp_entry_meta_normal_create(meta, entry_name, data_meta, em);
            break;
        case pom_grp_entry_type_list:
            if (data_meta == NULL) {
                CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: entry %d(%s): no meta configured!", i, entry_name);
                pom_grp_meta_free(meta);
                return NULL;
            }

            entry_meta = pom_grp_entry_meta_list_create(
                meta, entry_name, data_meta,
                entry_meta_data->m_page_size / dr_meta_size(data_meta),
                entry_meta_data->m_capacity,
                entry_meta_data->m_standalone,
                em);
            break;
        case pom_grp_entry_type_ba:
            entry_meta = pom_grp_entry_meta_ba_create(
                meta, entry_name,
                entry_meta_data->m_page_size, entry_meta_data->m_capacity,
                em);
            break;
        case pom_grp_entry_type_binary:
            entry_meta = pom_grp_entry_meta_binary_create(
                meta, entry_name,
                entry_meta_data->m_capacity,
                em);
            break;
        default:
            CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: not support entry type %d!", entry_meta_data->m_type);
            pom_grp_meta_free(meta);
            return NULL;
        }

        if (entry_meta == NULL) {
            CPE_ERROR(em, "pom_grp_entry_meta_build_from_bin: create entry %s fail!", entry_name);
            pom_grp_meta_free(meta);
            return NULL;
        }

        if (entry_meta->m_page_size != entry_meta_data->m_page_size) {
            CPE_ERROR(
                em, "pom_grp_entry_meta_build_from_bin: create entry %s: obj-size mismatch! %d and %d"
                , entry_name, entry_meta->m_page_size, entry_meta_data->m_page_size);
            pom_grp_meta_free(meta);
            return NULL;
        }

        if (entry_meta->m_obj_align != entry_meta_data->m_obj_align) {
            CPE_ERROR(
                em, "pom_grp_entry_meta_build_from_bin: create entry %s: obj-align mismatch! %d and %d"
                , entry_name, entry_meta->m_obj_align, entry_meta_data->m_obj_align);
            pom_grp_meta_free(meta);
            return NULL;
        }

        if (entry_meta->m_class_id != entry_meta_data->m_class_id) {
            CPE_ERROR(
                em, "pom_grp_entry_meta_build_from_bin: create entry %s: class-id mismatch! %d and %d"
                , entry_name, entry_meta->m_class_id, entry_meta_data->m_class_id);
            pom_grp_meta_free(meta);
            return NULL;
        }
    }

    if (meta_data->m_main_entry_name_pos) {
        meta->m_main_entry = pom_grp_entry_meta_find(meta, ((const char *)data) + meta_data->m_main_entry_name_pos);
        if (meta->m_main_entry == NULL) {
            CPE_ERROR(
                em, "pom_grp_entry_meta_build_from_bin: load main entry: main entry %s not exist!",
                ((const char *)data) + meta_data->m_main_entry_name_pos);
            pom_grp_meta_free(meta);
            return NULL;
        }
    }

    return meta;
}

static uint32_t pom_grp_entry_meta_build_to_bin_write_string(uint32_t * string_write_pos, void * data, size_t capacity, const char * str) {
    uint32_t r;
    size_t string_len = strlen(str) + 1;

    assert((string_len + *string_write_pos) <= capacity);
    memcpy(((char*)data) + *string_write_pos, str, string_len);
    r = *string_write_pos;
    *string_write_pos += string_len;
    return r;
}

void pom_grp_meta_write_to_bin(void * data, size_t capacity, pom_grp_meta_t meta) {
    pom_grp_entry_meta_t entry;
    uint32_t i;
    struct pom_grp_meta_data * meta_data;
    struct pom_grp_entry_meta_data * entry_meta_data;
    uint32_t string_write_pos;
    size_t data_size;

    assert(pom_grp_meta_calc_bin_size(meta) <= capacity);

    data_size = pom_grp_meta_calc_bin_size(meta);
    assert(data_size <= capacity);

    meta_data = (struct pom_grp_meta_data *)data;

    string_write_pos = 
        sizeof(struct pom_grp_meta_data) 
        + cpe_hash_table_count(&meta->m_entry_ht) * sizeof(struct pom_grp_entry_meta_data);

    meta_data->m_magic = 5346;
    meta_data->m_size = data_size;
    meta_data->m_name_pos = 
        pom_grp_entry_meta_build_to_bin_write_string(&string_write_pos, data, capacity, meta->m_name);
    meta_data->m_main_entry_name_pos = 0;
    meta_data->m_entry_count = cpe_hash_table_count(&meta->m_entry_ht);
    meta_data->m_omm_page_size = meta->m_omm_page_size;
    meta_data->m_omm_control_class_id = meta->m_control_class_id;

    entry_meta_data = (struct pom_grp_entry_meta_data *)(meta_data + 1);

    for(i = 0; i < meta->m_entry_count; ++i) {
        entry = meta->m_entry_buf[i];

        entry_meta_data->m_name_pos = 
            pom_grp_entry_meta_build_to_bin_write_string(&string_write_pos, data, capacity, entry->m_name);
        entry_meta_data->m_type = entry->m_type;
        entry_meta_data->m_meta_name_pos = 0;
        entry_meta_data->m_capacity = 0;
        entry_meta_data->m_class_id = entry->m_class_id;
        entry_meta_data->m_page_size = entry->m_page_size;
        entry_meta_data->m_obj_align = entry->m_obj_align;
        entry_meta_data->m_standalone = 0;
        entry_meta_data->m_reserve1 = 0;

        if (entry == meta->m_main_entry) {
            meta_data->m_main_entry_name_pos = entry_meta_data->m_name_pos;
        }

        switch(entry->m_type) {
        case pom_grp_entry_type_normal:
            entry_meta_data->m_meta_name_pos = 
                pom_grp_entry_meta_build_to_bin_write_string(
                    &string_write_pos, data, capacity,
                    dr_meta_name(entry->m_data.m_normal.m_data_meta));
            break;
        case pom_grp_entry_type_list:
            entry_meta_data->m_meta_name_pos = 
                pom_grp_entry_meta_build_to_bin_write_string(
                    &string_write_pos, data, capacity,
                    dr_meta_name(entry->m_data.m_list.m_data_meta));

            entry_meta_data->m_capacity = entry->m_data.m_list.m_capacity;
            entry_meta_data->m_standalone = entry->m_data.m_list.m_standalone;
            break;
        case pom_grp_entry_type_ba:
            entry_meta_data->m_capacity = entry->m_data.m_ba.m_bit_capacity;
            break;
        case pom_grp_entry_type_binary:
            entry_meta_data->m_capacity = entry->m_data.m_binary.m_capacity;
            break;
        }

        entry_meta_data += 1;
    }
}
