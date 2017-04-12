#ifndef PLUGIN_CHIPMUNK_DATA_BODY_I_H
#define PLUGIN_CHIPMUNK_DATA_BODY_I_H
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "plugin_chipmunk_data_scene_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_body {
    plugin_chipmunk_data_scene_t m_scene;
    TAILQ_ENTRY(plugin_chipmunk_data_body) m_next_for_scene;
    CHIPMUNK_BODY m_data;
    uint32_t m_fixture_count;
    plugin_chipmunk_data_fixture_list_t m_fixture_list;
};

#ifdef __cplusplus
}
#endif

#endif
