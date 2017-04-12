#ifndef UI_SPRITE_SOUND_MODULE_I_H
#define UI_SPRITE_SOUND_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_sound/ui_sprite_sound_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_sound_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    error_monitor_t m_em;
    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
