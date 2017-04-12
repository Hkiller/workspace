#ifndef UI_SPRITE_CTRL_MODULE_I_H
#define UI_SPRITE_CTRL_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "gd/app/app_types.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_module.h"
#include "protocol/ui/sprite_ctrl/ui_sprite_ctrl_evt.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_ctrl_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    error_monitor_t m_em;
    int m_debug;

    LPDRMETA m_meta_circle_state;
    LPDRMETA m_meta_turntable_data;
};

#ifdef __cplusplus
}
#endif

#endif
