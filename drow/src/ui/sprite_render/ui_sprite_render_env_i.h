#ifndef UI_SPRITE_RENDER_ENV_I_H
#define UI_SPRITE_RENDER_ENV_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_render_module_i.h"

struct ui_sprite_render_env {
    ui_sprite_render_module_t m_module;
    uint8_t m_debug;
    ui_vector_2 m_design_size;
    ui_vector_2 m_size;
    ui_transform m_base_transform;
    ui_transform m_transform;    
    ui_sprite_render_layer_t m_default_layer;
    ui_sprite_render_layer_list_t m_layers;
    uint32_t m_max_id;
    struct cpe_hash_table m_anims;
    ui_sprite_render_anim_list_t m_global_anims;
    ui_sprite_render_env_touch_processor_list_t m_touch_processors;
    ui_sprite_render_env_transform_monitor_list_t m_transform_monitors;
    ui_sprite_render_anim_list_t m_free_anims;
    ui_sprite_render_obj_world_list_t m_renders;
};

int ui_sprite_render_env_regist(ui_sprite_render_module_t module);
void ui_sprite_render_env_unregist(ui_sprite_render_module_t module);

ui_sprite_world_res_t ui_sprite_render_env_res_load(void * ctx, ui_sprite_world_t world, cfg_t cfg);

#endif
