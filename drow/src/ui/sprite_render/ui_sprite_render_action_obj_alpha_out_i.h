#ifndef UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT_I_H
#define UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT_I_H
#include "render/utils/ui_percent_decorator.h"
#include "ui/sprite_render/ui_sprite_render_action_obj_alpha_out.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_action_obj_alpha_out {
    ui_sprite_render_module_t m_module;
    char * m_cfg_anim_name;
	float m_cfg_take_time;
    struct ui_percent_decorator m_cfg_decorator;

    float m_runing_time;
    ui_runtime_render_second_color_mix_t m_saved_mix;
    float m_saved_alpha;
    ui_runtime_render_obj_ref_t m_render_obj_ref;
};

int ui_sprite_render_action_obj_alpha_out_regist(ui_sprite_render_module_t module);
void ui_sprite_render_action_obj_alpha_out_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
