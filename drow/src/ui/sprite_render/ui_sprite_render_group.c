#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_2d/ui_sprite_2d_part_binding.h"
#include "ui/sprite_render/ui_sprite_render_layer.h"
#include "ui_sprite_render_group_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_anim_i.h"

static int ui_sprite_render_group_do_bind(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform);
static void ui_sprite_render_group_do_unbind(ui_sprite_render_group_t group);
static void ui_sprite_render_group_calc_local_transform_self(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform, ui_transform_t r);

ui_sprite_render_group_t ui_sprite_render_group_create(ui_sprite_render_sch_t render_sch, const char * name) {
    ui_sprite_render_module_t module = render_sch->m_module;
    ui_sprite_render_group_t render_group;
    size_t name_len = strlen(name) + 1;
    char * p;

    if (ui_sprite_render_group_find_by_name(render_sch, name) != NULL) {
        CPE_ERROR(module->m_em, "crate render group %s: name duplicate!", name);
        return NULL;
    }

    render_group = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_render_group) + name_len);
    if (render_group == NULL) {
        CPE_ERROR(module->m_em, "crate render group %s: alloc fail!", name);
        return NULL;
    }

    p = (char *)(render_group + 1);
    memcpy(p, name, name_len);

    render_group->m_sch = render_sch;
    render_group->m_name = p;
    render_group->m_binding_part = NULL;
    render_group->m_2d_part = NULL;
    render_group->m_base_pos_of_entity = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
	render_group->m_base_pos_adj_policy = UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL;
    render_group->m_trans = UI_TRANSFORM_IDENTITY;
    render_group->m_accept_flip = 1;
    render_group->m_accept_scale = 1;
	render_group->m_accept_rotate = 1;
    render_group->m_adj_accept_scale = 1;
    render_group->m_adj_render_priority = 0.0f;
    render_group->m_world_trans = UI_TRANSFORM_IDENTITY;

    TAILQ_INIT(&render_group->m_anims);
    
    TAILQ_INSERT_TAIL(&render_sch->m_groups, render_group, m_next_for_sch);

    return render_group;
}

ui_sprite_render_group_t ui_sprite_render_group_clone(ui_sprite_render_sch_t render_sch, ui_sprite_render_group_t o) {
    ui_sprite_render_group_t group = ui_sprite_render_group_create(render_sch, o->m_name);

    if (group == NULL) return NULL;
 
    group->m_base_pos_of_entity = o->m_base_pos_of_entity;
	group->m_base_pos_adj_policy = o->m_base_pos_adj_policy;
    group->m_trans = o->m_trans;
    group->m_accept_flip = o->m_accept_flip;
    group->m_accept_scale = o->m_accept_scale;
	group->m_accept_rotate = o->m_accept_rotate;
    group->m_adj_accept_scale = o->m_adj_accept_scale;
    group->m_adj_render_priority = o->m_adj_render_priority;
    
    if (o->m_binding_part) {
        group->m_binding_part = cpe_str_mem_dup(render_sch->m_module->m_alloc, o->m_binding_part);
    }
    
    return group;
}

void ui_sprite_render_group_free(ui_sprite_render_group_t render_group) {
    ui_sprite_render_sch_t render_sch = render_group->m_sch;
    ui_sprite_render_module_t module = render_sch->m_module;

    assert(render_group->m_2d_part == NULL);

    while(!TAILQ_EMPTY(&render_group->m_anims)) {
        ui_sprite_render_anim_free(TAILQ_FIRST(&render_group->m_anims));
    }

    TAILQ_REMOVE(&render_sch->m_groups, render_group, m_next_for_sch);

    if (render_group->m_binding_part) {
        mem_free(module->m_alloc, render_group->m_binding_part);
        render_group->m_binding_part = NULL;
    }
    
    mem_free(module->m_alloc, render_group);
}

ui_sprite_render_group_t ui_sprite_render_group_find_by_name(ui_sprite_render_sch_t render_sch, const char * name) {
    ui_sprite_render_group_t render_group;

    TAILQ_FOREACH(render_group, &render_sch->m_groups, m_next_for_sch) {
        if (strcmp(render_group->m_name, name) == 0) return render_group;
    }

    return NULL;
}

