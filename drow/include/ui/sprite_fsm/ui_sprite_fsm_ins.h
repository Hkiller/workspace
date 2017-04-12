#ifndef UI_SPRITE_FSM_INS_H
#define UI_SPRITE_FSM_INS_H
#include "cpe/utils/buffer.h"
#include "ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*ins operations*/
ui_sprite_component_t ui_sprite_fsm_to_component(ui_sprite_fsm_ins_t fsm);
ui_sprite_entity_t ui_sprite_fsm_to_entity(ui_sprite_fsm_ins_t fsm);
ui_sprite_world_t ui_sprite_fsm_to_world(ui_sprite_fsm_ins_t fsm);

ui_sprite_fsm_ins_t ui_sprite_fsm_parent(ui_sprite_fsm_ins_t fsm);

int ui_sprite_fsm_set_default_state(ui_sprite_fsm_ins_t fsm, const char * state_name);
ui_sprite_fsm_state_t ui_sprite_fsm_default_state(ui_sprite_fsm_ins_t fsm);
int ui_sprite_fsm_set_default_call_state(ui_sprite_fsm_ins_t fsm, const char * state_name);
ui_sprite_fsm_state_t ui_sprite_fsm_default_call_state(ui_sprite_fsm_ins_t fsm);
ui_sprite_fsm_state_t ui_sprite_fsm_current_state(ui_sprite_fsm_ins_t fsm);

int ui_sprite_fsm_is_in_state(ui_sprite_fsm_ins_t fsm, const char * state_path);
int ui_sprite_fsm_have_state(ui_sprite_fsm_ins_t fsm, const char * state_path);
const char * ui_sprite_fsm_dump_path(ui_sprite_fsm_ins_t fsm, mem_buffer_t buffer);

int ui_sprite_fsm_ins_copy(ui_sprite_fsm_ins_t to_fsm, ui_sprite_fsm_ins_t from_fsm);
void ui_sprite_fsm_ins_states(ui_sprite_fsm_state_it_t it, ui_sprite_fsm_ins_t fsm);

void ui_sprite_fsm_ins_visit_actions(
    ui_sprite_fsm_ins_t fsm,
    void (*visitor)(void * ctx, ui_sprite_fsm_action_t action), void * ctx);

/*state operations*/
struct ui_sprite_fsm_state_it {
    ui_sprite_fsm_state_t (*next)(struct ui_sprite_fsm_state_it * it);
    char m_data[64];
};
#define ui_sprite_fsm_state_it_next(it) ((it)->next ? (it)->next(it) : NULL)

ui_sprite_fsm_state_t ui_sprite_fsm_state_create(ui_sprite_fsm_ins_t fsm, const char * name);
ui_sprite_fsm_state_t ui_sprite_fsm_state_clone(ui_sprite_fsm_ins_t fsm, ui_sprite_fsm_state_t from);
void ui_sprite_fsm_state_free(ui_sprite_fsm_state_t fsm_state);

ui_sprite_fsm_state_t ui_sprite_fsm_state_find_by_name(ui_sprite_fsm_ins_t fsm, const char * name);
ui_sprite_fsm_state_t ui_sprite_fsm_state_find_by_id(ui_sprite_fsm_ins_t fsm, uint16_t id);

ui_sprite_fsm_ins_t ui_sprite_fsm_state_fsm(ui_sprite_fsm_state_t fsm_state);
uint16_t ui_sprite_fsm_state_id(ui_sprite_fsm_state_t fsm_state);
const char * ui_sprite_fsm_state_name(ui_sprite_fsm_state_t fsm_state);
ui_sprite_event_t ui_sprite_fsm_state_enter_event(ui_sprite_fsm_state_t fsm_state);
int ui_sprite_fsm_state_set_enter_event(ui_sprite_fsm_state_t fsm_state, ui_sprite_event_t evt);
ui_sprite_event_t ui_sprite_fsm_state_local_enter_event(ui_sprite_fsm_state_t fsm_state);
void ui_sprite_fsm_state_actions(ui_sprite_fsm_action_it_t it, ui_sprite_fsm_state_t fsm_state);
ui_sprite_fsm_state_t ui_sprite_fsm_state_return_to(ui_sprite_fsm_state_t fsm_state);
uint16_t ui_sprite_fsm_state_count_left_actions(ui_sprite_fsm_state_t fsm_state);

