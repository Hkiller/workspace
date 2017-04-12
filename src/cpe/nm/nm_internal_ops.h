#ifndef CPE_NM_INTERNAL_OPS_H
#define CPE_NM_INTERNAL_OPS_H
#include "nm_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*mgr operations*/
#define nm_mgr_group_mask(nmm) ((cpe_ba_t)(nmm + 1))

/*node operations*/
nm_node_t
nm_node_alloc(
    nm_mgr_t nmm,
    const char * name, nm_node_category_t category,
    size_t bodyLen, size_t capacity);

uint32_t nm_node_hash(const nm_node_t node);
int nm_node_cmp(const nm_node_t l, const nm_node_t r);

/*binding operations*/
struct nm_binding * nm_binding_get(nm_mgr_t nmm);
void nm_binding_put(nm_mgr_t nmm, struct nm_binding * binding);

struct nm_binding *
nm_binding_create(struct nm_group * group, nm_node_t node);
void nm_binding_free(struct nm_binding * binding);

uint32_t nm_binding_node_name_hash(const struct nm_binding * binding);
int nm_binding_node_name_cmp(const struct nm_binding * l, const struct nm_binding * r);

#ifdef __cplusplus
}
#endif

#endif
