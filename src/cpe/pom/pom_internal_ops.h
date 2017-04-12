#ifndef CPE_POM_INTERNAL_OPS_H
#define CPE_POM_INTERNAL_OPS_H
#include "pom_internal_types.h"
#include "pom_page_head.h"

#ifdef __cplusplus
extern "C" {
#endif

/*pom_buffer operations*/
int pom_buffer_mgr_init(
    struct pom_buffer_mgr * pgm,
    size_t page_size,
    size_t buf_size,
    mem_allocrator_t alloc);

int pom_buffer_mgr_set_backend(
    struct pom_buffer_mgr * pgm,
    pom_backend_t backend,
    void * backend_ctx);

int pom_buffer_mgr_add_new_buffer(
    struct pom_buffer_mgr * pgm,
    pom_buffer_id_t buf_id,
    error_monitor_t em);

int pom_buffer_mgr_attach_old_buffer(
    struct pom_buffer_mgr * pgm,
    struct pom_class_mgr * classMgr,
    struct pom_debuger * debuger,
    pom_buffer_id_t buf_id,
    error_monitor_t em);

void * pom_buffer_mgr_find_page(struct pom_buffer_mgr * pgm, void * address);
void pom_buffer_mgr_fini(struct pom_buffer_mgr * pgm);

void * pom_page_get(struct pom_buffer_mgr * pgm, error_monitor_t em);

void * pom_buffer_mgr_get_buf(struct pom_buffer_mgr * pgm, pom_buffer_id_t buf_id, error_monitor_t em);

/*pom_class operations*/
int pom_class_mgr_init(struct pom_class_mgr * classMgr, mem_allocrator_t alloc);
void pom_class_mgr_fini(struct pom_class_mgr * classMgr);

pom_class_id_t
pom_class_add(
    struct pom_class_mgr * classMgr,
    const char * className,
    size_t object_size,
    size_t page_size,
    size_t align,
    error_monitor_t em);

int pom_class_add_with_id(
    struct pom_class_mgr * classMgr,
    pom_class_id_t classId,
    const char * className,
    size_t object_size,
    size_t page_size,
    size_t align,
    error_monitor_t em);

struct pom_class *
pom_class_get(struct pom_class_mgr * classMgr, pom_class_id_t classId);

struct pom_class *
pom_class_find(struct pom_class_mgr * classMgr, cpe_hash_string_t className);

int pom_class_add_new_page(
    struct pom_class * theClass,
    void * page,
    error_monitor_t em);

int pom_class_add_old_page(
    struct pom_class * theClass,
    void * page,
    error_monitor_t em);

int32_t pom_class_alloc_object(struct pom_class *cls);
int32_t pom_class_addr_2_object(struct pom_class *cls, void * page, void * addr);

void pom_class_free_object(struct pom_class *cls, int32_t value, error_monitor_t em);
void * pom_class_get_object(struct pom_class *cls, int32_t value, error_monitor_t em);

#define pom_class_ba_of_page(page) (cpe_ba_t)(((char*)(page)) + sizeof(struct pom_data_page_head))
#define pom_class_page_buf_len(page_count) (sizeof(void*) * (page_count))

/*debuger operations*/
struct pom_debuger * pom_debuger_create(mem_allocrator_t alloc, uint32_t m_stack_size, error_monitor_t em);
void pom_debuger_free(struct pom_debuger * debuger);

void pom_debuger_on_alloc(struct pom_debuger * debuger, pom_oid_t oid);
void pom_debuger_on_free(struct pom_debuger * debuger, pom_oid_t oid);
void pom_debuger_restore_one_page(struct pom_debuger * debuger, void * page_data);

#ifdef __cplusplus
}
#endif

#endif
