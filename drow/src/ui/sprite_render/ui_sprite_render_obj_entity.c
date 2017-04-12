#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "ui_sprite_render_obj_entity_i.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_anim_i.h"

int ui_sprite_render_obj_entity_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    ui_sprite_render_obj_entity_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_module = ctx;
    obj->m_world[0] = 0;
    obj->m_entity[0] = 0;
    return 0;
}

void ui_sprite_render_obj_entity_free(void * ctx, ui_runtime_render_obj_t render_obj) {
}

int ui_sprite_render_obj_entity_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    ui_sprite_render_obj_entity_t obj = ui_runtime_render_obj_data(render_obj);
    ui_sprite_entity_t entity;
    ui_sprite_world_t world;
    ui_sprite_render_env_t render_env;
    ui_sprite_render_sch_t render_sch;
    ui_sprite_2d_transform_t entity_transform;
    ui_vector_2 entity_scale;
    ui_vector_2 entity_pos;
    ui_sprite_render_anim_t anim;
    
    entity = ui_sprite_render_obj_entity_entity(obj);
    if (entity == NULL) return -1;

    world = ui_sprite_entity_world(entity);

    render_env = ui_sprite_render_env_find(world);
    if (render_env == NULL) return -1;

    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) return -1;

    if (render_sch->m_is_dirty) {
        ui_sprite_render_sch_sort_anims(render_sch);
        render_sch->m_is_dirty = 0;
    }

    entity_transform = ui_sprite_2d_transform_find(entity);
    if (entity_transform == NULL) return -1;

    entity_scale = ui_sprite_2d_transform_scale_pair(entity_transform);
    if (cpe_float_cmp(entity_scale.x, 0.0f, UI_FLOAT_PRECISION) == 0
        || cpe_float_cmp(entity_scale.y, 0.0f, UI_FLOAT_PRECISION) == 0)
    {
        return 0;
    }
    
    entity_pos = ui_sprite_2d_transform_origin_pos(entity_transform);
    
    TAILQ_FOREACH(anim, &render_sch->m_anims, m_next_for_sch) {
        ui_transform anim_o = anim->m_transform;
        ui_vector_2 pos;

        ui_transform_get_pos_2(&anim_o, &pos);
        pos.x -= entity_pos.x;
        pos.y -= entity_pos.y;
        ui_transform_set_pos_2(&anim_o, &pos);
        
        ui_transform_adj_by_parent(&anim_o, transform);

        ui_runtime_render_obj_ref_render(anim->m_render_obj_ref, context, clip_rect, &anim_o);

        /* ui_transform_get_pos_2(&anim_o, &pos); */
        /* printf( */
        /*     "xxxxx: render entity %d(%s), anim %d(%s) at (%f,%f)\n", */
        /*     ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), anim->m_anim_id, anim->m_anim_name, */
        /*     pos.x, pos.y); */
    }
    
    return 0;
}

static uint8_t ui_sprite_render_obj_entity_is_playing(void * ctx, ui_runtime_render_obj_t render_obj) {
    ui_sprite_render_obj_entity_t obj = ui_runtime_render_obj_data(render_obj);
    return ui_sprite_render_obj_entity_entity(obj) == NULL ? 0 : 1;
}

static int ui_sprite_render_obj_entity_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * arg_buf_will_change) {
    ui_sprite_render_obj_entity_t obj = ui_runtime_render_obj_data(render_obj);
    const char * str_value;

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "entity", ',', '='))) {
        ui_sprite_render_obj_entity_set_entity_name(obj, str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "world", ',', '='))) {
        ui_sprite_render_obj_entity_set_world_name(obj, str_value);
    }
    
    return 0;
}

const char * ui_sprite_render_obj_entity_world_name(ui_sprite_render_obj_entity_t render_obj_entity) {
    return render_obj_entity->m_world;
}

void ui_sprite_render_obj_entity_set_world_name(ui_sprite_render_obj_entity_t entity_obj, const char * world_name) {
    cpe_str_dup(entity_obj->m_world, sizeof(entity_obj->m_world), world_name);
}

ui_sprite_world_t ui_sprite_render_obj_entity_world(ui_sprite_render_obj_entity_t render_obj_entity) {
    if (render_obj_entity->m_world[0] == 0) return NULL;

    return ui_sprite_world_find(render_obj_entity->m_module->m_repo, render_obj_entity->m_world);
}

const char * ui_sprite_render_obj_entity_entity_name(ui_sprite_render_obj_entity_t render_obj_entity) {
    return render_obj_entity->m_entity;
}

void ui_sprite_render_obj_entity_set_entity_name(ui_sprite_render_obj_entity_t entity_obj, const char * entity_name) {
    cpe_str_dup(entity_obj->m_entity, sizeof(entity_obj->m_entity), entity_name);
}

ui_sprite_entity_t ui_sprite_render_obj_entity_entity(ui_sprite_render_obj_entity_t render_obj_entity) {
    ui_sprite_world_t world;
    
    if (render_obj_entity->m_world[0] == 0 || render_obj_entity->m_entity[0] == 0) return NULL;
    
    world = ui_sprite_world_find(render_obj_entity->m_module->m_repo, render_obj_entity->m_world);
    if (world == NULL) return NULL;

    if (render_obj_entity->m_entity[0] >= '0' && render_obj_entity->m_entity[0] <= '9') {
        return ui_sprite_entity_find_by_id(world, (uint32_t)atoi(render_obj_entity->m_entity));
    }
    else {
        return ui_sprite_entity_find_by_name(world, render_obj_entity->m_entity);
    }
}

int ui_sprite_render_obj_entity_regist(ui_sprite_render_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;
        
    if (module->m_runtime == NULL) return 0;
    
    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "entity", 0, sizeof(struct ui_sprite_render_obj_entity), module,
            ui_sprite_render_obj_entity_init,
            NULL,
            ui_sprite_render_obj_entity_setup,
            NULL,
            ui_sprite_render_obj_entity_free,
            ui_sprite_render_obj_entity_render,
            ui_sprite_render_obj_entity_is_playing,
            NULL,
            NULL);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj entity fail", ui_sprite_render_module_name(module));
        return -1;
    }

    return 0;
}

void ui_sprite_render_obj_entity_unregist(ui_sprite_render_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "entity");
        if (obj_meta) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}
