#include <assert.h>
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_camera.h"
#include "plugin_render_s3d_program_i.hpp"

static void plugin_render_s3d_camera_update_ortho(
    ui_matrix_4x4_t dst, float left, float right, float top, float bottom, float zNearPlane, float zFarPlane)
{
    assert(dst);
    assert(right != left);
    assert(top != bottom);
    assert(zFarPlane != zNearPlane);

    *dst = UI_MATRIX_4X4_ZERO;
    dst->m[0] = 2.0f / (right - left);
    dst->m[5] = 2.0f / (top - bottom);
    dst->m[10] =  1.0f / ( zFarPlane - zNearPlane);
    
    dst->m[12] = - (right + left) / (right - left);
    dst->m[13] = - (top + bottom) / (top - bottom);
    dst->m[14] = - (zFarPlane + zNearPlane) / (zFarPlane - zNearPlane);
    
    dst->m[15] = 1.0f;
}

void plugin_render_s3d_camera_update(void * ctx, ui_transform_t transform, ui_vector_2_t view_size, ui_runtime_render_camera_t camera) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
    
    switch(ui_runtime_render_camera_projection(camera)) {
    case ui_runtime_render_projection_2d: {
        transform->m_s = UI_VECTOR_3_IDENTITY;
        transform->m_r_p = 1;
        transform->m_init_p = 1;
        plugin_render_s3d_camera_update_ortho(&transform->m_m4, 0.0f, view_size->x, 0.0f, view_size->y, 1024.0f, -1024.0f);
        break;
    }
    case ui_runtime_render_projection_3d: {
        CPE_ERROR(module->m_em, "plugin_render_s3d_camera_update: not support projection 3d");
        break;
    }
    case ui_runtime_render_projection_custom:
        // assert(camera->m_update_fun);
        // camera->m_update_fun(camera->m_update_ctx, camera, &camera->m_transform);
        break;
    default:
        CPE_ERROR(module->m_em, "plugin_render_s3d_camera_update: unknown project");
        break;
    }
}