int ui_sprite_render_group_enter(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform) {
    ui_sprite_render_sch_t render_sch = group->m_sch;
    ui_sprite_render_module_t module = render_sch->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(render_sch));

    if (group->m_binding_part) {
        if (ui_sprite_render_group_do_bind(group, transform) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): group %s: enter: bind to part %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group->m_name, group->m_binding_part);
            return -1;
        }
    }

    ui_sprite_render_group_update_world_trans(group, transform);
        
    return 0;
}

void ui_sprite_render_group_exit(ui_sprite_render_group_t group) {
    if (group->m_binding_part) {
        ui_sprite_render_group_do_unbind(group);
    }
}

uint8_t ui_sprite_render_group_base_pos(ui_sprite_render_group_t group) {
    return group->m_base_pos_of_entity;
}

void ui_sprite_render_group_set_base_pos(ui_sprite_render_group_t group, uint8_t base_pos) {
	group->m_base_pos_of_entity = base_pos;
}

uint8_t ui_sprite_render_group_base_pos_adj_policy(ui_sprite_render_group_t group) {
	return group->m_base_pos_adj_policy;
}

void ui_sprite_render_group_set_base_pos_adj_policy(ui_sprite_render_group_t group, uint8_t base_pos_adj_policy) {
	group->m_base_pos_adj_policy = base_pos_adj_policy;
}

uint8_t ui_sprite_render_group_accept_flip(ui_sprite_render_group_t group) {
    return group->m_accept_flip;
}

void ui_sprite_render_group_set_accept_flip(ui_sprite_render_group_t group, uint8_t accept_flip) {
    group->m_accept_flip = accept_flip;
}

uint8_t ui_sprite_render_group_accept_scale(ui_sprite_render_group_t group) {
    return group->m_accept_scale;
}

void ui_sprite_render_group_set_accept_scale(ui_sprite_render_group_t group, uint8_t accept_scale) {
    group->m_accept_scale = accept_scale;
}

uint8_t ui_sprite_render_group_accept_rotate(ui_sprite_render_group_t group) {
	return group->m_accept_rotate;
}

void ui_sprite_render_group_set_accept_rotate(ui_sprite_render_group_t group, uint8_t accept_rotate) {
	group->m_accept_rotate = accept_rotate;
}

uint8_t ui_sprite_render_group_adj_accept_scale(ui_sprite_render_group_t group) {
    return group->m_adj_accept_scale;
}

void ui_sprite_render_group_set_adj_accept_scale(ui_sprite_render_group_t group, uint8_t adj_accept_scale) {
    group->m_adj_accept_scale = adj_accept_scale;

    if (ui_sprite_component_is_active(ui_sprite_component_from_data(group->m_sch))) {
        ui_sprite_render_sch_t sch = group->m_sch;
        ui_sprite_render_anim_t anim;

        TAILQ_FOREACH(anim, &sch->m_anims, m_next_for_sch) {
            if (anim->m_group != group) continue;
        
            ui_sprite_render_anim_set_priority(anim, sch->m_render_priority + group->m_adj_render_priority);
        }
    }
}

float ui_sprite_render_group_adj_render_priority(ui_sprite_render_group_t group) {
    return group->m_adj_render_priority;
}

void ui_sprite_render_group_set_adj_render_priority(ui_sprite_render_group_t group, float adj_render_priority) {
    float diff = adj_render_priority - group->m_adj_render_priority;
    ui_sprite_render_anim_t anim;
    
    group->m_adj_render_priority = adj_render_priority;

    TAILQ_FOREACH(anim, &group->m_anims, m_next_for_group) {
        anim->m_priority += diff;
        anim->m_layer->m_is_dirty = 1;
    }

    group->m_sch->m_is_dirty = 1;
}

