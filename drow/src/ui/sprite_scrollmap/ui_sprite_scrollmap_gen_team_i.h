#ifndef UI_SPRITE_SCROLLMAP_GEN_TEAM_I_H
#define UI_SPRITE_SCROLLMAP_GEN_TEAM_I_H
#include "ui/sprite_scrollmap/ui_sprite_scrollmap_gen_team.h"
#include "ui_sprite_scrollmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_scrollmap_gen_team {
    ui_sprite_scrollmap_module_t m_module;
    char * m_cfg_layer;
    char * m_cfg_res;
    char * m_layer;
    uint16_t m_team_id;
};

int ui_sprite_scrollmap_gen_team_regist(ui_sprite_scrollmap_module_t module);
void ui_sprite_scrollmap_gen_team_unregist(ui_sprite_scrollmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif
