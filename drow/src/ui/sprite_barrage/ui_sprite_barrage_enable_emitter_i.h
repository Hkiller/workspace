#ifndef UI_SPRITE_BARRAGE_ENABLE_EMITTER_I_H
#define UI_SPRITE_BARRAGE_ENABLE_EMITTER_I_H
#include "ui/sprite_barrage/ui_sprite_barrage_enable_emitter.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_barrage_enable_emitter {
    ui_sprite_barrage_module_t m_module;
    char * m_cfg_group_name;
    char * m_cfg_collision_event;
    uint8_t m_cfg_destory_bullets;
    uint32_t m_cfg_loop_count;
    char * m_group_name;
};

int ui_sprite_barrage_enable_emitter_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_enable_emitter_unregist(ui_sprite_barrage_module_t module);

#ifdef __cplusplus
}
#endif

#endif
