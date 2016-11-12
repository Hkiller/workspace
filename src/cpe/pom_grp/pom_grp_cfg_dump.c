#include <assert.h>
#include "cpe/utils/hex_utils.h"
#include "cpe/cfg/cfg_manage.h" 
#include "cpe/cfg/cfg_read.h" 
#include "cpe/dr/dr_cfg.h" 
#include "cpe/pom_grp/pom_grp_cfg.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_obj_cfg_dump_normal(
    cfg_t cfg,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    pom_grp_obj_t obj,
    error_monitor_t em)
{
    void * data;

    data = pom_grp_obj_normal_ex(mgr, obj, entry_meta);
    if (data == NULL) return 0;

    cfg = cfg_struct_add_struct(cfg, entry_meta->m_name, cfg_replace);
    if (cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: create sub cfg fail!", entry_meta->m_name);
        return -1;
    }

    if (dr_cfg_write(cfg, data, pom_grp_entry_meta_normal_meta(entry_meta), em) != 0) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: dump date fail!", entry_meta->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_obj_cfg_dump_list(
    cfg_t cfg,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    pom_grp_obj_t obj,
    error_monitor_t em)
{
    uint16_t i, count;
    LPDRMETA data_meta;
    cfg_t data_cfg;
    void * data;
    int rv;

    cfg = cfg_struct_add_seq(cfg, entry_meta->m_name, cfg_replace);
    if (cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: create sub cfg fail!", entry_meta->m_name);
        return -1;
    }

    data_meta = pom_grp_entry_meta_list_meta(entry_meta);

    rv = 0;

    count = pom_grp_obj_list_count_ex(mgr, obj, entry_meta);
    for(i = 0; i < count; ++i) {
        data = pom_grp_obj_list_at_ex(mgr, obj, entry_meta, i);
        if (data == NULL) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: get list item %d fail!", entry_meta->m_name, i);
            rv = -1;
            continue;
        }

        data_cfg = cfg_seq_add_struct(cfg);
        if (dr_cfg_write(data_cfg, data, data_meta, em) != 0) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: dump list item %d fail!", entry_meta->m_name, i);
            rv = -1;
            continue;
        }
    }

    return rv;
}

static int pom_grp_obj_cfg_dump_ba(
    cfg_t cfg,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    pom_grp_obj_t obj,
    error_monitor_t em)
{
    uint16_t i, count;
    cfg_t data_cfg;
    int rv;

    cfg = cfg_struct_add_seq(cfg, entry_meta->m_name, cfg_replace);
    if (cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: create sub cfg fail!", entry_meta->m_name);
        return -1;
    }

    rv = 0;

    count = pom_grp_obj_ba_bit_capacity_ex(mgr, obj, entry_meta);
    for(i = 0; i < count; ++i) {
        if (pom_grp_obj_ba_get_ex(mgr, obj, entry_meta, i) != cpe_ba_true) continue;

        data_cfg = cfg_seq_add_int32(cfg, i);
        if (data_cfg == NULL) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: add value %d fail!", entry_meta->m_name, i);
            rv = -1;
            continue;
        }
    }

    return rv;
}

static int pom_grp_obj_cfg_dump_binary(
    cfg_t cfg,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    pom_grp_obj_t obj,
    error_monitor_t em)
{
    void * data;
    char * str;
    struct mem_buffer buffer;
    int rv;

    data = pom_grp_obj_binary_ex(mgr, obj, entry_meta);
    if (data == NULL) return 0;

    mem_buffer_init(&buffer, NULL);

    rv = 0;

    str = cpe_hex_dup_buf(data, pom_grp_obj_binary_capacity_ex(mgr, obj, entry_meta), &buffer);
    if (str == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: dump data to hexx!", entry_meta->m_name);
        rv = -1;
    }
    else {
        if (cfg_struct_add_string(cfg, entry_meta->m_name, str, cfg_replace) == NULL) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump: %s: create sub string cfg fail!", entry_meta->m_name);
            rv = -1;
        }
    }

    mem_buffer_clear(&buffer);

    return rv;
}

int pom_grp_obj_cfg_dump(cfg_t cfg, pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em) {
    uint16_t i, count;
    int rv;
    pom_grp_meta_t meta;

    rv = 0;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump: no meta!");
        return -1;
    }

    count =  pom_grp_meta_entry_count(meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta = pom_grp_meta_entry_at(meta, i);
        assert(entry_meta);

        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            if (pom_grp_obj_cfg_dump_normal(cfg, meta, entry_meta, mgr, obj, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_list:
            if (pom_grp_obj_cfg_dump_list(cfg, meta, entry_meta, mgr, obj, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_obj_cfg_dump_ba(cfg, meta, entry_meta, mgr, obj, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_obj_cfg_dump_binary(cfg, meta, entry_meta, mgr, obj, em) != 0) rv = -1;
            break;
        }
    }

    return rv;
}

int pom_grp_obj_cfg_dump_all(cfg_t cfg, pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    int rv;
    struct pom_grp_obj_it obj_it;
    pom_grp_obj_t obj;

    if (cfg_type(cfg) != CPE_CFG_TYPE_SEQUENCE) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_all: can`t dump to %d cfg!", cfg_type(cfg));
        return -1;
    }

    rv = 0;

    pom_grp_objs(mgr, &obj_it);

    while((obj = pom_grp_obj_it_next(&obj_it))) {
        cfg_t child_cfg = cfg_seq_add_struct(cfg);
        if (child_cfg == NULL) {
            CPE_ERROR(em, "pom_grp_obj_cfg_dump_all: create sub cfg fail!");
            return -1;
        }

        if (pom_grp_obj_cfg_dump(child_cfg, mgr, obj, em) != 0) rv = -1;
    }

    return rv;
}

int pom_grp_obj_cfg_dump_to_stream(write_stream_t stream, pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em) {
    cfg_t root_cfg;
    pom_grp_meta_t meta;
    int rv;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: no meta!");
        return -1;
    }

    root_cfg = cfg_create(NULL);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: create root cfg fail!");
        return -1;
    }

    rv = 0;

    rv = pom_grp_obj_cfg_dump(root_cfg, mgr, obj, em);
    if (cfg_yaml_write(stream, root_cfg, em) != 0){
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: write cfg fail!");
        rv = -1;
        goto COMPLETE; 
    }

COMPLETE:
    cfg_free(root_cfg);
    return rv;
}

int pom_grp_obj_cfg_dump_all_to_stream(write_stream_t stream, pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    cfg_t root_cfg;
    cfg_t dump_cfg;
    pom_grp_meta_t meta;
    int rv;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: no meta!");
        return -1;
    }

    root_cfg = cfg_create(NULL);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: create root cfg fail!");
        return -1;
    }

    rv = 0;

    dump_cfg = cfg_struct_add_seq(root_cfg, pom_grp_meta_name(meta), cfg_replace);
    if (dump_cfg == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: create dump cfg fail!");
        rv = -1;
        goto COMPLETE; 
    }

    rv = pom_grp_obj_cfg_dump_all(dump_cfg, mgr, em);

    if (cfg_yaml_write(stream, dump_cfg, em) != 0){
        CPE_ERROR(em, "pom_grp_obj_cfg_dump_to_stream: write cfg fail!");
        rv = -1;
        goto COMPLETE; 
    }

COMPLETE:
    cfg_free(root_cfg);
    return rv;
}

