#include <assert.h>
#include "render/utils/ui_vector_2.h"
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "ui_runtime_render_camera_i.h"
#include "ui_runtime_render_backend_i.h"

ui_runtime_render_camera_t
ui_runtime_render_camera_create(ui_runtime_render_t render, const char * name) {
    ui_runtime_render_camera_t camera;

    camera = mem_alloc(render->m_module->m_alloc, sizeof(struct ui_runtime_render_camera));
    if (camera == NULL) {
        CPE_ERROR(render->m_module->m_em, "ui_runtime_render_camera_create: alloc fail");
        return NULL;
    }

    cpe_str_dup(camera->m_name, sizeof(camera->m_name), name);
    camera->m_render = render;
    camera->m_projection = ui_runtime_render_projection_2d;
    camera->m_transform = UI_TRANSFORM_IDENTITY;
    camera->m_update_fun = NULL;
    camera->m_update_ctx = NULL;
    ui_runtime_render_camera_update(camera);

    TAILQ_INSERT_TAIL(&render->m_cameras, camera, m_next);

    return camera;
}

void ui_runtime_render_camera_free(ui_runtime_render_camera_t camera) {
    ui_runtime_render_t render = camera->m_render;

    if (render->m_active_camera == camera) {
        render->m_active_camera = NULL;
    }

    TAILQ_REMOVE(&render->m_cameras, camera, m_next);
    mem_free(render->m_module->m_alloc, camera);
}

ui_runtime_render_projection_t ui_runtime_render_camera_projection(ui_runtime_render_camera_t camera) {
    return camera->m_projection;
}

int ui_runtime_render_camera_set_projection(ui_runtime_render_camera_t camera, ui_runtime_render_projection_t projection) {
    ui_runtime_module_t module = camera->m_render->m_module;

    if (projection == ui_runtime_render_projection_custom) {
        CPE_ERROR(module->m_em, "ui_runtime_render_camera_set_projection: can`t set projection custom!");
        return -1;
    }
    
    camera->m_projection = projection;
    ui_runtime_render_camera_update(camera);
    
    return 0;
}

int ui_runtime_render_camera_set_projection_custom(
    ui_runtime_render_camera_t camera,
    ui_runtime_render_camera_custom_update_fun_t update_fun, void * update_ctx)
{
    camera->m_projection = ui_runtime_render_projection_custom;
    camera->m_update_fun = update_fun;
    camera->m_update_ctx = update_ctx;

    ui_runtime_render_camera_update(camera);
    
    return 0;
}

void ui_runtime_render_camera_update(ui_runtime_render_camera_t camera) {
    ui_runtime_render_t render = camera->m_render;

    if (render->m_view_size.x > 0.0f && render->m_view_size.y > 0.0f && render->m_module->m_render_backend) {
        render->m_module->m_render_backend->m_camera_update(render->m_module->m_render_backend->m_ctx, &camera->m_transform, &render->m_view_size, camera);
    }
    else {
        camera->m_transform = UI_TRANSFORM_IDENTITY;
    }

    if (render->m_active_camera == camera) {
        ui_runtime_render_matrix_load(render, ui_runtime_render_matrix_stack_projection, &camera->m_transform);
        ui_runtime_render_matrix_load_identity(render, ui_runtime_render_matrix_stack_modelview);
    }
}

ui_runtime_render_camera_t ui_runtime_render_active_camera(ui_runtime_render_t render) {
    return render->m_active_camera;
}

void ui_runtime_render_set_active_camera(ui_runtime_render_t render, ui_runtime_render_camera_t camera) {
    if (render->m_active_camera != camera) {
        render->m_active_camera = camera;
        ui_runtime_render_camera_update(camera);
    }
}

/* void ui_runtime_render_set_matrix_proj_ortho_2d(ui_runtime_render_t render, float width, float height, float zNear, float zFar) { */
/* 	render->m_matrix_proj.m11 = 2.0f / width; */
/*     render->m_matrix_proj.m12 = 0.0f; */
/*     render->m_matrix_proj.m13 = 0.0f; */
/*     render->m_matrix_proj.m14 = -1.0f; */

/* 	render->m_matrix_proj.m21 = 0.0f; */
/*     render->m_matrix_proj.m22 = -2.0f / height; */
/*     render->m_matrix_proj.m23 = 0.0f; */
/*     render->m_matrix_proj.m24 =  1.0f; */
    
/* 	render->m_matrix_proj.m31 = 0.0f; */
/*     render->m_matrix_proj.m32 = 0.0f; */
/*     render->m_matrix_proj.m33 = 2.0f / (zNear - zFar); */
/*     render->m_matrix_proj.m34 = (zNear + zFar) / (zNear - zFar); */
    
/* 	render->m_matrix_proj.m41 = 0.0f; */
/*     render->m_matrix_proj.m42 = 0.0f; */
/*     render->m_matrix_proj.m43 = 0.0f; */
/*     render->m_matrix_proj.m44 = 1.0f; */
/* } */

/* void ui_runtime_render_set_matrix_proj_persp_2d(ui_runtime_render_t render, float width, float height, float zNear, float zFar, float x, float y) { */
/* 	render->m_matrix_proj.m11 = 2.0f * zNear / width; */
/* 	render->m_matrix_proj.m12 = 0.0f; */
/*     render->m_matrix_proj.m13 =  1.0f - 2.0f * x / width; */
/*     render->m_matrix_proj.m14 = 0.0f; */
    
/* 	render->m_matrix_proj.m21 = 0.0f; */
/*     render->m_matrix_proj.m22 = -2.0f * zNear / height; */
/* 	render->m_matrix_proj.m23 = -1.0f + 2.0f * y / height; */
/*     render->m_matrix_proj.m24 = 0.0f; */
    
/* 	render->m_matrix_proj.m31 = 0.0f; */
/*     render->m_matrix_proj.m32 = 0.0f; */
/*     render->m_matrix_proj.m33 = zFar / (zNear - zFar); */
/*     render->m_matrix_proj.m34 = (zNear * zFar) / (zNear - zFar); */
    
/* 	render->m_matrix_proj.m41 = 0.0f; */
/*     render->m_matrix_proj.m42 = 0.0f; */
/*     render->m_matrix_proj.m43 = -1.0f; */
/*     render->m_matrix_proj.m44 = 0.0f; */
/* } */
