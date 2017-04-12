#ifndef DROW_PLUGIN_CHIPMUNK_DATA_CONSTRAINT_H
#define DROW_PLUGIN_CHIPMUNK_DATA_CONSTRAINT_H
#include "protocol/plugin/chipmunk/chipmunk_info.h"
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_constraint_it {
    plugin_chipmunk_data_constraint_t (*next)(struct plugin_chipmunk_data_constraint_it * it);
    char m_data[64];
};

/*constraint*/
plugin_chipmunk_data_constraint_t plugin_chipmunk_data_constraint_create(plugin_chipmunk_data_scene_t scene);
void plugin_chipmunk_data_constraint_free(plugin_chipmunk_data_constraint_t constraint);

CHIPMUNK_CONSTRAINT * plugin_chipmunk_data_constraint_data(plugin_chipmunk_data_constraint_t constraint);

#define plugin_chipmunk_data_constraint_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
