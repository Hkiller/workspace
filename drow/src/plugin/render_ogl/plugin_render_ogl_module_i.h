#ifndef UI_RENDER_OGL_MODULE_I_H
#define UI_RENDER_OGL_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_types.h"
#include "plugin_render_ogl_gl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_render_ogl_module * plugin_render_ogl_module_t;
typedef struct plugin_render_ogl_cache * plugin_render_ogl_cache_t;
typedef struct plugin_render_ogl_texture * plugin_render_ogl_texture_t;
typedef struct plugin_render_ogl_program * plugin_render_ogl_program_t;
typedef struct plugin_render_ogl_program_attr * plugin_render_ogl_program_attr_t;
typedef struct plugin_render_ogl_program_unif * plugin_render_ogl_program_unif_t;
typedef struct plugin_render_ogl_shader * plugin_render_ogl_shader_t;
typedef struct plugin_render_ogl_queue * plugin_render_ogl_queue_t;
typedef struct plugin_render_ogl_batch_cmd * plugin_render_ogl_batch_cmd_t;
typedef struct plugin_render_ogl_batch_2d * plugin_render_ogl_batch_2d_t;
    
typedef TAILQ_HEAD(plugin_render_ogl_shader_list, plugin_render_ogl_shader) plugin_render_ogl_shader_list_t;
    
struct plugin_render_ogl_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;
    uint8_t m_debug;
    plugin_render_ogl_shader_list_t m_shaders;

    /*capacity*/
    uint8_t m_capacity_supports_shareable_vao;
    uint32_t m_capacity_vertex_vbo_size;
    uint32_t m_capacity_index_vbo_size;
    uint32_t m_capacity_active_texture_count;
    uint32_t m_capacity_install_texture_index; /*用于加载Texture的位置，不能用于运行时 */
    
    /*batch-2d*/
    plugin_render_ogl_batch_2d_t m_batch_2d;
    
    /*batch-3d*/

    /*op cache*/
    plugin_render_ogl_cache_t m_cache;
    
    /**/
    struct ui_runtime_render_statistics m_statistics;
};

const char * plugin_render_ogl_module_name(plugin_render_ogl_module_t module);

/*init*/
int plugin_render_ogl_module_init_backend(plugin_render_ogl_module_t module);
void plugin_render_ogl_module_fini_backend(plugin_render_ogl_module_t module);

int plugin_render_ogl_module_init_capacity(plugin_render_ogl_module_t module);
void plugin_render_ogl_module_fini_capacity(plugin_render_ogl_module_t module);

/*render*/
int plugin_render_ogl_render_bind(void * ctx, ui_runtime_render_t render);
void plugin_render_ogl_render_unbind(void * ctx, ui_runtime_render_t render);

/*camera*/
void plugin_render_ogl_camera_update(void * ctx, ui_transform_t transform, ui_vector_2_t view_size, ui_runtime_render_camera_t camera);

/*env*/
void plugin_render_ogl_setup_viewpoint(void * ctx, ui_runtime_render_t render);
    
/*commit*/
void plugin_render_ogl_commit_begin(void * ctx, ui_runtime_render_t render);
void plugin_render_ogl_commit_done(void * ctx, ui_runtime_render_t render, ui_runtime_render_statistics_t statistics);
void plugin_render_ogl_commit_group_begin(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type);
void plugin_render_ogl_commit_group_done(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type);
void plugin_render_ogl_commit_cmd(void * ctx, ui_runtime_render_t render, ui_runtime_render_cmd_t cmd);
    
/*bind*/
void plugin_render_ogl_bind_state(plugin_render_ogl_module_t module, ui_runtime_render_state_t state);
    
int plugin_render_ogl_bind_program_state(
    plugin_render_ogl_module_t module, ui_runtime_render_program_state_t program_state, ui_transform_t mvp);

int plugin_render_ogl_bind_pass(
    plugin_render_ogl_module_t module, ui_runtime_render_pass_t pass, ui_transform_t mvp);

/*commit attr mask*/
#define plugin_render_ogl_combine_attr_flag                         \
    ((uint32_t)((1u << ui_runtime_render_program_attr_position)     \
                | (1u << ui_runtime_render_program_attr_color)      \
                | (1u << ui_runtime_render_program_attr_texcoord)))
    
#ifdef __cplusplus
}
#endif

#endif 
