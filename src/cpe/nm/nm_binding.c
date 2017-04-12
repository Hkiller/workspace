#include <assert.h>
#include <string.h>
#include "nm_internal_ops.h"

void nm_binding_free(struct nm_binding * binding) {
    nm_mgr_t nmm;
    assert(binding);
    assert(binding->m_group);
    assert(binding->m_node);
    assert(binding->m_group->m_mgr == binding->m_node->m_mgr);

    nmm = binding->m_node->m_mgr;

    cpe_hash_table_remove_by_ins(
        &binding->m_group->m_members,
        binding);

    TAILQ_REMOVE(
        &binding->m_node->m_to_group_bindings,
        binding,
        m_qh);

    nm_binding_put(nmm, binding);
}

struct nm_binding *
nm_binding_create(struct nm_group * group, nm_node_t node) {
    struct nm_binding * binding;

    assert(group);
    assert(node);
    assert(group->m_mgr == node->m_mgr);

    binding = nm_binding_get(group->m_mgr);
    if (binding == NULL) return NULL;

    binding->m_group = group;
    binding->m_node = node;

    /*insert to group*/
    cpe_hash_entry_init(&binding->m_hh_for_group);
    if (cpe_hash_table_insert_unique(&group->m_members, binding) != 0) {
        nm_binding_put(group->m_mgr, binding);
        return NULL;
    }

    /*link to node*/
    TAILQ_INSERT_HEAD(&node->m_to_group_bindings, binding, m_qh);

    return binding;
}

struct nm_binding * nm_binding_get(nm_mgr_t nmm) {
    assert(nmm);
    if (TAILQ_EMPTY(&nmm->m_binding_free_list)) {
        return
            (struct nm_binding *)
            mem_buffer_alloc(
                &nmm->m_binding_buffer,
                sizeof(struct nm_binding));
    }
    else {
        struct nm_binding * r =
            TAILQ_FIRST(&nmm->m_binding_free_list);
        TAILQ_REMOVE(&nmm->m_binding_free_list, r, m_qh);
        return r;
    }
}

void nm_binding_put(nm_mgr_t nmm, struct nm_binding * binding) {
    TAILQ_INSERT_HEAD(&nmm->m_binding_free_list, binding, m_qh);
}

uint32_t nm_binding_node_name_hash(const struct nm_binding * binding) {
    return cpe_hs_value(binding->m_node->m_name);
}

int nm_binding_node_name_cmp(
    const struct nm_binding * l, const struct nm_binding * r)
{
    return cpe_hs_cmp(l->m_node->m_name, r->m_node->m_name) == 0;
}
