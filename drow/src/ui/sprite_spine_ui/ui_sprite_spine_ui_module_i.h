#ifndef UI_SPRITE_SPINE_UI_MODULE_I_H
#define UI_SPRITE_SPINE_UI_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_spine_ui/ui_sprite_spine_ui_module.h"

struct ui_sprite_spine_ui_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    ui_sprite_render_module_t m_sprite_render;
    ui_sprite_ui_module_t m_ui_module;
    error_monitor_t m_em;
    int m_debug;

    struct mem_buffer m_dump_buffer;
};

#endif