#include <assert.h>
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "nm_internal_ops.h"

nm_node_t
nm_group_create(nm_mgr_t nmm, const char * name, size_t capacity) {
    assert(nmm);
    assert(name);

    return nm_node_alloc(
        nmm, name,
        nm_node_group, sizeof(struct nm_group),
        capacity);
}

int nm_group_add_member(nm_node_t grp, nm_node_t sub) {
    if(grp == NULL
       || sub == NULL
       || grp->m_category != nm_node_group)
    {
        return -1;
    }

    return (nm_binding_create(nm_group_from_node(grp), sub) == NULL) ? -1 : 0;
}

nm_node_t nm_group_next_member(nm_node_it_t it) {
    struct nm_node_in_group_it * nodeIt;
    struct nm_binding * binding;

    assert(it);
    nodeIt = (struct nm_node_in_group_it *)it;

    binding = (struct nm_binding *)cpe_hash_it_next(&nodeIt->m_hash_it);
    return binding ? binding->m_node : NULL;
}

int nm_group_members(nm_node_it_t it, nm_node_t node) {
    struct nm_node_in_group_it * nodeIt;

    assert(it);
    if (node == NULL
        || node->m_category != nm_node_group) return -1;

    nodeIt = (struct nm_node_in_group_it *)it;
    nodeIt->m_next_fun = nm_group_next_member;
    cpe_hash_it_init(&nodeIt->m_hash_it, &(nm_group_from_node(node))->m_members);

    return 0;
}

void nm_group_free_members(nm_node_t node) {
    struct cpe_hash_it it;
    struct nm_binding * binding;

    assert(node->m_category == nm_node_group);
    if (node->m_category != nm_node_group) return;

    cpe_hash_it_init(&it, &(nm_group_from_node(node))->m_members);
    while((binding = (struct nm_binding *)cpe_hash_it_next(&it))) {
        struct nm_binding * next =  (struct nm_binding *)cpe_hash_it_next(&it);
        nm_node_free(binding->m_node);
        binding = next;
    }
}

int nm_group_member_count(nm_node_t node) {
    if (node->m_category != nm_node_group) {
        return -1;
    }
    else {
        return cpe_hash_table_count(&(nm_group_from_node(node))->m_members);
    }
}

nm_node_t nm_group_find_member_nc(nm_node_t node, const char * name) {
    size_t nameLen = cpe_hs_len_to_binary_len(strlen(name));
    char buf[CPE_STACK_BUF_LEN(nameLen)];
    cpe_hs_init((cpe_hash_string_t)buf, sizeof(buf), name);
    return nm_group_find_member(node, (cpe_hash_string_t)buf);
}

nm_node_t nm_group_find_member(nm_node_t node, cpe_hash_string_t name) {
    struct nm_binding * binding;
    struct nm_node buf_node;
    struct nm_binding buf_binding;

    if (node->m_category != nm_node_group) {
        return NULL;
    }

    buf_binding.m_node = &buf_node;
    buf_node.m_name = name;

    binding = (struct nm_binding *)
        cpe_hash_table_find(&(nm_group_from_node(node))->m_members, &buf_binding);

    return binding == NULL ? NULL : binding->m_node;
}
