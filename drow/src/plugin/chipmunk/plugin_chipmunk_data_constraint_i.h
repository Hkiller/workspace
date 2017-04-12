#ifndef PLUGIN_CHIPMUNK_DATA_CONSTRAINT_I_H
#define PLUGIN_CHIPMUNK_DATA_CONSTRAINT_I_H
#include "plugin/chipmunk/plugin_chipmunk_data_constraint.h"
#include "plugin_chipmunk_data_scene_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_constraint {
    plugin_chipmunk_data_scene_t m_scene;
    TAILQ_ENTRY(plugin_chipmunk_data_constraint) m_next_for_scene;
    CHIPMUNK_CONSTRAINT m_data;
};

#ifdef __cplusplus
}
#endif

#endif
