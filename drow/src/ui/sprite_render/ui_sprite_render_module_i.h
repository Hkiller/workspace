#ifndef UI_SPRITE_RENDER_MODULE_I_H
#define UI_SPRITE_RENDER_MODULE_I_H
#include "cpe/utils/hash.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_render/ui_sprite_render_module.h"

typedef struct ui_sprite_render_env_touch_processor * ui_sprite_render_env_touch_processor_t;
typedef struct ui_sprite_render_env_transform_monitor * ui_sprite_render_env_transform_monitor_t;

typedef TAILQ_HEAD(ui_sprite_render_obj_world_list, ui_sprite_render_obj_world) ui_sprite_render_obj_world_list_t;
typedef TAILQ_HEAD(ui_sprite_render_env_touch_processor_list, ui_sprite_render_env_touch_processor) ui_sprite_render_env_touch_processor_list_t;
typedef TAILQ_HEAD(ui_sprite_render_env_transform_monitor_list, ui_sprite_render_env_transform_monitor) ui_sprite_render_env_transform_monitor_list_t;
typedef TAILQ_HEAD(ui_sprite_render_layer_list, ui_sprite_render_layer) ui_sprite_render_layer_list_t;
typedef TAILQ_HEAD(ui_sprite_render_group_list, ui_sprite_render_group) ui_sprite_render_group_list_t;
typedef TAILQ_HEAD(ui_sprite_render_anim_list, ui_sprite_render_anim) ui_sprite_render_anim_list_t;
typedef TAILQ_HEAD(ui_sprite_render_def_list, ui_sprite_render_def) ui_sprite_render_def_list_t;

struct ui_sprite_render_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    ui_runtime_module_t m_runtime;
    plugin_app_env_monitor_t m_suspend_monitor;
    uint8_t m_app_pause;
    struct cpe_hash_table m_obj_creators;
    error_monitor_t m_em;
    int m_debug;
    struct mem_buffer m_tmp_buffer;
};

int ui_sprite_render_suspend_monitor_regist(ui_sprite_render_module_t module);
void ui_sprite_render_suspend_monitor_unregist(ui_sprite_render_module_t module);

#endif
