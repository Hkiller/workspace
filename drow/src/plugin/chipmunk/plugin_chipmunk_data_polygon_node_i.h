#ifndef PLUGIN_CHIPMUNK_DATA_POLYGON_NODE_I_H
#define PLUGIN_CHIPMUNK_DATA_POLYGON_NODE_I_H
#include "plugin/chipmunk/plugin_chipmunk_data_polygon_node.h"
#include "plugin_chipmunk_data_fixture_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_polygon_node {
    plugin_chipmunk_data_fixture_t m_fixture;
    TAILQ_ENTRY(plugin_chipmunk_data_polygon_node) m_next_for_fixture;
    CHIPMUNK_POLYGON_NODE m_data;
};

#ifdef __cplusplus
}
#endif

#endif
