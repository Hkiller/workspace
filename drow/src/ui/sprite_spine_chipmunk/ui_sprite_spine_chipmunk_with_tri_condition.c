#include <assert.h>
#include "spine/Skeleton.h"
#include "chipmunk/chipmunk.h"
#include "chipmunk/cpPolyShape.h"
#include "spine/BoundingBoxAttachment.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/utils/ui_transform.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_tri/ui_sprite_tri_condition.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_tri_scope.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_tri_have_entity.h"
#include "ui/sprite_spine/ui_sprite_spine_tri_on_part_state.h"
#include "ui_sprite_spine_chipmunk_with_tri_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_chipmunk_with_tri_scope_ctx {
    ui_sprite_spine_chipmunk_with_tri_t m_with_tri;
    spSlot * m_slot;
};

static ui_sprite_chipmunk_tri_scope_update_result_t
ui_sprite_spine_chipmunk_with_tri_scope_update(void * ctx, void * shape, void * body) {
    struct ui_sprite_spine_chipmunk_with_tri_scope_ctx * scope_ctx = (struct ui_sprite_spine_chipmunk_with_tri_scope_ctx*)ctx;
    ui_sprite_spine_chipmunk_module_t module = scope_ctx->m_with_tri->m_module;
    spBoundingBoxAttachment * binding_box;
    ui_transform trans;
    int i, j;
    float worldVertices[100];
    cpVect pts[50];
    
    if (scope_ctx->m_slot->attachment == NULL) return ui_sprite_chipmunk_tri_scope_update_skip;
    if (scope_ctx->m_slot->attachment->type != SP_ATTACHMENT_BOUNDING_BOX) return ui_sprite_chipmunk_tri_scope_update_skip;

    if (ui_sprite_render_anim_calc_obj_world_transform(scope_ctx->m_with_tri->m_render_anim, &trans) != 0) return ui_sprite_chipmunk_tri_scope_update_skip;

    binding_box = (spBoundingBoxAttachment *)scope_ctx->m_slot->attachment;
    if (binding_box->super.verticesCount > CPE_ARRAY_SIZE(worldVertices)) {
        CPE_ERROR(
            module->m_em, "ui_sprite_spine_chipmunk_with_tri_scope_update: binding box point count %d overflow, capacity=%d!",
            (int)binding_box->super.verticesCount, (int)CPE_ARRAY_SIZE(worldVertices));
        return ui_sprite_chipmunk_tri_scope_update_fail;
    }
    
    spBoundingBoxAttachment_computeWorldVertices(binding_box, scope_ctx->m_slot, worldVertices);

    if (trans.m_s.x * trans.m_s.y < 0.0f) {
        for(i = 0, j = 0; j + 1 < binding_box->super.verticesCount; i++, j+=2) {
            ui_vector_2 pt = UI_VECTOR_2_INITLIZER(worldVertices[j], - worldVertices[j + 1]);
            ui_transform_inline_adj_vector_2(&trans, &pt);
            pts[i].x = pt.x;
            pts[i].y = pt.y;
        }
    }
    else {
        for(i = 0, j = binding_box->super.verticesCount; j >= 2; i++, j-=2) {
            ui_vector_2 pt = UI_VECTOR_2_INITLIZER(worldVertices[j - 2], - worldVertices[j - 1]);
            ui_transform_inline_adj_vector_2(&trans, &pt);
            pts[i].x = pt.x;
            pts[i].y = pt.y;
        }
    }

    /* printf("\n\nxxxx: begin build shape\n"); */
    /* for(i = 0; i < binding_box->verticesCount / 2; ++i) { */
    /*     printf("    (%f,%f)\n", pts[i].x, pts[i].y); */
    /* } */
    /* printf("xxxx: end build shape\n"); */

    if (cpPolyShapeInitRaw((cpPolyShape *)shape, (cpBody*)body, binding_box->super.verticesCount / 2, pts, 0.0f) == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_with_tri_scope_update: init shape fail!");
        return ui_sprite_chipmunk_tri_scope_update_fail;
    }
    
    return ui_sprite_chipmunk_tri_scope_update_success;
}
    
