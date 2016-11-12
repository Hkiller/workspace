#ifndef CPE_NM_NODE_H
#define CPE_NM_NODE_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/memory.h"
#include "nm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*mgr operations*/
nm_node_t nm_mgr_find_node(nm_mgr_t nmm, cpe_hash_string_t name);
nm_node_t nm_mgr_find_node_nc(nm_mgr_t nmm, const char * name);
int nm_mgr_nodes(nm_node_it_t it, nm_mgr_t nmm);

/*node operations*/
nm_mgr_t nm_node_mgr(nm_node_t node);
const char * nm_node_name(nm_node_t node);
cpe_hash_string_t nm_node_name_hs(nm_node_t node);
nm_node_category_t nm_node_category(nm_node_t node);
size_t nm_node_capacity(nm_node_t node);
void * nm_node_data(nm_node_t node);

int nm_node_groups(nm_node_it_t it, nm_node_t node);
nm_node_t nm_node_from_data(void * data);

nm_node_type_t nm_node_type(nm_node_t node);
const char * nm_node_type_name(nm_node_t node);

/*gruop operations*/
int nm_group_members(nm_node_it_t it, nm_node_t group);
int nm_group_member_count(nm_node_t group);
nm_node_t nm_group_find_member(nm_node_t group, cpe_hash_string_t name);
nm_node_t nm_group_find_member_nc(nm_node_t group, const char * name);

/*iterator operations*/
#define nm_node_next(it) (it)->m_next_fun((it))

#ifdef __cplusplus
}
#endif

#endif
