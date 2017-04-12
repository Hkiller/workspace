#include "ui_sprite_chipmunk_obj_updator_i.h"

ui_sprite_chipmunk_obj_updator_t
ui_sprite_chipmunk_obj_updator_create(
    ui_sprite_chipmunk_obj_t obj,
    ui_sprite_chipmunk_obj_updateor_update_fun_t update_fun,
    ui_sprite_chipmunk_obj_updateor_clean_fun_t clean_fun,
    size_t data_capacity)
{
    ui_sprite_chipmunk_module_t module = obj->m_env->m_module;
    ui_sprite_chipmunk_obj_updator_t updator;

    updator = (ui_sprite_chipmunk_obj_updator_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_updator) + data_capacity);
    if (updator == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_updator_create: alloc fail!");
        return NULL;
    }

    updator->m_obj = obj;
    updator->m_update_fun = update_fun;
    updator->m_clean_fun = clean_fun;
    updator->m_data_capacity = data_capacity;

    TAILQ_INSERT_TAIL(&obj->m_updators, updator, m_next_for_obj);
    
    return updator;
}
    
void ui_sprite_chipmunk_obj_updator_free(ui_sprite_chipmunk_obj_updator_t updator) {
    ui_sprite_chipmunk_obj_t obj = updator->m_obj;
    ui_sprite_chipmunk_module_t module = obj->m_env->m_module;

    if (updator->m_clean_fun) updator->m_clean_fun(updator);

    TAILQ_REMOVE(&obj->m_updators, updator, m_next_for_obj);

    mem_free(module->m_alloc, updator);
}

void * ui_sprite_chipmunk_obj_updator_data(ui_sprite_chipmunk_obj_updator_t updator) {
    return updator + 1;
}

