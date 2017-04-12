#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_touch_move_i.h"

ui_sprite_fsm_action_t ui_sprite_touch_move_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_touch_mgr_t module = ctx;
    ui_sprite_touch_move_t touch_move = ui_sprite_touch_move_create(fsm_state, name);
    int32_t finger_count;

    if (touch_move == NULL) {
        CPE_ERROR(module->m_em, "%s: create touch_move action: create fail!", ui_sprite_touch_mgr_name(module));
        return NULL;
    }

    finger_count = cfg_get_int32(cfg, "finger-count", 1);
    if (finger_count <= 0 || finger_count > UI_SPRITE_TOUCH_MAX_FINGER_COUNT) {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: finger-count %d error!",
            ui_sprite_touch_mgr_name(module), finger_count);
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_finger_count(touch_move, finger_count) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set finger-count %d error!",
            ui_sprite_touch_mgr_name(module), finger_count);
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_is_capture(
            touch_move, cfg_get_uint8(cfg, "is-capture", ui_sprite_touch_move_is_capture(touch_move)))
            != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set is-capture error!",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_is_grab(
            touch_move, cfg_get_uint8(cfg, "is-grab", ui_sprite_touch_move_is_grab(touch_move)))
            != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set is-grab error!",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_z(
            touch_move, cfg_get_float(cfg, "z-index", ui_sprite_touch_move_z(touch_move)))
            != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set is-grab error!",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }
    
    ui_sprite_touch_move_set_threshold(
        touch_move, cfg_get_uint16(cfg, "threshold", ui_sprite_touch_move_threshold(touch_move)));

    if (ui_sprite_touch_move_set_on_begin(touch_move, cfg_get_string(cfg, "on-begin", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set on-begin fail",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_on_move(touch_move, cfg_get_string(cfg, "on-move", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set on-move fail",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_on_end(touch_move, cfg_get_string(cfg, "on-end", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set on-end fail",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    if (ui_sprite_touch_move_set_on_cancel(touch_move, cfg_get_string(cfg, "on-cancel", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_move action: set on-cancel fail",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_move_free(touch_move);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(touch_move);
}

