#ifndef UI_SPRITE_CHIPMUNK_ENV_I_H
#define UI_SPRITE_CHIPMUNK_ENV_I_H
#include "chipmunk/chipmunk_private.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_env {
    ui_sprite_chipmunk_module_t m_module;
    plugin_chipmunk_env_t m_env;
    uint32_t m_collision_type;
    uint8_t m_debug;
    uint8_t m_max_finger_count;
    uint8_t m_dft_threshold;
    uint32_t m_ground_mask;

    uint8_t m_process_touch;
    cpShapeFilter m_touch_filter;
    
    ui_sprite_chipmunk_tri_scope_list_t m_scopes;
    ui_sprite_chipmunk_touch_trace_list_t m_touch_traces;
};

int ui_sprite_chipmunk_env_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_env_unregist(ui_sprite_chipmunk_module_t module);

ui_sprite_world_res_t ui_sprite_chipmunk_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);
    
#ifdef __cplusplus
}
#endif

#endif
