#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "plugin_chipmunk_data_polygon_node_i.h"
#include "plugin_chipmunk_data_fixture_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_data_polygon_node_t
plugin_chipmunk_data_polygon_node_create(plugin_chipmunk_data_fixture_t data_fixture) {
    plugin_chipmunk_module_t module = data_fixture->m_body->m_scene->m_module;
    plugin_chipmunk_data_polygon_node_t data_polygon_node;

    data_polygon_node = (plugin_chipmunk_data_polygon_node_t)mem_alloc(module->m_alloc, sizeof(struct plugin_chipmunk_data_polygon_node));
    if(data_polygon_node == NULL) {
        CPE_ERROR(module->m_em, "create polygon_node proto: alloc fail!");
        return NULL;
    }

    bzero(data_polygon_node, sizeof(*data_polygon_node));
    data_polygon_node->m_fixture = data_fixture;

    data_fixture->m_polygon_node_count++;
    TAILQ_INSERT_TAIL(&data_fixture->m_polygon_node_list, data_polygon_node, m_next_for_fixture);

    return data_polygon_node;
}

void plugin_chipmunk_data_polygon_node_free(plugin_chipmunk_data_polygon_node_t data_polygon_node) {
    plugin_chipmunk_data_fixture_t data_fixture = data_polygon_node->m_fixture;
    plugin_chipmunk_module_t module = data_fixture->m_body->m_scene->m_module;
    
    data_fixture->m_polygon_node_count--;
    TAILQ_REMOVE(&data_fixture->m_polygon_node_list, data_polygon_node, m_next_for_fixture);

    mem_free(module->m_alloc, data_polygon_node);
}

CHIPMUNK_POLYGON_NODE * plugin_chipmunk_data_polygon_node_data(plugin_chipmunk_data_polygon_node_t polygon_node) {
    return &polygon_node->m_data;
}

plugin_chipmunk_data_polygon_node_t plugin_chipmunk_data_polygon_node_head(plugin_chipmunk_data_fixture_t fixture) {
    return TAILQ_FIRST(&fixture->m_polygon_node_list);
}
    
plugin_chipmunk_data_polygon_node_t plugin_chipmunk_data_polygon_node_next(plugin_chipmunk_data_polygon_node_t node) {
    return TAILQ_NEXT(node, m_next_for_fixture);
}
    
#ifdef __cplusplus
}
#endif
