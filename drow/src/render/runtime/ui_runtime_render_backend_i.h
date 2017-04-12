#ifndef UI_RUNTIME_RENDER_BACKEND_I_H
#define UI_RUNTIME_RENDER_BACKEND_I_H
#include "render/runtime/ui_runtime_render_backend.h"
#include "ui_runtime_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_backend {
    ui_runtime_module_t m_module;
    TAILQ_ENTRY(ui_runtime_render_backend) m_next;
    struct ui_runtime_render_statistics m_last_commit_statics;
    char m_name[64];
    void * m_ctx;
    /*render*/
    ui_runtime_render_bind_fun_t m_render_bind;
    ui_runtime_render_unbind_fun_t m_render_unbind;
    /*camera*/
    ui_runtime_render_camera_update_fun_t m_camera_update;
    /*program*/
    uint32_t m_program_capacity;
    ui_runtime_render_program_init_fun_t m_program_init;
    ui_runtime_render_program_fini_fun_t m_program_fini;
    /*program_attr*/
    uint32_t m_program_attr_capacity;
    ui_runtime_render_program_attr_init_fun_t m_program_attr_init;
    ui_runtime_render_program_attr_fini_fun_t m_program_attr_fini;
    /*program_unif*/
    uint32_t m_program_unif_capacity;
    ui_runtime_render_program_unif_init_fun_t m_program_unif_init;
    ui_runtime_render_program_unif_fini_fun_t m_program_unif_fini;
    /*env*/
    ui_runtime_render_state_save_fun_t m_state_save;
    ui_runtime_render_state_restore_fun_t m_state_restore;
    /*commit*/
    ui_runtime_render_commit_begin_fun_t m_commit_being;
    ui_runtime_render_commit_done_fun_t m_commit_done;
    ui_runtime_render_commit_group_begin_fun_t m_commit_group_being;
    ui_runtime_render_commit_group_done_fun_t m_commit_group_done;
    ui_runtime_render_commit_cmd_fun_t m_commit_cmd;
};

#ifdef __cplusplus
}
#endif

#endif 
