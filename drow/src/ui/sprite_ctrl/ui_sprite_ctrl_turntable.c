#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui_sprite_ctrl_turntable_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"

ui_sprite_ctrl_turntable_t ui_sprite_ctrl_turntable_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_CTRL_TURNTABLE_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
}

ui_sprite_ctrl_turntable_t ui_sprite_ctrl_turntable_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_CTRL_TURNTABLE_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
}

void ui_sprite_ctrl_turntable_free(ui_sprite_ctrl_turntable_t ctrl) {
    ui_sprite_component_t component = ui_sprite_component_from_data(ctrl);
    ui_sprite_component_free(component);
}

UI_SPRITE_CTRL_TURNTABLE_DEF const * ui_sprite_ctrl_turntable_def(ui_sprite_ctrl_turntable_t turntable) {
    return &turntable->m_def;
}

void ui_sprite_ctrl_turntable_track_circle(
    ui_sprite_ctrl_turntable_t turntable, ui_vector_2_t scale,
    ui_vector_2_t r, ui_vector_2 const * base, float angle);

void ui_sprite_ctrl_turntable_track_ellipse(
    ui_sprite_ctrl_turntable_t turntable, ui_vector_2_t scale,
    ui_vector_2_t r, ui_vector_2 const * base, float angle);

static int ui_sprite_ctrl_turntable_cmp_slot(void const * l, void const * r) {
    float diff = *(float const *)l - *(float const *)r;
    return diff < 0.0f
        ? -1
        : diff > 0.0f
        ? 1
        : 0;
}

int ui_sprite_ctrl_turntable_set_def(ui_sprite_ctrl_turntable_t turntable, UI_SPRITE_CTRL_TURNTABLE_DEF const * input_def) {
    ui_sprite_ctrl_module_t module = turntable->m_module;
    ui_sprite_ctrl_turntable_track_fun_t track_fun = NULL;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(turntable));
    uint16_t i;
    UI_SPRITE_CTRL_TURNTABLE_DEF def;
    uint16_t focuse_slot_idx = 0;
    float diff_to_focuse = 360.0f;

    def = *input_def;

    if (def.scale_min != 0.0f || def.scale_max != 0.0f) {
        if (def.scale_min < 0.0f || def.scale_max < 0.0f || def.scale_min >= def.scale_max) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): turntable: set def: scale range (%f~%f) error",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def.scale_min, def.scale_max);
            return -1;
        }
    }

    for(i = 0; i < def.slot_count; ++i) {
        def.slots[i] = cpe_math_angle_regular(def.slots[i]);
    }
    qsort(def.slots, def.slot_count, sizeof(def.slots[0]), ui_sprite_ctrl_turntable_cmp_slot);

    def.focuse_angle = cpe_math_angle_regular(def.focuse_angle);
    for(i = 0; i < def.slot_count; ++i) {
        float diff = fabs(cpe_math_angle_diff(def.focuse_angle, def.slots[i]));
        if (diff < diff_to_focuse) {
            diff_to_focuse = diff;
            focuse_slot_idx = i;
        }
    }

    switch(def.track_type) {
    case 0:
        track_fun = NULL;
        break;
    case UI_SPRITE_CTRL_TURNTABLE_TRACK_TYPE_CIRCLE:
        track_fun = ui_sprite_ctrl_turntable_track_circle;
        break;
    case UI_SPRITE_CTRL_TURNTABLE_TRACK_TYPE_ELLIPSE:
        track_fun = ui_sprite_ctrl_turntable_track_ellipse;
        break;
    default:
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: set def: track type %d is unknown",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def.track_type);
        return -1;
    }

    turntable->m_def = def;
    turntable->m_focuse_slot_idx = focuse_slot_idx;
    turntable->m_track_fun = track_fun;

    return 0;
}

