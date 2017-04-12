#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_touch_scale_i.h"
#include "ui_sprite_touch_touchable_i.h"

ui_sprite_touch_scale_t ui_sprite_touch_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_TOUCH_SCALE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_touch_scale_free(ui_sprite_touch_scale_t scale) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(scale);
    ui_sprite_fsm_action_free(fsm_action);
}

uint8_t ui_sprite_touch_scale_is_capture(ui_sprite_touch_scale_t scale) {
    return scale->m_responser.m_is_capture;
}

int ui_sprite_touch_scale_set_is_capture(ui_sprite_touch_scale_t scale, uint8_t is_capture) {
    return ui_sprite_touch_responser_set_is_capture(&scale->m_responser, is_capture);
}

uint8_t ui_sprite_touch_scale_is_grab(ui_sprite_touch_scale_t scale) {
    return scale->m_responser.m_is_grab;
}

int ui_sprite_touch_scale_set_is_grab(ui_sprite_touch_scale_t scale, uint8_t is_grab) {
    return ui_sprite_touch_responser_set_is_grab(&scale->m_responser, is_grab);
}

float ui_sprite_touch_scale_z(ui_sprite_touch_scale_t scale) {
    return scale->m_responser.m_z;
}

int ui_sprite_touch_scale_set_z(ui_sprite_touch_scale_t scale, float z) {
    scale->m_responser.m_z = z;
    return 0;
}

uint8_t ui_sprite_touch_scale_finger_count(ui_sprite_touch_scale_t scale) {
    return scale->m_responser.m_finger_count;
}

int ui_sprite_touch_scale_set_finger_count(ui_sprite_touch_scale_t scale, uint8_t finger_count) {
    return ui_sprite_touch_responser_set_finger_count(&scale->m_responser, finger_count);
}

uint16_t ui_sprite_touch_scale_threshold(ui_sprite_touch_scale_t scale) {
    return scale->m_responser.m_threshold;
}

void ui_sprite_touch_scale_set_threshold(ui_sprite_touch_scale_t scale, uint16_t threshold) {
    scale->m_responser.m_threshold = threshold;
}

const char * ui_sprite_touch_scale_on_begin(ui_sprite_touch_scale_t scale) {
    return scale->m_on_begin;
}

int ui_sprite_touch_scale_set_on_begin(ui_sprite_touch_scale_t scale, const char * on_begin) {
    ui_sprite_touch_mgr_t mgr = scale->m_responser.m_touchable->m_env->m_mgr;

    if (scale->m_on_begin) mem_free(mgr->m_alloc, scale->m_on_begin);

    if (on_begin) {
        scale->m_on_begin = cpe_str_mem_dup(mgr->m_alloc, on_begin);
        return scale->m_on_begin == NULL ? -1 : 0;
    }
    else {
        scale->m_on_begin = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_scale_on_scale(ui_sprite_touch_scale_t scale) {
    return scale->m_on_scale;
}

int ui_sprite_touch_scale_set_on_scale(ui_sprite_touch_scale_t scale, const char * on_scale) {
    ui_sprite_touch_mgr_t mgr = scale->m_responser.m_touchable->m_env->m_mgr;

    if (scale->m_on_scale) mem_free(mgr->m_alloc, scale->m_on_scale);

    if (on_scale) {
        scale->m_on_scale = cpe_str_mem_dup(mgr->m_alloc, on_scale);
        return scale->m_on_scale == NULL ? -1 : 0;
    }
    else {
        scale->m_on_scale = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_scale_on_end(ui_sprite_touch_scale_t scale) {
    return scale->m_on_end;
}

int ui_sprite_touch_scale_set_on_end(ui_sprite_touch_scale_t scale, const char * on_end) {
    ui_sprite_touch_mgr_t mgr = scale->m_responser.m_touchable->m_env->m_mgr;

    if (scale->m_on_end) mem_free(mgr->m_alloc, scale->m_on_end);

    if (on_end) {
        scale->m_on_end = cpe_str_mem_dup(mgr->m_alloc, on_end);
        return scale->m_on_end == NULL ? -1 : 0;
    }
    else {
        scale->m_on_end = NULL;
        return 0;
    }
}

const char * ui_sprite_touch_scale_on_cancel(ui_sprite_touch_scale_t scale) {
    return scale->m_on_cancel;
}

int ui_sprite_touch_scale_set_on_cancel(ui_sprite_touch_scale_t scale, const char * on_cancel) {
    ui_sprite_touch_mgr_t mgr = scale->m_responser.m_touchable->m_env->m_mgr;

    if (scale->m_on_cancel) mem_free(mgr->m_alloc, scale->m_on_cancel);

    if (on_cancel) {
        scale->m_on_cancel = cpe_str_mem_dup(mgr->m_alloc, on_cancel);
        return scale->m_on_cancel == NULL ? -1 : 0;
    }
    else {
        scale->m_on_cancel = NULL;
        return 0;
    }
}

static void ui_sprite_touch_scale_update_data(ui_sprite_touch_scale_t scale) {
    ui_sprite_touch_responser_binding_t binding;

    assert(scale->m_responser.m_binding_count == scale->m_responser.m_finger_count);

    bzero(&scale->m_state, sizeof(scale->m_state));

    TAILQ_FOREACH(binding, &scale->m_responser.m_bindings, m_next_for_responser) {
        UI_SPRITE_TOUCH_POS_BINDING * start_pos = scale->m_state.start_pos + scale->m_state.finger_count;
        UI_SPRITE_TOUCH_POS_BINDING * cur_pos = scale->m_state.curent_pos + scale->m_state.finger_count;
        
        scale->m_state.finger_count++;

        start_pos->screen_pos.x = binding->m_start_screen_pt.x;
        start_pos->screen_pos.y = binding->m_start_screen_pt.y;
        start_pos->world_pos.x = binding->m_start_world_pt.x;
        start_pos->world_pos.y = binding->m_start_world_pt.y;

        cur_pos->screen_pos.x = binding->m_cur_screen_pt.x;
        cur_pos->screen_pos.y = binding->m_cur_screen_pt.y;
        cur_pos->world_pos.x = binding->m_cur_world_pt.x;
        cur_pos->world_pos.y = binding->m_cur_world_pt.y;
    }

    if (scale->m_responser.m_finger_count == 2) {
    }
}

static void ui_sprite_touch_scale_process_begin(void * ctx) {
    ui_sprite_touch_scale_t scale = ctx;

    ui_sprite_touch_scale_update_data(scale);

    if (scale->m_on_begin) {
        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(scale), scale->m_on_begin, NULL);
    }
}

static void ui_sprite_touch_scale_process_move(void * ctx) {
    ui_sprite_touch_scale_t scale = ctx;

    ui_sprite_touch_scale_update_data(scale);

    if (scale->m_on_scale) {
        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(scale), scale->m_on_scale, NULL);
    }
}

static void ui_sprite_touch_scale_process_end(void * ctx) {
    ui_sprite_touch_scale_t scale = ctx;

    if (scale->m_on_end) {
        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(scale), scale->m_on_end, NULL);
    }

    bzero(&scale->m_state, sizeof(scale->m_state));
}

static void ui_sprite_touch_scale_process_cancel(void * ctx) {
    ui_sprite_touch_scale_t scale = ctx;

    if (scale->m_on_cancel) {
        ui_sprite_fsm_action_build_and_send_event(
            ui_sprite_fsm_action_from_data(scale), scale->m_on_cancel, NULL);
    }

    bzero(&scale->m_state, sizeof(scale->m_state));
}

static int ui_sprite_touch_scale_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    return ui_sprite_touch_responser_enter(&scale->m_responser);
}

