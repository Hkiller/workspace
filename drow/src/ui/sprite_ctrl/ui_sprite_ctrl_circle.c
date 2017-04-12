#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_ctrl_circle_i.h"

ui_sprite_ctrl_circle_t ui_sprite_ctrl_circle_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CTRL_CIRCLE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ctrl_circle_free(ui_sprite_ctrl_circle_t ctrl) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ctrl);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_ctrl_circle_sync_update(ui_sprite_ctrl_circle_t ctrl) {
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctrl);

    if (ctrl->m_is_working && ctrl->m_keep_send_span > 0.0f && ctrl->m_on_move) {
        if (!ui_sprite_fsm_action_is_update(action)) {
            ui_sprite_fsm_action_start_update(action);
        }
    }
    else {
        if (ui_sprite_fsm_action_is_update(action)) {
            ui_sprite_fsm_action_stop_update(action);
        }
    }
}


const char * ui_sprite_ctrl_circle_on_begin(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_on_begin;
}

int ui_sprite_ctrl_circle_set_on_begin(ui_sprite_ctrl_circle_t ctrl, const char * on_begin) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;

    if (ctrl->m_on_begin) mem_free(module->m_alloc, ctrl->m_on_begin);

    if (on_begin) {
        ctrl->m_on_begin = cpe_str_mem_dup(module->m_alloc, on_begin);
        return ctrl->m_on_begin == NULL ? -1 : 0;
    }
    else {
        ctrl->m_on_begin = NULL;
        return 0;
    }
}

const char * ui_sprite_ctrl_circle_on_move(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_on_move;
}

int ui_sprite_ctrl_circle_set_on_move(ui_sprite_ctrl_circle_t ctrl, const char * on_move) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;
    int r;

    if (ctrl->m_on_move) mem_free(module->m_alloc, ctrl->m_on_move);

    if (on_move) {
        ctrl->m_on_move = cpe_str_mem_dup(module->m_alloc, on_move);
        r = NULL ? -1 : 0;
    }
    else {
        ctrl->m_on_move = NULL;
        r = 0;
    }

    ui_sprite_ctrl_circle_sync_update(ctrl);

    return r;
}

const char * ui_sprite_ctrl_circle_on_done(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_on_done;
}

int ui_sprite_ctrl_circle_set_on_done(ui_sprite_ctrl_circle_t ctrl, const char * on_done) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;

    if (ctrl->m_on_done) mem_free(module->m_alloc, ctrl->m_on_done);

    if (on_done) {
        ctrl->m_on_done = cpe_str_mem_dup(module->m_alloc, on_done);
        return ctrl->m_on_done == NULL ? -1 : 0;
    }
    else {
        ctrl->m_on_done = NULL;
        return 0;
    }
}

const char * ui_sprite_ctrl_circle_on_cancel(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_on_cancel;
}

int ui_sprite_ctrl_circle_set_on_cancel(ui_sprite_ctrl_circle_t ctrl, const char * on_cancel) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;

    if (ctrl->m_on_cancel) mem_free(module->m_alloc, ctrl->m_on_cancel);

    if (on_cancel) {
        ctrl->m_on_cancel = cpe_str_mem_dup(module->m_alloc, on_cancel);
        return ctrl->m_on_cancel == NULL ? -1 : 0;
    }
    else {
        ctrl->m_on_cancel = NULL;
        return 0;
    }
}

float ui_sprite_ctrl_circle_keep_send_span(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_keep_send_span;
}

void ui_sprite_ctrl_circle_set_keep_send_span(ui_sprite_ctrl_circle_t ctrl, float span) {
    ctrl->m_keep_send_span = span;

    if (ui_sprite_fsm_action_is_active(ui_sprite_fsm_action_from_data(ctrl))) {
        ui_sprite_ctrl_circle_sync_update(ctrl);
    }
}

uint8_t ui_sprite_ctrl_circle_negative(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_negative;
}

void ui_sprite_ctrl_circle_set_negative(ui_sprite_ctrl_circle_t ctrl, uint8_t negative) {
    ctrl->m_negative = negative;
}

uint8_t ui_sprite_ctrl_circle_do_scale(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_do_scale;
}