int ui_sprite_ctrl_turntable_add_member(ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member) {
    ui_sprite_ctrl_module_t module = turntable->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(turntable);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_entity_t member_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member));

    if (member->m_turntable) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: add member: member %d(%s) already join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), 
            ui_sprite_entity_id(member_entity), ui_sprite_entity_name(member_entity));
        return -1;
    }

    member->m_turntable = turntable;
    TAILQ_INSERT_TAIL(&turntable->m_members, member, m_next_for_turntable);

    return 0;
}

void ui_sprite_ctrl_turntable_set_focuse_member(ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member) {
    ui_sprite_ctrl_module_t module = turntable->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(turntable));

    if (member) {
        ui_sprite_entity_t member_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member));

        if (member->m_turntable == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): turntable: set focuse member: member %d(%s) not join turntable!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(member_entity), ui_sprite_entity_name(member_entity));
            return;
        }

        if (member->m_turntable != turntable) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): turntable: set focuse member: member %d(%s) is join other turntable!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(member_entity), ui_sprite_entity_name(member_entity));
            return;
        }
    }

    if (turntable->m_focuse_member == member) return;

    if (turntable->m_focuse_member) {
        ui_sprite_component_t component = ui_sprite_component_from_data(turntable->m_focuse_member);
        ui_sprite_entity_t focuse_entity = ui_sprite_component_entity(component);

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable: set focuse member: member %d(%s) unselected!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(focuse_entity), ui_sprite_entity_name(focuse_entity));
        }

        if (turntable->m_focuse_member->m_on_unselect) {
            ui_sprite_entity_build_and_send_event(focuse_entity, turntable->m_focuse_member->m_on_unselect, NULL);
        }

        turntable->m_data.focuse_entity_id = 0;
    }

    turntable->m_focuse_member = member;
    if (turntable->m_focuse_member) {
        ui_sprite_component_t component = ui_sprite_component_from_data(turntable->m_focuse_member);
        ui_sprite_entity_t focuse_entity = ui_sprite_component_entity(component);

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable: set focuse member: member %d(%s) selected!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(focuse_entity), ui_sprite_entity_name(focuse_entity));
        }

        if (turntable->m_focuse_member->m_on_select) {
            ui_sprite_entity_build_and_send_event(focuse_entity, turntable->m_focuse_member->m_on_select, NULL);
        }

        turntable->m_data.focuse_entity_id = ui_sprite_entity_id(focuse_entity);
    }
}

void ui_sprite_ctrl_turntable_remove_member(ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member) {
    ui_sprite_ctrl_module_t module = turntable->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(turntable);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_entity_t member_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member));

    if (member->m_turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: remove member: member %d(%s) not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(member_entity), ui_sprite_entity_name(member_entity));
        return;
    }

    if (member->m_turntable != turntable) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: remove member: member %d(%s) is join other turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(member_entity), ui_sprite_entity_name(member_entity));
        return;
    }

    if (turntable->m_focuse_member == member) {
        ui_sprite_ctrl_turntable_set_focuse_member(turntable, NULL);
    }

    assert(turntable->m_focuse_member != member);

    TAILQ_REMOVE(&turntable->m_members, member, m_next_for_turntable);
    member->m_turntable = NULL;
}

