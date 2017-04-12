#ifndef USF_MONGO_CLI_TYPES_H
#define USF_MONGO_CLI_TYPES_H
#include "usf/logic/logic_types.h"
#include "usf/mongo_driver/mongo_driver_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mongo_cli_proxy * mongo_cli_proxy_t;
typedef struct mongo_cli_result * mongo_cli_result_t;

typedef enum mongo_cli_pkg_parse_evt {
    mongo_cli_pkg_parse_begin,
    mongo_cli_pkg_parse_end,
    mongo_cli_pkg_parse_data
} mongo_cli_pkg_parse_evt_t;

typedef enum mongo_cli_pkg_parse_result {
    mongo_cli_pkg_parse_success = 1,
    mongo_cli_pkg_parse_fail = 2,
    mongo_cli_pkg_parse_next = 3
} mongo_cli_pkg_parse_result_t;
    
typedef mongo_cli_pkg_parse_result_t
(*mongo_cli_pkg_parser)(
    void * ctx, 
    logic_require_t require, mongo_pkg_t pkg, logic_data_t * result_data, LPDRMETA result_meta,
    mongo_cli_pkg_parse_evt_t evt, const void * bson_input, size_t bson_capacity);

#ifdef __cplusplus
}
#endif

#endif
