#ifndef UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_I_H
#define UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_move_to_entity.h"
#include "plugin/chipmunk/plugin_chipmunk_env_updator.h"
#include "ui_sprite_chipmunk_module_i.h"
#include "ui_sprite_chipmunk_obj_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_chipmunk_move_to_entity_type {
    ui_sprite_chipmunk_move_to_entity_unknown,
    ui_sprite_chipmunk_move_to_entity_gravitation,
    ui_sprite_chipmunk_move_to_entity_spring,    
    ui_sprite_chipmunk_move_to_entity_move,
};
    
struct ui_sprite_chipmunk_move_to_entity {
    ui_sprite_chipmunk_module_t m_module;
    enum ui_sprite_chipmunk_move_to_entity_type m_type;

    char * m_cfg_target_entity;
    char * m_cfg_damping;
    union {
        struct {
            char * m_cfg_G;    
            char * m_cfg_max_distance;
            char * m_cfg_min_distance;
        } m_cfg_gravitation;
        struct {
            char * m_cfg_K;
            char * m_cfg_max_distance;
            char * m_cfg_base_distance;
        } m_cfg_spring;            
        struct {
            char * m_cfg_max_speed;
            char * m_cfg_max_accel;
            uint8_t m_cfg_force;
            uint8_t m_cfg_stop;
        } m_cfg_move;
    };

    union {
        struct {
            float m_min_distance;
            float m_max_distance;
            float m_G;
        } m_gravitation;
        struct {
            float m_base_distance;
            float m_max_distance;
            float m_K;
        } m_spring;
        struct {
            float m_max_speed;
            float m_max_accel;
            uint8_t m_force;
            uint8_t m_stop;
        } m_move;
    };
    float m_damping;
    uint32_t m_target_entity;
    float m_step_duration;
    plugin_chipmunk_env_t m_env;
    plugin_chipmunk_env_updator_t m_updator;
};

int ui_sprite_chipmunk_move_to_entity_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_move_to_entity_unregist(ui_sprite_chipmunk_module_t module);

int ui_sprite_chipmunk_move_to_entity_gravitation_load(ui_sprite_chipmunk_move_to_entity_t move_to_entity, cfg_t cfg);
void ui_sprite_chipmunk_move_to_entity_gravitation_free(ui_sprite_chipmunk_move_to_entity_t move_to_entity);
int ui_sprite_chipmunk_move_to_entity_gravitation_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm);
int ui_sprite_chipmunk_move_to_entity_gravitation_copy(
    ui_sprite_chipmunk_move_to_entity_t to_move_to_entity, ui_sprite_chipmunk_move_to_entity_t from_move_to_entity);
cpVect ui_sprite_chipmunk_move_to_entity_gravitation_work(
    ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t target_entity, ui_sprite_chipmunk_obj_body_t target_body, ui_vector_2_t target_pos,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body, ui_vector_2_t self_pos);

int ui_sprite_chipmunk_move_to_entity_spring_load(ui_sprite_chipmunk_move_to_entity_t move_to_entity, cfg_t cfg);
void ui_sprite_chipmunk_move_to_entity_spring_free(ui_sprite_chipmunk_move_to_entity_t move_to_entity);
int ui_sprite_chipmunk_move_to_entity_spring_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm);
int ui_sprite_chipmunk_move_to_entity_spring_copy(
    ui_sprite_chipmunk_move_to_entity_t to_move_to_entity, ui_sprite_chipmunk_move_to_entity_t from_move_to_entity);
cpVect ui_sprite_chipmunk_move_to_entity_spring_work(
    ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t target_entity, ui_sprite_chipmunk_obj_body_t target_body, ui_vector_2_t target_pos,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body, ui_vector_2_t self_pos);

int ui_sprite_chipmunk_move_to_entity_move_load(ui_sprite_chipmunk_move_to_entity_t move_to_entity, cfg_t cfg);
void ui_sprite_chipmunk_move_to_entity_move_free(ui_sprite_chipmunk_move_to_entity_t move_to_entity);
int ui_sprite_chipmunk_move_to_entity_move_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm);
int ui_sprite_chipmunk_move_to_entity_move_copy(
    ui_sprite_chipmunk_move_to_entity_t to_move_to_entity, ui_sprite_chipmunk_move_to_entity_t from_move_to_entity);
cpVect ui_sprite_chipmunk_move_to_entity_move_work(
    ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t target_entity, ui_sprite_chipmunk_obj_body_t target_body, ui_vector_2_t target_pos,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body, ui_vector_2_t self_pos);

#ifdef __cplusplus
}
#endif

#endif