void ui_sprite_ctrl_circle_set_do_scale(ui_sprite_ctrl_circle_t ctrl, uint8_t do_scale) {
    ctrl->m_do_scale = do_scale;
}

uint8_t ui_sprite_ctrl_circle_do_rotate(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_do_rotate;
}

void ui_sprite_ctrl_circle_set_do_rotate(ui_sprite_ctrl_circle_t ctrl, uint8_t do_rotate) {
    ctrl->m_do_rotate = do_rotate;
}

float ui_sprite_ctrl_circle_screen_min(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_screen_min;
}

float ui_sprite_ctrl_circle_screen_max(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_screen_max;
}

int ui_sprite_ctrl_circle_set_screen_range(ui_sprite_ctrl_circle_t ctrl, float _min, float _max) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;

    if (_min >= _max || _min < 0.0f) {
        CPE_ERROR(module->m_em, "ctrl-circle: screen range [%f ~ %f] error!", _min, _max);
        return -1;
    }

    ctrl->m_screen_min = _min;
    ctrl->m_screen_max = _max;

    return 0;
}

float ui_sprite_ctrl_circle_logic_min(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_logic_min;
}

float ui_sprite_ctrl_circle_logic_max(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_logic_max;
}

int ui_sprite_ctrl_circle_set_logic_range(ui_sprite_ctrl_circle_t ctrl, float _min, float _max) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;

    if (_min >= _max || _min < 0.0f) {
        CPE_ERROR(module->m_em, "ctrl-circle: logic range [%f ~ %f] error!", _min, _max);
        return -1;
    }

    ctrl->m_logic_min = _min;
    ctrl->m_logic_max = _max;

    return 0;
}

float ui_sprite_ctrl_circle_angle_min(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_angle_min;
}

float ui_sprite_ctrl_circle_angle_max(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_angle_max;
}

int ui_sprite_ctrl_circle_set_angle_range(ui_sprite_ctrl_circle_t ctrl, float _min, float _max) {
    ui_sprite_ctrl_module_t module = ctrl->m_module;

    if (_min >= _max || _min < -180.0f || _max > 180.0f) {
        CPE_ERROR(module->m_em, "ctrl-circle: logic range [%f ~ %f] error!", _min, _max);
        return -1;
    }

    ctrl->m_angle_min = _min;
    ctrl->m_angle_max = _max;

    return 0;
}

float ui_sprite_ctrl_circle_logic_base(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_logic_base;
}

void ui_sprite_ctrl_circle_set_logic_base(ui_sprite_ctrl_circle_t ctrl, float base_value) {
    ctrl->m_logic_base = base_value;
}

float ui_sprite_ctrl_circle_cancel_distance(ui_sprite_ctrl_circle_t ctrl) {
    return ctrl->m_cancel_distance;
}

void ui_sprite_ctrl_circle_set_cancel_distance(ui_sprite_ctrl_circle_t ctrl, float cancel_distance) {
    ctrl->m_cancel_distance = cancel_distance;
}

static void ui_sprite_ctrl_circle_on_begin_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_circle_t ctrl = ctx;

    if (ctrl->m_is_working) return;

    ctrl->m_is_working = 1;
    ctrl->m_last_send_time = 0.0f;
    ctrl->m_max_distance = 0.0f;

    if (ctrl->m_on_begin) {
        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(ctrl), ctrl->m_on_begin, NULL);
    }

    ui_sprite_ctrl_circle_sync_update(ctrl);
}

