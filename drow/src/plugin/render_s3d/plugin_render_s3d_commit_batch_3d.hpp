#ifndef UI_RENDER_S3D_MERGE_3D_I_H
#define UI_RENDER_S3D_MERGE_3D_I_H
#include "plugin_render_s3d_module_i.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void plugin_render_s3d_batch_3d_commit(plugin_render_s3d_module_t module);

int plugin_render_s3d_batch_3d_init(plugin_render_s3d_module_t module);
void plugin_render_s3d_batch_3d_fini(plugin_render_s3d_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif 
