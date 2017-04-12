#ifndef UI_SPRITE_CTRL_TRACK_MGR_I_H
#define UI_SPRITE_CTRL_TRACK_MGR_I_H
#include "ui/sprite_ctrl/ui_sprite_ctrl_track_mgr.h"
#include "ui_sprite_ctrl_track_i.h"
#include "ui_sprite_ctrl_track_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ctrl_track_mgr {
    ui_sprite_ctrl_module_t m_module;
    ui_sprite_ctrl_track_list_t m_tracks;
    ui_sprite_ctrl_track_meta_list_t m_metas;
};

int ui_sprite_ctrl_track_mgr_regist(ui_sprite_ctrl_module_t module);
void ui_sprite_ctrl_track_mgr_unregist(ui_sprite_ctrl_module_t module);
ui_sprite_world_res_t ui_sprite_ctrl_track_mgr_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);
    
#ifdef __cplusplus
}
#endif

#endif