int ui_sprite_spine_chipmunk_with_tri_build_condition(
    ui_sprite_spine_chipmunk_module_t module, ui_sprite_entity_t entity, plugin_spine_obj_t spine_obj,
    ui_sprite_spine_chipmunk_with_tri_t with_tri, spSlot * slot,
    ui_sprite_tri_rule_t rule, char * cfg)
{
    char * args;
    char * sep;
    ui_sprite_chipmunk_tri_scope_t scope = NULL;
    char * str_value;
    struct ui_sprite_spine_chipmunk_with_tri_scope_ctx scope_ctx;
    
    args = strchr(cfg, ':');
    if (args == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: condition %s format error(no sep)!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), cfg);
        return -1;
    }

    *args = 0;
    args = cpe_str_trim_head(args + 1);
    
    if (strcmp(cfg, "E") == 0) {
        ui_sprite_chipmunk_tri_have_entity_t have_entity = ui_sprite_chipmunk_tri_have_entity_create(rule);
        if (have_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create have-entity condition fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        str_value = cpe_str_read_and_remove_arg(args, "pt", ',', '=');
        if (str_value == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: have-entity pt not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
        
        if (ui_sprite_chipmunk_tri_have_entity_set_name_pattern(have_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: have-entity set pt %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
            return -1;
        }

        if ((str_value = cpe_str_read_and_remove_arg(args, "ct", ',', '='))) {
            if (ui_sprite_chipmunk_tri_have_entity_set_require_count(have_entity, str_value) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: have-entity set ct %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
                return -1;
            }
        }
        
        scope = ui_sprite_chipmunk_tri_have_entity_scope(have_entity);
    }
    else if (strcmp(cfg, "P_S") == 0) {
        ui_sprite_spine_tri_on_part_state_t on_part_state;
        uint8_t include_transition = 0;
        
        on_part_state = ui_sprite_spine_tri_on_part_state_create(rule);
        if (on_part_state == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: create on part stste condition fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        str_value = args;

        sep = strchr(str_value, '.');
        if (sep == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: on part state: part state %s format error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
            return -1;
        }

        *sep = 0;

        if (str_value[0] == '+') {
            include_transition = 1;
            str_value ++;
        }

        if (ui_sprite_spine_tri_on_part_state_set_part_name(on_part_state, str_value) != 0
            || ui_sprite_spine_tri_on_part_state_set_part_state(on_part_state, sep + 1) != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: on part state: set name or state fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        ui_sprite_spine_tri_on_part_state_set_obj(on_part_state, spine_obj);
        
        ui_sprite_spine_tri_on_part_state_set_include_transition(on_part_state, include_transition);
        return 0;
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: unknown condition type %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), cfg);
        return -1;
    }

    assert(scope);

    /*碰撞掩码 */
    if ((str_value = cpe_str_read_and_remove_arg(args, "c-m", ',', '='))) {
        uint32_t masks;
        ui_sprite_chipmunk_env_t sprite_chipmunk_env;
        
        sprite_chipmunk_env = ui_sprite_chipmunk_env_find(ui_sprite_entity_world(entity));
        if (sprite_chipmunk_env == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: no chipmunk env!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
            
        if (plugin_chipmunk_env_masks(ui_sprite_chipmunk_env_env(sprite_chipmunk_env), &masks, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with tri: get c-m %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
            return -1;
        }

        ui_sprite_chipmunk_tri_scope_set_mask(scope, masks);
    }

    /*碰撞范围 */
    scope_ctx.m_with_tri = with_tri;
    scope_ctx.m_slot = slot;
    ui_sprite_chipmunk_tri_scope_set_dynamic(scope, ui_sprite_spine_chipmunk_with_tri_scope_update, &scope_ctx, sizeof(scope_ctx));
    
    return 0;
}

#ifdef __cplusplus
}
#endif
