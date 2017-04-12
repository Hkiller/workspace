#ifndef UI_SPRITE_2D_MOVE_TO_FOLLOW_I_H
#define UI_SPRITE_2D_MOVE_TO_FOLLOW_I_H
#include "render/utils/ui_vector_2.h"
#include "ui/sprite_2d/ui_sprite_2d_move.h"
#include "ui_sprite_2d_module_i.h"
#include "render/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_move {
    ui_sprite_2d_module_t m_module;

    /*config*/
    uint8_t m_cfg_is_to_entity;
    union {
        struct {
            char * m_cfg_target_entity;
            uint8_t m_cfg_process_x;
            uint8_t m_cfg_process_y;
            uint8_t m_cfg_pos_policy;
        } m_to_entity;
        struct {
            char * m_cfg_x;
            char * m_cfg_y;
        } m_to_pos;
    };
    char * m_cfg_max_speed;
    char * m_cfg_take_time;
    struct ui_percent_decorator m_cfg_decorator;

    /*runtime*/
    uint8_t m_process_x;
    uint8_t m_process_y;
    float m_max_speed;
    float m_take_time;
    float m_runing_time;
    ui_vector_2 m_begin_pos;
    
    ui_vector_2 m_cur_pos;
    ui_vector_2 m_target_pos;
    float m_to_target_moved_distance;
};

int ui_sprite_2d_move_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_move_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
