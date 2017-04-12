#ifndef UI_SPRITE_RENDER_CHANGE_BLEND_I_H
#define UI_SPRITE_RENDER_CHANGE_BLEND_I_H
#include "render/utils/ui_percent_decorator.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "ui/sprite_render/ui_sprite_render_change_second_color.h"
#include "ui_sprite_render_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_change_second_color {
    ui_sprite_render_module_t m_module;
    char * m_anim_name;
    ui_color m_change_to_color;
	float m_change_take_time;
	uint16_t m_loop_count;
    struct ui_runtime_render_second_color m_second_color;
    struct ui_percent_decorator m_decorator;

	uint16_t m_runing_loop_count;
    float m_runing_time;
    struct ui_runtime_render_second_color m_saved;
    ui_runtime_render_obj_ref_t m_render_obj_ref;
};

int ui_sprite_render_change_second_color_regist(ui_sprite_render_module_t module);
void ui_sprite_render_change_second_color_unregist(ui_sprite_render_module_t module);

#ifdef __cplusplus
}
#endif

#endif
