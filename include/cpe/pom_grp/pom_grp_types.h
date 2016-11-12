#ifndef CPE_POM_GRP_TYPES_H
#define CPE_POM_GRP_TYPES_H
#include "cpe/pom/pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pom_grp_obj * pom_grp_obj_t;
typedef struct pom_grp_obj_mgr * pom_grp_obj_mgr_t;
typedef struct pom_grp_store * pom_grp_store_t;
typedef struct pom_grp_store_table * pom_grp_store_table_t;
typedef struct pom_grp_store_entry * pom_grp_store_entry_t;

typedef enum pom_grp_entry_type {
    pom_grp_entry_type_normal
    , pom_grp_entry_type_list
    , pom_grp_entry_type_ba
    , pom_grp_entry_type_binary
} pom_grp_entry_type_t;

struct pom_grp_range {
    void * begin;
    void * end;
};

typedef struct pom_grp_entry_meta * pom_grp_entry_meta_t;
typedef struct pom_grp_meta * pom_grp_meta_t;

typedef struct pom_grp_entry_meta_it {
    pom_grp_entry_meta_t (*next)(struct pom_grp_entry_meta_it * it);
    pom_grp_entry_meta_t m_data;
} * pom_grp_entry_meta_it_t;

typedef struct pom_grp_obj_it {
    struct pom_obj_it m_data;
} * pom_grp_obj_it_t;

typedef struct pom_grp_store_table_it {
    pom_grp_store_table_t (*next)(struct pom_grp_store_table_it * it);
    char m_data[16];
} * pom_grp_store_table_it_t;

typedef struct pom_grp_store_entry_it {
    pom_grp_store_entry_t (*next)(struct pom_grp_store_entry_it * it);
    char m_data[16];
} * pom_grp_store_entry_it_t;

#ifdef __cplusplus
}
#endif

#endif
