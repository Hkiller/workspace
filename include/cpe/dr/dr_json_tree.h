#ifndef CPE_DR_JSON_TREE_H
#define CPE_DR_JSON_TREE_H
#include "yajl/yajl_tree.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_json_tree_read(
    void * result,
    size_t capacity,
    yajl_val input,
    LPDRMETA meta,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
