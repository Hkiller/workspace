#include <assert.h> 
#include "cpe/pal/pal_stdio.h"
#include "conn_svr_ops.h"

conn_svr_backend_t
conn_svr_backend_create(conn_svr_t svr, uint16_t svr_type) {
    conn_svr_backend_t backend;
    backend = mem_alloc(svr->m_alloc, sizeof(struct conn_svr_backend));
    if (backend == NULL) {
        CPE_ERROR(svr->m_em, "%s: create backend: malloc fail!", conn_svr_name(svr));
        return NULL;
    }

    backend->m_svr = svr;
    backend->m_svr_type = svr_type;

    cpe_hash_entry_init(&backend->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_backends, backend) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: create backend: insert fail, svr_type %d already exist!",
            conn_svr_name(svr), svr_type);
        mem_free(svr->m_alloc, backend);
        return NULL;
    }
    
    return backend;
}

void conn_svr_backend_free(conn_svr_backend_t backend) {
    conn_svr_t svr = backend->m_svr;
    assert(svr);

    cpe_hash_table_remove_by_ins(&svr->m_backends, backend);

    mem_free(svr->m_alloc, backend);
}

void conn_svr_backend_free_all(conn_svr_t svr) {
    struct cpe_hash_it backend_it;
    conn_svr_backend_t backend;

    cpe_hash_it_init(&backend_it, &svr->m_backends);

    backend = cpe_hash_it_next(&backend_it);
    while(backend) {
        conn_svr_backend_t next = cpe_hash_it_next(&backend_it);
        conn_svr_backend_free(backend);
        backend = next;
    }
}

conn_svr_backend_t
conn_svr_backend_find(conn_svr_t svr, uint16_t svr_type) {
    struct conn_svr_backend key;

    key.m_svr_type = svr_type;

    return cpe_hash_table_find(&svr->m_backends, &key);
}

uint32_t conn_svr_backend_hash(conn_svr_backend_t backend) {
    return (uint32_t)backend->m_svr_type;
}

int conn_svr_backend_eq(conn_svr_backend_t l, conn_svr_backend_t r) {
    return l->m_svr_type == r->m_svr_type;
}