void ui_sprite_ctrl_turntable_update_members_transform(ui_sprite_ctrl_turntable_t turntable) {
    ui_sprite_ctrl_module_t module = turntable->m_module;
    ui_sprite_ctrl_turntable_member_t member;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(turntable));
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    ui_sprite_render_sch_t render_sch = ui_sprite_render_sch_find(entity);
    ui_vector_2 base_pos;
    ui_vector_2 scale = ui_sprite_2d_transform_scale_pair(transform);

    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: turntable no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    base_pos = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_CENTER, 0);

    TAILQ_FOREACH(member, &turntable->m_members, m_next_for_turntable) {
        ui_sprite_entity_t member_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member));
        ui_sprite_2d_transform_t member_transform = ui_sprite_2d_transform_find(member_entity);
        ui_sprite_render_sch_t member_render_sch = ui_sprite_render_sch_find(member_entity);
        ui_vector_2 member_scale = scale;
        ui_vector_2 pos;
        float angel_to_focuse = fabs(cpe_math_angle_diff(member->m_angle, turntable->m_def.focuse_angle));

        if (member_transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): turntable: member %d(%s) no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(member_entity), ui_sprite_entity_name(member_entity));
            continue;
        }

        turntable->m_track_fun(turntable, &scale, &pos, &base_pos, member->m_angle);
        angel_to_focuse = fabs(cpe_math_angle_diff(member->m_angle, turntable->m_def.focuse_angle));

        if (render_sch && member_render_sch) {
            ui_sprite_render_sch_set_render_priority(
                member_render_sch,
                ui_sprite_render_sch_render_priority(render_sch) + (angel_to_focuse <= 90.0f ? 1 : -1));
        }
        
        if (turntable->m_def.scale_min < turntable->m_def.scale_max) {
            float percent;
            float s;
            
            assert(angel_to_focuse <= 180.0f);

            percent = (180.0f - angel_to_focuse) / 180.0f;

            s =
                turntable->m_def.scale_min
                + (turntable->m_def.scale_max - turntable->m_def.scale_min) * percent;

            member_scale.x *= s;
            member_scale.y *= s;
        }

        ui_sprite_2d_transform_set_origin_pos(member_transform, pos);
        ui_sprite_2d_transform_set_scale_pair(member_transform, member_scale);
    }
}

void ui_sprite_ctrl_turntable_update_members_angle_no_slots(
    ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t base_member, float base_angle)
{
    ui_sprite_ctrl_module_t module = turntable->m_module;
    uint16_t member_count = 0;
    ui_sprite_ctrl_turntable_member_t member;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(turntable));
    float step_angle;
    float angle;

    assert(turntable);
    assert(base_member);
    assert(base_member->m_turntable == turntable);

    TAILQ_FOREACH(member, &turntable->m_members, m_next_for_turntable) {
        ++member_count;
    }

    if (member_count == 0) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable: update members: no member, skip",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        return;
    }

    step_angle = 360.0f / member_count;
    if (step_angle > turntable->m_def.max_angel_step) step_angle = turntable->m_def.max_angel_step;

    base_member->m_angle = cpe_math_angle_regular(base_angle);

    for(member = TAILQ_PREV(base_member, ui_sprite_ctrl_turntable_member_list, m_next_for_turntable)
            , angle = base_angle + step_angle;
        member != TAILQ_END(&turntable->m_members);
        member = TAILQ_PREV(member, ui_sprite_ctrl_turntable_member_list, m_next_for_turntable)
            , angle = angle + step_angle
        )
    {
        member->m_angle = cpe_math_angle_regular(angle);
    }

    for(member = TAILQ_NEXT(base_member, m_next_for_turntable)
            , angle = base_angle - step_angle;
        member != TAILQ_END(&turntable->m_members);
        member = TAILQ_NEXT(member, m_next_for_turntable)
            , angle = angle - step_angle
        )
    {
        member->m_angle = cpe_math_angle_regular(angle);
    }
}

