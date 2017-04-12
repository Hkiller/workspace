#ifndef UI_SPRITE_CFG_LOADER_I_H
#define UI_SPRITE_CFG_LOADER_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_cfg_resource_loader * ui_sprite_cfg_resource_loader_t;
typedef struct ui_sprite_cfg_comp_loader * ui_sprite_cfg_comp_loader_t;
typedef struct ui_sprite_cfg_action_loader * ui_sprite_cfg_action_loader_t;
typedef TAILQ_HEAD(ui_sprite_cfg_context_list,  ui_sprite_cfg_context) ui_sprite_cfg_context_list_t;
    
struct ui_sprite_cfg_loader {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    struct mem_buffer m_dump_buffer;
    struct cpe_hash_table m_resource_loaders;
    struct cpe_hash_table m_comp_loaders;
    struct cpe_hash_table m_action_loaders;
    ui_sprite_cfg_context_list_t m_contexts;
};

#ifdef __cplusplus
}
#endif

#endif
