#include <assert.h>
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "nm_internal_ops.h"

nm_mgr_t nm_mgr_create(mem_allocrator_t alloc) {
    nm_mgr_t nmm =
        mem_alloc(
            alloc,
            sizeof(struct nm_mgr));

    if (nmm == NULL) return NULL;

    nmm->m_alloc = alloc;
    if (cpe_hash_table_init(
            &nmm->m_nodes,
            alloc,
            (cpe_hash_fun_t)nm_node_hash,
            (cpe_hash_eq_t)nm_node_cmp,
            CPE_HASH_OBJ2ENTRY(nm_node, m_hh_for_mgr),
            0) != 0)
    {
        mem_free(alloc, nmm);
        return NULL;
    }

    mem_buffer_init(&nmm->m_binding_buffer, alloc);
    TAILQ_INIT(&nmm->m_binding_free_list);

    return nmm;
}

void nm_mgr_free(nm_mgr_t nmm) {
    struct cpe_hash_it node_it;
    nm_node_t node;

    if (nmm == NULL) return;

    cpe_hash_it_init(&node_it, &nmm->m_nodes);
    node = (nm_node_t)cpe_hash_it_next(&node_it);
    while(node) {
        nm_node_t next = (nm_node_t)cpe_hash_it_next(&node_it);
        nm_node_free(node);
        node = next;
    }

    cpe_hash_table_fini(&nmm->m_nodes);

    mem_buffer_clear(&nmm->m_binding_buffer);

    mem_free(nmm->m_alloc, nmm);
}

nm_node_t
nm_mgr_find_node_nc(nm_mgr_t nmm, const char * name) {
    size_t nameLen = cpe_hs_len_to_binary_len(strlen(name));
    char nameBuf[CPE_STACK_BUF_LEN(nameLen)];
    cpe_hs_init((cpe_hash_string_t)nameBuf, CPE_STACK_BUF_LEN(nameLen), name);
    return nm_mgr_find_node(nmm, (cpe_hash_string_t)nameBuf);
}

nm_node_t
nm_mgr_find_node(nm_mgr_t nmm, cpe_hash_string_t name) {
    struct nm_node buf;

    buf.m_name = name;

    return (nm_node_t)cpe_hash_table_find(&nmm->m_nodes, &buf);
}

nm_node_t nm_mgr_next_node(nm_node_it_t it) {
    struct nm_node_in_mgr_it * nodeIt;

    assert(it);
    nodeIt = (struct nm_node_in_mgr_it *)it;

    return (nm_node_t)cpe_hash_it_next(&nodeIt->m_hash_it);
}

int nm_mgr_nodes(nm_node_it_t it, nm_mgr_t nmm) {
    struct nm_node_in_mgr_it * nodeIt;

    assert(it);
    if (nmm == NULL) return -1;

    nodeIt = (struct nm_node_in_mgr_it *)it;
    nodeIt->m_next_fun = nm_mgr_next_node;
    cpe_hash_it_init(&nodeIt->m_hash_it, &nmm->m_nodes);

    return 0;
}

void nm_mgr_free_nodes_with_type_name(nm_mgr_t nmm, const char * type) {
    struct nm_node_it it;
    nm_node_t node;

    nm_mgr_nodes(&it, nmm);

    while((node = nm_node_next(&it))) {
        nm_node_t next = nm_node_next(&it);;

        if (strcmp(nm_node_type_name(node), type) == 0) {
            nm_node_free(node);
        }

        node = next;
    }
}

void nm_mgr_free_nodes_with_type(nm_mgr_t nmm, nm_node_type_t type) {
    struct nm_node_it it;
    nm_node_t node;

    nm_mgr_nodes(&it, nmm);

    while((node = nm_node_next(&it))) {
        nm_node_t next = nm_node_next(&it);;

        if (node->m_type == type) {
            nm_node_free(node);
        }

        node = next;
    }
}
