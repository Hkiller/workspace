#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/random.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/spine/plugin_spine_obj_part_state.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_control_entity_i.h"
#include "ui_sprite_spine_controled_obj_i.h"

ui_sprite_spine_control_entity_slot_t
ui_sprite_spine_control_entity_slot_create(
    ui_sprite_spine_control_entity_t control_entity, struct spSlot * slot,
    enum ui_sprite_spine_control_entity_slot_mode mode,
    const char * name)
{
    ui_sprite_spine_module_t module = control_entity->m_module;
    ui_sprite_spine_control_entity_slot_t entity_slot;

    entity_slot = TAILQ_FIRST(&module->m_free_control_entity_slots);
    if (entity_slot) {
        TAILQ_REMOVE(&module->m_free_control_entity_slots, entity_slot, m_next);
    }
    else {
        entity_slot = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_spine_control_entity_slot));
        if (slot == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_control_entity_slot_create: alloc fail!");
            return NULL;
        }
    }

    entity_slot->m_owner = control_entity;
    entity_slot->m_slot = slot;
    entity_slot->m_mode = mode;
    entity_slot->m_is_active = 0;
    cpe_str_dup(entity_slot->m_name, sizeof(entity_slot->m_name), name);
    entity_slot->m_setups = NULL;
    TAILQ_INIT(&entity_slot->m_controled_objs);
    
    /* switch(entity_slot->m_mode) { */
    /* case ui_sprite_spine_control_entity_slot_mode_attack: */
    /* case ui_sprite_spine_control_entity_slot_mode_bind: */
    /* case ui_sprite_spine_control_entity_slot_mode_fush: */
    /* } */
    
    TAILQ_INSERT_TAIL(&control_entity->m_slots, entity_slot, m_next);

    return entity_slot;
}

void ui_sprite_spine_control_entity_slot_free(ui_sprite_spine_control_entity_slot_t slot) {
    ui_sprite_spine_control_entity_t control_entity = slot->m_owner;
    ui_sprite_spine_module_t module = control_entity->m_module;

    while(!TAILQ_EMPTY(&slot->m_controled_objs)) {
        ui_sprite_spine_controled_obj_set_slot(TAILQ_FIRST(&slot->m_controled_objs), NULL);
    }

    if (slot->m_setups) {
        mem_free(module->m_alloc, slot->m_setups);
        slot->m_setups = NULL;
    }
    
    TAILQ_REMOVE(&control_entity->m_slots, slot, m_next);

    slot->m_owner = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_control_entity_slots, slot, m_next);
}

void ui_sprite_spine_control_entity_slot_real_free(ui_sprite_spine_control_entity_slot_t slot) {
    ui_sprite_spine_module_t module = (void*)slot->m_owner;

    TAILQ_REMOVE(&module->m_free_control_entity_slots, slot, m_next);

    mem_free(module->m_alloc, slot);
}

static void ui_sprite_spine_control_entity_slot_update_entity(
    ui_sprite_spine_module_t module, ui_sprite_entity_t entity,
    ui_sprite_entity_t control_entity, ui_sprite_spine_controled_obj_t controled_obj, ui_transform_t transform)
{
    if (controled_obj == NULL) {
        assert(control_entity);
        controled_obj = ui_sprite_spine_controled_obj_find(control_entity);
        if (controled_obj == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): controled entity %d(%s) no controled obj",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(control_entity), ui_sprite_entity_name(control_entity));
            return;
        }
    }

    if (controled_obj->m_is_binding) {
        ui_sprite_2d_transform_t p2d_transform;
        
        if (control_entity == NULL) {
            control_entity = ui_sprite_component_entity(ui_sprite_component_from_data(controled_obj));
        }
        
        p2d_transform = ui_sprite_2d_transform_find(control_entity);
        if (p2d_transform) {
            ui_sprite_2d_transform_set_trans(p2d_transform, transform);
        }
    }
}

static ui_sprite_spine_controled_obj_t
ui_sprite_spine_control_entity_slot_attach_entity(
    ui_sprite_spine_module_t module, ui_sprite_entity_t entity,
    ui_sprite_entity_t controled_entity, ui_sprite_spine_control_entity_slot_t slot)
{
    ui_sprite_spine_controled_obj_t controled_obj;
    
    controled_obj = ui_sprite_spine_controled_obj_find(controled_entity);
    if (controled_obj == NULL) {
        controled_obj = ui_sprite_spine_controled_obj_create(controled_entity);
        if (controled_obj == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): attach entity %d(%s) create controled obj fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_id(controled_entity), ui_sprite_entity_name(controled_entity));
            return NULL;
        }
    }

    ui_sprite_spine_controled_obj_set_slot(controled_obj, slot);
    
    return controled_obj;
}