void ui_sprite_fsm_state_append_addition_source(
    ui_sprite_fsm_state_t fsm_state,
    dr_data_source_t * data_source,
    struct ui_sprite_fsm_addition_source_ctx * ctx);
    
/*action operations*/
struct ui_sprite_fsm_action_it {
    ui_sprite_fsm_action_t (*next)(struct ui_sprite_fsm_action_it * it);
    char m_data[64];
};
#define ui_sprite_fsm_action_it_next(it) ((it)->next ? (it)->next(it) : NULL)

ui_sprite_fsm_action_t ui_sprite_fsm_action_create(ui_sprite_fsm_state_t fsm_state, const char * name, const char * type_name);
ui_sprite_fsm_action_t ui_sprite_fsm_action_clone(ui_sprite_fsm_state_t fsm_state, ui_sprite_fsm_action_t from);
void ui_sprite_fsm_action_free(ui_sprite_fsm_action_t fsm_action);

int ui_sprite_fsm_action_enter(ui_sprite_fsm_action_t fsm_action);

ui_sprite_fsm_action_t ui_sprite_fsm_action_find_by_name(ui_sprite_fsm_state_t fsm_state, const char * name);

ui_sprite_component_t ui_sprite_fsm_action_to_component(ui_sprite_fsm_action_t fsm_action);
ui_sprite_entity_t ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_t fsm_action);
ui_sprite_world_t ui_sprite_fsm_action_to_world(ui_sprite_fsm_action_t fsm_action);
ui_sprite_fsm_action_meta_t ui_sprite_fsm_action_meta(ui_sprite_fsm_action_t fsm_action);

ui_sprite_fsm_action_t ui_sprite_fsm_action_find(ui_sprite_fsm_state_t fsm_state, const char * name);

int ui_sprite_fsm_action_set_follow_to(ui_sprite_fsm_action_t fsm_action, const char * follow_to);

ui_sprite_fsm_action_state_t ui_sprite_fsm_action_runing_state(ui_sprite_fsm_action_t fsm_action);

ui_sprite_fsm_state_t ui_sprite_fsm_action_state(ui_sprite_fsm_action_t fsm_action);
const char * ui_sprite_fsm_action_name(ui_sprite_fsm_action_t fsm_action);
const char * ui_sprite_fsm_action_type_name(ui_sprite_fsm_action_t fsm_action);

uint8_t ui_sprite_fsm_action_apply_enter_evt(ui_sprite_fsm_action_t fsm_action);
void ui_sprite_fsm_action_set_apply_enter_evt(ui_sprite_fsm_action_t fsm_action, uint8_t apply_enter_evt);

const char * ui_sprite_fsm_action_work(ui_sprite_fsm_action_t fsm_action);
int ui_sprite_fsm_action_set_work(ui_sprite_fsm_action_t fsm_action, const char * work);

uint8_t ui_sprite_fsm_action_is_update(ui_sprite_fsm_action_t fsm_action);
int ui_sprite_fsm_action_start_update(ui_sprite_fsm_action_t fsm_action);
void ui_sprite_fsm_action_stop_update(ui_sprite_fsm_action_t fsm_action);
void ui_sprite_fsm_action_sync_update(ui_sprite_fsm_action_t fsm_action, uint8_t is_start);

void ui_sprite_fsm_action_send_event(
    ui_sprite_fsm_action_t fsm_action,
    LPDRMETA meta, void const * data, size_t size);

void ui_sprite_fsm_action_send_event_to(
    ui_sprite_fsm_action_t fsm_action, const char * target,
    LPDRMETA meta, void const * data, size_t size);

ui_sprite_event_t ui_sprite_fsm_action_build_event(
    ui_sprite_fsm_action_t fsm_action,
    mem_allocrator_t alloc, const char * def, dr_data_source_t data_source);
    
