#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_with_collision_src_i.h"
#include "ui_sprite_chipmunk_with_collision_i.h"
#include "ui_sprite_chipmunk_env_i.h"

ui_sprite_chipmunk_with_collision_src_t
ui_sprite_chipmunk_with_collision_src_create(ui_sprite_chipmunk_with_collision_t with_collision) {
    ui_sprite_chipmunk_with_collision_src_t src =
        (ui_sprite_chipmunk_with_collision_src_t)mem_calloc(
            with_collision->m_module->m_alloc, sizeof(struct ui_sprite_chipmunk_with_collision_src));
    if (src == NULL) return NULL;

    src->m_with_collision = with_collision;
    TAILQ_INIT(&src->m_shapes);

    TAILQ_INSERT_TAIL(&with_collision->m_srcs, src, m_next);

    return src;
}

void ui_sprite_chipmunk_with_collision_src_free(ui_sprite_chipmunk_with_collision_src_t src) {
    ui_sprite_chipmunk_with_collision_t with_collision = src->m_with_collision;
    
    while(!TAILQ_EMPTY(&src->m_shapes)) {
        ui_sprite_chipmunk_with_collision_shape_free(TAILQ_FIRST(&src->m_shapes));
    }

    TAILQ_REMOVE(&with_collision->m_srcs, src, m_next);
    mem_free(with_collision->m_module->m_alloc, src);
}

int ui_sprite_chipmunk_with_collision_src_set_load_from(ui_sprite_chipmunk_with_collision_src_t src, const char * load_from) {
    cpe_str_dup(src->m_res, sizeof(src->m_res), load_from);
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_name(ui_sprite_chipmunk_with_collision_src_t src, const char * name) {
    cpe_str_dup(src->m_name, sizeof(src->m_name), name);
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_is_main(ui_sprite_chipmunk_with_collision_src_t src, uint8_t is_main) {
    src->m_is_main = is_main;
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_category(ui_sprite_chipmunk_with_collision_src_t src, const char * category) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(src->m_with_collision);
    ui_sprite_world_t world = ui_sprite_fsm_action_to_world(fsm_action);
    ui_sprite_chipmunk_env_t env = ui_sprite_chipmunk_env_find(world);
    ui_sprite_chipmunk_module_t module = env->m_module;
    
    if (plugin_chipmunk_env_masks(env->m_env, &src->m_body_attrs.m_category, category) != 0) {
        CPE_ERROR(
            module->m_em, "%s: ui_sprite_chipmunk_with_collision_src_set_category: category %s load fail!",
            ui_sprite_chipmunk_module_name(module), category);
        return -1;
    }
    src->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_CATEGORY;
    
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_mask(ui_sprite_chipmunk_with_collision_src_t src, const char * mask) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(src->m_with_collision);
    ui_sprite_world_t world = ui_sprite_fsm_action_to_world(fsm_action);
    ui_sprite_chipmunk_env_t env = ui_sprite_chipmunk_env_find(world);
    ui_sprite_chipmunk_module_t module = env->m_module;
    
    if (plugin_chipmunk_env_masks(env->m_env, &src->m_body_attrs.m_mask, mask) != 0) {
        CPE_ERROR(
            module->m_em, "%s: ui_sprite_chipmunk_with_collision_src_set_mask: mask %s load fail!",
            ui_sprite_chipmunk_module_name(module), mask);
        return -1;
    }
    src->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MASK;
    
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_group(ui_sprite_chipmunk_with_collision_src_t src, uint32_t group) {
    src->m_body_attrs.m_group = group;
    src->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_GROUP;
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_mass(ui_sprite_chipmunk_with_collision_src_t src, float mass) {
    src->m_body_attrs.m_mass = mass;
    src->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MASS;
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_moment(ui_sprite_chipmunk_with_collision_src_t src, float moment) {
    src->m_body_attrs.m_moment = moment;
    src->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MOMENT;
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_is_free(ui_sprite_chipmunk_with_collision_src_t src, uint8_t is_free) {
    src->m_body_attrs.m_is_free = is_free;
    return 0;
}

int ui_sprite_chipmunk_with_collision_src_set_type(ui_sprite_chipmunk_with_collision_src_t src, chipmunk_obj_type_t obj_type) {
    src->m_body_attrs.m_type = obj_type;
    src->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_TYPE;
    return 0;
}
    
int ui_sprite_chipmunk_with_collision_src_set_runing_mode(ui_sprite_chipmunk_with_collision_src_t src, ui_sprite_chipmunk_runing_mode_t runing_mode) {
    src->m_body_attrs.m_runing_mode = runing_mode;
    return 0;
}
    
int ui_sprite_chipmunk_with_collision_src_set_gravity(ui_sprite_chipmunk_with_collision_src_t src, UI_SPRITE_CHIPMUNK_GRAVITY * gravity) {
    src->m_body_attrs.m_gravity = *gravity;
    return 0;
}
