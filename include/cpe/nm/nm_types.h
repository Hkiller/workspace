#ifndef CPE_NM_TYPES_H
#define CPE_NM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nm_node_category {
    nm_node_group,
    nm_node_instance
} nm_node_category_t;

typedef struct nm_mgr * nm_mgr_t;
typedef struct nm_node * nm_node_t;
typedef struct nm_node_type * nm_node_type_t;
typedef struct nm_node_it * nm_node_it_t;

typedef nm_node_t (*nm_it_next_fun)(nm_node_it_t it);

struct nm_node_it {
    nm_it_next_fun m_next_fun;
    char m_reserve[32];
};

struct nm_node_type {
    const char * name;
    void (*destruct)(nm_node_t node);
};

#ifdef __cplusplus
}
#endif

#endif