static void ui_sprite_render_group_on_update(ui_sprite_2d_part_t part, void * ctx) {
    ui_sprite_render_group_t group = ctx;

    if (ui_sprite_2d_part_trans_updated(part)) {
        ui_sprite_render_anim_t anim;
        
        ui_sprite_render_group_update_world_trans(group, ui_sprite_2d_part_transform(part));

        TAILQ_FOREACH(anim, &group->m_sch->m_anims, m_next_for_sch) {
            if (anim->m_group == group) {
                ui_sprite_render_anim_set_transform(anim, &group->m_world_trans);
            }
        }
    }
}

void ui_sprite_render_group_do_unbind(ui_sprite_render_group_t group) {
    assert(group->m_2d_part);
    ui_sprite_2d_part_remove_bindings(group->m_2d_part, group);
    group->m_2d_part = NULL;
}

int ui_sprite_render_group_do_bind(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform) {
    ui_sprite_2d_part_t part;

    assert(group->m_2d_part == NULL);

    assert(transform);
    
    part = ui_sprite_2d_part_find(transform, group->m_binding_part);
    if (part == NULL) {
        ui_transform local_transform;
        
        part = ui_sprite_2d_part_create(transform, group->m_binding_part);
        if (part == NULL) {
            ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(group->m_sch));
            CPE_ERROR(
                group->m_sch->m_module->m_em, "entity %d(%s): group %s: create part %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group->m_name, group->m_binding_part);
            return -1;
        }

        ui_sprite_render_group_calc_local_transform_self(group, transform, &local_transform);
        ui_sprite_2d_part_set_trans(part, &local_transform);
        ui_sprite_2d_part_dispatch_event(part);
    }

    if (ui_sprite_2d_part_binding_create(
            part, group, ui_sprite_render_group_on_update)
        == NULL)
    {
        ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(group->m_sch));
        CPE_ERROR(
            group->m_sch->m_module->m_em, "entity %d(%s): group %s: create bidning at part %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group->m_name, group->m_binding_part);
        return -1;
    }

    group->m_2d_part = part;
    
    return 0;
}

int ui_sprite_render_group_set_binding_part(ui_sprite_render_group_t group, const char * binding_part) {
    ui_sprite_render_module_t module = group->m_sch->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(group->m_sch);
    
    if (group->m_binding_part) {
        if (ui_sprite_component_is_active(component)) {
            ui_sprite_render_group_do_unbind(group);
        }
        mem_free(module->m_alloc, group->m_binding_part);
    }

    if (binding_part) {
        group->m_binding_part = cpe_str_mem_dup(module->m_alloc, binding_part);
        if (group->m_binding_part == NULL) {
            ui_sprite_entity_t entity = ui_sprite_component_entity(component);
            CPE_ERROR(
                module->m_em, "entity %d(%s): group %s: dup binding part %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group->m_name, binding_part);
            return -1;
        }
    }
    else {
        group->m_binding_part = NULL;
    }

    if (group->m_binding_part) {
        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_t entity = ui_sprite_component_entity(component);
            ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
                
            if (ui_sprite_render_group_do_bind(group, transform) != 0) {
                mem_free(module->m_alloc, group->m_binding_part);
                group->m_binding_part = NULL;
                return -1;
            }
        }
    }

    return 0;
}

const char * ui_sprite_render_group_binding_part(ui_sprite_render_group_t group) {
    return group->m_binding_part;
}

ui_transform_t ui_sprite_render_group_world_transform(ui_sprite_render_group_t group) {
    return &group->m_world_trans;
}

int ui_sprite_render_group_calc_local_transform(ui_sprite_render_group_t group, ui_transform_t r) {
    if (group->m_2d_part == NULL) {
        ui_sprite_component_t component = ui_sprite_component_from_data(group->m_sch);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);
        ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
        ui_sprite_render_group_calc_local_transform_self(group, transform, r);
    }
    else {
        *r = *ui_sprite_2d_part_trans(group->m_2d_part);
    }

    return 0;
}

static ui_sprite_render_group_t ui_sprite_render_group_next(struct ui_sprite_render_group_it * it) {
    ui_sprite_render_group_t * data = (ui_sprite_render_group_t *)(it->m_data);
    ui_sprite_render_group_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sch);

    return r;
}

