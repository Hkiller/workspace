#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_touch_move_i.h"
#include "ui_sprite_touch_touchable_i.h"

ui_sprite_touch_move_t ui_sprite_touch_move_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_TOUCH_MOVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_touch_move_free(ui_sprite_touch_move_t move) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move);
    ui_sprite_fsm_action_free(fsm_action);
}

uint8_t ui_sprite_touch_move_is_capture(ui_sprite_touch_move_t move) {
    return move->m_is_capture;
}

int ui_sprite_touch_move_set_is_capture(ui_sprite_touch_move_t move, uint8_t is_capture) {
    move->m_is_capture = is_capture;
    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(move))) {
        return ui_sprite_touch_responser_set_is_capture(&move->m_responser, is_capture);
    }
    else {
        return 0;
    }
}

uint8_t ui_sprite_touch_move_is_grab(ui_sprite_touch_move_t move) {
    return move->m_is_grab;
}

int ui_sprite_touch_move_set_is_grab(ui_sprite_touch_move_t move, uint8_t is_grab) {
    move->m_is_grab = is_grab;
    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(move))) {
        return ui_sprite_touch_responser_set_is_grab(&move->m_responser, is_grab);
    }
    else {
        return 0;
    }
}

float ui_sprite_touch_move_z(ui_sprite_touch_move_t move) {
    return move->m_responser.m_z;
}

int ui_sprite_touch_move_set_z(ui_sprite_touch_move_t move, float z) {
    move->m_responser.m_z = z;
    return 0;
}

uint8_t ui_sprite_touch_move_finger_count(ui_sprite_touch_move_t move) {
    return move->m_finger_count;
}

int ui_sprite_touch_move_set_finger_count(ui_sprite_touch_move_t move, uint8_t finger_count) {
    move->m_finger_count = finger_count;
    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(move))) {
        return ui_sprite_touch_responser_set_finger_count(&move->m_responser, finger_count);
    }
    else {
        return 0;
    }
}

uint16_t ui_sprite_touch_move_threshold(ui_sprite_touch_move_t move) {
    return move->m_threshold;
}

void ui_sprite_touch_move_set_threshold(ui_sprite_touch_move_t move, uint16_t threshold) {
    move->m_threshold = threshold;

    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(move))) {
        move->m_responser.m_threshold = threshold;
    }
}

float ui_sprite_touch_move_stick_duration(ui_sprite_touch_move_t move) {
    return move->m_stick_duration;
}

void ui_sprite_touch_move_set_stick_duration(ui_sprite_touch_move_t move, float stick_duration) {
    move->m_stick_duration = stick_duration;
}

const char * ui_sprite_touch_move_on_begin(ui_sprite_touch_move_t move) {
    return move->m_on_begin;
}

