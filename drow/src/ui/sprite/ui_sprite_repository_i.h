#ifndef UI_SPRITE_REPOSITORY_I_H
#define UI_SPRITE_REPOSITORY_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_types.h"
#include "ui/sprite/ui_sprite_repository.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_entity_list, ui_sprite_entity) ui_sprite_entity_list_t;

typedef struct ui_sprite_group_binding * ui_sprite_group_binding_t;
typedef TAILQ_HEAD(ui_sprite_group_binding_list, ui_sprite_group_binding) ui_sprite_group_binding_list_t;

typedef struct ui_sprite_event_meta * ui_sprite_event_meta_t;
typedef TAILQ_HEAD(ui_sprite_component_list, ui_sprite_component) ui_sprite_component_list_t;
typedef TAILQ_HEAD(ui_sprite_word_list, ui_sprite_world) ui_sprite_word_list_t;

typedef struct ui_sprite_attr_monitor_binding * ui_sprite_attr_monitor_binding_t;
typedef TAILQ_HEAD(ui_sprite_attr_monitor_binding_list, ui_sprite_attr_monitor_binding) ui_sprite_attr_monitor_binding_list_t;
typedef TAILQ_HEAD(ui_sprite_attr_monitor_list, ui_sprite_attr_monitor) ui_sprite_attr_monitor_list_t;

struct ui_sprite_repository {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_sprite_word_list_t m_worlds;
    struct cpe_hash_table m_component_metas;
    struct cpe_hash_table m_event_metas;
    xcomputer_t m_computer;
    int m_debug;

    struct mem_buffer m_dump_buffer;
};

int ui_sprite_repository_register_functions(ui_sprite_repository_t repo);

#ifdef __cplusplus
}
#endif

#endif
