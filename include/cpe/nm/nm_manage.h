#ifndef CPE_NM_MANAGE_H
#define CPE_NM_MANAGE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/hash_string.h"
#include "nm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

nm_mgr_t nm_mgr_create(mem_allocrator_t alloc);
void nm_mgr_free(nm_mgr_t nmm);

void nm_mgr_free_nodes_with_type_name(nm_mgr_t nmm, const char * type);
void nm_mgr_free_nodes_with_type(nm_mgr_t nmm, nm_node_type_t type);

nm_node_t nm_group_create(nm_mgr_t nmm, const char * name, size_t capacity);
int nm_group_add_member(nm_node_t grp, nm_node_t sub);

nm_node_t nm_instance_create(nm_mgr_t nmm, const char * name, size_t capacity);

void nm_node_free(nm_node_t node);

void nm_node_set_type(nm_node_t node, nm_node_type_t type);

void nm_group_free_members(nm_node_t node);

#ifdef __cplusplus
}
#endif

#endif
