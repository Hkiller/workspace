#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta_build.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_meta_build_from_cfg_entry_normal(
    pom_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_type;
    LPDRMETA data_meta;

    data_type  = cfg_get_string(entry_cfg, "data-type", NULL);
    if (data_type == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-normal not configure data-type!",
            cfg_name(entry_cfg));
        return -1;
    }

    data_meta = dr_lib_find_meta_by_name(metalib, data_type);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-normal data-type %s not exist!",
            cfg_name(entry_cfg), data_type);
        return -1;
    }

    if (pom_grp_entry_meta_normal_create(meta, cfg_name(entry_cfg), data_meta, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: create normal entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}

static int pom_grp_meta_build_from_cfg_entry_list(
    pom_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_type;
    LPDRMETA data_meta;
    const char * data_capacity;
    int list_count;
    int group_count;
    int standalone;

    data_type  = cfg_get_string(entry_cfg, "data-type", NULL);
    if (data_type == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-list not configure data-type!",
            cfg_name(entry_cfg));
        return -1;
    }

    data_meta = dr_lib_find_meta_by_name(metalib, data_type);
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-list data-type %s not exist!",
            cfg_name(entry_cfg), data_type);
        return -1;
    }

    standalone = cfg_get_int32(entry_cfg, "standalone", 0);

    list_count = cfg_get_int32(entry_cfg, "capacity", -1);
    if (list_count == -1) {
        data_capacity = cfg_get_string(entry_cfg, "capacity", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-list not configure capacity!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&list_count, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-list capacity %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    group_count = cfg_get_int32(entry_cfg, "group-count", -1);
    if (group_count == -1) {
        data_capacity = cfg_get_string(entry_cfg, "group-count", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-list not configure group-count!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&list_count, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-list group-count %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    if (list_count <= 0) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-list capacity %d error!",
            cfg_name(entry_cfg), list_count);
        return -1;
    }

    if (group_count <= 0) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-list group-count %d error!",
            cfg_name(entry_cfg), group_count);
        return -1;
    }

    if (pom_grp_entry_meta_list_create(meta, cfg_name(entry_cfg), data_meta, group_count, list_count, standalone, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: create list entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}

static int pom_grp_meta_build_from_cfg_entry_ba(
    pom_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_capacity;
    int bit_capacity;
    int byte_per_page;

    bit_capacity = cfg_get_int32(entry_cfg, "bit-capacity", -1);
    if (bit_capacity == -1) {
        data_capacity = cfg_get_string(entry_cfg, "bit-capacity", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-ba not configure bit-capacity!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&bit_capacity, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-ba bit-capacity %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    if (bit_capacity <= 0) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-ba capacity %d error!",
            cfg_name(entry_cfg), bit_capacity);
        return -1;
    }

    byte_per_page = cfg_get_int32(entry_cfg, "byte-per-page", -1);
    if (byte_per_page == -1) {
        byte_per_page = (int)cpe_ba_bytes_from_bits(bit_capacity);
    }

    if (pom_grp_entry_meta_ba_create(meta, cfg_name(entry_cfg), byte_per_page, bit_capacity, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: create ba entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}
 
static int pom_grp_meta_build_from_cfg_entry_binary(
    pom_grp_meta_t meta, cfg_t entry_cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    const char * data_capacity;
    int binary_count;

    binary_count = cfg_get_int32(entry_cfg, "capacity", -1);
    if (binary_count == -1) {
        data_capacity = cfg_get_string(entry_cfg, "capacity", NULL);
        if (data_capacity == NULL) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-binary not configure capacity!",
                cfg_name(entry_cfg));
            return -1;
        }

        if (dr_lib_find_macro_value(&binary_count, metalib, data_capacity) != 0) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-binary capacity %s not exist in metalib!",
                cfg_name(entry_cfg), data_capacity);
            return -1;
        }
    }

    if (binary_count <= 0) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: entry-binary capacity %d error!",
            cfg_name(entry_cfg), binary_count);
        return -1;
    }

    if (pom_grp_entry_meta_binary_create(meta, cfg_name(entry_cfg), binary_count, em) == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_from_cfg: entry %s: create binary entry fail!",
            cfg_name(entry_cfg));
        return -1;
    }

    return 0;
}
 
pom_grp_meta_t
pom_grp_meta_build_from_cfg(
    mem_allocrator_t alloc, 
    uint32_t omm_page_size,
    cfg_t cfg, LPDRMETALIB metalib, error_monitor_t em)
{
    pom_grp_meta_t meta;
    struct cfg_it entry_it;
    cfg_t entry_cfg;
    const char * main_entry;
    int rv;

    meta = pom_grp_meta_create(alloc, cfg_name(cfg), omm_page_size);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_from_cfg: create pom_grp_meta fail!");
        return NULL;
    }

    rv = 0;

    cfg_it_init(&entry_it, cfg_find_cfg(cfg, "attributes"));
    while((entry_cfg = cfg_it_next(&entry_it))) {
        const char * entry_type; 

        if (cfg_child_count(entry_cfg) != 1) {
            CPE_ERROR(em, "pom_grp_meta_build_from_cfg: entry config child-count error!");
            ++rv;
            continue;
        }

        entry_cfg = cfg_child_only(entry_cfg);

        entry_type = cfg_get_string(entry_cfg, "entry-type", NULL);
        if (strcmp(entry_type, "normal") == 0) {
            if (pom_grp_meta_build_from_cfg_entry_normal(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else if (strcmp(entry_type, "list") == 0){
            if (pom_grp_meta_build_from_cfg_entry_list(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else if (strcmp(entry_type, "ba") == 0) {
            if (pom_grp_meta_build_from_cfg_entry_ba(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else if (strcmp(entry_type, "binary") == 0) {
            if (pom_grp_meta_build_from_cfg_entry_binary(meta, entry_cfg, metalib, em) != 0) {
                ++rv;
            }
        }
        else {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: entry %s: entry-type %s unknown!",
                cfg_name(entry_cfg), entry_type);
            ++rv;
            continue;
        }
    }

    if ((main_entry = cfg_get_string(cfg, "main-entry", NULL))) {
        if (pom_grp_meta_set_main_entry(meta, main_entry) != 0) {
            CPE_ERROR(
                em, "pom_grp_meta_build_from_cfg: set main_entry %s fail!", main_entry);
            rv = -1;
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

