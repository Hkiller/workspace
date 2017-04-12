#ifndef UI_SPRITE_CFG_CONTEXT_H
#define UI_SPRITE_CFG_CONTEXT_H
#include "ui_sprite_cfg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_cfg_context_t ui_sprite_cfg_context_create(ui_sprite_cfg_loader_t, cfg_t cfg);
void ui_sprite_cfg_context_free(ui_sprite_cfg_context_t context);
    
#ifdef __cplusplus
}
#endif

#endif