int ui_sprite_touch_move_set_on_begin(ui_sprite_touch_move_t move, const char * on_begin) {
    ui_sprite_touch_mgr_t mgr = move->m_mgr;

    if (move->m_on_begin) mem_free(mgr->m_alloc, move->m_on_begin);

    if (on_begin) {
        move->m_on_begin = cpe_str_mem_dup(mgr->m_alloc, on_begin);
        return move->m_on_begin == NULL ? -1 : 0;
    }
    else {
        move->m_on_begin = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_move_on_move(ui_sprite_touch_move_t move) {
    return move->m_on_move;
}

int ui_sprite_touch_move_set_on_move(ui_sprite_touch_move_t move, const char * on_move) {
    ui_sprite_touch_mgr_t mgr = move->m_mgr;

    if (move->m_on_move) mem_free(mgr->m_alloc, move->m_on_move);

    if (on_move) {
        move->m_on_move = cpe_str_mem_dup(mgr->m_alloc, on_move);
        return move->m_on_move == NULL ? -1 : 0;
    }
    else {
        move->m_on_move = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_move_on_end(ui_sprite_touch_move_t move) {
    return move->m_on_end;
}

int ui_sprite_touch_move_set_on_end(ui_sprite_touch_move_t move, const char * on_end) {
    ui_sprite_touch_mgr_t mgr = move->m_mgr;

    if (move->m_on_end) mem_free(mgr->m_alloc, move->m_on_end);

    if (on_end) {
        move->m_on_end = cpe_str_mem_dup(mgr->m_alloc, on_end);
        return move->m_on_end == NULL ? -1 : 0;
    }
    else {
        move->m_on_end = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_move_on_cancel(ui_sprite_touch_move_t move) {
    return move->m_on_cancel;
}

int ui_sprite_touch_move_set_on_cancel(ui_sprite_touch_move_t move, const char * on_cancel) {
    ui_sprite_touch_mgr_t mgr = move->m_mgr;

    if (move->m_on_cancel) mem_free(mgr->m_alloc, move->m_on_cancel);

    if (on_cancel) {
        move->m_on_cancel = cpe_str_mem_dup(mgr->m_alloc, on_cancel);
        return move->m_on_cancel == NULL ? -1 : 0;
    }
    else {
        move->m_on_cancel = NULL;
        return 0;
    }
}

static void ui_sprite_touch_move_send_event(ui_sprite_touch_move_t move, const char * event) {
    ui_sprite_fsm_action_build_and_send_event(ui_sprite_fsm_action_from_data(move), event, NULL);
}

static void ui_sprite_touch_move_process_begin(void * ctx) {
    ui_sprite_touch_move_t move = ctx;
    ui_sprite_touch_mgr_t mgr = move->m_mgr;
    ui_sprite_touch_responser_binding_t binding;
    
    assert(move->m_responser.m_binding_count > 0);
    binding = TAILQ_FIRST(&move->m_responser.m_bindings);
    assert(binding);

    move->m_state.start_world_pos.x = binding->m_start_world_pt.x;
    move->m_state.start_world_pos.y = binding->m_start_world_pt.y;
    move->m_state.start_screen_pos.x = binding->m_start_screen_pt.x;
    move->m_state.start_screen_pos.y = binding->m_start_screen_pt.y;
    move->m_state.start_time_ms = ui_sprite_touch_mgr_cur_time(mgr);

    move->m_state.pre_world_pos.x = binding->m_pre_world_pt.x;
    move->m_state.pre_world_pos.y = binding->m_pre_world_pt.y;
    move->m_state.pre_screen_pos.x = binding->m_pre_screen_pt.x;
    move->m_state.pre_screen_pos.y = binding->m_pre_screen_pt.y;
    move->m_state.pre_time_ms = move->m_state.start_time_ms;

    move->m_state.cur_world_pos.x = binding->m_cur_world_pt.x;
    move->m_state.cur_world_pos.y = binding->m_cur_world_pt.y;
    move->m_state.cur_screen_pos.x = binding->m_cur_screen_pt.x;
    move->m_state.cur_screen_pos.y = binding->m_cur_screen_pt.y;
    move->m_state.cur_time_ms = move->m_state.start_time_ms;

    move->m_state.world_speed.x = 0.0f;
    move->m_state.world_speed.y = 0.0f;
    move->m_state.screen_speed.x = 0.0f;
    move->m_state.screen_speed.y = 0.0f;

    if (move->m_on_begin) {
        ui_sprite_touch_move_send_event(move, move->m_on_begin);
    }
}

static void ui_sprite_touch_move_process_move(void * ctx) {
    ui_sprite_touch_move_t move = ctx;
    ui_sprite_touch_mgr_t mgr = move->m_mgr;
    float delta_time;
    ui_sprite_touch_responser_binding_t binding;
    float stick_percent;

    assert(move->m_responser.m_binding_count > 0);
    binding = TAILQ_FIRST(&move->m_responser.m_bindings);
    assert(binding);

    move->m_state.pre_world_pos = move->m_state.cur_world_pos;
    move->m_state.pre_screen_pos = move->m_state.cur_screen_pos;
    move->m_state.pre_time_ms = move->m_state.cur_time_ms;

    move->m_state.cur_world_pos.x = binding->m_cur_world_pt.x;
    move->m_state.cur_world_pos.y = binding->m_cur_world_pt.y;
    move->m_state.cur_screen_pos.x = binding->m_cur_screen_pt.x;
    move->m_state.cur_screen_pos.y = binding->m_cur_screen_pt.y;
    move->m_state.cur_time_ms = ui_sprite_touch_mgr_cur_time(mgr);

    delta_time = ((float)(move->m_state.cur_time_ms - move->m_state.pre_time_ms)) / 1000.0f;

    if (delta_time > 0) {
        if (delta_time > move->m_stick_duration) {
            stick_percent = 0.0f;
        }
        else {
            stick_percent = delta_time / move->m_stick_duration;
        }

        move->m_state.world_speed.x = move->m_state.world_speed.x * stick_percent;
        move->m_state.world_speed.y = move->m_state.world_speed.y * stick_percent;
        move->m_state.screen_speed.x = move->m_state.screen_speed.x * stick_percent;
        move->m_state.screen_speed.y = move->m_state.screen_speed.y * stick_percent;

        move->m_state.world_speed.x += (move->m_state.cur_world_pos.x - move->m_state.pre_world_pos.x) / delta_time * (1.0f - stick_percent);
        move->m_state.world_speed.y += (move->m_state.cur_world_pos.y - move->m_state.pre_world_pos.y) / delta_time * (1.0f - stick_percent);
        move->m_state.screen_speed.x += (move->m_state.cur_screen_pos.x - move->m_state.pre_screen_pos.x) / delta_time * (1.0f - stick_percent);
        move->m_state.screen_speed.y += (move->m_state.cur_screen_pos.y - move->m_state.pre_screen_pos.y) / delta_time * (1.0f - stick_percent);
    }

    /* if (fabs(move->m_state.pre_world_pos.x - move->m_state.cur_world_pos.x) > 0.1 */
    /*     && fabs(move->m_state.pre_world_pos.y - move->m_state.cur_world_pos.y) > 0.1 */
    if (move->m_on_move) {
        ui_sprite_touch_move_send_event(move, move->m_on_move);
    }
}

static void ui_sprite_touch_move_process_end(void * ctx) {
    ui_sprite_touch_move_t move = ctx;

    if (move->m_on_end) {
        ui_sprite_touch_move_send_event(move, move->m_on_end);
    }
}

static void ui_sprite_touch_move_process_cancel(void * ctx) {
    ui_sprite_touch_move_t move = ctx;

    if (move->m_on_cancel) {
        ui_sprite_touch_move_send_event(move, move->m_on_cancel);
    }
}

static int ui_sprite_touch_move_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_touch_touchable_t touchable = ui_sprite_touch_touchable_find(entity);

    if (touchable == NULL) {
        CPE_ERROR(
            mgr->m_em, "entity %d(%s): create %s: touchable not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), UI_SPRITE_TOUCH_MOVE_NAME);
        return -1;
    }

    if (ui_sprite_touch_responser_init(&move->m_responser, entity, touchable) != 0) {
        return -1;
    }

    move->m_responser.m_finger_count = move->m_finger_count;
    move->m_responser.m_is_capture = move->m_is_capture;
    move->m_responser.m_is_grab = move->m_is_grab;
    move->m_responser.m_threshold = move->m_threshold;
    move->m_responser.m_on_begin = ui_sprite_touch_move_process_begin;
    move->m_responser.m_on_move = ui_sprite_touch_move_process_move;
    move->m_responser.m_on_end = ui_sprite_touch_move_process_end;
    move->m_responser.m_on_cancel = ui_sprite_touch_move_process_cancel;
    
    return ui_sprite_touch_responser_enter(&move->m_responser);
}

static void ui_sprite_touch_move_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_move_t move = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_touch_responser_exit(&move->m_responser);

    ui_sprite_touch_responser_fini(&move->m_responser);
}

static int ui_sprite_touch_move_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_move_t move = ui_sprite_fsm_action_data(fsm_action);

    bzero(move, sizeof(struct ui_sprite_touch_move));

    move->m_mgr = mgr;
    move->m_finger_count = 1;
    move->m_stick_duration = 1.0f;
    move->m_threshold = mgr->m_dft_threshold;

    return 0;
}

static void ui_sprite_touch_move_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_move_t move = ui_sprite_fsm_action_data(fsm_action);

    if (move->m_on_begin) mem_free(mgr->m_alloc, move->m_on_begin);
    if (move->m_on_move) mem_free(mgr->m_alloc, move->m_on_move);
    if (move->m_on_end) mem_free(mgr->m_alloc, move->m_on_end);
    if (move->m_on_cancel) mem_free(mgr->m_alloc, move->m_on_cancel);
}

static int ui_sprite_touch_move_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_move_t to_move = ui_sprite_fsm_action_data(to);
    ui_sprite_touch_move_t from_move = ui_sprite_fsm_action_data(from);

    to_move->m_mgr = mgr;
    to_move->m_finger_count = from_move->m_finger_count;
    to_move->m_is_capture = from_move->m_is_capture;
    to_move->m_is_grab = from_move->m_is_grab;
    to_move->m_threshold = from_move->m_threshold;
    to_move->m_on_begin = from_move->m_on_begin ? cpe_str_mem_dup(mgr->m_alloc, from_move->m_on_begin) : NULL;
    to_move->m_on_move = from_move->m_on_move ? cpe_str_mem_dup(mgr->m_alloc, from_move->m_on_move) : NULL;
    to_move->m_on_end = from_move->m_on_end ? cpe_str_mem_dup(mgr->m_alloc, from_move->m_on_end) : NULL;
    to_move->m_on_cancel = from_move->m_on_cancel ? cpe_str_mem_dup(mgr->m_alloc, from_move->m_on_cancel) : NULL;

    return 0;
}

int ui_sprite_touch_move_regist(ui_sprite_touch_mgr_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm, UI_SPRITE_TOUCH_MOVE_NAME, sizeof(struct ui_sprite_touch_move));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch move register: meta create fail",
            ui_sprite_touch_mgr_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_data_meta(
        meta,
        module->m_meta_move_state,
        CPE_ENTRY_START(ui_sprite_touch_move, m_state),
        CPE_ENTRY_SIZE(ui_sprite_touch_move, m_state));

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_touch_move_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_touch_move_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_touch_move_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_touch_move_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_touch_move_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_TOUCH_MOVE_NAME, ui_sprite_touch_move_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_touch_move_unregist(ui_sprite_touch_mgr_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm, UI_SPRITE_TOUCH_MOVE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch move unregister: meta not exist",
            ui_sprite_touch_mgr_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_TOUCH_MOVE_NAME);
    }
}

const char * UI_SPRITE_TOUCH_MOVE_NAME = "touch-move";
