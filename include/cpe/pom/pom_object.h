#ifndef POM_POBJECT_H
#define POM_POBJECT_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_oid_t
pom_obj_alloc(
    pom_mgr_t omm,
    cpe_hash_string_t className,
    error_monitor_t em);

void pom_obj_free(
    pom_mgr_t omm,
    pom_oid_t oid,
    error_monitor_t em);

void * pom_obj_get(
    pom_mgr_t omm, 
    pom_oid_t oid,
    error_monitor_t em);

pom_oid_t
pom_obj_id_from_addr(
    pom_mgr_t omm,
    void * data,
    error_monitor_t em);

pom_class_t
pom_obj_class(
    pom_mgr_t omm,
    pom_oid_t oid,
    error_monitor_t em);

#define pom_obj_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#define pom_oid_make(__class_id, __base_oid) ((((uint32_t)(__class_id)) << 24) | ((uint32_t)(__base_oid)))

#ifdef __cplusplus
}
#endif

#endif
