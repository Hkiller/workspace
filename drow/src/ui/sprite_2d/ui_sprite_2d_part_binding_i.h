#ifndef UI_SPRITE_2D_PART_BINDING_I_H
#define UI_SPRITE_2D_PART_BINDING_I_H
#include "ui/sprite_2d/ui_sprite_2d_part_binding.h"
#include "ui_sprite_2d_part_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_part_binding {
    ui_sprite_2d_part_t m_part;
    TAILQ_ENTRY(ui_sprite_2d_part_binding) m_next;
    void * m_ctx;
    ui_sprite_2d_part_on_updated_fun_t m_on_updated;
};

void ui_sprite_2d_part_binding_real_free(
    ui_sprite_2d_module_t module, ui_sprite_2d_part_binding_t emitter);


#ifdef __cplusplus
}
#endif

#endif
