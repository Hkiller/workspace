#include "cpe/pom/pom_manage.h"

pom_buffer_id_t pom_backend_mem_alloc(size_t size, void * context) {
    return (pom_buffer_id_t)mem_alloc((mem_allocrator_t)context, size);
}

void pom_backend_mem_clear(struct pom_buffer_id_it * buf_ids, void * context) {
    pom_buffer_id_t bufId;
    while((bufId = pom_next_buffer_id(buf_ids)) != POM_INVALID_BUFFER_ID) {
        mem_free((mem_allocrator_t)context, (void*)bufId);
    }
}

struct pom_backend g_pom_backend_mem = {
    pom_backend_mem_alloc
    , NULL
    , pom_backend_mem_clear
};

int pom_mgr_set_backend_memory(pom_mgr_t omm, mem_allocrator_t alloc) {
    return pom_mgr_set_backend(omm, &g_pom_backend_mem, alloc);
}
