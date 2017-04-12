#ifndef UI_RUNTIME_RENDER_CAMERA_I_H
#define UI_RUNTIME_RENDER_CAMERA_I_H
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_camera.h"
#include "ui_runtime_render_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_camera {
    ui_runtime_render_t m_render;
    TAILQ_ENTRY(ui_runtime_render_camera) m_next;
    char m_name[64];
    ui_runtime_render_projection_t m_projection;
    struct ui_transform m_transform;
    ui_runtime_render_camera_custom_update_fun_t m_update_fun;
    void * m_update_ctx;
};

#ifdef __cplusplus
}
#endif

#endif
