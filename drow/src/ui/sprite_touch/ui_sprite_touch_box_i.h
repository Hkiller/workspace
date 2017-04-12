#ifndef UI_SPRITE_TOUCH_BOX_I_H
#define UI_SPRITE_TOUCH_BOX_I_H
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_touch/ui_sprite_touch_box.h"
#include "ui_sprite_touch_touchable_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_box {
    ui_sprite_touch_touchable_t m_touchable;
    UI_SPRITE_TOUCH_SHAPE m_shape;
    uint32_t m_box_anim_id;

    TAILQ_ENTRY(ui_sprite_touch_box) m_next_for_touchable;
};

#ifdef __cplusplus
}
#endif

#endif
