#include "gd/app/app_context.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_ops.h"

int bpg_bind_binding_create(
    bpg_bind_manage_t mgr,
    uint64_t client_id,
    uint32_t connection_id)
{
    struct bpg_bind_binding * binding;

    binding = mem_alloc(mgr->m_alloc, sizeof(struct bpg_bind_binding));
    if (binding == NULL) return -1;

    binding->m_client_id = client_id;
    binding->m_connection_id = connection_id;

    cpe_hash_entry_init(&binding->m_hh_client);
    cpe_hash_entry_init(&binding->m_hh_connection);

    if (cpe_hash_table_insert_unique(&mgr->m_cliensts, binding) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: create client binding %d ==> %d: client is already exist!",
            bpg_bind_manage_name(mgr), (int)client_id, (int)connection_id);
        mem_free(mgr->m_alloc, binding);
        return -1;
    }

    if (cpe_hash_table_insert_unique(&mgr->m_connections, binding) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: create client binding %d ==> %d: connection is already exist!",
            bpg_bind_manage_name(mgr), (int)client_id, (int)connection_id);
        cpe_hash_table_remove_by_ins(&mgr->m_cliensts, binding);
        mem_free(mgr->m_alloc, binding);
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: create client binding %d ==> %d: success!",
            bpg_bind_manage_name(mgr), (int)client_id, (int)connection_id);
    }

    return 0;
}

void bpg_bind_binding_free(bpg_bind_manage_t mgr, struct bpg_bind_binding * binding) {
    cpe_hash_table_remove_by_ins(&mgr->m_cliensts, binding);
    cpe_hash_table_remove_by_ins(&mgr->m_connections, binding);
    mem_free(mgr->m_alloc, binding);
}

struct bpg_bind_binding *
bpg_bind_binding_find_by_client_id(bpg_bind_manage_t mgr, uint64_t client_id) {
    struct bpg_bind_binding key;
    key.m_client_id = client_id;

    return (struct bpg_bind_binding *)cpe_hash_table_find(&mgr->m_cliensts, &key);
}

struct bpg_bind_binding *
bpg_bind_binding_find_by_connection_id(bpg_bind_manage_t mgr, uint32_t connection_id) {
    struct bpg_bind_binding key;
    key.m_connection_id = connection_id;

    return (struct bpg_bind_binding *)cpe_hash_table_find(&mgr->m_connections, &key);
}

uint32_t bpg_bind_binding_client_id_hash(const struct bpg_bind_binding * binding) {
    return (uint32_t)binding->m_client_id;
}

int bpg_bind_binding_client_id_cmp(const struct bpg_bind_binding * l, const struct bpg_bind_binding * r) {
    return l->m_client_id == r->m_client_id;
}

uint32_t bpg_bind_binding_connection_id_hash(const struct bpg_bind_binding * binding) {
    return (uint32_t)binding->m_connection_id;
}

int bpg_bind_binding_connection_id_cmp(const struct bpg_bind_binding * l, const struct bpg_bind_binding * r) {
    return l->m_connection_id == r->m_connection_id;
}

void bpg_bind_binding_free_all(bpg_bind_manage_t mgr) {
    struct cpe_hash_it binding_it;
    struct bpg_bind_binding * binding;

    cpe_hash_it_init(&binding_it, &mgr->m_cliensts);

    binding = cpe_hash_it_next(&binding_it);
    while(binding) {
        struct bpg_bind_binding * next = cpe_hash_it_next(&binding_it);
        bpg_bind_binding_free(mgr, binding);
        binding = next;
    }
}