static int ui_sprite_spine_control_entity_slot_setup_entity(ui_sprite_spine_module_t module, ui_sprite_entity_t entity, char * def) {
    char * name_end;
    char * value;
    char name_end_v;
    plugin_spine_obj_part_t part;
    
    name_end = strchr(def, '=');
    if (name_end == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): control entity: create entity: setup: def %s format error",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
        return -1;
    }

    value = cpe_str_trim_head(name_end + 1);

    name_end = cpe_str_trim_tail(name_end, def);
    name_end_v = *name_end;
    *name_end = 0;

    part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, def, module->m_em);
    if (part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): control entity: create entity: setup: part %s not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def);
        *name_end = name_end_v;
        return -1;
    }

    if (value[0]) {
        if (plugin_spine_obj_part_apply_transition_force(part, value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): control entity: create entity: setup: part %s ==> %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def, value);
            *name_end = name_end_v;
            return -1;
        }
    }
    else {
        uint32_t state_count = plugin_spine_obj_part_state_count(part);
        if (state_count > 0) {
            struct plugin_spine_obj_part_state_it it;
            uint8_t idx = cpe_rand_dft(state_count);
            plugin_spine_obj_part_state_t state;
            
            plugin_spine_obj_part_states(part, &it);

            while(idx > 0) {
                plugin_spine_obj_part_state_it_next(&it);
                --idx;
            }

            state = plugin_spine_obj_part_state_it_next(&it);
            assert(state);

            if (plugin_spine_obj_part_set_cur_state(part, state) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): control entity: create entity: setup: part %s rand state to %s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), def, plugin_spine_obj_part_state_name(state));
                *name_end = name_end_v;
                return -1;
            }
        }
    }
    
    *name_end = name_end_v;
    
    return 0;
}

static void ui_sprite_spine_control_entity_slot_create_entity(
    ui_sprite_spine_module_t module, ui_sprite_entity_t entity, ui_sprite_spine_control_entity_slot_t slot,
    ui_transform_t transform, uint8_t bind)
{
    ui_sprite_entity_t controled_entity;

    controled_entity = ui_sprite_entity_create(ui_sprite_entity_world(entity), "", slot->m_name);
    if (controled_entity == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): control entity: create entity from proto %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->m_name);
        return;
    }
    
    if (bind) {
        ui_sprite_spine_controled_obj_t controled_obj =
            ui_sprite_spine_control_entity_slot_attach_entity(module, entity, controled_entity, slot);
        if (controled_obj == NULL) {
            ui_sprite_entity_free(controled_entity);
            return;
        }

        if (transform) {
            ui_sprite_spine_control_entity_slot_update_entity(module, entity, controled_entity, controled_obj, transform);
        }
    }    
    else {
        if (transform) {
            ui_sprite_spine_control_entity_slot_update_entity(module, entity, controled_entity, NULL, transform);
        }
    }

    if (ui_sprite_entity_enter(controled_entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): control entity: create entity from proto %s: enter fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->m_name);
        ui_sprite_entity_free(controled_entity);
        return;
    }

    if (slot->m_setups) {
        char * v = slot->m_setups;
        char * s;
        
        while((s = strchr(v, ','))) {
            *s = 0;
            if (ui_sprite_spine_control_entity_slot_setup_entity(module, controled_entity, v) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): control entity: setup from %s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), v);
                *s = ',';
                ui_sprite_entity_free(controled_entity);
                return;
            }
            
            *s = ',';
            v = cpe_str_trim_head(s + 1);
        }

        if (v[0]) {
            if (ui_sprite_spine_control_entity_slot_setup_entity(module, controled_entity, v) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): control entity: setup from %s fail",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), v);
                ui_sprite_entity_free(controled_entity);
                return;
            }
        }
    }
}

