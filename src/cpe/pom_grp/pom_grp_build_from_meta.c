#include "cpe/utils/error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta_build.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_meta_build_from_meta_entry_normal(pom_grp_meta_t meta, LPDRMETAENTRY entry, error_monitor_t em) {
    LPDRMETA data_meta;
    size_t blob_size;

    data_meta = dr_entry_ref_meta(entry);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: entry-normal meta not exist!",
            dr_entry_name(entry));
        return -1;
    }

    blob_size = pom_grp_omm_page_size(meta);
    if (blob_size > (size_t)USHRT_MAX) blob_size = USHRT_MAX;

    if (dr_meta_size(data_meta) > blob_size) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: size %d overflow, max blob size is %d!",
            dr_entry_name(entry), (int)dr_meta_size(data_meta), (int)blob_size);
        return -1;
    }

    if (pom_grp_entry_meta_normal_create(meta, dr_entry_name(entry), data_meta, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: create normal entry fail!",
            dr_entry_name(entry));
        return -1;
    }

    return 0;
}

static int pom_grp_meta_build_from_meta_entry_list(pom_grp_meta_t meta, LPDRMETAENTRY entry, error_monitor_t em) {
    LPDRMETA data_meta;
    int list_count;
    int group_count;
    int standalone;
    size_t blob_size;

    data_meta = dr_entry_ref_meta(entry);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: entry-list meta not exist!",
            dr_entry_name(entry));
        return -1;
    }

    list_count = dr_entry_array_count(entry);
    if (list_count <= 0) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: list-count %d invalid!",
            dr_entry_name(entry), list_count);
        return -1;
    }

    blob_size = pom_grp_omm_page_size(meta);
    if (blob_size > (size_t)USHRT_MAX) blob_size = USHRT_MAX;

    if (dr_meta_size(data_meta) > blob_size) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: size %d overflow, max blob size is %d!",
            dr_entry_name(entry), (int)dr_meta_size(data_meta), (int)blob_size);
        return -1;
    }

    group_count = (int)(blob_size / dr_meta_size(data_meta));
    if (group_count > list_count) { 
        group_count = list_count;
    }

    if (group_count > 16) {
        group_count /= 4;
    }

    standalone = 0;

    if (pom_grp_entry_meta_list_create(meta, dr_entry_name(entry), data_meta, group_count, list_count, standalone, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: create list entry fail!",
            dr_entry_name(entry));
        return -1;
    }

    return 0;
}

static int pom_grp_meta_build_from_meta_entry_ba(pom_grp_meta_t meta, LPDRMETAENTRY entry, error_monitor_t em) {
    int bit_capacity;
    int byte_per_page;
    size_t blob_size;

    byte_per_page = (int)dr_entry_size(entry);
    bit_capacity = byte_per_page * 8;

    blob_size = pom_grp_omm_page_size(meta);
    if (blob_size > (size_t)USHRT_MAX) blob_size = USHRT_MAX;

    if (pom_grp_entry_meta_ba_create(meta, dr_entry_name(entry), byte_per_page, bit_capacity, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_meta: entry %s: create ba entry fail!",
            dr_entry_name(entry));
        return -1;
    }

    return 0;
}

pom_grp_meta_t
pom_grp_meta_build_from_meta(mem_allocrator_t alloc, uint32_t omm_page_size, LPDRMETA dr_meta, error_monitor_t em) {
    pom_grp_meta_t meta;
    int rv;
    size_t i;

    meta = pom_grp_meta_create(alloc, dr_meta_name(dr_meta), omm_page_size);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_from_cfg: create pom_grp_meta fail!");
        return NULL;
    }

    rv = 0;

    for(i = 0; i < dr_meta_entry_num(dr_meta); ++i) {
        LPDRMETAENTRY entry = dr_meta_entry_at(dr_meta, (int)i);
        int entry_type = dr_entry_type(entry);
        int element_count = dr_entry_array_count(entry);

        if (entry_type == CPE_DR_TYPE_STRUCT) {
            if (element_count == 1) {
                if (pom_grp_meta_build_from_meta_entry_normal(meta, entry, em) != 0) {
                    ++rv;
                }
            }
            else {
                if (pom_grp_meta_build_from_meta_entry_list(meta, entry, em) != 0) {
                    ++rv;
                }
            }
        }
        else if (entry_type == CPE_DR_TYPE_UINT8
                 && element_count > 1
                 && dr_entry_array_refer_entry(entry) == NULL)
        {
            if (pom_grp_meta_build_from_meta_entry_ba(meta, entry, em) != 0) {
                ++rv;
            }
        }
    }

    if (pom_grp_meta_entry_count(meta) > 0) {
        if (pom_grp_meta_set_main_entry(meta, pom_grp_entry_meta_name(pom_grp_meta_entry_at(meta, 0))) != 0) {
            ++rv;
        }
    }

    if (rv == 0) {
        return meta;
    }
    else {
        pom_grp_meta_free(meta);
        return NULL;
    }
}

