#include <assert.h>
#include <string.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "nm_internal_ops.h"

nm_node_t
nm_node_alloc(
    nm_mgr_t nmm,
    const char * name,
    nm_node_category_t category,
    size_t bodyLen,
    size_t capacity)
{
    char * buf;
    nm_node_t node;
    size_t nameLen;

    assert(nmm);
    assert(name);

    nameLen = cpe_hs_len_to_binary_len(strlen(name));
    CPE_PAL_ALIGN_DFT(nameLen);

    buf = mem_alloc(nmm->m_alloc, nameLen + bodyLen + capacity);
    if (buf == NULL) return NULL;

    cpe_hs_init((cpe_hash_string_t)buf, nameLen, name);

    node = (nm_node_t)(buf + nameLen + bodyLen - sizeof(struct nm_node));

    node->m_mgr = nmm;
    node->m_category = category;
    node->m_name = (cpe_hash_string_t)buf;
    node->m_data_capacity = capacity;
    node->m_type = NULL;

    TAILQ_INIT(&node->m_to_group_bindings);

    cpe_hash_entry_init(&node->m_hh_for_mgr);
    if (cpe_hash_table_insert_unique(&nmm->m_nodes, node) != 0) {
        mem_free(nmm->m_alloc, buf);
        return NULL;
    }

    if (node->m_category == nm_node_group) {
        struct nm_group * group;

        group = nm_group_from_node(node);

        if (cpe_hash_table_init(
                &group->m_members,
                nmm->m_alloc,
                (cpe_hash_fun_t)nm_binding_node_name_hash,
                (cpe_hash_eq_t)nm_binding_node_name_cmp,
                CPE_HASH_OBJ2ENTRY(nm_binding, m_hh_for_group),
                0) != 0)
        {
            cpe_hash_table_remove_by_ins(&nmm->m_nodes, node);
            mem_free(nmm->m_alloc, buf);
            return NULL;
        }
    }

    return node;
}

void nm_node_free(nm_node_t node) {
    if (node == NULL) return;
    if (node->m_type && node->m_type->destruct) {
        node->m_type->destruct(node);
    }

    while(!TAILQ_EMPTY(&node->m_to_group_bindings)) {
        nm_binding_free(TAILQ_FIRST(&node->m_to_group_bindings));
    }

    if (node->m_category == nm_node_group) {
        struct cpe_hash_it member_it;
        struct nm_binding * member;
        struct nm_group * group;
        
        group = nm_group_from_node(node);

        cpe_hash_it_init(&member_it, &group->m_members);
        member = cpe_hash_it_next((&member_it));
        while(member) {
            struct nm_binding * next = cpe_hash_it_next(&member_it);
            nm_binding_free(member);
            member = next; 
        }
        cpe_hash_table_fini(&group->m_members);
    }

    cpe_hash_table_remove_by_ins(&node->m_mgr->m_nodes, node);

    mem_free(node->m_mgr->m_alloc, node->m_name);
}

size_t nm_node_capacity(nm_node_t node) {
    return node->m_data_capacity;
}

void * nm_node_data(nm_node_t node) {
    return (void*)(node + 1);
}

uint32_t nm_node_hash(const nm_node_t node) {
    return cpe_hs_value(node->m_name);
}

int nm_node_cmp(const nm_node_t l, const nm_node_t r) {
    return cpe_hs_value(l->m_name) == cpe_hs_value(r->m_name)
        && strcmp(cpe_hs_data(l->m_name), cpe_hs_data(r->m_name)) == 0;
}

const char * nm_node_name(nm_node_t node) {
    return cpe_hs_data(node->m_name);
}

void nm_node_set_type(nm_node_t node, nm_node_type_t type) {
    node->m_type = type;
}

nm_node_type_t nm_node_type(nm_node_t node) {
    return node->m_type;
}

const char * nm_node_type_name(nm_node_t node) {
    return node->m_type ? node->m_type->name : NULL;
}

cpe_hash_string_t
nm_node_name_hs(nm_node_t node) {
    return node->m_name;
}

nm_node_category_t nm_node_category(nm_node_t node) {
    return node->m_category;
}

nm_node_t nm_node_next_group(nm_node_it_t it) {
    struct nm_node_groups_it * groupIt;
    nm_node_t rv;

    assert(it);
    groupIt = (struct nm_node_groups_it *)it;

    if (groupIt->m_curent == NULL) return NULL;

    rv = nm_node_from_group(groupIt->m_curent->m_group);
    groupIt->m_curent = TAILQ_NEXT(groupIt->m_curent, m_qh);
    return rv;
}

int nm_node_groups(nm_node_it_t it, nm_node_t node) {
    struct nm_node_groups_it * groupIt;

    assert(it);
    if (node == NULL) return -1;

    groupIt = (struct nm_node_groups_it *)it;
    groupIt->m_next_fun = nm_node_next_group;
    groupIt->m_curent = TAILQ_FIRST(&node->m_to_group_bindings);

    return 0;
}

nm_mgr_t nm_node_mgr(nm_node_t node) {
    return node->m_mgr;
}

nm_node_t nm_node_from_data(void * data) {
    return ((nm_node_t)data) - 1;
}
