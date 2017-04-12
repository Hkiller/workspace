#ifndef UI_SPRITE_TOUCH_TOUCHABLE_I_H
#define UI_SPRITE_TOUCH_TOUCHABLE_I_H
#include "ui/sprite_touch/ui_sprite_touch_touchable.h"
#include "ui_sprite_touch_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_touchable {
    ui_sprite_touch_env_t m_env;
    float m_z;
    ui_sprite_touch_box_list_t m_boxes;
    ui_sprite_touch_responser_list_t m_responsers;
};

int ui_sprite_touch_touchable_regist(ui_sprite_touch_mgr_t module);
void ui_sprite_touch_touchable_unregist(ui_sprite_touch_mgr_t module);
    
int ui_sprite_touch_touchable_load(void * ctx, ui_sprite_component_t component, cfg_t cfg);
    
int ui_sprite_touch_touchable_is_point_in(ui_sprite_touch_touchable_t touchable, ui_vector_2 world_pt);

#ifdef __cplusplus
}
#endif

#endif
