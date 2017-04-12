#include <assert.h>
#include "spine/Atlas.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "plugin/spine_manip/plugin_spine_manip_import.h"
#include "plugin_spine_manip_i.h"

struct ui_model_import_spine_anim_ctx {
    uint32_t m_module_id;
    ui_ed_mgr_t m_ed_mgr;
    const char * m_sprite_path;
    ui_ed_src_t m_sprite_src;
    ui_ed_obj_t m_sprite_obj;
};

static UI_IMG_BLOCK * spine_build_img_block(
    spAtlasRegion* region, struct ui_model_import_spine_anim_ctx * ctx, ui_ed_obj_t module_obj, uint32_t img_id, const char * pic, error_monitor_t em);
static UI_FRAME * spine_build_frame_block(
    spAtlasRegion* region, struct ui_model_import_spine_anim_ctx * ctx, uint32_t img_id, error_monitor_t em);

int plugin_spine_manip_import(
    ui_ed_mgr_t ed_mgr, const char * module_path,
    const char * atlas_file, const char * pic, error_monitor_t em)
{
    ui_ed_src_t module_src;
    struct ui_model_import_spine_anim_ctx ctx;
    uint32_t id = 0;
    spAtlas * atlas = NULL;
	spAtlasRegion* region;

    ctx.m_ed_mgr = ed_mgr;
    ctx.m_sprite_path = module_path;
    ctx.m_sprite_src = NULL;
    ctx.m_sprite_obj = NULL;

    /*读取配置文件 */
    atlas = spAtlas_createFromFile(atlas_file, NULL);
    if (atlas == NULL) {
        CPE_ERROR(em, "check create module at %s: read atlas %s fail!", module_path, atlas_file);
        return -1;
    }

    module_src = ui_ed_src_check_create(ed_mgr, module_path, ui_data_src_type_module);
    if (module_src == NULL) {
        CPE_ERROR(em, "check create module at %s fail!", module_path);
        spAtlas_dispose(atlas);
        return -1;
    }
    ctx.m_module_id = ui_ed_src_id(module_src);
    ui_ed_obj_remove_childs(ui_ed_src_root_obj(module_src));
    ui_ed_src_strings_clear(module_src);

    /*创建图片 */
	for(region = atlas->regions; region; region = region->next) {
        if (spine_build_img_block(region, &ctx, ui_ed_src_root_obj(module_src), id, pic, em) == 0) {
            spAtlas_dispose(atlas);
            ui_ed_src_delete(module_src);
            return -1;
        }

        ++id;
    }

    if (ctx.m_sprite_src == NULL) {
        ui_ed_src_t old_sprite_src = ui_ed_src_find_by_path(ctx.m_ed_mgr, ctx.m_sprite_path, ui_data_src_type_sprite);
        if (old_sprite_src) {
            ui_ed_src_delete(old_sprite_src);
        }
    }

    spAtlas_dispose(atlas);
    return 0;
}

static UI_IMG_BLOCK * spine_build_img_block(
    spAtlasRegion* region, struct ui_model_import_spine_anim_ctx * ctx,
    ui_ed_obj_t module_obj, uint32_t img_id, const char * pic, error_monitor_t em)
{
    ui_ed_obj_t img_block_obj;
    UI_IMG_BLOCK * img_block;

    img_block_obj = ui_ed_obj_new(module_obj, ui_ed_obj_type_img_block);
    if (img_block_obj == NULL) {
        CPE_ERROR(em, "spine_build_img_block: create img_block fail!");
        return NULL;
    }
    img_block = ui_ed_obj_data(img_block_obj);
    assert(img_block);

    /*填写img_block*/
    if (ui_ed_obj_set_id(img_block_obj, img_id) != 0) {
        CPE_ERROR(em, "spine_build_img_block:: set id %d fail!", img_id);
        return NULL;
    }

    img_block->name_id = ui_ed_src_msg_alloc(ui_ed_obj_src(module_obj), region->name);
    img_block->use_img = ui_ed_src_msg_alloc(ui_ed_obj_src(module_obj), pic);

    img_block->src_x = region->x;
    img_block->src_y = region->y;
    if (region->rotate) {
        img_block->src_w = region->height;
        img_block->src_h = region->width;
    }
    else {
        img_block->src_w = region->width;
        img_block->src_h = region->height;
    }

    img_block->flag = 1;

    if (region->rotate
        || region->originalWidth != region->width
        || region->originalHeight != region->height
        || region->offsetX != 0
        || region->offsetY != 0
        )
    {
        if (spine_build_frame_block(region, ctx, img_id, em) == NULL) return NULL;
    }

    return img_block;
}

