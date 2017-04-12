#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"
#include "pom_grp_data.h"

int pom_grp_obj_mgr_buf_init(
    LPDRMETALIB metalib,
    pom_grp_meta_t meta,
    void * data, size_t data_capacity,
    error_monitor_t em)
{
    struct pom_grp_obj_control_data * control;
    pom_mgr_t omm;

    size_t base_size;
    size_t bin_size;
    size_t lib_size;
    size_t total_head_size;
    size_t size_tmp;

    base_size = sizeof(struct pom_grp_obj_control_data);
    CPE_PAL_ALIGN_DFT(base_size);

    bin_size = pom_grp_meta_calc_bin_size(meta);
    CPE_PAL_ALIGN_DFT(bin_size);

    lib_size = dr_lib_size(metalib);
    CPE_PAL_ALIGN_DFT(lib_size);

    total_head_size = base_size + bin_size + lib_size;

    if (total_head_size >= data_capacity) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create_by_init: data buf too small! require "FMT_SIZE_T", but only "FMT_SIZE_T""
            ": control size "FMT_SIZE_T", om-meta size "FMT_SIZE_T", metalib size "FMT_SIZE_T"",
            total_head_size, data_capacity, base_size, bin_size, lib_size);
        return -1;
    }

    control = (struct pom_grp_obj_control_data *)data;

    control->m_magic = OM_GRP_OBJ_CONTROL_MAGIC;
    control->m_head_version = 1;

    size_tmp = sizeof(struct pom_grp_obj_control_data);
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_objmeta_start = (uint32_t)size_tmp;
    control->m_objmeta_size = (uint32_t)pom_grp_meta_calc_bin_size(meta);
    pom_grp_meta_write_to_bin(((char *)data) + control->m_objmeta_start, control->m_objmeta_size, meta);

    size_tmp = control->m_objmeta_size;
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_metalib_start = (uint32_t)(control->m_objmeta_start + size_tmp);
    control->m_metalib_size = (uint32_t)dr_lib_size(metalib);
    memcpy(((char *)data) + control->m_metalib_start, (void *)metalib, control->m_metalib_size);

    size_tmp = control->m_metalib_size;
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_data_start = (uint32_t)(control->m_metalib_start + size_tmp);
    control->m_data_size = (uint32_t)data_capacity - control->m_data_start;
    bzero(((char *)data) + control->m_data_start, control->m_data_size);

    omm = pom_mgr_create(NULL, meta->m_omm_page_size, control->m_data_size);
    if (omm == NULL) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create_by_init: create omm for init data buf fail, page-size=%d, buf-size=%u!",
            meta->m_omm_page_size, control->m_data_size);
        return -1;
    }

    if (pom_mgr_add_new_buffer(
            omm,
            (pom_buffer_id_t)(((char*)data) + control->m_data_start),
            em) != 0)
    {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create_by_init: create omm for init data buf fail, page-size=%d, buf-size=%u!",
            meta->m_omm_page_size, control->m_data_size);
        pom_mgr_free(omm);
        return -1;
    }

    pom_mgr_free(omm);
    return 0;
}

