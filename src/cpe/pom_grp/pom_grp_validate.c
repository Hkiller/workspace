#include "cpe/utils/range_bitarry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom/pom_class.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"
#include "pom_grp_data.h"

static void pom_grp_obj_validate_normal(
    pom_grp_meta_t meta, pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em)
{
}

static void pom_grp_obj_validate_list(
    pom_grp_meta_t meta, pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em)
{
    pom_oid_t * oids = (pom_oid_t *)obj;
    uint16_t * count = ((uint16_t *)(((char *)obj) + mgr->m_meta->m_size_buf_start)) + entry_meta->m_data.m_list.m_size_idx;
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t page_count;
    uint16_t page_pos;

    if (*count > entry_meta->m_data.m_list.m_capacity) {
        CPE_ERROR(
            em, "pom_obj %p: list %s count overflow, count=%d, capacity=%d",
            obj, entry_meta->m_name, *count, (int)entry_meta->m_data.m_list.m_capacity);
        return;
    }

    element_size = dr_meta_size(entry_meta->m_data.m_list.m_data_meta);
    count_in_page = entry_meta->m_page_size / element_size;
    page_count = *count / count_in_page;

    if (page_count > entry_meta->m_page_count) {
        CPE_ERROR(
            em, "pom_obj %p: list %s page count overflow, count=%d, capacity=%d",
            obj, entry_meta->m_name, page_count, (int)entry_meta->m_page_count);
        page_count = entry_meta->m_page_count;
    }

    for(page_pos = 0; page_pos < page_count; ++page_pos) {
        if (oids[entry_meta->m_page_begin + page_pos] == POM_INVALID_OID) {
            CPE_ERROR(
                em, "pom_obj %p: list %s page %d is INVALID!",
                obj, entry_meta->m_name, page_pos);
        }
    }
}

static void pom_grp_obj_validate_ba(
    pom_grp_meta_t meta, pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em)
{
}

static void pom_grp_obj_validate_binary(
    pom_grp_meta_t meta, pom_grp_entry_meta_t entry_meta,
    pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em)
{
    
}

static void pom_grp_obj_validate_i(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em) {
    pom_grp_meta_t meta;
    int i, count;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_mgr no meta!");
        return;
    }

    count =  pom_grp_meta_entry_count(meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta = pom_grp_meta_entry_at(meta, i);
        if (entry_meta == NULL) {
            CPE_ERROR(em, "pom_grp_obj_mgr entry %d no meta!", i);
            continue;
        }

        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            pom_grp_obj_validate_normal(meta, entry_meta, mgr, obj, em);
            break;
        case pom_grp_entry_type_list:
            pom_grp_obj_validate_list(meta, entry_meta, mgr, obj, em);
            break;
        case pom_grp_entry_type_ba:
            pom_grp_obj_validate_ba(meta, entry_meta, mgr, obj, em);
            break;
        case pom_grp_entry_type_binary:
            pom_grp_obj_validate_binary(meta, entry_meta, mgr, obj, em);
            break;
        }
    }
}

static void pom_grp_obj_mgr_validate_buf_range(pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    struct cpe_range_mgr buffer_ragnes;
    struct pom_grp_obj_it obj_it;
    pom_grp_obj_t obj;
    pom_grp_meta_t meta;
    int entry_pos, entry_count;

    meta = pom_grp_obj_mgr_meta(mgr);
    if (meta == NULL) return;

    entry_count =  pom_grp_meta_entry_count(meta);

    if (cpe_range_mgr_init(&buffer_ragnes, mgr->m_alloc) != 0) {
        CPE_ERROR(em, "init buffer range fail");
        return;
    }

    pom_grp_objs(mgr, &obj_it);
    while((obj = pom_grp_obj_it_next(&obj_it))) {
        pom_oid_t * oids = (pom_oid_t *)obj;

        if (cpe_range_is_conflict(&buffer_ragnes, (ptr_int_t)obj, (ptr_int_t)(((char *)obj) + meta->m_control_obj_size))) {
            CPE_ERROR(em, "pom_obj %p: control page conflicit", obj);
        }
        cpe_range_put_range(&buffer_ragnes, (ptr_int_t)obj, (ptr_int_t)(((char *)obj)));

        for(entry_pos = 0; entry_pos < entry_count; ++entry_pos) {
            pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_at(meta, entry_pos);
            uint16_t page_pos;

            for(page_pos = 0; page_pos < entry_meta->m_page_count; ++page_pos) {
                pom_oid_t oid = oids[entry_meta->m_page_begin + page_pos];
                char * data;

                if (oid == POM_INVALID_OID) continue;

                data = pom_obj_get(mgr->m_omm, oid, em);
                if (data == NULL) {
                    CPE_ERROR(em, "pom_obj %p: page %s.%d not allocked", obj, entry_meta->m_name, page_pos);
                    continue;
                }

                if (cpe_range_is_conflict(&buffer_ragnes, (ptr_int_t)data, (ptr_int_t)(data + entry_meta->m_page_size))) {
                    CPE_ERROR(em, "pom_obj %p: page %s.%d conflicit", obj, entry_meta->m_name, page_pos);
                }
                cpe_range_put_range(&buffer_ragnes, (ptr_int_t)data, (ptr_int_t)(data + entry_meta->m_page_size));
            }
        }
    }

    cpe_range_mgr_fini(&buffer_ragnes);
}

static void pom_grp_obj_mgr_validate_i(pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    struct pom_grp_obj_it obj_it;
    pom_grp_obj_t obj;

    pom_grp_objs(mgr, &obj_it);

    while((obj = pom_grp_obj_it_next(&obj_it))) {
        pom_grp_obj_validate_i(mgr, obj, em);
    }

    pom_grp_obj_mgr_validate_buf_range(mgr, em);
}

int pom_grp_obj_mgr_validate(pom_grp_obj_mgr_t mgr, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        pom_grp_obj_mgr_validate_i(mgr, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        pom_grp_obj_mgr_validate_i(mgr, &logError);
    }

    return ret;
}

int pom_grp_obj_validate(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        pom_grp_obj_validate_i(mgr, obj, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        pom_grp_obj_validate_i(mgr, obj, &logError);
    }

    return ret;
}
