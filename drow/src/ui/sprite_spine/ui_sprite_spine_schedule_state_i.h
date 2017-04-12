#ifndef UI_SPRITE_SPINE_SCHEDULE_STATE_I_H
#define UI_SPRITE_SPINE_SCHEDULE_STATE_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_anim.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "ui/sprite_spine/ui_sprite_spine_schedule_state.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_spine_schedule_state_node * ui_sprite_spine_schedule_state_node_t;
typedef TAILQ_HEAD(ui_sprite_spine_schedule_state_node_list, ui_sprite_spine_schedule_state_node) ui_sprite_spine_schedule_state_node_list_t;
    
struct ui_sprite_spine_schedule_state_node {
    TAILQ_ENTRY(ui_sprite_spine_schedule_state_node) m_next;
    char * m_cfg_state;
    char * m_cfg_loop_count;

    char * m_state;
    uint16_t m_loop_count;
    uint16_t m_runing_count;
    uint8_t m_started;
};

struct ui_sprite_spine_schedule_state {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_part;
    char * m_cfg_loop_count;
    ui_sprite_spine_schedule_state_node_list_t m_nodes;

    char * m_part;
    uint16_t m_loop_count;
    uint16_t m_runing_count;
    ui_sprite_spine_schedule_state_node_t m_cur_node;
};

int ui_sprite_spine_schedule_state_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_schedule_state_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