static void ui_sprite_ctrl_turntable_update_members_angle_with_slots(
    ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t base_member, float base_angle)
{
    ui_sprite_ctrl_turntable_member_t member;
    int base_pos;
    float base_percent;
    int pos;

    base_angle = cpe_math_angle_regular(base_angle);

    if (base_angle < turntable->m_def.slots[0]) {
        float last_slot = turntable->m_def.slots[turntable->m_def.slot_count - 1];

        assert((base_angle + 360.0f) >= last_slot);

        base_pos = turntable->m_def.slot_count - 1;
        base_percent = (base_angle + 360.0f - last_slot) / (turntable->m_def.slots[0] + 360.0f - last_slot);
    }
    else {
        for(base_pos = 0; base_pos < turntable->m_def.slot_count; ++base_pos) {
            float slot = turntable->m_def.slots[base_pos];
            float next_slot = 
                (base_pos + 1) < turntable->m_def.slot_count
                ? turntable->m_def.slots[base_pos + 1]
                : turntable->m_def.slots[0] + 360;

            if (base_angle < next_slot) {
                base_percent = (base_angle - slot) / (next_slot - slot);
                break;
            }
        }

        assert(base_pos < turntable->m_def.slot_count);
    }

    for(member = TAILQ_PREV(base_member, ui_sprite_ctrl_turntable_member_list, m_next_for_turntable)
            , pos = base_pos - 1;
        member != TAILQ_END(&turntable->m_members);
        member = TAILQ_PREV(member, ui_sprite_ctrl_turntable_member_list, m_next_for_turntable)
            , pos = pos - 1
        )
    {
        if (pos < 0) {
            pos = turntable->m_def.slot_count - 1;
            member->m_angle =
                turntable->m_def.slots[pos]
                + (turntable->m_def.slots[0] + 360.0f - turntable->m_def.slots[pos]) * base_percent;
        }
        else {
            assert((pos + 1) < turntable->m_def.slot_count);
            member->m_angle =
                turntable->m_def.slots[pos]
                + (turntable->m_def.slots[pos + 1] - turntable->m_def.slots[pos]) * base_percent;
        }

        member->m_angle = cpe_math_angle_regular(member->m_angle);
    }

    pos = base_pos;
    for(member = base_member; member != TAILQ_END(&turntable->m_members); member = TAILQ_NEXT(member, m_next_for_turntable)) {
        assert(pos < turntable->m_def.slot_count);

        if (pos + 1 < turntable->m_def.slot_count) {
            member->m_angle =
                turntable->m_def.slots[pos]
                + (turntable->m_def.slots[pos + 1] - turntable->m_def.slots[pos]) * base_percent;

            pos++;
        }
        else {
            member->m_angle =
                turntable->m_def.slots[pos]
                 + (turntable->m_def.slots[0] + 360.0f - turntable->m_def.slots[pos]) * base_percent;

            pos = 0;
        }

        member->m_angle = cpe_math_angle_regular(member->m_angle);
    }
}

void ui_sprite_ctrl_turntable_update_members_angle(
    ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t base_member, float base_angle)
{
    if (turntable->m_def.slot_count > 0) {
        ui_sprite_ctrl_turntable_update_members_angle_with_slots(turntable, base_member, base_angle);
    }
    else {
        ui_sprite_ctrl_turntable_update_members_angle_no_slots(turntable, base_member, base_angle);
    }
}

ui_sprite_ctrl_turntable_member_t
ui_sprite_ctrl_turntable_find_focuse_member(ui_sprite_ctrl_turntable_t turntable, float diff_to_focuse) {
    ui_sprite_ctrl_turntable_member_t member;
    ui_sprite_ctrl_turntable_member_t r = NULL;

    TAILQ_FOREACH(member, &turntable->m_members, m_next_for_turntable) {
        float diff = fabs(cpe_math_angle_diff(member->m_angle, turntable->m_def.focuse_angle));
        if (diff < diff_to_focuse) {
            diff_to_focuse = diff;
            r = member;
        }
    }

    return r;
}

float ui_sprite_ctrl_turntable_calc_member_angle(
    ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member,
    ui_sprite_ctrl_turntable_member_t base_member, float base_member_angle)
{
    if (base_member->m_angle == base_member_angle) {
        return member->m_angle;
    }
    else {
        float save_angle = base_member->m_angle;
        float r;

        ui_sprite_ctrl_turntable_update_members_angle(turntable, base_member, base_member_angle);
        r = member->m_angle;
        ui_sprite_ctrl_turntable_update_members_angle(turntable, base_member, save_angle);

        return r;
    }
}

int ui_sprite_ctrl_turntable_pos(ui_sprite_ctrl_turntable_t turntable, ui_vector_2 * pos) {
    ui_sprite_entity_t turntable_entity;
    ui_sprite_2d_transform_t turntable_transform;

    turntable_entity = ui_sprite_component_entity(ui_sprite_component_from_data(turntable));
    turntable_transform = ui_sprite_2d_transform_find(turntable_entity);
    if (turntable_transform == NULL) return -1;

    *pos = ui_sprite_2d_transform_origin_pos(turntable_transform);

    return 0;
}