static void ui_sprite_ctrl_circle_on_move_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_circle_t ctrl = ctx;
    ui_sprite_ctrl_module_t module = ctrl->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(ctx));
    ui_sprite_2d_transform_t trans = ui_sprite_2d_transform_find(entity);
    struct dr_data_source data_source[1];
    float distance = 0.0f;
    UI_SPRITE_EVT_CTRL_CIRCLE_MOVE const * evt_data = evt->data;

    if (!ctrl->m_is_working) return;

    assert(ctrl->m_screen_max > 0.0f);

    data_source[0].m_data.m_meta = module->m_meta_circle_state;
    data_source[0].m_data.m_data = &ctrl->m_state;
    data_source[0].m_data.m_size = sizeof(ctrl->m_state);
    data_source[0].m_next = NULL;

    distance = cpe_math_distance(evt_data->center_pos.x, evt_data->center_pos.y, evt_data->cur_pos.x, evt_data->cur_pos.y);
    if (distance > ctrl->m_max_distance) {
        ctrl->m_max_distance = distance;
    }
    else if (ctrl->m_cancel_distance > 0 && ((ctrl->m_max_distance - distance) > ctrl->m_cancel_distance)) {
        if (ctrl->m_on_cancel) {
            ui_sprite_fsm_action_build_and_send_event(
                ui_sprite_fsm_action_from_data(ctrl), ctrl->m_on_cancel, data_source);
        }

        ctrl->m_is_working = 0;
        ui_sprite_ctrl_circle_sync_update(ctrl);
        return;
    }

    ctrl->m_state.center_pos = evt_data->center_pos;
    ctrl->m_state.cur_pos = evt_data->cur_pos;

    if (distance < ctrl->m_screen_min) {
        ctrl->m_state.percent = 0.0f;
    }
    else {
        if (distance > ctrl->m_screen_max) distance = ctrl->m_screen_max;
        ctrl->m_state.percent = (distance - ctrl->m_screen_min) / (ctrl->m_screen_max - ctrl->m_screen_min);
    }

    ctrl->m_state.logic_percent = ctrl->m_logic_min + (ctrl->m_logic_max - ctrl->m_logic_min) * ctrl->m_state.percent;
    ctrl->m_state.logic_value = ctrl->m_logic_base * ctrl->m_state.logic_percent;

    if (ctrl->m_state.percent > 0) {
        ctrl->m_state.angle = 
            ctrl->m_negative
            ? cpe_math_angle(evt_data->cur_pos.x, evt_data->cur_pos.y, evt_data->center_pos.x, evt_data->center_pos.y)
            : cpe_math_angle(evt_data->center_pos.x, evt_data->center_pos.y, evt_data->cur_pos.x, evt_data->cur_pos.y);

        if (ctrl->m_state.angle < ctrl->m_angle_min) ctrl->m_state.angle = ctrl->m_angle_min;
        if (ctrl->m_state.angle > ctrl->m_angle_max) ctrl->m_state.angle = ctrl->m_angle_max;
    }

    if (ctrl->m_do_rotate) {
        if (trans == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): ctrl-circle: do rotate: no trans!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        else {
            ui_sprite_2d_transform_set_angle(trans, ctrl->m_state.angle);
        }
    }

    if (ctrl->m_do_scale) {
        if (trans == NULL) { 
            CPE_ERROR(
                module->m_em, "entity %d(%s): ctrl-circle: do scale: no trans!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        else {
            ui_sprite_2d_transform_set_scale(trans, ctrl->m_state.logic_percent);
        }
    }

    if (ctrl->m_on_move) {
        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(ctrl), ctrl->m_on_move, data_source);
    }

    ctrl->m_last_send_time = 0.0f;

    ui_sprite_ctrl_circle_sync_update(ctrl);
}

static void ui_sprite_ctrl_circle_on_complete_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_circle_t ctrl = ctx;
    ui_sprite_ctrl_module_t module = ctrl->m_module;
    struct dr_data_source data_source[1];

    if (!ctrl->m_is_working) return;

    data_source[0].m_data.m_meta = module->m_meta_circle_state;
    data_source[0].m_data.m_data = &ctrl->m_state;
    data_source[0].m_data.m_size = sizeof(ctrl->m_state);
    data_source[0].m_next = NULL;

    if (ctrl->m_state.percent <= 0.0f) {
        if (ctrl->m_on_cancel) {
            ui_sprite_fsm_action_build_and_send_event(
                ui_sprite_fsm_action_from_data(ctrl), ctrl->m_on_cancel, data_source);
        }
    }
    else {
        if (ctrl->m_on_done) {
            ui_sprite_fsm_action_build_and_send_event(
                ui_sprite_fsm_action_from_data(ctrl), ctrl->m_on_done, data_source);
        }
    }

    ctrl->m_is_working = 0;
    ui_sprite_ctrl_circle_sync_update(ctrl);
}

