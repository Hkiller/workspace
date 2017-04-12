#include <assert.h>
#include "cpe/pom/pom_object.h"
#include "cpe/pom/pom_error.h"
#include "cpe/pom/pom_manage.h"
#include "pom_internal_ops.h"

pom_oid_t pom_obj_alloc(
    pom_mgr_t omm,
    cpe_hash_string_t className,
    error_monitor_t em)
{
    struct pom_class * theClass;
    int32_t baseOid;
    pom_oid_t oid;

    assert(omm);
    assert(className);

    assert(omm->m_auto_validate == 0 || pom_mgr_validate(omm, em) == 0);

    theClass = pom_class_find(&omm->m_classMgr, className);
    if (theClass == NULL) {
        CPE_ERROR_EX(
            em, pom_class_not_exist,
            "theClass %s not exist!", cpe_hs_data(className));
        return POM_INVALID_OID; 
    }

    baseOid = pom_class_alloc_object(theClass);
    if (baseOid < 0) {
        void * newPage = pom_page_get(&omm->m_bufMgr, em);
        if (newPage == NULL) {
            CPE_ERROR_EX(
                em, pom_no_memory,
                "object of theClass %s alloc new page fail!", cpe_hs_data(className));
            return POM_INVALID_OID;
        }

        if (pom_class_add_new_page(theClass, newPage, em) != 0) {
            CPE_ERROR_EX(
                em, pom_no_memory,
                "object of theClass %s add page fail!", cpe_hs_data(className));
            return POM_INVALID_OID;
        }

        baseOid = pom_class_alloc_object(theClass);
        if (baseOid < 0) {
            CPE_ERROR_EX(
                em, pom_no_memory,
                "object of theClass %s alloc oid fail!", cpe_hs_data(className));
            return POM_INVALID_OID;
        }
    }

    if (baseOid > 0xFFFFFF) {
        CPE_ERROR_EX(
            em, pom_no_memory,
            "object of theClass %s count overflow!", cpe_hs_data(className));
        pom_class_free_object(theClass, baseOid, NULL);
        return POM_INVALID_OID; 
    }

    oid = pom_oid_make(theClass->m_id, baseOid);

    if (omm->m_debuger) pom_debuger_on_alloc(omm->m_debuger, oid);
    assert(omm->m_auto_validate == 0 || pom_mgr_validate(omm, em) == 0);

    return oid;
}

void pom_obj_free(
    pom_mgr_t omm,
    pom_oid_t oid,
    error_monitor_t em)
{
    pom_class_id_t classId;
    struct pom_class * theClass;

    assert(omm);

    classId = ((uint32_t)oid) >> 24;
    theClass = pom_class_get(&omm->m_classMgr, classId);

    if (theClass == NULL) {
        CPE_ERROR_EX(em, pom_class_not_exist, "theClass id=%d not exist!", classId);
        return;
    }

    if (omm->m_debuger) pom_debuger_on_free(omm->m_debuger, oid);

    assert(omm->m_auto_validate == 0 || pom_mgr_validate(omm, em) == 0);

    pom_class_free_object(theClass, oid & 0xFFFFFF, em);
}

void * pom_obj_get(
    pom_mgr_t omm, 
    pom_oid_t oid,
    error_monitor_t em)
{
    pom_class_id_t classId;
    struct pom_class * theClass;

    assert(omm);

    classId = ((uint32_t)oid) >> 24;
    theClass = pom_class_get(&omm->m_classMgr, classId);

    if (theClass == NULL) {
        CPE_ERROR_EX(
            em, pom_class_not_exist,
            "theClass id=%d not exist!", classId);
        return NULL;
    }

    return pom_class_get_object(theClass, oid & 0xFFFFFF, em);
}

pom_class_t
pom_obj_class(
    pom_mgr_t omm,
    pom_oid_t oid,
    error_monitor_t em)
{
    pom_class_id_t classId;

    assert(omm);

    classId = ((uint32_t)oid) >> 24;
    return pom_class_get(&omm->m_classMgr, classId);
}

pom_oid_t
pom_obj_id_from_addr(
    pom_mgr_t omm,
    void * data,
    error_monitor_t em)
{
    void * page;
    struct pom_data_page_head * head;
    struct pom_class * theClass;
    int32_t baseOid;

    assert(omm);

    page = pom_buffer_mgr_find_page(&omm->m_bufMgr, data);
    if (page == NULL) {
        CPE_ERROR_EX(
            em, pom_invalid_address,
            "address to oid: address is invalid");
        return POM_INVALID_OID;
    }

    head = (struct pom_data_page_head *)page;

    if (head->m_classId == POM_INVALID_CLASSID) {
        CPE_ERROR_EX(
            em, pom_invalid_address,
            "address to oid: address is invalid, not allocked!");
        return POM_INVALID_OID;
    }

    theClass = pom_class_get(&omm->m_classMgr, head->m_classId);
    if (theClass == NULL) {
        CPE_ERROR_EX(
            em, pom_class_not_exist,
            "theClass id=%d not exist!", head->m_classId);
        return POM_INVALID_OID;
    }

    baseOid = pom_class_addr_2_object(theClass, page, data);
    if (baseOid < 0) {
        CPE_ERROR_EX(
            em, pom_invalid_address,
            "address to oid: internal error, get base oid fail!");
        return POM_INVALID_OID;
    }

    return (((uint32_t)theClass->m_id) << 24) | ((uint32_t)baseOid);
}
