#ifndef DROW_PLUGIN_CHIPMUNK_DATA_POLYGON_NODE_H
#define DROW_PLUGIN_CHIPMUNK_DATA_POLYGON_NODE_H
#include "protocol/plugin/chipmunk/chipmunk_info.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*polygon_node*/
plugin_chipmunk_data_polygon_node_t plugin_chipmunk_data_polygon_node_create(plugin_chipmunk_data_fixture_t fixture);
void plugin_chipmunk_data_polygon_node_free(plugin_chipmunk_data_polygon_node_t polygon_node);

CHIPMUNK_POLYGON_NODE * plugin_chipmunk_data_polygon_node_data(plugin_chipmunk_data_polygon_node_t polygon_node);

plugin_chipmunk_data_polygon_node_t plugin_chipmunk_data_polygon_node_head(plugin_chipmunk_data_fixture_t fixture);
plugin_chipmunk_data_polygon_node_t plugin_chipmunk_data_polygon_node_next(plugin_chipmunk_data_polygon_node_t node);
    
#ifdef __cplusplus
}
#endif

#endif
