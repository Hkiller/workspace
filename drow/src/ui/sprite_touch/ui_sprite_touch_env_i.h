#ifndef UI_SPRITE_TOUCH_ENV_I_H
#define UI_SPRITE_TOUCH_ENV_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite_touch/ui_sprite_touch_env.h"
#include "ui_sprite_touch_mgr_i.h"

struct ui_sprite_touch_env {
    ui_sprite_touch_mgr_t m_mgr;
    uint8_t m_debug;
    ui_sprite_touch_trace_list_t m_touch_traces;
    ui_sprite_touch_responser_list_t m_active_responsers;
    ui_sprite_touch_responser_list_t m_waiting_responsers;
};

int ui_sprite_touch_env_regist(ui_sprite_touch_mgr_t module);
void ui_sprite_touch_env_unregist(ui_sprite_touch_mgr_t module);

ui_sprite_world_res_t ui_sprite_touch_env_res_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

struct ui_sprite_touch_active_responser_info {
    ui_sprite_touch_responser_t m_grab_active_responser;
    uint8_t m_have_binding_responser;
    float m_binding_z;
};

void ui_sprite_touch_env_calc_active_responser_info(ui_sprite_touch_env_t env, struct ui_sprite_touch_active_responser_info * info);

void ui_sprite_touch_process_trace_begin(ui_sprite_touch_env_t env, ui_sprite_touch_trace_t trace, ui_vector_2_t screen_pt, ui_vector_2_t world_pt);

#endif
