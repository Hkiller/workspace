#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_collision_i.h"
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_collision_shape_t
ui_sprite_chipmunk_with_collision_shape_create(ui_sprite_chipmunk_with_collision_src_t src) {
    ui_sprite_chipmunk_module_t module = src->m_with_collision->m_module;
    
    ui_sprite_chipmunk_with_collision_shape_t shape =
        (ui_sprite_chipmunk_with_collision_shape_t)mem_calloc(
            module->m_alloc, sizeof(struct ui_sprite_chipmunk_with_collision_shape));
    if (shape == NULL) return NULL;

    shape->m_src = src;
    TAILQ_INSERT_TAIL(&src->m_shapes, shape, m_next);

    return shape;
}

ui_sprite_chipmunk_with_collision_shape_t
ui_sprite_chipmunk_with_collision_shape_clone(
    ui_sprite_chipmunk_with_collision_src_t src, ui_sprite_chipmunk_with_collision_shape_t from)
{
    ui_sprite_chipmunk_with_collision_shape_t shape = ui_sprite_chipmunk_with_collision_shape_create(src);
    if (shape == NULL) return NULL;

    shape->m_data = from->m_data;

    if (ui_sprite_chipmunk_with_collision_shape_set_density(shape, from->m_density) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_mass(shape, from->m_mass) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_elasticity(shape, from->m_elasticity) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_friction(shape, from->m_friction) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_collision_group(shape, from->m_collision_group) != 0
        || ui_sprite_chipmunk_with_collision_shape_set_shape_def(shape, from->m_shape_def) != 0
        )
    {
        ui_sprite_chipmunk_with_collision_shape_free(shape);
        return NULL;
    }
    
    return shape;
}

void ui_sprite_chipmunk_with_collision_shape_free(ui_sprite_chipmunk_with_collision_shape_t shape) {
    ui_sprite_chipmunk_with_collision_src_t src = shape->m_src;
    ui_sprite_chipmunk_module_t module = src->m_with_collision->m_module;

    TAILQ_REMOVE(&src->m_shapes, shape, m_next);

    if (shape->m_density) mem_free(module->m_alloc, shape->m_density);
    if (shape->m_mass) mem_free(module->m_alloc, shape->m_mass);
    if (shape->m_elasticity) mem_free(module->m_alloc, shape->m_elasticity);
    if (shape->m_friction) mem_free(module->m_alloc, shape->m_friction);
    if (shape->m_collision_group) mem_free(module->m_alloc, shape->m_collision_group);
    if (shape->m_shape_def) mem_free(module->m_alloc, shape->m_shape_def);
    
    mem_free(module->m_alloc, shape);
}

int ui_sprite_chipmunk_with_collision_shape_set_density(ui_sprite_chipmunk_with_collision_shape_t shape, const char * density) {
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    
    if (shape->m_density) mem_free(module->m_alloc, shape->m_density);

    if (density) {
        ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
        shape->m_density = cpe_str_mem_dup(module->m_alloc, density);
        return shape->m_density ? 0 : -1;
    }
    else {
        shape->m_density = NULL;
        return 0;
    }
}

int ui_sprite_chipmunk_with_collision_shape_set_mass(ui_sprite_chipmunk_with_collision_shape_t shape, const char * mass) {
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    
    if (shape->m_mass) mem_free(module->m_alloc, shape->m_mass);

    if (mass) {
        ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
        shape->m_mass = cpe_str_mem_dup(module->m_alloc, mass);
        return shape->m_mass ? 0 : -1;
    }
    else {
        shape->m_mass = NULL;
        return 0;
    }
}

int ui_sprite_chipmunk_with_collision_shape_set_elasticity(ui_sprite_chipmunk_with_collision_shape_t shape, const char * elasticity) {
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    
    if (shape->m_elasticity) mem_free(module->m_alloc, shape->m_elasticity);

    if (elasticity) {
        ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
        shape->m_elasticity = cpe_str_mem_dup(module->m_alloc, elasticity);
        return shape->m_elasticity ? 0 : -1;
    }
    else {
        shape->m_elasticity = NULL;
        return 0;
    }
}

int ui_sprite_chipmunk_with_collision_shape_set_friction(ui_sprite_chipmunk_with_collision_shape_t shape, const char * friction) {
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    
    if (shape->m_friction) mem_free(module->m_alloc, shape->m_friction);

    if (friction) {
        ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
        shape->m_friction = cpe_str_mem_dup(module->m_alloc, friction);
        return shape->m_friction ? 0 : -1;
    }
    else {
        shape->m_friction = NULL;
        return 0;
    }
}

int ui_sprite_chipmunk_with_collision_shape_set_collision_group(ui_sprite_chipmunk_with_collision_shape_t shape, const char * collision_group) {
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    
    if (shape->m_collision_group) mem_free(module->m_alloc, shape->m_collision_group);

    if (collision_group) {
        ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
        shape->m_collision_group = cpe_str_mem_dup(module->m_alloc, collision_group);
        return shape->m_collision_group ? 0 : -1;
    }
    else {
        shape->m_collision_group = NULL;
        return 0;
    }
}

int ui_sprite_chipmunk_with_collision_shape_set_shape_def(ui_sprite_chipmunk_with_collision_shape_t shape, const char * shape_def) {
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    
    if (shape->m_shape_def) mem_free(module->m_alloc, shape->m_shape_def);

    if (shape_def) {
        ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
        shape->m_shape_def = cpe_str_mem_dup_trim(module->m_alloc, shape_def);
        return shape->m_shape_def ? 0 : -1;
    }
    else {
        shape->m_shape_def = NULL;
        return 0;
    }
}
    
int ui_sprite_chipmunk_with_collision_shape_calc_data(
    ui_sprite_chipmunk_with_collision_shape_t shape, CHIPMUNK_FIXTURE * data)
{
    ui_sprite_chipmunk_module_t module = shape->m_src->m_with_collision->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(shape->m_src->m_with_collision);
    
    *data = shape->m_data;

    if (shape->m_mass && ui_sprite_fsm_action_check_calc_float(&data->mass, shape->m_mass, action, NULL, module->m_em) != 0) return -1;
    if (shape->m_density && ui_sprite_fsm_action_check_calc_float(&data->density, shape->m_density, action, NULL, module->m_em) != 0) return -1;
    if (shape->m_elasticity && ui_sprite_fsm_action_check_calc_float(&data->elasticity, shape->m_elasticity, action, NULL, module->m_em) != 0) return -1;
    if (shape->m_friction && ui_sprite_fsm_action_check_calc_float(&data->friction, shape->m_friction, action, NULL, module->m_em) != 0) return -1;
    if (shape->m_collision_group
        && ui_sprite_fsm_action_check_calc_uint32(&data->collision_group, shape->m_collision_group, action, NULL, module->m_em)
        != 0) return -1;

    if (shape->m_shape_def) {
        const char * shape_def =
            ui_sprite_fsm_action_check_calc_str(
                &module->m_dump_buffer, shape->m_shape_def, action, NULL, module->m_em);
        if (shape_def == NULL) {
            CPE_ERROR(
                module->m_em, "%s: with_collision_action: calc shape data: calc shape def from %s fail!",
                ui_sprite_chipmunk_module_name(module), shape->m_shape_def);
            return -1;
        }

        if (ui_sprite_chipmunk_load_fixture_shape_from_str(module, data, shape_def) != 0) {
            CPE_ERROR(
                module->m_em, "%s: with_collision_action: calc shape data: load shape from str %s fail!",
                ui_sprite_chipmunk_module_name(module), shape_def);
            return -1;
        }
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
