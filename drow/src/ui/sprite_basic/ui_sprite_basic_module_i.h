#ifndef UI_SPRITE_BASIC_MODULE_I_H
#define UI_SPRITE_BASIC_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_basic/ui_sprite_basic_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    error_monitor_t m_em;
    LPDRMETA m_meta_gen_entities_data;
    int m_debug;

    struct mem_buffer m_tmp_buff;
};

#ifdef __cplusplus
}
#endif

#endif
