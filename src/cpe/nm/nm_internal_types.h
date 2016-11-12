#ifndef CPE_NM_INTERNAL_TYPES_H
#define CPE_NM_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "cpe/nm/nm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct nm_group;
struct nm_binding;
typedef TAILQ_HEAD(nm_binding_list, nm_binding) nm_binding_list_t;

struct nm_mgr {
    mem_allocrator_t m_alloc;
    struct cpe_hash_table m_nodes;

    struct mem_buffer m_binding_buffer;
    nm_binding_list_t m_binding_free_list;
};

#define CPE_NM_NODE_HEAD()                       \
    nm_mgr_t m_mgr;                          \
    nm_node_category_t m_category;           \
    nm_node_type_t m_type;                   \
    cpe_hash_string_t m_name;                   \
    size_t m_data_capacity;                     \
                                                \
    struct cpe_hash_entry m_hh_for_mgr;         \
    nm_binding_list_t m_to_group_bindings

struct nm_node {
    CPE_NM_NODE_HEAD();
};

struct nm_binding {
    struct nm_group * m_group;
    nm_node_t m_node;

    struct cpe_hash_entry m_hh_for_group;
    TAILQ_ENTRY(nm_binding) m_qh; /*for free list or node*/
};

struct nm_group {
    struct cpe_hash_table m_members;
    CPE_NM_NODE_HEAD();
};

struct nm_instance {
    CPE_NM_NODE_HEAD();
};

struct nm_node_groups_it {
    nm_it_next_fun m_next_fun;
    struct nm_binding * m_curent;
};
extern char check_node_groups_it_size[sizeof(struct nm_node_it) - sizeof(struct nm_node_groups_it)];

struct nm_node_in_mgr_it {
    nm_it_next_fun m_next_fun;
    struct cpe_hash_it m_hash_it;
};
extern char check_node_in_mgr_it_size[sizeof(struct nm_node_it) - sizeof(struct nm_node_in_mgr_it)];

struct nm_node_in_group_it {
    nm_it_next_fun m_next_fun;
    struct cpe_hash_it m_hash_it;
};
extern char check_node_in_group_it_size[sizeof(struct nm_node_it) - sizeof(struct nm_node_in_group_it)];

#define nm_group_from_node(n) ((struct nm_group*)(((char*)(n)) - ((sizeof(struct nm_group) - sizeof(struct nm_node)))))
#define nm_node_from_group(n) ((nm_node_t)(((char*)(n)) + ((sizeof(struct nm_group) - sizeof(struct nm_node)))))

#define nm_instance_from_node(n) ((struct nm_instance*)(((char*)(n)) - ((sizeof(struct nm_instance) - sizeof(struct nm_node)))))
#define nm_node_from_instance(n) ((nm_node_t)(((char*)(n)) + ((sizeof(struct nm_instance) - sizeof(struct nm_node)))))

#ifdef __cplusplus
}
#endif

#endif