void ui_sprite_fsm_action_build_and_send_event(
    ui_sprite_fsm_action_t fsm_action,
    const char * event_def, dr_data_source_t data_source);

int ui_sprite_fsm_action_add_event_handler(
    ui_sprite_fsm_action_t fsm_action, ui_sprite_event_scope_t scope,
    const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx);

int ui_sprite_fsm_action_add_attr_monitor(
    ui_sprite_fsm_action_t fsm_action,
    const char * attrs, ui_sprite_attr_monitor_fun_t fun, void * ctx);

int ui_sprite_fsm_action_add_attr_monitor_by_def(
    ui_sprite_fsm_action_t fsm_action,
    const char * def, ui_sprite_attr_monitor_fun_t fun, void * ctx);

int ui_sprite_fsm_action_add_attr_monitor_by_defs(
    ui_sprite_fsm_action_t fsm_action,
    const char * * defs, uint16_t def_count, ui_sprite_attr_monitor_fun_t fun, void * ctx);
    
int ui_sprite_fsm_action_set_attr(
    ui_sprite_fsm_action_t fsm_action,
    const char * path, const char * value, dr_data_source_t data_source);

int ui_sprite_fsm_action_bulk_set_attrs(
    ui_sprite_fsm_action_t fsm_action, const char * def, dr_data_source_t data_source);

uint8_t ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_t fsm_action);

ui_sprite_fsm_action_life_circle_t ui_sprite_fsm_action_life_circle(ui_sprite_fsm_action_t fsm_action);
int ui_sprite_fsm_action_set_life_circle(ui_sprite_fsm_action_t fsm_action, ui_sprite_fsm_action_life_circle_t  life_circle);

float ui_sprite_fsm_action_duration(ui_sprite_fsm_action_t fsm_action);
float ui_sprite_fsm_action_runing_time(ui_sprite_fsm_action_t fsm_action);
int ui_sprite_fsm_action_set_duration(ui_sprite_fsm_action_t fsm_action, float duration);
int ui_sprite_fsm_action_set_duration_calc(ui_sprite_fsm_action_t fsm_action, const char * duration_calc);
int ui_sprite_fsm_action_set_condition(ui_sprite_fsm_action_t fsm_action, const char * condition);

void * ui_sprite_fsm_action_data(ui_sprite_fsm_action_t fsm_action);
size_t ui_sprite_fsm_action_data_size(ui_sprite_fsm_action_t fsm_action);
ui_sprite_fsm_action_t ui_sprite_fsm_action_from_data(void * data);

void ui_sprite_fsm_action_append_addition_source(
    ui_sprite_fsm_action_t fsm_action, 
    dr_data_source_t * data_source,
    struct ui_sprite_fsm_addition_source_ctx * ctx);
    
/*convertor*/
ui_sprite_fsm_convertor_t
ui_sprite_fsm_convertor_create(
    ui_sprite_fsm_action_t action,
    const char * event, const char * condition, const char * convert_to);

ui_sprite_fsm_convertor_t
ui_sprite_fsm_convertor_clone(
    ui_sprite_fsm_action_t action, ui_sprite_fsm_convertor_t from);

void ui_sprite_fsm_convertor_free(ui_sprite_fsm_convertor_t convertor);


/*transaction*/
ui_sprite_fsm_transition_t
ui_sprite_fsm_transition_create(
    ui_sprite_fsm_state_t state,
    const char * event, const char * to_state, const char * call_state, const char * condition);

ui_sprite_fsm_transition_t
ui_sprite_fsm_transition_clone(
    ui_sprite_fsm_state_t fsm_state, ui_sprite_fsm_transition_t from);

void ui_sprite_fsm_transition_free(ui_sprite_fsm_transition_t transition);

/*proto operations*/
ui_sprite_fsm_ins_t ui_sprite_fsm_proto_create(ui_sprite_world_t world, const char * name);
void ui_sprite_fsm_proto_free(ui_sprite_fsm_ins_t proto_fsm);
ui_sprite_fsm_ins_t ui_sprite_fsm_proto_find(ui_sprite_world_t world, const char * name);

#ifdef __cplusplus
}
#endif

#endif
