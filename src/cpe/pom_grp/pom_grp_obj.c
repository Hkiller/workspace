#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pom/pom_class.h"
#include "cpe/pom/pom_object.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "pom_grp_internal_ops.h"

pom_grp_obj_t
pom_grp_obj_alloc(pom_grp_obj_mgr_t mgr) {
    pom_oid_t control_oid;
    pom_grp_obj_t obj;
    assert(mgr);

    control_oid =
        pom_obj_alloc(mgr->m_omm, pom_grp_control_class_name, mgr->m_em);
    if (control_oid == POM_INVALID_OID) {
        CPE_ERROR(mgr->m_em, "om_mgr_obj_alloc: alloc control buf fail!");
        return NULL;
    }

    obj = (pom_grp_obj_t)pom_obj_get(mgr->m_omm, control_oid, mgr->m_em);
    if (obj == NULL) {
        CPE_ERROR(mgr->m_em, "om_mgr_obj_alloc: get control buf fail!");
        return NULL;
    }

    bzero(obj, mgr->m_meta->m_control_obj_size);

    POM_GRP_VALIDATE_OBJ(mgr, obj);
    return obj;
}

void pom_grp_obj_free(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj) {
    int i;
    pom_oid_t * oids = (pom_oid_t *)obj;
    pom_oid_t control_oid = pom_obj_id_from_addr(mgr->m_omm, obj, mgr->m_em);

    if (control_oid == POM_INVALID_OID) {
        CPE_ERROR(mgr->m_em, "om_mgr_obj_free: convert control oid fail!");
        return;
    }

    for(i = 0; i < mgr->m_meta->m_page_count; ++i) {
        if (oids[i] != POM_INVALID_OID) {
            pom_obj_free(mgr->m_omm, oids[i], mgr->m_em);
        }
    }

    pom_obj_free(mgr->m_omm, control_oid, mgr->m_em);
}

void pom_grp_obj_clear(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj) {
    int i;
    pom_oid_t * oids = (pom_oid_t *)obj;

    for(i = 0; i < mgr->m_meta->m_page_count; ++i) {
        if (oids[i] != POM_INVALID_OID) {
            pom_obj_free(mgr->m_omm, oids[i], mgr->m_em);
        }
    }

    bzero(obj, mgr->m_meta->m_control_obj_size);
}

int pom_grp_obj_is_empty(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj) {
    int i;
    pom_oid_t * oids = (pom_oid_t *)obj;

    for(i = 0; i < mgr->m_meta->m_page_count; ++i) {
        if (oids[i] != POM_INVALID_OID) return 0;
    }

    return 1;
}

pom_oid_t pom_grp_obj_oid(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj) {
    return pom_obj_id_from_addr(mgr->m_omm, obj, mgr->m_em);
}

uint16_t pom_grp_obj_page_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj) {
    uint16_t r;
    int i;
    pom_oid_t * oids = (pom_oid_t *)obj;

    r = 0;

    for(i = 0; i < mgr->m_meta->m_page_count; ++i) {
        if (oids[i] != POM_INVALID_OID) ++r;
    }

    return r;
}

uint16_t pom_grp_obj_page_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj) {
    return mgr->m_meta->m_page_count;
}

pom_oid_t pom_grp_obj_page_oid(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, uint16_t pos) {
    pom_oid_t * oids = (pom_oid_t *)obj;
    assert(pos < mgr->m_meta->m_page_count);
    return oids[pos];
}

void pom_grp_objs(pom_grp_obj_mgr_t mgr, pom_grp_obj_it_t it) {
    pom_class_t the_class;

    the_class = pom_mgr_get_class(mgr->m_omm, mgr->m_meta->m_control_class_id);
    assert(the_class);

    pom_class_objects(the_class, &it->m_data);
}
