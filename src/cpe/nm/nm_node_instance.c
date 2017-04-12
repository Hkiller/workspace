#include <assert.h>
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "nm_internal_ops.h"

nm_node_t
nm_instance_create(nm_mgr_t nmm, const char *  name, size_t capacity) {
    assert(nmm);
    assert(name);

    return nm_node_alloc(
        nmm, name,
        nm_node_instance, sizeof(struct nm_instance),
        capacity);
}
