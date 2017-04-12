#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/model_manip/model_manip_cocos.h"
#include "model_manip_cocos_utils.h"

UI_IMG_BLOCK * cocos_build_img_block(cfg_t frame_cfg, ui_ed_obj_t module_obj, uint32_t img_id, const char * pic, error_monitor_t em) {
    const char * frame = cfg_get_string(frame_cfg, "frame", NULL);
    ui_ed_obj_t img_block_obj;
    UI_IMG_BLOCK * img_block;
    int x, y, w, h;
    char * p;
    char frame_name_buf[256];
    
    if (frame == NULL) {
        CPE_ERROR(em, "cocos_build_img_block: read frame fail!");
        return NULL;
    }

    sscanf(frame, "{{%d,%d},{%d,%d}}", &x, &y, &w, &h);

    img_block_obj = ui_ed_obj_new(module_obj, ui_ed_obj_type_img_block);
    if (img_block_obj == NULL) {
        CPE_ERROR(em, "cocos_build_img_block: create img_block fail!");
        return NULL;
    }

    if (ui_ed_obj_set_id(img_block_obj, img_id) != 0) {
        CPE_ERROR(em, "cocos_build_img_block:: set id %d fail!", img_id);
        return NULL;
    }

    img_block = ui_ed_obj_data(img_block_obj);
    assert(img_block);

    cpe_str_dup(frame_name_buf, sizeof(frame_name_buf), cfg_name(frame_cfg));
    if ((p = strrchr(frame_name_buf, '.'))) *p = 0;
    
    /*填写img_block*/
    img_block->name_id = ui_ed_src_msg_alloc(ui_ed_obj_src(module_obj), frame_name_buf);
    img_block->use_img = ui_ed_src_msg_alloc(ui_ed_obj_src(module_obj), pic);
    
    img_block->src_x = x;
    img_block->src_y = y;

    if (cfg_get_uint8(frame_cfg, "rotated", 0)) {
        img_block->src_w = h;
        img_block->src_h = w;
    }
    else {
        img_block->src_w = w;
        img_block->src_h = h;
    }

    img_block->flag = 1;

    return img_block;
}

