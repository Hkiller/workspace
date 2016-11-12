#ifndef CPE_POM_MANAGE_H
#define CPE_POM_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_mgr_t
pom_mgr_create(
    mem_allocrator_t alloc,
    size_t page_size,
    size_t buffer_size);

void pom_mgr_free(pom_mgr_t mgr);

int pom_mgr_set_backend(
    pom_mgr_t mgr,
    pom_backend_t backend,
    void * backend_ctx);

pom_class_id_t
pom_mgr_add_class(
    pom_mgr_t omm,
    const char * className,
    size_t object_size,
    size_t align,
    error_monitor_t em);

int pom_mgr_add_class_with_id(
    pom_mgr_t omm,
    pom_class_id_t classId,
    const char * className,
    size_t object_size,
    size_t align,
    error_monitor_t em);

size_t pom_mgr_page_size(pom_mgr_t omm);
size_t pom_mgr_buf_size(pom_mgr_t omm);

void pom_mgr_set_auto_validate(pom_mgr_t omm, int auto_validate);
int pom_mgr_auto_validate(pom_mgr_t omm);

void pom_mgr_buffers(struct pom_buffer_it * it, pom_mgr_t omm);
void * pom_next_buffer(struct pom_buffer_it * it);

void pom_mgr_buffer_ids(struct pom_buffer_id_it * it, pom_mgr_t omm);
pom_buffer_id_t pom_next_buffer_id(struct pom_buffer_id_it * it);

int pom_mgr_set_backend_memory(pom_mgr_t omm, mem_allocrator_t alloc);

int pom_mgr_add_new_buffer(pom_mgr_t omm, pom_buffer_id_t buf_id, error_monitor_t em);
int pom_mgr_attach_old_buffer(pom_mgr_t omm, pom_buffer_id_t buf_id, error_monitor_t em);

void pom_mgr_dump_page_info(write_stream_t stream, pom_mgr_t mgr, int level);
void pom_mgr_dump_alloc_info(write_stream_t stream, pom_mgr_t mgr, int level);

int pom_mgr_validate(pom_mgr_t mgr, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