static UI_FRAME * spine_build_frame_block(
    spAtlasRegion* region, struct ui_model_import_spine_anim_ctx * ctx, uint32_t img_id, error_monitor_t em)
{
    ui_ed_obj_t frame_obj;
    ui_ed_obj_t frame_img_obj;
    UI_FRAME * frame_block;
    UI_IMG_REF * img_ref;

    if (ctx->m_sprite_src == NULL) {
        ctx->m_sprite_src = ui_ed_src_check_create(ctx->m_ed_mgr, ctx->m_sprite_path, ui_data_src_type_sprite);
        if (ctx->m_sprite_src == NULL) {
            CPE_ERROR(em, "check create sprite at %s fail!", ctx->m_sprite_path);
            return NULL;
        }

        ctx->m_sprite_obj = ui_ed_src_root_obj(ctx->m_sprite_src);
        assert(ctx->m_sprite_obj);
        ui_ed_obj_remove_childs(ctx->m_sprite_obj);
    }

    frame_obj = ui_ed_obj_new(ctx->m_sprite_obj, ui_ed_obj_type_frame);
    if (frame_obj == NULL) {
        CPE_ERROR(em, "spine_build_frame: create frame fail!");
        return NULL;
    }
    frame_block = ui_ed_obj_data(frame_obj);
    assert(frame_block);

    /*填写frame*/
    if (ui_ed_obj_set_id(frame_obj, img_id) != 0) {
        CPE_ERROR(em, "spine_build_frame:: set id %d fail!", img_id);
        return NULL;
    }
    cpe_str_dup(frame_block->name, sizeof(frame_block->name), region->name);
    bzero(&frame_block->bounding, sizeof(frame_block->bounding));
    frame_block->bound_custom = 1;
    frame_block->accept_scale = 0;

    /*frame_img*/
    frame_img_obj = ui_ed_obj_new(frame_obj, ui_ed_obj_type_frame_img);
    if (frame_img_obj == NULL) {
        CPE_ERROR(em, "spine_build_frame: create frame img fail!");
        return NULL;
    }
    img_ref = ui_ed_obj_data(frame_img_obj);
    assert(img_ref);

    img_ref->module_id = ctx->m_module_id;
    img_ref->img_block_id = img_id;
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

    frame_block->bounding.lt = - region->offsetX;
    frame_block->bounding.tp = - (region->originalHeight - region->offsetY);
    frame_block->bounding.rt = frame_block->bounding.lt + region->originalWidth;
    frame_block->bounding.bm = frame_block->bounding.tp + region->originalHeight;

    if (region->rotate) {
        img_ref->trans.world_trans.trans.value[0] = region->width;
        img_ref->trans.world_trans.trans.value[1] = - region->height;
        img_ref->trans.world_trans.trans.value[2] = 0.0f;

        img_ref->trans.local_trans.angle = 90.0f;
    }
    else {
        img_ref->trans.world_trans.trans.value[0] = 0.0f;
        img_ref->trans.world_trans.trans.value[1] = - region->height;
        img_ref->trans.world_trans.trans.value[2] = 0.0f;
        
        img_ref->trans.local_trans.angle = 0.0f;
    }

    return frame_block;
}
