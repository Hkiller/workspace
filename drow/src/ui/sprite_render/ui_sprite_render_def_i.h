#ifndef UI_SPRITE_RENDER_DEF_I_H
#define UI_SPRITE_RENDER_DEF_I_H
#include "ui/sprite_render/ui_sprite_render_def.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_def {
    ui_sprite_render_sch_t m_sch;
    uint8_t m_auto_start;
    const char * m_anim_name;
    const char * m_anim_res;

    TAILQ_ENTRY(ui_sprite_render_def) m_next_for_sch;
};

#ifdef __cplusplus
}
#endif

#endif
