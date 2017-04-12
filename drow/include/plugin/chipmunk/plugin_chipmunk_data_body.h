#ifndef DROW_PLUGIN_CHIPMUNK_DATA_BODY_H
#define DROW_PLUGIN_CHIPMUNK_DATA_BODY_H
#include "protocol/plugin/chipmunk/chipmunk_info.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_body_it {
    plugin_chipmunk_data_body_t (*next)(struct plugin_chipmunk_data_body_it * it);
    char m_data[64];
};

/*body*/
plugin_chipmunk_data_body_t plugin_chipmunk_data_body_create(plugin_chipmunk_data_scene_t scene);
void plugin_chipmunk_data_body_free(plugin_chipmunk_data_body_t body);

CHIPMUNK_BODY * plugin_chipmunk_data_body_data(plugin_chipmunk_data_body_t body);

plugin_chipmunk_data_body_t plugin_chipmunk_data_body_find_by_name(plugin_chipmunk_data_scene_t scene, const char * name);
uint32_t plugin_chipmunk_data_body_fixture_count(plugin_chipmunk_data_body_t body);
void plugin_chipmunk_data_body_fixtures(plugin_chipmunk_data_fixture_it_t body_it, plugin_chipmunk_data_body_t body);

#define plugin_chipmunk_data_body_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
