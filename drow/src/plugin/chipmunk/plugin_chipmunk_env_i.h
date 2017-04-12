#ifndef PLUGIN_CHIPMUNK_ENV_I_H
#define PLUGIN_CHIPMUNK_ENV_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/dr/dr_types.h"
#include "chipmunk/chipmunk_private.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_mask {
    char m_name[64];
};

struct plugin_chipmunk_collision_type {
    char m_name[64];
};
    
struct plugin_chipmunk_env {
    plugin_chipmunk_module_t m_module;
    int m_debug;
    cpSpace m_space;
    float m_ptm;
    float m_time_scale;
    float m_left_time;
    float m_step_duration;
    plugin_chipmunk_env_updator_list_t m_updators;

    struct plugin_chipmunk_collision_type m_collision_types[32];
    struct plugin_chipmunk_mask m_mask[32];
};

#ifdef __cplusplus
}
#endif

#endif