static void ui_sprite_touch_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_touch_responser_exit(&scale->m_responser);
}

static int ui_sprite_touch_scale_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_scale_t scale = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_touch_touchable_t touchable = ui_sprite_touch_touchable_find(entity);

    if (touchable == NULL) {
        CPE_ERROR(
            mgr->m_em, "entity %d(%s): create %s: touchable not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), UI_SPRITE_TOUCH_SCALE_NAME);
        return -1;
    }

    bzero(scale, sizeof(struct ui_sprite_touch_scale));

    if (ui_sprite_touch_responser_init(&scale->m_responser, entity, touchable) != 0) {
        return -1;
    }

    scale->m_mgr = mgr;
    scale->m_responser.m_finger_count = 2;
    scale->m_responser.m_threshold = 2;
    scale->m_responser.m_on_begin = ui_sprite_touch_scale_process_begin;
    scale->m_responser.m_on_move = ui_sprite_touch_scale_process_move;
    scale->m_responser.m_on_end = ui_sprite_touch_scale_process_end;
    scale->m_responser.m_on_cancel = ui_sprite_touch_scale_process_cancel;

    return 0;
}

static void ui_sprite_touch_scale_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_scale_t scale = ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_touch_responser_fini(&scale->m_responser);

    if (scale->m_on_scale) {
        mem_free(mgr->m_alloc, scale->m_on_scale);
        scale->m_on_scale = NULL;
    }
}

static int ui_sprite_touch_scale_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_touch_mgr_t mgr = ctx;
    ui_sprite_touch_scale_t to_scale = ui_sprite_fsm_action_data(to);
    ui_sprite_touch_scale_t from_scale = ui_sprite_fsm_action_data(from);

    if (ui_sprite_touch_responser_init(
            &to_scale->m_responser,
            ui_sprite_fsm_action_to_entity(to),
            ui_sprite_fsm_action_data(to))
        != 0) 
    {
        return -1;
    }

    to_scale->m_responser.m_finger_count = from_scale->m_responser.m_finger_count;
    to_scale->m_on_scale = from_scale->m_on_scale ? cpe_str_mem_dup(mgr->m_alloc, from_scale->m_on_scale) : NULL;

    return -1;
}

int ui_sprite_touch_scale_regist(ui_sprite_touch_mgr_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm, UI_SPRITE_TOUCH_SCALE_NAME, sizeof(struct ui_sprite_touch_scale));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch scale register: meta create fail",
            ui_sprite_touch_mgr_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_data_meta(
        meta,
        module->m_meta_scale_state,
        CPE_ENTRY_START(ui_sprite_touch_scale, m_state),
        CPE_ENTRY_SIZE(ui_sprite_touch_scale, m_state));

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_touch_scale_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_touch_scale_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_touch_scale_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_touch_scale_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_touch_scale_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_TOUCH_SCALE_NAME, ui_sprite_touch_scale_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_touch_scale_unregist(ui_sprite_touch_mgr_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm, UI_SPRITE_TOUCH_SCALE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: touch scale unregister: meta not exist",
            ui_sprite_touch_mgr_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_TOUCH_SCALE_NAME);
    }
}

const char * UI_SPRITE_TOUCH_SCALE_NAME = "touch-scale";
