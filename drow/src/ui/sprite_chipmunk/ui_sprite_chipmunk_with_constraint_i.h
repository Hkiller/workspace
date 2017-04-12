#ifndef UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_I_H
#define UI_SPRITE_CHIPMUNK_WITH_CONSTRAINT_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_constraint.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_with_constraint_info * ui_sprite_chipmunk_with_constraint_info_t;

struct ui_sprite_chipmunk_with_constraint_info {
    char * m_install_to;
    uint8_t m_constraint_type;
    TAILQ_ENTRY(ui_sprite_chipmunk_with_constraint_info) m_next;
    union {
        struct {
            char * m_rate;
        } m_simple_motor;
        struct {
            char * m_min;
            char * m_max;
        } m_rotary_limit;
    };
};

struct ui_sprite_chipmunk_with_constraint {
    ui_sprite_chipmunk_module_t m_module;
    TAILQ_HEAD(ui_sprite_chipmunk_with_constraint_info_list, ui_sprite_chipmunk_with_constraint_info) m_constraints;
};

int ui_sprite_chipmunk_with_constraint_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_constraint_unregist(ui_sprite_chipmunk_module_t module);

ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_constraint_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);
    
ui_sprite_chipmunk_with_constraint_info_t
ui_sprite_chipmunk_with_constraint_info_create(
    ui_sprite_chipmunk_with_constraint_t with_constraint, const char * install_to, uint8_t constraint_type);
ui_sprite_chipmunk_with_constraint_info_t
ui_sprite_chipmunk_with_constraint_info_copy(
    ui_sprite_chipmunk_with_constraint_t with_constraint, ui_sprite_chipmunk_with_constraint_info_t from);
void ui_sprite_chipmunk_with_constraint_info_free(
    ui_sprite_chipmunk_with_constraint_t with_constraint, ui_sprite_chipmunk_with_constraint_info_t constraint_info);
    
#ifdef __cplusplus
}
#endif

#endif
