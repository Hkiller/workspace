#ifndef UI_SPRITE_MOVING_MODULE_I_H
#define UI_SPRITE_MOVING_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_moving/ui_sprite_moving_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_moving_obj_node * ui_sprite_moving_obj_node_t;
typedef TAILQ_HEAD(ui_sprite_moving_obj_node_list, ui_sprite_moving_obj_node) ui_sprite_moving_obj_node_list_t;

struct ui_sprite_moving_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    ui_sprite_cfg_loader_t m_loader;
    plugin_moving_module_t m_moving_module;
    error_monitor_t m_em;
    int m_debug;

    ui_sprite_moving_obj_node_list_t m_free_moving_obj_nodes;

    struct mem_buffer m_tmp_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
