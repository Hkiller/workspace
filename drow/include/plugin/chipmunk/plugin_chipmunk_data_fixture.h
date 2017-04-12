#ifndef DROW_PLUGIN_CHIPMUNK_DATA_FIXTURE_H
#define DROW_PLUGIN_CHIPMUNK_DATA_FIXTURE_H
#include "protocol/plugin/chipmunk/chipmunk_info.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_fixture_it {
    plugin_chipmunk_data_fixture_t (*next)(struct plugin_chipmunk_data_fixture_it * it);
    char m_data[64];
};

/*fixture*/
plugin_chipmunk_data_fixture_t plugin_chipmunk_data_fixture_create(plugin_chipmunk_data_body_t body);
void plugin_chipmunk_data_fixture_free(plugin_chipmunk_data_fixture_t fixture);

CHIPMUNK_FIXTURE * plugin_chipmunk_data_fixture_data(plugin_chipmunk_data_fixture_t fixture);

uint32_t plugin_chipmunk_data_fixture_polygon_node_count(plugin_chipmunk_data_fixture_t fixture);

#define plugin_chipmunk_data_fixture_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
