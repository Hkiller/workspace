#include "ui_sprite_moving_obj_i.h"

ui_sprite_moving_obj_node_t ui_sprite_moving_obj_node_create(ui_sprite_moving_obj_t obj) {
    ui_sprite_moving_obj_node_t obj_node;

    obj_node = TAILQ_FIRST(&obj->m_module->m_free_moving_obj_nodes);
    if (obj_node) {
        TAILQ_REMOVE(&obj->m_module->m_free_moving_obj_nodes, obj_node, m_next);
    }
    else {
        obj_node = mem_alloc(obj->m_module->m_alloc, sizeof(struct ui_sprite_moving_obj_node));
        if (obj_node == NULL) {
            CPE_ERROR(obj->m_module->m_em, "ui_sprite_moving_obj_node_create: alloc fail");
            return NULL;
        }
    }

    TAILQ_INSERT_TAIL(&obj->m_node_stack, obj_node, m_next);
    
    return obj_node;
}

void ui_sprite_moving_obj_node_free(ui_sprite_moving_obj_t obj, ui_sprite_moving_obj_node_t obj_node) {
    TAILQ_REMOVE(&obj->m_node_stack, obj_node, m_next);
    TAILQ_INSERT_TAIL(&obj->m_module->m_free_moving_obj_nodes, obj_node, m_next);
}

void ui_sprite_moving_obj_node_real_free(ui_sprite_moving_module_t module, ui_sprite_moving_obj_node_t obj_node) {
    TAILQ_REMOVE(&module->m_free_moving_obj_nodes, obj_node, m_next);
    mem_free(module->m_alloc, obj_node);
}
