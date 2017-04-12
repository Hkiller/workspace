#ifndef PLUGIN_SPINE_OBJ_IK_I_H
#define PLUGIN_SPINE_OBJ_IK_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/spine/plugin_spine_obj_ik.h"
#include "plugin_spine_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum plugin_spine_obj_ik_move_type {
    plugin_spine_obj_ik_move_by_angle = 1,
    plugin_spine_obj_ik_move_by_pos = 2
};

struct plugin_spine_obj_ik {
    plugin_spine_obj_t m_obj;
    TAILQ_ENTRY(plugin_spine_obj_ik) m_next;
    char m_name[32];
    struct spIkConstraint * m_ik;
    float m_duration;
    float m_worked_time;
    float m_pos_speed;
    float m_angle_speed;
    float m_angle_min;
    float m_angle_max;

    uint8_t m_move_type;
    union {
        struct {
            float m_angle_start;
            float m_angle_target;
            float m_distance_start;
            float m_distance_target;
        } m_by_angle;
        struct {
            ui_vector_2 m_start_pos;
            ui_vector_2 m_target_pos;
        } m_by_pos;
    };
};

void plugin_spine_obj_ik_update(plugin_spine_obj_ik_t ik, float delta);
void plugin_spine_obj_ik_real_free_all(plugin_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