static void ui_sprite_spine_control_entity_slot_attach_target(
    ui_sprite_spine_module_t module, ui_sprite_entity_t entity, ui_sprite_spine_control_entity_slot_t slot)
{
    if (slot->m_name[0] == '*') {
        ui_sprite_group_t group;
        ui_sprite_entity_it_t entity_it;
        ui_sprite_entity_t target;

        group = ui_sprite_group_find_by_name(ui_sprite_entity_world(entity), slot->m_name + 1);
        if (group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): attach entity group %s not exist",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->m_name + 1);
            return;
        }

        entity_it = ui_sprite_group_entities(module->m_alloc, group);
        while((target = ui_sprite_entity_it_next(entity_it))) {
            //printf("xxxxxx: process target %s\n", ui_sprite_entity_name(target));
            ui_sprite_spine_control_entity_slot_attach_entity(module, entity, target, slot);
        }
    }
    else {
        ui_sprite_entity_t target;

        target = ui_sprite_entity_find_by_name(ui_sprite_entity_world(entity), slot->m_name);
        if (target) {
            ui_sprite_spine_control_entity_slot_attach_entity(module, entity, target, slot);
        }
    }
}

void ui_sprite_spine_control_entity_slot_update(
    ui_sprite_entity_t entity, ui_sprite_spine_control_entity_slot_t slot, ui_transform_t anim_transform)
{
    ui_sprite_spine_module_t module = slot->m_owner->m_module;
    uint8_t is_active;
    ui_transform bone_transform;
    uint8_t is_transform_init = 0;
    
    is_active = slot->m_slot->attachment ? 1 : 0;
    if (slot->m_is_active != is_active) {
        slot->m_is_active = is_active;
        
        switch(slot->m_mode) {
        case ui_sprite_spine_control_entity_slot_mode_attach:
            if (is_active) {
                ui_sprite_spine_control_entity_slot_attach_target(module, entity, slot);
            }
            else {
                ui_sprite_spine_controled_obj_t controled_obj;

                TAILQ_FOREACH(controled_obj, &slot->m_controled_objs, m_next) {
                    ui_sprite_entity_t check =
                        ui_sprite_component_entity(
                            ui_sprite_component_from_data(controled_obj));
                    if (strcmp(ui_sprite_entity_name(check), slot->m_name) == 0) {
                        ui_sprite_spine_controled_obj_set_slot(controled_obj, NULL);
                        break;
                    }
                }                
            }
            break;
        case ui_sprite_spine_control_entity_slot_mode_bind:
            if (is_active) {
                ui_sprite_spine_control_entity_slot_create_entity(module, entity, slot, NULL, 1);
            }
            else {
                while(!TAILQ_EMPTY(&slot->m_controled_objs)) {
                    ui_sprite_entity_t bind_entity =
                        ui_sprite_component_entity(
                            ui_sprite_component_from_data(
                                TAILQ_FIRST(&slot->m_controled_objs)));
                    ui_sprite_entity_set_destory(bind_entity);
                }
            }
            break;
        case ui_sprite_spine_control_entity_slot_mode_flush:
            if (is_active) {
                assert(!is_transform_init);
                plugin_spine_bone_calc_transform(slot->m_slot->bone, &bone_transform);
                if (anim_transform) ui_transform_adj_by_parent(&bone_transform, anim_transform);
                is_transform_init = 1;
                
                ui_sprite_spine_control_entity_slot_create_entity(module, entity, slot, &bone_transform, 0);
            }
            break;
        }
    }

    if (is_active && slot->m_mode == ui_sprite_spine_control_entity_slot_mode_attach) {
        if (slot->m_name[0] == '*' || TAILQ_EMPTY(&slot->m_controled_objs)) {
            ui_sprite_spine_control_entity_slot_attach_target(module, entity, slot);
        }
    }

    if (is_active && !TAILQ_EMPTY(&slot->m_controled_objs)) {
        ui_sprite_spine_controled_obj_t controled_obj;
        
        if (!is_transform_init) {
            if (plugin_spine_bone_calc_transform(slot->m_slot->bone, &bone_transform) == 0) {
                if (anim_transform) ui_transform_adj_by_parent(&bone_transform, anim_transform);
                is_transform_init = 1;
            }
        }

        if (is_transform_init) {
            TAILQ_FOREACH(controled_obj, &slot->m_controled_objs, m_next) {
                ui_sprite_spine_control_entity_slot_update_entity(
                    module, entity, NULL, controled_obj, &bone_transform);
            }
        }
    }
}
