#ifndef UI_RENDER_S3D_MODULE_I_H
#define UI_RENDER_S3D_MODULE_I_H
#include <AS3/AS3.h>
#include <Flash++.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_types.h"
#include "AGAL/AGAL.h"

using namespace AS3::ui;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_render_s3d_module * plugin_render_s3d_module_t;
typedef struct plugin_render_s3d_cache * plugin_render_s3d_cache_t;
typedef struct plugin_render_s3d_texture * plugin_render_s3d_texture_t;
typedef struct plugin_render_s3d_program * plugin_render_s3d_program_t;
typedef struct plugin_render_s3d_program_attr * plugin_render_s3d_program_attr_t;
typedef struct plugin_render_s3d_program_unif * plugin_render_s3d_program_unif_t;
typedef struct plugin_render_s3d_queue * plugin_render_s3d_queue_t;
typedef struct plugin_render_s3d_batch_cmd * plugin_render_s3d_batch_cmd_t;
typedef struct plugin_render_s3d_batch_2d * plugin_render_s3d_batch_2d_t;
    
struct plugin_render_s3d_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_cache_manager_t m_cache_mgr;
    ui_runtime_module_t m_runtime;
    uint8_t m_debug;

    flash::display::Stage m_stage;
    flash::display::Stage3D m_s3d;
    flash::display3D::Context3D m_ctx3d;
    com::adobe::utils::AGALMiniAssembler m_assembler;
    
    /*cache*/
    plugin_render_s3d_cache_t m_cache;
    
    /*capacity*/
    uint32_t m_capacity_vertex_size;
    uint32_t m_capacity_index_size;
    uint32_t m_capacity_active_texture_count;
    uint32_t m_capacity_install_texture_index; /*用于加载Texture的位置，不能用于运行时 */
    
    /*batch-2d*/
    plugin_render_s3d_batch_2d_t m_batch_2d;
    
    /*batch-3d*/

    /**/
    struct ui_runtime_render_statistics m_statistics;
};

const char * plugin_render_s3d_module_name(plugin_render_s3d_module_t module);

/*init*/
int plugin_render_s3d_module_init_backend(plugin_render_s3d_module_t module);
void plugin_render_s3d_module_fini_backend(plugin_render_s3d_module_t module);

int plugin_render_s3d_module_init_capacity(plugin_render_s3d_module_t module);
void plugin_render_s3d_module_fini_capacity(plugin_render_s3d_module_t module);

/*render*/
int plugin_render_s3d_render_bind(void * ctx, ui_runtime_render_t render);
void plugin_render_s3d_render_unbind(void * ctx, ui_runtime_render_t render);

/*camera*/
void plugin_render_s3d_camera_update(void * ctx, ui_transform_t transform, ui_vector_2_t view_size, ui_runtime_render_camera_t camera);

/*env*/
void plugin_render_s3d_setup_viewpoint(void * ctx, ui_runtime_render_t render);

/*commit*/
void plugin_render_s3d_commit_begin(void * ctx, ui_runtime_render_t render);
void plugin_render_s3d_commit_done(void * ctx, ui_runtime_render_t render, ui_runtime_render_statistics_t statistics);
void plugin_render_s3d_commit_group_begin(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type);
void plugin_render_s3d_commit_group_done(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type);
void plugin_render_s3d_commit_cmd(void * ctx, ui_runtime_render_t render, ui_runtime_render_cmd_t cmd);
    
/*bind*/
void plugin_render_s3d_bind_state(plugin_render_s3d_module_t module, ui_runtime_render_state_t state);
    
int plugin_render_s3d_bind_program_state(
    plugin_render_s3d_module_t module, ui_runtime_render_program_state_t program_state, ui_transform_t mvp);

int plugin_render_s3d_bind_pass(
    plugin_render_s3d_module_t module, flash::display3D::VertexBuffer3D & v_buf,
    ui_runtime_render_pass_t pass, ui_transform_t mvp);

/*commit attr mask*/
#define plugin_render_s3d_combine_attr_flag                         \
    ((uint32_t)((1u << ui_runtime_render_program_attr_position)     \
                | (1u << ui_runtime_render_program_attr_color)      \
                | (1u << ui_runtime_render_program_attr_texcoord)))
    
#ifdef __cplusplus
}
#endif

#endif 
