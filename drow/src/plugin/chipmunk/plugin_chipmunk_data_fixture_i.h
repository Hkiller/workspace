#ifndef PLUGIN_CHIPMUNK_DATA_FIXTURE_I_H
#define PLUGIN_CHIPMUNK_DATA_FIXTURE_I_H
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin_chipmunk_data_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_fixture {
    plugin_chipmunk_data_body_t m_body;
    TAILQ_ENTRY(plugin_chipmunk_data_fixture) m_next_for_body;
    CHIPMUNK_FIXTURE m_data;
    uint32_t m_polygon_node_count;
    plugin_chipmunk_data_polygon_node_list_t m_polygon_node_list;
};

#ifdef __cplusplus
}
#endif

#endif
