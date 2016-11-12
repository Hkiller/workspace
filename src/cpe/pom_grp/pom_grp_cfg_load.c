#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/hex_utils.h"
#include "cpe/cfg/cfg_manage.h" 
#include "cpe/cfg/cfg_read.h" 
#include "cpe/dr/dr_cfg.h" 
#include "cpe/dr/dr_metalib_manage.h" 
#include "cpe/pom_grp/pom_grp_cfg.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_obj_cfg_load_normal(
    pom_grp_obj_t obj,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    cfg_t cfg,
    error_monitor_t em)
{
    void * data;
    size_t capacity;

    data = pom_grp_obj_normal_check_or_create_ex(mgr, obj, entry_meta);
    if (data == NULL) return 0;

    capacity = pom_grp_entry_meta_normal_capacity(entry_meta);
    bzero(data, capacity);
    if (dr_cfg_read(data, capacity, cfg, pom_grp_entry_meta_normal_meta(entry_meta), 0, em) != 0) {
        CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: read date fail!", entry_meta->m_name);
        return -1;
    }

    return 0;
}

static int pom_grp_obj_cfg_load_list(
    pom_grp_obj_t obj,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    cfg_t cfg,
    error_monitor_t em)
{
    struct cfg_it it;
    cfg_t element_cfg;
    uint16_t count;
    LPDRMETA data_meta;
    char * buf;
    size_t buf_capacity;
    int rv;

    if (pom_grp_obj_list_clear_ex(mgr, obj, entry_meta) != 0) {
        CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: clear list fail!", entry_meta->m_name);
        return -1;
    }

    cfg_it_init(&it, cfg);
    
    data_meta = pom_grp_entry_meta_list_meta(entry_meta);

    rv = 0;

    buf_capacity = dr_meta_size(data_meta);
    buf = mem_alloc(mgr->m_alloc, buf_capacity);
    if (buf == NULL) {
        CPE_ERROR(
            em, "pom_grp_obj_cfg_load: %s: alloc tmp buf fail, size=%d!",
            entry_meta->m_name, (int)dr_meta_size(data_meta));
        return -1;
    }

    count = 0;
    while((element_cfg = cfg_it_next(&it))) {
        ++count;

        if (dr_cfg_read(buf, buf_capacity, cfg, data_meta, 0, em) != 0) {
            CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: %d: read date fail!", entry_meta->m_name, (int)count);
            rv = -1;
            continue;
        }

        if (pom_grp_obj_list_append_ex(mgr, obj, entry_meta, buf) != 0) {
            CPE_ERROR(
                em, "pom_grp_obj_cfg_load: %s: %d: append fail!",
                entry_meta->m_name, (int)count);
            rv = -1;
            continue;
        }
    }

    mem_free(mgr->m_alloc, buf);
    return rv;
}

static int pom_grp_obj_cfg_load_ba(
    pom_grp_obj_t obj,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    cfg_t cfg,
    error_monitor_t em)
{
    /* uint16_t i, count; */
    /* cfg_t data_cfg; */
    /* int rv; */

    /* cfg = cfg_struct_add_seq(cfg, entry_meta->m_name, cfg_replace); */
    /* if (cfg == NULL) { */
    /*     CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: create sub cfg fail!", entry_meta->m_name); */
    /*     return -1; */
    /* } */

    /* rv = 0; */

    /* count = pom_grp_obj_ba_bit_capacity_ex(mgr, obj, entry_meta); */
    /* for(i = 0; i < count; ++i) { */
    /*     if (pom_grp_obj_ba_get_ex(mgr, obj, entry_meta, i) != cpe_ba_true) continue; */

    /*     data_cfg = cfg_seq_add_int32(cfg, i); */
    /*     if (data_cfg == NULL) { */
    /*         CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: add value %d fail!", entry_meta->m_name, i); */
    /*         rv = -1; */
    /*         continue; */
    /*     } */
    /* } */

    /* return rv; */
    return 0;
}

static int pom_grp_obj_cfg_load_binary(
    pom_grp_obj_t obj,
    pom_grp_meta_t meta,
    pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr,
    cfg_t cfg,
    error_monitor_t em)
{
    /* void * data; */
    /* char * str; */
    /* struct mem_buffer buffer; */
    /* int rv; */

    /* data = pom_grp_obj_binary_ex(mgr, obj, entry_meta); */
    /* if (data == NULL) return 0; */

    /* mem_buffer_init(&buffer, NULL); */

    /* rv = 0; */

    /* str = cpe_hex_dup_buf(data, pom_grp_obj_binary_capacity_ex(mgr, obj, entry_meta), &buffer); */
    /* if (str == NULL) { */
    /*     CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: dump data to hexx!", entry_meta->m_name); */
    /*     rv = -1; */
    /* } */
    /* else { */
    /*     if (cfg_struct_add_string(cfg, entry_meta->m_name, str, cfg_replace) == NULL) { */
    /*         CPE_ERROR(em, "pom_grp_obj_cfg_load: %s: create sub string cfg fail!", entry_meta->m_name); */
    /*         rv = -1; */
    /*     } */
    /* } */

    /* mem_buffer_clear(&buffer); */

    /* return rv; */
    return 0;
}

int pom_grp_obj_cfg_load(pom_grp_obj_t obj, pom_grp_obj_mgr_t mgr, cfg_t cfg, error_monitor_t em) {
    uint16_t i, count;
    int rv;
    pom_grp_meta_t meta;

    rv = 0;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_cfg_load: no meta!");
        return -1;
    }

    count =  pom_grp_meta_entry_count(meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta;
        cfg_t entry_cfg;

        entry_meta = pom_grp_meta_entry_at(meta, i);
        assert(entry_meta);

        entry_cfg = cfg_struct_find_cfg(cfg, entry_meta->m_name);
        if (entry_cfg == NULL) continue;

        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            if (pom_grp_obj_cfg_load_normal(obj, meta, entry_meta, mgr, entry_cfg, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_list:
            if (pom_grp_obj_cfg_load_list(obj, meta, entry_meta, mgr, entry_cfg, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_obj_cfg_load_ba(obj, meta, entry_meta, mgr, entry_cfg, em) != 0) rv = -1;
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_obj_cfg_load_binary(obj, meta, entry_meta, mgr, entry_cfg, em) != 0) rv = -1;
            break;
        }
    }

    return rv;
}

int pom_grp_obj_cfg_load_all(pom_grp_obj_mgr_t mgr, cfg_t cfg, error_monitor_t em);
