#ifndef USF_MONGO_DRIVER_SYSTEM_H
#define USF_MONGO_DRIVER_SYSTEM_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mongo_doc * mongo_doc_t;

typedef enum mongo_driver_error {
    mongo_driver_success = 0
    , mongo_driver_not_connected
} mongo_driver_error_t;

typedef struct mongo_doc_it {
    mongo_doc_t (*next)(struct mongo_doc_it * it);
    char m_data[16];
} * mongo_doc_it_t;

typedef enum  mongo_data_error {
    mongo_data_error_duplicate_key = 11000
} mongo_data_error_t;

typedef struct mongo_host_port * mongo_host_port_t;
typedef struct mongo_driver * mongo_driver_t;
typedef struct mongo_pkg * mongo_pkg_t;

typedef enum mongo_read_mode {
    mongo_read_unknown             = 0,
    mongo_read_primary             = (1 << 0),
    mongo_read_secondary           = (1 << 1),
    mongo_read_primary_preferred   = (1 << 2) | mongo_read_primary,
    mongo_read_secondary_preferred = (1 << 2) | mongo_read_secondary,
    mongo_read_nearest             = (1 << 3) | mongo_read_secondary,
} mongo_read_mode_t;

typedef enum mongo_topology_type {
    mongo_topology_type_unknown,
    mongo_topology_type_sharded,
    mongo_topology_type_rs_no_primary,
    mongo_topology_type_rs_with_primary,
    mongo_topology_type_single,
    mongo_topology_type_count,
} mongo_topology_type_t;
        
#ifdef __cplusplus
}
#endif

#endif
