#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pom/pom_object.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_obj.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "pom_grp_internal_ops.h"

uint16_t pom_grp_obj_binary_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_binary_capacity: entry %s not exist!", entry);
        return 0;
    }

    return pom_grp_obj_binary_capacity_ex(mgr, obj, entry_meta);
}

void * pom_grp_obj_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_binary: entry %s not exist!", entry);
        return NULL;
    }

    return pom_grp_obj_binary_ex(mgr, obj, entry_meta);
}

void * pom_grp_obj_binary_check_or_create(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_binary_check_or_create: entry %s not exist!", entry);
        return NULL;
    }

    return pom_grp_obj_binary_check_or_create_ex(mgr, obj, entry_meta);
}

int pom_grp_obj_binary_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void const * data, uint16_t capacity) {
    pom_grp_entry_meta_t entry_meta = pom_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "pom_grp_obj_binary_set_ex: entry %s not exist!", entry);
        return -1;
    }

    return pom_grp_obj_binary_set_ex(mgr, obj, entry_meta, data, capacity);
}

uint16_t pom_grp_obj_binary_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_binary);

    return entry->m_page_size;
}

void * pom_grp_obj_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    pom_oid_t * oid;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_binary);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;

    if (*oid == POM_INVALID_OID) return NULL;

    return pom_obj_get(mgr->m_omm, *oid, mgr->m_em);
}

void * pom_grp_obj_binary_check_or_create_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry) {
    pom_oid_t * oid;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_binary);

    oid = ((pom_oid_t *)obj) +entry->m_page_begin;

    if (*oid == POM_INVALID_OID) {
        *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (oid == POM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_mgr_obj_binary_check_or_create_ex: alloc %s buf fail!", entry->m_name);
            return NULL;
        }
    }

    POM_GRP_VALIDATE_OBJ(mgr, obj);

    return pom_obj_get(mgr->m_omm, *oid, mgr->m_em);
}

int pom_grp_obj_binary_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void const * data, uint16_t capacity) {
    pom_oid_t * oid;
    void * r;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_binary);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;

    if (*oid == POM_INVALID_OID) {
        *oid = pom_obj_alloc(mgr->m_omm, pom_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (oid == POM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_mgr_obj_binary_set: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    r = pom_obj_get(mgr->m_omm, *oid, mgr->m_em);
    assert(r);

    memcpy(r, data, entry->m_page_size);

    return 0;
}

int pom_grp_obj_binary_get_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data, uint16_t capacity) {
    pom_oid_t * oid;
    void * r;

    assert(entry);
    assert(obj);
    assert(entry->m_type == pom_grp_entry_type_binary);

    oid = ((pom_oid_t *)obj) + entry->m_page_begin;

    if (capacity > entry->m_page_size) {
        bzero(((char*)data) + entry->m_page_size, capacity - entry->m_page_size);
        capacity = entry->m_page_size;
    }

    if (*oid == POM_INVALID_OID) {
        bzero(data, capacity);
    }
    else {
        r = pom_obj_get(mgr->m_omm, *oid, mgr->m_em);
        assert(r);
        memcpy(data, r, capacity);
    }

    return 0;
}
