#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "aom_internal_types.h"
#include "aom_data.h"

static LPDRMETALIB aom_obj_mgr_build_metalib(LPDRMETA meta, mem_buffer_t buffer, error_monitor_t em);

int aom_obj_mgr_buf_calc_capacity(size_t * result, LPDRMETA meta, uint32_t record_count, error_monitor_t em) {
    struct mem_buffer buffer;
    LPDRMETALIB metalib;
    size_t total_size;
    size_t size_tmp;

    mem_buffer_init(&buffer, NULL);
    metalib = aom_obj_mgr_build_metalib(meta, &buffer, em);
    if (metalib == NULL) {
        mem_buffer_clear(&buffer);
        return -1;
    }

    total_size = 0;

    /*head*/
    size_tmp = sizeof(struct aom_obj_control_data);
    CPE_PAL_ALIGN_DFT(size_tmp);
    total_size += size_tmp;

    /*metalib*/
    size_tmp = dr_lib_size(metalib);
    CPE_PAL_ALIGN_DFT(size_tmp);
    total_size += size_tmp;

    /*records*/
    size_tmp = dr_meta_size(meta) * record_count;
    total_size += size_tmp;

    *result = total_size;

    return 0;
}

int aom_obj_mgr_buf_init(
    LPDRMETA meta,
    void * data, size_t data_capacity,
    error_monitor_t em)
{
    struct aom_obj_control_data * control;
    struct mem_buffer buffer;
    size_t base_size;
    size_t lib_size;
    size_t total_head_size;
    size_t size_tmp;
    LPDRMETALIB metalib;

    mem_buffer_init(&buffer, NULL);
    metalib = aom_obj_mgr_build_metalib(meta, &buffer, em);
    if (metalib == NULL) {
        mem_buffer_clear(&buffer);
        return -1;
    }

    base_size = sizeof(struct aom_obj_control_data);
    CPE_PAL_ALIGN_DFT(base_size);

    lib_size = dr_lib_size(metalib);
    CPE_PAL_ALIGN_DFT(lib_size);

    total_head_size = base_size + lib_size;

    if (total_head_size >= data_capacity) {
        CPE_ERROR(
            em, "aom_obj_buff_init: data buf too small! require "FMT_SIZE_T", but only "FMT_SIZE_T""
            ": control size "FMT_SIZE_T", metalib size "FMT_SIZE_T"",
            total_head_size, data_capacity, base_size, lib_size);
        mem_buffer_clear(&buffer);
        return -1;
    }

    control = (struct aom_obj_control_data *)data;

    control->m_magic = OM_GRP_OBJ_CONTROL_MAGIC;
    control->m_head_version = 1;
    cpe_str_dup(control->m_meta_name, sizeof(control->m_meta_name), dr_meta_name(meta));

    size_tmp = sizeof(struct aom_obj_control_data);
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_metalib_start = size_tmp;
    control->m_metalib_size = dr_lib_size(metalib);
    memcpy(((char *)data) + control->m_metalib_start, (void *)metalib, control->m_metalib_size);

    size_tmp = control->m_metalib_size;
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_data_start = control->m_metalib_start + size_tmp;
    control->m_data_size = data_capacity - control->m_data_start;
    bzero(((char *)data) + control->m_data_start, control->m_data_size);

    mem_buffer_clear(&buffer);

    return 0;
}

static LPDRMETALIB aom_obj_mgr_build_metalib(LPDRMETA meta, mem_buffer_t buffer, error_monitor_t em) {
    struct DRInBuildMetaLib * inbuild_lib;

    if (dr_meta_key_entry_num(meta) == 0) {
        CPE_ERROR(em, "aom_obj_buff_init: meta %s have no key!", dr_meta_name(meta));
        return NULL;
    }

    inbuild_lib = dr_inbuild_create_lib();
    if (inbuild_lib == NULL) {
        CPE_ERROR(em, "aom_obj_buff_init: create inbuild metalib fail!");
        return NULL;
    }

    if (dr_inbuild_metalib_copy_meta_r(inbuild_lib, meta) == NULL) {
        CPE_ERROR(em, "aom_obj_buff_init: copy meta fail!");
        dr_inbuild_free_lib(inbuild_lib);
        return NULL;
    }

    if (dr_inbuild_tsort(inbuild_lib, em) != 0) {
        CPE_ERROR(em, "aom_obj_buff_init: sort meta fail!");
        dr_inbuild_free_lib(inbuild_lib);
        return NULL;
    }

    mem_buffer_clear_data(buffer);
    if (dr_inbuild_build_lib(buffer, inbuild_lib, em) != 0) {
        CPE_ERROR(em, "aom_obj_buff_init: build metalib fail!");
        dr_inbuild_free_lib(inbuild_lib);
        return NULL;
    }

    dr_inbuild_free_lib(inbuild_lib);

    return (LPDRMETALIB)mem_buffer_make_continuous(buffer, 0);
}
