#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "render/model_manip/model_manip_cocos.h"
#include "ops.h"

int do_cocos_effect_import(
    gd_app_context_t app, ui_data_mgr_t data_mgr,
    const char * to_effect, const char * to_module,
    const char * plist, const char * pic, uint8_t frame_duration,
    const char * str_frame_position, const char * str_frame_order)
{
    ui_ed_mgr_t ed_mgr = NULL;
    ui_manip_action_import_frame_position_t frame_position;
    ui_manip_action_import_frame_order_t frame_order;

    if (strcmp(str_frame_position, "center") == 0) {
        frame_position = ui_manip_action_import_frame_center;
    }
    else if (strcmp(str_frame_position, "center-left") == 0) {
        frame_position = ui_manip_action_import_frame_center_left;
    }
    else if (strcmp(str_frame_position, "center-right") == 0) {
        frame_position = ui_manip_action_import_frame_center_right;
    }
    else if (strcmp(str_frame_position, "bottom-center") == 0) {
        frame_position = ui_manip_action_import_frame_bottom_center;
    }
    else if (strcmp(str_frame_position, "bottom-left") == 0) {
        frame_position = ui_manip_action_import_frame_bottom_left;
    }
    else if (strcmp(str_frame_position, "bottom-right") == 0) {
        frame_position = ui_manip_action_import_frame_bottom_right;
    }
    else if (strcmp(str_frame_position, "top-center") == 0) {
        frame_position = ui_manip_action_import_frame_top_center;
    }
    else if (strcmp(str_frame_position, "top-left") == 0) {
        frame_position = ui_manip_action_import_frame_top_left;
    }
    else if (strcmp(str_frame_position, "top-right") == 0) {
        frame_position = ui_manip_action_import_frame_top_right;
    }
    else {
        APP_CTX_ERROR(app, "unknown frame-position %s!", str_frame_position);
        return -1;
    }

    if (strcmp(str_frame_order, "native") == 0) {
        frame_order = ui_manip_action_import_frame_order_native;
    }
    else if (strcmp(str_frame_order, "postfix") == 0) {
        frame_order = ui_manip_action_import_frame_order_postfix;
    }
    else {
        APP_CTX_ERROR(app, "unknown frame-order %s!", str_frame_order);
        return -1;
    }

    ed_mgr = ui_ed_mgr_find_nc(app, NULL);
    assert(ed_mgr);

    if (ui_model_import_cocos_action(
        ed_mgr,
        to_effect,
        to_module,
        plist,
        pic,
        frame_duration,
        frame_position,
        frame_order,
        gd_app_em(app))
        != 0)
    {
        APP_CTX_ERROR(app, "import cocos action fail!");
        return -1;
    }

    return 0;
}
