#ifndef UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_I_H
#define UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin/chipmunk/plugin_chipmunk_env_updator.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_attractor.h"
#include "ui_sprite_chipmunk_module_i.h"
#include "ui_sprite_chipmunk_obj_body_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_chipmunk_with_attractor_type {
    ui_sprite_chipmunk_with_attractor_unknown,
    ui_sprite_chipmunk_with_attractor_gravitation,
    ui_sprite_chipmunk_with_attractor_spring,    
    ui_sprite_chipmunk_with_attractor_force,
};
    
struct ui_sprite_chipmunk_with_attractor {
    ui_sprite_chipmunk_module_t m_module;
    enum ui_sprite_chipmunk_with_attractor_type m_type;

    uint8_t m_cfg_cache;
    char * m_cfg_shape;
    char * m_cfg_category;
    char * m_cfg_mask;
    char * m_cfg_group;
    char * m_cfg_damping;
    char * m_cfg_obj_group;
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
            char * m_cfg_force;
        } m_cfg_force;            
    };

    CHIPMUNK_SHAPE m_query_shape;
    uint32_t m_category;
    uint32_t m_mask;
    uint32_t m_group;
    
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
            float m_force;
        } m_force;
    };
    struct ui_sprite_chipmunk_obj_body_group m_body_group;
    float m_step_duration;
    float m_damping;
    float m_self_mass;
    ui_sprite_group_t m_obj_group;
    ui_vector_2 m_self_pos;
    plugin_chipmunk_env_t m_env;
    plugin_chipmunk_env_updator_t m_updator;
};

int ui_sprite_chipmunk_with_attractor_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_attractor_unregist(ui_sprite_chipmunk_module_t module);

/*gravitation*/
int ui_sprite_chipmunk_with_attractor_gravitation_load(ui_sprite_chipmunk_with_attractor_t with_attractor, cfg_t cfg);
int ui_sprite_chipmunk_with_attractor_gravitation_copy(
    ui_sprite_chipmunk_with_attractor_t to_with_attractor, ui_sprite_chipmunk_with_attractor_t from_with_attractor);
void ui_sprite_chipmunk_with_attractor_gravitation_free(ui_sprite_chipmunk_with_attractor_t with_attractor);
int ui_sprite_chipmunk_with_attractor_gravitation_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_with_attractor_t with_attractor,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm);
cpVect ui_sprite_chipmunk_with_attractor_gravitation_work(ui_sprite_chipmunk_with_attractor_t with_attractor, ui_sprite_chipmunk_obj_body_t body);
    

/*spring*/
int ui_sprite_chipmunk_with_attractor_spring_load(ui_sprite_chipmunk_with_attractor_t with_attractor, cfg_t cfg);
int ui_sprite_chipmunk_with_attractor_spring_copy(
    ui_sprite_chipmunk_with_attractor_t to_with_attractor, ui_sprite_chipmunk_with_attractor_t from_with_attractor);
void ui_sprite_chipmunk_with_attractor_spring_free(ui_sprite_chipmunk_with_attractor_t with_attractor);
int ui_sprite_chipmunk_with_attractor_spring_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_with_attractor_t with_attractor,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm);
cpVect ui_sprite_chipmunk_with_attractor_spring_work(ui_sprite_chipmunk_with_attractor_t with_attractor, ui_sprite_chipmunk_obj_body_t body);

#ifdef __cplusplus
}
#endif

#endif