static int ui_sprite_ctrl_circle_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_ctrl_module_t module = ctx;

    ctrl->m_max_distance = 0.0f;

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_ctrl_circle_begin", ui_sprite_ctrl_circle_on_begin_event, ctrl) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_ctrl_circle_move", ui_sprite_ctrl_circle_on_move_event, ctrl) != 0
        || ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self,
            "ui_sprite_evt_ctrl_circle_complete", ui_sprite_ctrl_circle_on_complete_event, ctrl) != 0
        )
    {
        CPE_ERROR(module->m_em, "camera ctrl enter: add eventer handler fail!");
        return -1;
    }

    return 0;
}

static void ui_sprite_ctrl_circle_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static void ui_sprite_ctrl_circle_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_fsm_action_data(fsm_action);

    assert(ctrl->m_is_working);
    assert(ctrl->m_keep_send_span > 0);
    assert(ctrl->m_on_move);

    ctrl->m_last_send_time += delta;

    if (ctrl->m_last_send_time >= ctrl->m_keep_send_span) {
        struct dr_data_source data_source[1];

        data_source[0].m_data.m_meta = module->m_meta_circle_state;
        data_source[0].m_data.m_data = &ctrl->m_state;
        data_source[0].m_data.m_size = sizeof(ctrl->m_state);
        data_source[0].m_next = NULL;

        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(ctrl), ctrl->m_on_move, data_source);


        while(ctrl->m_last_send_time >= ctrl->m_keep_send_span) {
            ctrl->m_last_send_time -= ctrl->m_keep_send_span;
        }
    }
}

static int ui_sprite_ctrl_circle_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_fsm_action_data(fsm_action);

    bzero(ctrl, sizeof(*ctrl));

    ctrl->m_module = ctx;
    ctrl->m_logic_base = 1.0f;
    ctrl->m_negative = 1;
    ctrl->m_angle_min = -180.0f;
    ctrl->m_angle_max = 180.0f;

    return 0;
}

static void ui_sprite_ctrl_circle_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_fsm_action_data(fsm_action);

    if (ctrl->m_on_begin) mem_free(module->m_alloc, ctrl->m_on_begin);
    if (ctrl->m_on_move) mem_free(module->m_alloc, ctrl->m_on_move);
    if (ctrl->m_on_done) mem_free(module->m_alloc, ctrl->m_on_done);
    if (ctrl->m_on_cancel) mem_free(module->m_alloc, ctrl->m_on_cancel);
}

