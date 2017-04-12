#ifndef UI_SPRITE_MOVING_OBJ_I_H
#define UI_SPRITE_MOVING_OBJ_I_H
#include "ui/sprite_moving/ui_sprite_moving_obj.h"
#include "ui_sprite_moving_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_moving_obj {
    ui_sprite_moving_module_t m_module;
    ui_sprite_moving_obj_node_list_t m_node_stack;
    uint8_t m_is_suspend;
    float m_time_scale;
};

struct ui_sprite_moving_obj_node {
    TAILQ_ENTRY(ui_sprite_moving_obj_node) m_next;
    plugin_moving_node_t m_moving_node;
    void * m_ctx;
    ui_sprite_moving_obj_node_destory_fun_t m_destory;
};
        
int ui_sprite_moving_obj_regist(ui_sprite_moving_module_t module);
void ui_sprite_moving_obj_unregist(ui_sprite_moving_module_t module);

ui_sprite_moving_obj_node_t ui_sprite_moving_obj_node_create(ui_sprite_moving_obj_t obj);
void ui_sprite_moving_obj_node_free(ui_sprite_moving_obj_t obj, ui_sprite_moving_obj_node_t obj_node);
void ui_sprite_moving_obj_node_real_free(ui_sprite_moving_module_t module, ui_sprite_moving_obj_node_t obj_node);
    
#ifdef __cplusplus
}
#endif

#endif
