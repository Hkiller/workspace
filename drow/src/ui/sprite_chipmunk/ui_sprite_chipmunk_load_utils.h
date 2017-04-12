#ifndef UI_SPRITE_CHIPMUNK_LOAD_UTILS_H
#define UI_SPRITE_CHIPMUNK_LOAD_UTILS_H
#include "chipmunk/chipmunk_private.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ui_sprite_chipmunk_build_layer_masks(ui_sprite_chipmunk_env_t env, cfg_t cfg);
int ui_sprite_chipmunk_get_obj_type(ui_sprite_chipmunk_env_t env, chipmunk_obj_type_t * obj_type, const char * str_obj_type);
int ui_sprite_chipmunk_get_runing_mode(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_runing_mode_t * runing_mode, const char * str_runing_mode);

int ui_sprite_chipmunk_load_fixture_data(ui_sprite_chipmunk_env_t env, CHIPMUNK_FIXTURE * fixture_data, cfg_t shape_cfg);
int ui_sprite_chipmunk_load_fixture_shape(ui_sprite_chipmunk_env_t env, CHIPMUNK_FIXTURE * fixture_data, cfg_t shape_cfg);
int ui_sprite_chipmunk_load_fixture_shape_from_str(ui_sprite_chipmunk_module_t module, CHIPMUNK_FIXTURE * fixture_data, const char * shape_def);
int ui_sprite_chipmunk_load_shape_from_str(ui_sprite_chipmunk_module_t module, CHIPMUNK_SHAPE * shape_data, const char * shape_def);

typedef struct ui_sprite_chipmunk_body_attrs {
    ui_sprite_chipmunk_runing_mode_t m_runing_mode;
    UI_SPRITE_CHIPMUNK_GRAVITY m_gravity;
    ui_vector_2 m_position;
    float m_angle;
    uint8_t m_is_main;
    uint8_t m_is_free;
    uint32_t m_category;
    uint32_t m_mask;
    uint32_t m_group;
    float m_mass;
    float m_moment;
    chipmunk_obj_type_t m_type;
    uint32_t m_attr_flags;
} * ui_sprite_chipmunk_body_attrs_t;

int ui_sprite_chipmunk_load_body_attrs(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_body_attrs_t body_attrs, cfg_t body_cfg);
int ui_sprite_chipmunk_load_gravity(ui_sprite_chipmunk_env_t env, UI_SPRITE_CHIPMUNK_GRAVITY * gravity, cfg_t body_cfg);
    
#ifdef __cplusplus
}
#endif

#endif
