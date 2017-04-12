#include <assert.h>
#include "ui_sprite_chipmunk_obj_runtime_group_i.h"

ui_sprite_chipmunk_obj_runtime_group_t
ui_sprite_chipmunk_obj_runtime_group_create(
    ui_sprite_chipmunk_obj_runtime_group_list_t * owner, ui_sprite_chipmunk_obj_body_t body, uint32_t group_id)
{
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_runtime_group_t group;

    group = (ui_sprite_chipmunk_obj_runtime_group_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_runtime_group));
    if (group == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_runtime_group_create: alloc fail!");
        return NULL;
    }

    group->m_body = (ui_sprite_chipmunk_obj_t)body;
    group->m_group_id = group_id;
    TAILQ_INSERT_HEAD(&body->m_runtime_groups, group, m_next_for_body);
    TAILQ_INSERT_TAIL(owner, group, m_next_for_owner);

    ui_sprite_chipmunk_obj_body_update_collision(body);

    return group;
}

void ui_sprite_chipmunk_obj_runtime_group_free(
    ui_sprite_chipmunk_module_t module,
    ui_sprite_chipmunk_obj_runtime_group_list_t * owner, ui_sprite_chipmunk_obj_runtime_group_t group)
{
    if (group->m_body) {
        ui_sprite_chipmunk_obj_runtime_group_unbind(group, (ui_sprite_chipmunk_obj_body_t)group->m_body);
        assert(group->m_body == NULL);
    }

    TAILQ_REMOVE(owner, group, m_next_for_owner);

    mem_free(module->m_alloc, group);
}

void ui_sprite_chipmunk_obj_runtime_group_unbind(ui_sprite_chipmunk_obj_runtime_group_t group, ui_sprite_chipmunk_obj_body_t body) {
    assert(group->m_body == (ui_sprite_chipmunk_obj_t)body);
    uint8_t need_update = 0;

    if (TAILQ_FIRST(&body->m_runtime_groups) == group) {
        need_update = 1;
    }
    
    TAILQ_REMOVE(&body->m_runtime_groups, group, m_next_for_body);
    group->m_body = NULL;

    if (need_update) {
        ui_sprite_chipmunk_obj_body_update_collision(body);
    }
}