UI_ACTOR_FRAME * cocos_build_actor_frame(
    cfg_t frame_cfg, ui_ed_obj_t layer_obj, uint32_t module_id, UI_IMG_BLOCK const * img_block, uint32_t idx,
    uint32_t frame_duration, ui_manip_action_import_frame_position_t frame_position, error_monitor_t em)
{
    ui_ed_obj_t actor_frame_obj;
    UI_ACTOR_FRAME * actor_frame;
    UI_IMG_REF * img_ref;

    actor_frame_obj = ui_ed_obj_new(layer_obj, ui_ed_obj_type_actor_frame);
    if (actor_frame_obj == NULL) {
        CPE_ERROR(em, "create actor_frame fail!");
        return NULL;
    }
    actor_frame = ui_ed_obj_data(actor_frame_obj);
    assert(actor_frame);

    actor_frame->texture.type = UI_TEXTURE_REF_IMG;

    img_ref = &actor_frame->texture.data.img;
    img_ref->module_id = module_id;
    img_ref->img_block_id = img_block->id;
    img_ref->freedom = 0;

    img_ref->trans.ol.width = 1;
    img_ref->trans.ol.color.a = 1;

    img_ref->trans.background.a = 1;

    img_ref->trans.local_trans.scale.value[0] = 1;
    img_ref->trans.local_trans.scale.value[1] = 1;
    img_ref->trans.local_trans.scale.value[2] = 1;

    img_ref->trans.world_trans.scale.value[0] = 1;
    img_ref->trans.world_trans.scale.value[1] = 1;
    img_ref->trans.world_trans.scale.value[2] = 1;

    if (cfg_get_uint8(frame_cfg, "rotated", 0)) {
        img_ref->trans.local_trans.angle = -90;

        switch(frame_position) {
        case ui_manip_action_import_frame_center:
            img_ref->trans.local_trans.trans.value[0] = - ((float)img_block->src_h / 2);
            img_ref->trans.local_trans.trans.value[1] = (float)img_block->src_w / 2;
            break;
        case ui_manip_action_import_frame_center_left:
            img_ref->trans.local_trans.trans.value[0] = 0;
            img_ref->trans.local_trans.trans.value[1] = (float)img_block->src_w / 2;
            break;
        case ui_manip_action_import_frame_center_right:
            img_ref->trans.local_trans.trans.value[0] = - (float)img_block->src_h;
            img_ref->trans.local_trans.trans.value[1] = (float)img_block->src_w / 2;
            break;
        case ui_manip_action_import_frame_bottom_center:
            img_ref->trans.local_trans.trans.value[0] = - ((float)img_block->src_h / 2);
            img_ref->trans.local_trans.trans.value[1] = 0;
            break;
        case ui_manip_action_import_frame_bottom_left:
            img_ref->trans.local_trans.trans.value[0] = 0;
            img_ref->trans.local_trans.trans.value[1] = 0;
            break;
        case ui_manip_action_import_frame_bottom_right:
            img_ref->trans.local_trans.trans.value[0] = - ((float)img_block->src_h);
            img_ref->trans.local_trans.trans.value[1] = 0;
            break;
        case ui_manip_action_import_frame_top_center:
            img_ref->trans.local_trans.trans.value[0] = - ((float)img_block->src_h / 2);
            img_ref->trans.local_trans.trans.value[1] = (float)img_block->src_w;
            break;
        case ui_manip_action_import_frame_top_left:
            img_ref->trans.local_trans.trans.value[0] = 0;
            img_ref->trans.local_trans.trans.value[1] = (float)img_block->src_w;
            break;
        case ui_manip_action_import_frame_top_right:
            img_ref->trans.local_trans.trans.value[0] = - (float)img_block->src_h;
            img_ref->trans.local_trans.trans.value[1] = (float)img_block->src_w;
            break;
        default:
            CPE_ERROR(em, "cocos_build_action_frame:: not support frame-position %d!", frame_position);
            return NULL;
        }
    }
    else {
        switch(frame_position) {
        case ui_manip_action_import_frame_center:
            img_ref->trans.local_trans.trans.value[0] = - ((int)img_block->src_w / 2);
            img_ref->trans.local_trans.trans.value[1] = - ((int)img_block->src_h / 2);
            break;
        case ui_manip_action_import_frame_center_left:
            img_ref->trans.local_trans.trans.value[0] = 0;
            img_ref->trans.local_trans.trans.value[1] = - ((int)img_block->src_h / 2);
            break;
        case ui_manip_action_import_frame_center_right:
            img_ref->trans.local_trans.trans.value[0] = - (int)img_block->src_w;
            img_ref->trans.local_trans.trans.value[1] = - ((int)img_block->src_h / 2);
            break;
        case ui_manip_action_import_frame_bottom_center:
            img_ref->trans.local_trans.trans.value[0] = - ((int)img_block->src_w / 2);
            img_ref->trans.local_trans.trans.value[1] = - (int)img_block->src_h;
            break;
        case ui_manip_action_import_frame_bottom_left:
            img_ref->trans.local_trans.trans.value[0] = 0;
            img_ref->trans.local_trans.trans.value[1] = - (int)img_block->src_h;
            break;
        case ui_manip_action_import_frame_bottom_right:
            img_ref->trans.local_trans.trans.value[0] = - (int)img_block->src_w;
            img_ref->trans.local_trans.trans.value[1] = - (int)img_block->src_h;
            break;
        case ui_manip_action_import_frame_top_center:
            img_ref->trans.local_trans.trans.value[0] = - ((int)img_block->src_w / 2);
            img_ref->trans.local_trans.trans.value[1] = 0;
            break;
        case ui_manip_action_import_frame_top_left:
            img_ref->trans.local_trans.trans.value[0] = 0;
            img_ref->trans.local_trans.trans.value[1] = 0;
            break;
        case ui_manip_action_import_frame_top_right:
            img_ref->trans.local_trans.trans.value[0] = - (int)img_block->src_w;
            img_ref->trans.local_trans.trans.value[1] = 0;
            break;
        default:
            CPE_ERROR(em, "cocos_build_action_frame:: not support frame-position %d!", frame_position);
            return NULL;
        }
    }

    actor_frame->start_frame = idx * frame_duration;

    return actor_frame;
}
