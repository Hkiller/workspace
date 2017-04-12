#ifndef UI_SPRITE_CAMERA_I_H
#define UI_SPRITE_CAMERA_I_H
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_camera/ui_sprite_camera_env.h"
#include "ui_sprite_camera_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_camera_env {
    ui_sprite_camera_module_t m_module;
    ui_sprite_render_env_t m_render;
    ui_vector_2 m_limit_lt;
    ui_vector_2 m_limit_rb;

    float m_scale_min;
    float m_scale_max;

    uint32_t m_max_op_id;
    uint32_t m_curent_op_id;

    /*镜头轨道 */
    enum ui_sprite_camera_trace_type m_trace_type;
    ui_vector_2 m_trace_screen_pos;
    ui_vector_2 m_trace_world_pos;
    union {
        struct {
            float m_base_y;
            float m_dy_dx;
        } m_by_x;
        struct {
            float m_base_x;
            float m_dx_dy;
        } m_by_y;
    } m_trace_line;
};

int ui_sprite_camera_env_regist(ui_sprite_camera_module_t module);
void ui_sprite_camera_env_unregist(ui_sprite_camera_module_t module);

int ui_sprite_camera_env_pos_of_entity(ui_vector_2 * pos, ui_sprite_world_t world, uint32_t entity_id, const char * entity_name, uint8_t pos_of_entity);

uint32_t ui_sprite_camera_env_start_op(ui_sprite_camera_env_t camera);
void ui_sprite_camera_env_stop_op(ui_sprite_camera_env_t camera, uint32_t op_id);

void ui_sprite_camera_env_adj_camera_in_limit(ui_sprite_camera_env_t camera, ui_vector_2 * camera_pos, ui_vector_2_t camera_scale);
void ui_sprite_camera_env_adj_camera_in_limit_with_lock_pos(
    ui_sprite_camera_env_t camera, ui_vector_2 * camera_pos, ui_vector_2_t camera_scale,
    ui_vector_2 const * lock_pos_in_screen, ui_vector_2 const * lock_pos_in_world);


/*根据锁定规则，camera位置中的一个计算另外一个 */
float ui_sprite_camera_env_trace_x2y(ui_sprite_camera_env_t camera, float camera_x, ui_vector_2_t scale);
float ui_sprite_camera_env_trace_y2x(ui_sprite_camera_env_t camera, float camera_y, ui_vector_2_t scale);

/*根据屏幕锁定点的世界坐标和屏幕坐标中的一个，计算另外一个屏幕坐标 */
float ui_sprite_camera_env_screen_x2y_lock_x(ui_sprite_camera_env_t camera, float screen_x, ui_vector_2 workd_pos, ui_vector_2_t scale);
float ui_sprite_camera_env_screen_y2x_lock_y(ui_sprite_camera_env_t camera, float screen_y, ui_vector_2 workd_pos, ui_vector_2_t scale);
    
#ifdef __cplusplus
}
#endif

#endif
