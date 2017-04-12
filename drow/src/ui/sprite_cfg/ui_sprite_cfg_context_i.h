#ifndef UI_SPRITE_CFG_CONTEXT_I_H
#define UI_SPRITE_CFG_CONTEXT_I_H
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_context.h"
#include "ui_sprite_cfg_loader_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_cfg_context {
    ui_sprite_cfg_loader_t m_loader;
    cfg_t m_cfg;
    TAILQ_ENTRY(ui_sprite_cfg_context) m_next_for_loader;
};

void ui_sprite_cfg_context_free_all(ui_sprite_cfg_loader_t loader);

#ifdef __cplusplus
}
#endif

#endif