static int ui_sprite_ctrl_circle_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_circle_t to_ctrl = ui_sprite_fsm_action_data(to);
    ui_sprite_ctrl_circle_t from_ctrl = ui_sprite_fsm_action_data(from);

    if (ui_sprite_ctrl_circle_init(to, ctx)) return -1;

    to_ctrl->m_do_rotate = from_ctrl->m_do_rotate;
    to_ctrl->m_do_scale = from_ctrl->m_do_scale;
    to_ctrl->m_negative = from_ctrl->m_negative;
    to_ctrl->m_keep_send_span = from_ctrl->m_keep_send_span;
    to_ctrl->m_on_begin = from_ctrl->m_on_begin ? cpe_str_mem_dup(module->m_alloc, from_ctrl->m_on_begin) : NULL;
    to_ctrl->m_on_move = from_ctrl->m_on_move ? cpe_str_mem_dup(module->m_alloc, from_ctrl->m_on_move) : NULL;
    to_ctrl->m_on_done = from_ctrl->m_on_done ? cpe_str_mem_dup(module->m_alloc, from_ctrl->m_on_done) : NULL;
    to_ctrl->m_on_cancel = from_ctrl->m_on_cancel ? cpe_str_mem_dup(module->m_alloc, from_ctrl->m_on_cancel) : NULL;
    to_ctrl->m_screen_min = from_ctrl->m_screen_min;
    to_ctrl->m_screen_max = from_ctrl->m_screen_max;
    to_ctrl->m_logic_min = from_ctrl->m_logic_min;
    to_ctrl->m_logic_max = from_ctrl->m_logic_max;
    to_ctrl->m_angle_min = from_ctrl->m_angle_min;
    to_ctrl->m_angle_max = from_ctrl->m_angle_max;
    to_ctrl->m_logic_base = from_ctrl->m_logic_base;

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_ctrl_circle_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_ctrl_circle_create(fsm_state, name);
    cfg_t child_cfg;

    if (ctrl == NULL) {
        CPE_ERROR(module->m_em, "%s: create ctrl_circle action: create fail!", ui_sprite_ctrl_module_name(module));
        return NULL;
    }

    ui_sprite_ctrl_circle_set_keep_send_span(ctrl, cfg_get_float(cfg, "keep-send-span", 0.0f));

    if (ui_sprite_ctrl_circle_set_on_begin(ctrl, cfg_get_string(cfg, "on-begin", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: set on-begin fail",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if (ui_sprite_ctrl_circle_set_on_move(ctrl, cfg_get_string(cfg, "on-move", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: set on-move fail",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if (ui_sprite_ctrl_circle_set_on_done(ctrl, cfg_get_string(cfg, "on-done", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: set on-done fail",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if (ui_sprite_ctrl_circle_set_on_cancel(ctrl, cfg_get_string(cfg, "on-cancel", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: set on-cancel fail",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "screen-range"))) {
        if (ui_sprite_ctrl_circle_set_screen_range(
                ctrl, cfg_get_float(child_cfg, "min", 0.0f), cfg_get_float(child_cfg, "max", 0.0f))
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: create ctrl_circle action: set screen-range fail",
                ui_sprite_ctrl_module_name(module));
            ui_sprite_ctrl_circle_free(ctrl);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: screen-range not configured",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "angle-range"))) {
        if (ui_sprite_ctrl_circle_set_angle_range(
                ctrl, cfg_get_float(child_cfg, "min", -180.0f), cfg_get_float(child_cfg, "max", 180.0f))
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: create ctrl_circle action: set angle-range fail",
                ui_sprite_ctrl_module_name(module));
            ui_sprite_ctrl_circle_free(ctrl);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: angle-range not configured",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "logic-range"))) {
        if (ui_sprite_ctrl_circle_set_logic_range(
                ctrl, cfg_get_float(child_cfg, "min", 0.0f), cfg_get_float(child_cfg, "max", 0.0f))
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: create ctrl_circle action: set logic-range fail",
                ui_sprite_ctrl_module_name(module));
            ui_sprite_ctrl_circle_free(ctrl);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create ctrl_circle action: logic-range not configured",
            ui_sprite_ctrl_module_name(module));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    ui_sprite_ctrl_circle_set_negative(ctrl, cfg_get_uint8(cfg, "negative", ui_sprite_ctrl_circle_negative(ctrl)));
    ui_sprite_ctrl_circle_set_do_rotate(ctrl, cfg_get_uint8(cfg, "do-rotate", ui_sprite_ctrl_circle_do_rotate(ctrl)));
    ui_sprite_ctrl_circle_set_do_scale(ctrl, cfg_get_uint8(cfg, "do-scale", ui_sprite_ctrl_circle_do_scale(ctrl)));
    ui_sprite_ctrl_circle_set_logic_base(ctrl, cfg_get_float(cfg, "logic-base", ui_sprite_ctrl_circle_logic_base(ctrl)));
    ui_sprite_ctrl_circle_set_cancel_distance(ctrl, cfg_get_float(cfg, "cancel-distance", ui_sprite_ctrl_circle_cancel_distance(ctrl)));

    return ui_sprite_fsm_action_from_data(ctrl);
}

int ui_sprite_ctrl_circle_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CTRL_CIRCLE_NAME, sizeof(struct ui_sprite_ctrl_circle));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl register: meta create fail",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ctrl_circle_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ctrl_circle_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ctrl_circle_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ctrl_circle_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ctrl_circle_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ctrl_circle_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CTRL_CIRCLE_NAME, ui_sprite_ctrl_circle_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ctrl_circle_unregist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CTRL_CIRCLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl unregister: meta not exist",
            ui_sprite_ctrl_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CTRL_CIRCLE_NAME);
    }
}

const char * UI_SPRITE_CTRL_CIRCLE_NAME = "ctrl-circle";

