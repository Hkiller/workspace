#ifndef UI_RUNTIME_MODULE_I_H
#define UI_RUNTIME_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "render/runtime/ui_runtime_module.h"
#include "protocol/render/model/ui_object_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_runtime_texture_data * ui_runtime_texture_data_t;
typedef struct ui_runtime_render_obj_child * ui_runtime_render_obj_child_t;
    
typedef TAILQ_HEAD(ui_runtime_render_program_list, ui_runtime_render_program) ui_runtime_render_program_list_t;
typedef TAILQ_HEAD(ui_runtime_render_program_attr_list, ui_runtime_render_program_attr) ui_runtime_render_program_attr_list_t;
typedef TAILQ_HEAD(ui_runtime_render_program_unif_list, ui_runtime_render_program_unif) ui_runtime_render_program_unif_list_t;
typedef TAILQ_HEAD(ui_runtime_render_program_state_list, ui_runtime_render_program_state) ui_runtime_render_program_state_list_t;
typedef TAILQ_HEAD(ui_runtime_render_program_state_attr_list, ui_runtime_render_program_state_attr) ui_runtime_render_program_state_attr_list_t;
typedef TAILQ_HEAD(ui_runtime_render_program_state_unif_list, ui_runtime_render_program_state_unif) ui_runtime_render_program_state_unif_list_t;

typedef TAILQ_HEAD(ui_runtime_render_camera_list, ui_runtime_render_camera) ui_runtime_render_camera_list_t;
typedef TAILQ_HEAD(ui_runtime_render_queue_list, ui_runtime_render_queue) ui_runtime_render_queue_list_t;
typedef TAILQ_HEAD(ui_runtime_render_cmd_list, ui_runtime_render_cmd) ui_runtime_render_cmd_list_t;
typedef TAILQ_HEAD(ui_runtime_render_material_list, ui_runtime_render_material) ui_runtime_render_material_list_t;    
typedef TAILQ_HEAD(ui_runtime_render_technique_list, ui_runtime_render_technique) ui_runtime_render_technique_list_t;    
typedef TAILQ_HEAD(ui_runtime_render_pass_list, ui_runtime_render_pass) ui_runtime_render_pass_list_t;

typedef TAILQ_HEAD(ui_runtime_render_state_list, ui_runtime_render_state) ui_runtime_render_state_list_t;
    
typedef TAILQ_HEAD(ui_runtime_render_obj_list, ui_runtime_render_obj) ui_runtime_render_obj_list_t;
typedef TAILQ_HEAD(ui_runtime_render_obj_child_list, ui_runtime_render_obj_child) ui_runtime_render_obj_child_list_t;
typedef TAILQ_HEAD(ui_runtime_render_obj_ref_list, ui_runtime_render_obj_ref) ui_runtime_render_obj_ref_list_t;

typedef TAILQ_HEAD(ui_runtime_sound_backend_list, ui_runtime_sound_backend) ui_runtime_sound_backend_list_t;
typedef TAILQ_HEAD(ui_runtime_sound_chanel_list, ui_runtime_sound_chanel) ui_runtime_sound_chanel_list_t;
typedef TAILQ_HEAD(ui_runtime_sound_group_list, ui_runtime_sound_group) ui_runtime_sound_group_list_t;
typedef TAILQ_HEAD(ui_runtime_sound_playing_list, ui_runtime_sound_playing) ui_runtime_sound_playing_list_t;
    
struct ui_runtime_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;

    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;

    ui_runtime_runing_level_t m_runing_level;
    
    /*render */
    ui_runtime_render_backend_t m_render_backend;
    ui_runtime_render_t m_render;
    ui_runtime_render_state_list_t m_free_states;
    ui_runtime_render_cmd_list_t m_free_cmds;
    ui_runtime_render_queue_list_t m_free_queues;
    ui_runtime_render_material_list_t m_free_materials;
    ui_runtime_render_technique_list_t m_free_techniques;
    ui_runtime_render_pass_list_t m_free_passes;
    ui_runtime_render_program_state_list_t m_free_program_states;
    ui_runtime_render_program_state_attr_list_t m_free_program_state_attrs;
    ui_runtime_render_program_state_unif_list_t m_free_program_state_unifs;
    
    struct cpe_hash_table m_obj_metas;
    ui_runtime_render_obj_meta_t m_obj_metas_by_id[UI_OBJECT_TYPE_MAX - UI_OBJECT_TYPE_MIN];
    struct cpe_hash_table m_named_objs;
    uint32_t m_obj_count;
    ui_runtime_render_obj_list_t m_objs;
    ui_runtime_render_obj_list_t m_update_objs;
    struct cpe_hash_table m_obj_childs;
    ui_runtime_render_obj_child_list_t m_free_obj_childs;

    /*sound*/
    float m_sound_volume;
    uint8_t m_sound_logic_pause;

    uint16_t m_sound_chanel_count;
    ui_runtime_sound_group_list_t m_sound_groups;

    uint32_t m_sound_playing_max_id;
    uint16_t m_sound_playing_count;
    ui_runtime_sound_playing_list_t m_sound_playings;

    ui_runtime_sound_backend_list_t m_sound_backends;
    
    ui_runtime_sound_playing_list_t m_free_playings;
    
    /**/
    struct mem_buffer m_dump_buffer;
};

int ui_runtime_module_init_sound_updator(ui_runtime_module_t module);
void ui_runtime_module_fini_sound_updator(ui_runtime_module_t module);

int ui_runtime_module_init_sound_res_plugin(ui_runtime_module_t module);
void ui_runtime_module_fini_sound_res_plugin(ui_runtime_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
