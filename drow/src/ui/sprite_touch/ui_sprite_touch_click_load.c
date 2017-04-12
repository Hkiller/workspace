#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_touch_click_i.h"

ui_sprite_fsm_action_t ui_sprite_touch_click_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_touch_mgr_t module = ctx;
    ui_sprite_touch_click_t touch_click = ui_sprite_touch_click_create(fsm_state, name);
    int32_t finger_count;

    if (touch_click == NULL) {
        CPE_ERROR(module->m_em, "%s: create touch_click action: create fail!", ui_sprite_touch_mgr_name(module));
        return NULL;
    }

    finger_count = cfg_get_int32(cfg, "finger-count", 1);
    if (finger_count <= 0 || finger_count > UI_SPRITE_TOUCH_MAX_FINGER_COUNT) {
        CPE_ERROR(
            module->m_em, "%s: create touch_click action: finger-count %d error!",
            ui_sprite_touch_mgr_name(module), finger_count);
        ui_sprite_touch_click_free(touch_click);
        return NULL;
    }

    if (ui_sprite_touch_click_set_finger_count(touch_click, finger_count) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_click action: set finger-count %d error!",
            ui_sprite_touch_mgr_name(module), finger_count);
        ui_sprite_touch_click_free(touch_click);
        return NULL;
    }

    if (ui_sprite_touch_click_set_is_grab(
            touch_click, cfg_get_uint8(cfg, "is-grab", ui_sprite_touch_click_is_grab(touch_click)))
            != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create touch_click action: set is-grab error!",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_click_free(touch_click);
        return NULL;
    }

    if (ui_sprite_touch_click_set_z(
            touch_click, cfg_get_float(cfg, "z-index", ui_sprite_touch_click_z(touch_click)))
            != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create touch_click action: set z-index error!",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_click_free(touch_click);
        return NULL;
    }

    if (ui_sprite_touch_click_set_on_click_down(touch_click, cfg_get_string(cfg, "on-click-down", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_click action: set on-click-down fail",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_click_free(touch_click);
        return NULL;
    }

    if (ui_sprite_touch_click_set_on_click_up(touch_click, cfg_get_string(cfg, "on-click-up", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create touch_click action: set on-click-up fail",
            ui_sprite_touch_mgr_name(module));
        ui_sprite_touch_click_free(touch_click);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(touch_click);
}