void ui_sprite_render_sch_groups(ui_sprite_render_group_it_t emitter_it, ui_sprite_render_sch_t render_sch) {
    *(ui_sprite_render_group_t *)(emitter_it->m_data) = TAILQ_FIRST(&render_sch->m_groups);
    emitter_it->next = ui_sprite_render_group_next;
}

static void ui_sprite_render_group_calc_local_transform_self(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform, ui_transform_t r) {
    ui_vector_2 pos;

    *r = group->m_trans;

    ui_transform_get_pos_2(r, &pos);
    
    if (!group->m_adj_accept_scale && group->m_accept_scale) {
        ui_vector_2 entity_scale = ui_sprite_2d_transform_scale_pair(transform);
        if (cpe_float_cmp(entity_scale.x, 0.0f, UI_FLOAT_PRECISION) != 0
            && cpe_float_cmp(entity_scale.x, 0.0f, UI_FLOAT_PRECISION) != 0)
        {
            pos.x /= entity_scale.x;
            pos.y /= entity_scale.y;
        }
    }
    
    if (group->m_base_pos_of_entity != UI_SPRITE_2D_TRANSFORM_POS_ORIGIN) {
        ui_vector_2 base_pos;

        base_pos = ui_sprite_2d_transform_local_pos(transform, group->m_base_pos_of_entity, 0);
        pos.x += base_pos.x;
        pos.y += base_pos.y;
    }

    ui_transform_set_pos_2(r, &pos);
}

void ui_sprite_render_group_update_world_trans(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform) {
    ui_transform world_t;
    ui_transform local_t;
    ui_vector_3 adj_scale;
    uint8_t adj_scale_p = 0;
    
    assert(transform);

    /*计算世界坐标 */
    ui_sprite_2d_transform_calc_trans(transform, &world_t);

    /*计算本地坐标 */
    if (group->m_2d_part == NULL) {
        ui_sprite_render_group_calc_local_transform_self(group, transform, &local_t);
    }
    else {
        local_t = *ui_sprite_2d_part_trans(group->m_2d_part);
    }

    /*调整本地的处理 */
    if (!group->m_accept_flip) {
        adj_scale.x = world_t.m_s.x < 0.0f ? local_t.m_s.x * -1.0f : local_t.m_s.x;
        adj_scale.y = world_t.m_s.y < 0.0f ? local_t.m_s.y * -1.0f : local_t.m_s.y;
        adj_scale.z = world_t.m_s.z < 0.0f ? local_t.m_s.z * -1.0f : local_t.m_s.z;
        adj_scale_p = 1;
    }

    if (!group->m_accept_scale) {
        if (adj_scale_p == 0) {
            adj_scale = local_t.m_s;
            adj_scale_p = 1;
        }

        if (cpe_float_cmp(world_t.m_s.x, 0.0f, UI_FLOAT_PRECISION) != 0) adj_scale.x /= fabs(world_t.m_s.x);
        if (cpe_float_cmp(world_t.m_s.y, 0.0f, UI_FLOAT_PRECISION) != 0) adj_scale.y /= fabs(world_t.m_s.y);
        if (cpe_float_cmp(world_t.m_s.z, 0.0f, UI_FLOAT_PRECISION) != 0) adj_scale.z /= fabs(world_t.m_s.z);
    }

    if (adj_scale_p) {
        ui_transform_set_scale(&local_t, &adj_scale);
    }

    /*角度直接通过世界旋转处理 */
    if (!group->m_accept_rotate) {
        ui_transform_set_quation(&world_t, &UI_QUATERNION_IDENTITY);
    }

    /*叠加世界坐标 */
    group->m_world_trans = local_t;
    ui_transform_adj_by_parent(&group->m_world_trans, &world_t);
}

ui_transform_t ui_sprite_render_group_local_trans(ui_sprite_render_group_t group) {
    return &group->m_trans;
}

void ui_sprite_render_group_set_local_trans(ui_sprite_render_group_t group, ui_transform_t local_trans) {
    group->m_trans = *local_trans;
}