static void ui_sprite_ctrl_turntable_on_transform_update(void * ctx) {
    ui_sprite_ctrl_turntable_t turntable = ctx;
    ui_sprite_ctrl_turntable_update_members_transform(turntable);
}

static int ui_sprite_ctrl_turntable_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_ctrl_turntable_t turntable = ui_sprite_component_data(component);
    ui_sprite_ctrl_module_t module = turntable->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);

    if (turntable->m_track_fun == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: enter: track not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_component_add_attr_monitor(
            component, "transform.pos", ui_sprite_ctrl_turntable_on_transform_update, turntable)
        == NULL)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: enter: add attr monitor fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_ctrl_turntable_exit(ui_sprite_component_t component, void * ctx) {
}

static int ui_sprite_ctrl_turntable_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_ctrl_turntable_t ctrl = ui_sprite_component_data(component);

    bzero(ctrl, sizeof(*ctrl));

    ctrl->m_module = ctx;
    TAILQ_INIT(&ctrl->m_members);

    return 0;
}

static void ui_sprite_ctrl_turntable_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_ctrl_turntable_t ctrl = ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&ctrl->m_members)) {
        ui_sprite_ctrl_turntable_remove_member(ctrl, TAILQ_FIRST(&ctrl->m_members));
    }
}

static int ui_sprite_ctrl_turntable_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_t to_ctrl = ui_sprite_component_data(to);
    ui_sprite_ctrl_turntable_t from_ctrl = ui_sprite_component_data(from);
    ui_sprite_entity_t to_entity = ui_sprite_component_entity(to);

    if (ui_sprite_ctrl_turntable_init(to, ctx)) return -1;

    if (ui_sprite_ctrl_turntable_set_def(to_ctrl, &from_ctrl->m_def) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: copy: set def fail!",
            ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity));
        return -1;
    }

    return 0;
}

uint32_t ui_sprite_ctrl_turntable_start_op(ui_sprite_ctrl_turntable_t turntable) {
    turntable->m_curent_op_id = ++turntable->m_max_op_id;
    return turntable->m_curent_op_id;
}

void ui_sprite_ctrl_turntable_stop_op(ui_sprite_ctrl_turntable_t turntable, uint32_t op_id) {
    if (turntable->m_curent_op_id == op_id) {
        turntable->m_curent_op_id = 0;
    }
}

int ui_sprite_ctrl_turntable_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(module->m_repo, UI_SPRITE_CTRL_TURNTABLE_NAME, sizeof(struct ui_sprite_ctrl_turntable));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: tuntable component register: meta create fail",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_ctrl_turntable_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_ctrl_turntable_exit, module);
    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_ctrl_turntable_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_ctrl_turntable_copy, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_ctrl_turntable_fini, module);

    ui_sprite_component_meta_set_data_meta(
        meta,
        module->m_meta_turntable_data,
        CPE_ENTRY_START(ui_sprite_ctrl_turntable, m_data),
        CPE_ENTRY_SIZE(ui_sprite_ctrl_turntable, m_data));

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_comp_loader(
                module->m_loader, UI_SPRITE_CTRL_TURNTABLE_NAME, ui_sprite_ctrl_turntable_load, module)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: %s register: register loader fail",
                ui_sprite_ctrl_module_name(module), UI_SPRITE_CTRL_TURNTABLE_NAME);
            ui_sprite_component_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_ctrl_turntable_unregist(ui_sprite_ctrl_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_CTRL_TURNTABLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl unregister: meta not exist",
            ui_sprite_ctrl_module_name(module));
        return;
    }

    ui_sprite_component_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_NAME);
    }
}

const char * UI_SPRITE_CTRL_TURNTABLE_NAME = "Turntable";

