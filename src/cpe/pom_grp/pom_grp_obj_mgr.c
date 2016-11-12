#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/range_bitarry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "pom_grp_internal_ops.h"
#include "pom_grp_data.h"

static int pom_grp_meta_init_omm(pom_mgr_t omm, pom_grp_meta_t meta, error_monitor_t em);

pom_grp_obj_mgr_t
pom_grp_obj_mgr_create(
    mem_allocrator_t alloc,
    void * data,
    size_t data_capacity,
    error_monitor_t em)
{
    pom_grp_obj_mgr_t obj_mgr;

    struct pom_grp_obj_control_data * control;

    if (data_capacity < sizeof(struct pom_grp_obj_control_data)) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create: not enouth size, size="FMT_SIZE_T", control_data_size="FMT_SIZE_T"!",
            data_capacity, sizeof(struct pom_grp_obj_control_data));
        return NULL;
    }

    control = (struct pom_grp_obj_control_data *)data;

    if(control->m_magic != OM_GRP_OBJ_CONTROL_MAGIC) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create: matic mismatch!, %d and %d",
            control->m_magic, OM_GRP_OBJ_CONTROL_MAGIC);
        return NULL;
    }

    if(control->m_head_version != 1) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create: not support version %d",
            control->m_head_version);
        return NULL;
    }

    if (data_capacity < (control->m_objmeta_start + control->m_objmeta_size)) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create: not enouth size, size="FMT_SIZE_T", objmeta end at %u!",
            data_capacity, control->m_objmeta_start + control->m_objmeta_size)
        return NULL;
    }

    if (data_capacity < (control->m_metalib_start + control->m_metalib_size)) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create: not enouth size, size="FMT_SIZE_T", metalib end at %u!",
            data_capacity, control->m_metalib_start + control->m_metalib_size)
        return NULL;
    }

    if (data_capacity < (control->m_data_start + control->m_data_size)) {
        CPE_ERROR(
            em, "pom_grp_obj_mgr_create: not enouth size, size="FMT_SIZE_T", data end at %u!",
            data_capacity, control->m_data_start + control->m_data_size)
        return NULL;
    }

    obj_mgr = (pom_grp_obj_mgr_t)mem_alloc(alloc, sizeof(struct pom_grp_obj_mgr));
    if (obj_mgr == NULL) {
        CPE_ERROR(em, "pom_grp_obj_mgr_create: create fail!");
        return NULL;
    }

    obj_mgr->m_alloc = alloc;
    obj_mgr->m_em = em;
    obj_mgr->m_auto_validate = 0;
    obj_mgr->m_full_base = (char *)data;
    obj_mgr->m_full_capacity = (uint32_t)data_capacity;
    obj_mgr->m_metalib = (LPDRMETALIB)(obj_mgr->m_full_base + control->m_metalib_start);

    obj_mgr->m_meta =
        pom_grp_meta_build_from_bin(
            alloc,
            obj_mgr->m_full_base + control->m_objmeta_start,
            obj_mgr->m_metalib,
            em);

    if (obj_mgr->m_meta == NULL) {
        CPE_ERROR(em, "pom_grp_obj_mgr_create: create meta fail!");
        mem_free(alloc, obj_mgr);
        return NULL;
    }

    obj_mgr->m_omm =
        pom_mgr_create(
            alloc,
            obj_mgr->m_meta->m_omm_page_size,
            control->m_data_size);
    if (obj_mgr->m_omm == NULL) {
        CPE_ERROR(em, "pom_grp_obj_mgr_create: create omm fail!");
        pom_grp_meta_free(obj_mgr->m_meta);
        mem_free(alloc, obj_mgr);
        return NULL;
    }

    if (pom_grp_meta_init_omm(obj_mgr->m_omm, obj_mgr->m_meta, em) != 0) {
        pom_mgr_free(obj_mgr->m_omm);
        pom_grp_meta_free(obj_mgr->m_meta);
        mem_free(alloc, obj_mgr);
        return NULL;
    }


    if (pom_mgr_attach_old_buffer(
            obj_mgr->m_omm, 
            (pom_buffer_id_t)(obj_mgr->m_full_base + control->m_data_start),
            em) != 0)
    {
        CPE_ERROR(em, "pom_grp_obj_mgr_create: prepaire alloc range fail!");
        pom_mgr_free(obj_mgr->m_omm);
        pom_grp_meta_free(obj_mgr->m_meta);
        mem_free(alloc, obj_mgr);
        return NULL;
    }

    return obj_mgr;
}

void pom_grp_obj_mgr_free(pom_grp_obj_mgr_t mgr) {
    pom_mgr_free(mgr->m_omm);
    pom_grp_meta_free(mgr->m_meta);
    mem_free(mgr->m_alloc, mgr);
}

void * pom_grp_obj_mgr_data(pom_grp_obj_mgr_t mgr) {
    return mgr->m_full_base;
}

size_t pom_grp_obj_mgr_data_capacity(pom_grp_obj_mgr_t mgr) {
    return mgr->m_full_capacity;
}

pom_mgr_t pom_grp_obj_mgr_pom(pom_grp_obj_mgr_t mgr) {
    return mgr->m_omm;
}

pom_grp_meta_t pom_grp_obj_mgr_meta(pom_grp_obj_mgr_t mgr) {
    return mgr->m_meta;
}

void pom_grp_obj_mgr_set_auto_validate(pom_grp_obj_mgr_t mgr, int auto_validate) {
    mgr->m_auto_validate = auto_validate;
}

int pom_grp_obj_mgr_auto_validate(pom_grp_obj_mgr_t mgr) {
    return mgr->m_auto_validate;
}

int pom_grp_meta_init_omm(pom_mgr_t omm, pom_grp_meta_t meta, error_monitor_t em) {
    pom_grp_entry_meta_t entry;
    uint16_t i;

    if (pom_mgr_add_class_with_id(
            omm,
            meta->m_control_class_id,
            cpe_hs_data(pom_grp_control_class_name),
            meta->m_control_obj_size,
            2,
            em) != 0)
    {
        CPE_ERROR(em, "pom_grp_meta_init_omm: register control page fail!");
        return -1;
    }

    for(i = 0; i < meta->m_entry_count; ++i) {
        entry = meta->m_entry_buf[i];

        if (pom_mgr_add_class_with_id(
                omm,
                entry->m_class_id,
                entry->m_name,
                entry->m_page_size,
                entry->m_obj_align,
                em) != 0)
        {
            CPE_ERROR(
                em, "pom_grp_meta_init_omm: register page for %s(id=%d, page-size=%d, obj-align=%d) fail!",
                entry->m_name, entry->m_class_id, entry->m_page_size, entry->m_obj_align);
            return -1;
        }
    }

    return 0;
}

CPE_HS_DEF_VAR(pom_grp_control_class_name, "pom_grp_control");
