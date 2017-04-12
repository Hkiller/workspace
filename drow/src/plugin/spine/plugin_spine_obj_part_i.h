#ifndef PLUGIN_SPINE_OBJ_PART_I_H
#define PLUGIN_SPINE_OBJ_PART_I_H
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/spine/plugin_spine_obj_part_state.h"
#include "plugin_spine_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct plugin_spine_obj_part {
    plugin_spine_obj_t m_obj;
    TAILQ_ENTRY(plugin_spine_obj_part) m_next;
    char m_name[32];
    plugin_spine_obj_track_t m_track;
    uint8_t m_state_count;
    plugin_spine_obj_part_state_list_t m_states;
    plugin_spine_obj_part_state_t m_cur_state;
    plugin_spine_obj_part_transition_t m_enter_transition;
    uint8_t m_enter_anim_pos;
    plugin_spine_obj_anim_t m_enter_anim;
    plugin_spine_obj_anim_t m_enter_anim_delay_destory;
    plugin_spine_obj_anim_t m_state_anim;
};

struct plugin_spine_obj_part_state {
    plugin_spine_obj_part_t m_part;
    TAILQ_ENTRY(plugin_spine_obj_part_state) m_next;
    plugin_spine_obj_part_transition_list_t m_as_from_transitions;
    plugin_spine_obj_part_transition_list_t m_as_to_transitions;
    spAnimation * m_animation;
    char m_name[32];
};

struct plugin_spine_obj_part_transition_anim {
    spAnimation * m_animation;
    uint8_t m_loop_count;
};
    
struct plugin_spine_obj_part_transition {
    plugin_spine_obj_part_state_t m_from;
    TAILQ_ENTRY(plugin_spine_obj_part_transition) m_next_for_from;
    plugin_spine_obj_part_state_t m_to;
    TAILQ_ENTRY(plugin_spine_obj_part_transition) m_next_for_to;
    uint8_t m_animation_count;
    struct plugin_spine_obj_part_transition_anim m_animations[8];
    char m_name[32];
};
    
void plugin_spine_obj_part_real_free_all(plugin_spine_module_t module);
void plugin_spine_obj_part_state_real_free_all(plugin_spine_module_t module);
void plugin_spine_obj_part_transition_real_free_all(plugin_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
