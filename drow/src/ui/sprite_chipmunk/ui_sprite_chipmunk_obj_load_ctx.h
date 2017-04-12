#ifndef UI_SPRITE_CHIPMUNK_OBJ_LOAD_CTX_H
#define UI_SPRITE_CHIPMUNK_OBJ_LOAD_CTX_H
#include "ui_sprite_chipmunk_obj_i.h"
#include "cpe/utils/hash.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "plugin/chipmunk/plugin_chipmunk_data_constraint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_obj_body_load_info * ui_sprite_chipmunk_obj_body_load_info_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_body_load_info_list, ui_sprite_chipmunk_obj_body_load_info) ui_sprite_chipmunk_obj_body_load_info_list_t;

typedef struct ui_sprite_chipmunk_obj_constraint_info * ui_sprite_chipmunk_obj_constraint_info_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_constraint_info_list, ui_sprite_chipmunk_obj_constraint_info) ui_sprite_chipmunk_obj_constraint_info_list_t;

enum ui_sprite_chipmunk_obj_body_load_state {
    ui_sprite_chipmunk_obj_body_load_state_loaded,
    ui_sprite_chipmunk_obj_body_load_state_not_load,
    ui_sprite_chipmunk_obj_body_load_state_need_load,
    ui_sprite_chipmunk_obj_body_load_state_skip,
};

struct ui_sprite_chipmunk_obj_body_load_info {
    uint32_t m_body_id;
    struct cpe_hash_entry m_hh_for_id;
    char m_body_name[64];
    struct cpe_hash_entry m_hh_for_name;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_body_load_info) m_next;
    ui_sprite_chipmunk_obj_body_t m_body;
    plugin_chipmunk_data_body_t m_body_data;
    enum ui_sprite_chipmunk_obj_body_load_state m_state;
    ui_sprite_chipmunk_obj_constraint_info_list_t m_constraints_as_a;
    ui_sprite_chipmunk_obj_constraint_info_list_t m_constraints_as_b;
};

struct ui_sprite_chipmunk_obj_constraint_info {
    struct ui_sprite_chipmunk_obj_body_load_info * body_a;
    struct ui_sprite_chipmunk_obj_body_load_info * body_b;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_constraint_info) m_next_for_a;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_constraint_info) m_next_for_b;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_constraint_info) m_next_for_ctx;    
    plugin_chipmunk_data_constraint_t m_constraint_data;
    uint8_t m_need_process;
};
    
struct ui_sprite_chipmunk_obj_load_ctx {
    ui_sprite_chipmunk_module_t m_module;
    ui_sprite_entity_t m_entity;
    ui_sprite_chipmunk_obj_t m_obj;

    ui_sprite_chipmunk_obj_body_load_info_list_t m_body_load_infos;
    struct cpe_hash_table m_body_load_infos_by_name;
    struct cpe_hash_table m_body_load_infos_by_id;
    
    ui_sprite_chipmunk_obj_constraint_info_list_t m_constraint_infos;
};

int ui_sprite_chipmunk_obj_load_ctx_init(struct ui_sprite_chipmunk_obj_load_ctx * ctx, ui_sprite_chipmunk_obj_t obj);
void ui_sprite_chipmunk_obj_load_ctx_fini(struct ui_sprite_chipmunk_obj_load_ctx * ctx);

struct ui_sprite_chipmunk_obj_body_load_info * 
ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_id(struct ui_sprite_chipmunk_obj_load_ctx * ctx, uint32_t id);
struct ui_sprite_chipmunk_obj_body_load_info * 
ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_name(struct ui_sprite_chipmunk_obj_load_ctx * ctx, const char * name);

int ui_sprite_chipmunk_obj_load_ctx_load_loaded_bodies(struct ui_sprite_chipmunk_obj_load_ctx * ctx);
int ui_sprite_chipmunk_obj_load_ctx_load_scene_bodies(struct ui_sprite_chipmunk_obj_load_ctx * ctx, plugin_chipmunk_data_scene_t scene);
int ui_sprite_chipmunk_obj_load_ctx_load_scene_joins(struct ui_sprite_chipmunk_obj_load_ctx * ctx, plugin_chipmunk_data_scene_t scene);
int ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state(struct ui_sprite_chipmunk_obj_load_ctx * ctx, const char * name);
    
#ifdef __cplusplus
}
#endif

#endif
