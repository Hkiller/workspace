#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_camera_utils.h"

static void ui_sprite_camera_adj_camera_in_limit_no_trace(
    ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_vector_2 * camera_pos, ui_vector_2_t camera_scale)
{
    float view_screen_x;
    float view_screen_y;

    if (!ui_sprite_camera_env_have_limit(camera)) return;

    view_screen_x = screen_size->x * (camera_scale->x);
    if (view_screen_x > (camera->m_limit_rb.x - camera->m_limit_lt.x)) {
        view_screen_x = (camera->m_limit_rb.x - camera->m_limit_lt.x);
        camera_scale->x = view_screen_x / screen_size->x;
    }

    view_screen_y = screen_size->y * (camera_scale->y);
    if (view_screen_y > (camera->m_limit_rb.y - camera->m_limit_lt.y)) {
        view_screen_y = (camera->m_limit_rb.y - camera->m_limit_lt.y);
        camera_scale->y = view_screen_y / screen_size->y;
        view_screen_x = screen_size->x * camera_scale->x;
    }

    if (camera_pos->x + view_screen_x > camera->m_limit_rb.x) {
        camera_pos->x = camera->m_limit_rb.x - view_screen_x;
        assert(camera_pos->x >= camera->m_limit_lt.x);
    }
    else if (camera_pos->x < camera->m_limit_lt.x) {
        camera_pos->x = camera->m_limit_lt.x;
    }

    if (camera_pos->y + view_screen_y > camera->m_limit_rb.y) {
        camera_pos->y = camera->m_limit_rb.y - view_screen_y;
        assert(camera_pos->y >= camera->m_limit_lt.y);
    }
    else if (camera_pos->y < camera->m_limit_lt.y) {
        camera_pos->y = camera->m_limit_lt.y;
    }
}

static void ui_sprite_camera_adj_camera_in_limit_trace_x_lock_l(
    ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_vector_2_t camera_pos, ui_vector_2_t camera_scale,
    ui_rect * screen_rect)
{
    float lock_world_y;

    lock_world_y = camera->m_trace_world_pos.y + (screen_rect->lt.x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;
    if (lock_world_y < camera->m_limit_lt.y || lock_world_y > camera->m_limit_rb.y) return;

    if (camera->m_trace_line.m_by_x.m_base_y > 0) {
        float max_scale_up = (lock_world_y - camera->m_limit_lt.y) / (screen_size->y * camera->m_trace_line.m_by_x.m_base_y);
        if (camera_scale->y > max_scale_up) camera_scale->y = max_scale_up;
    }

    if (camera->m_trace_line.m_by_x.m_base_y < 1.0f) {
        float max_scale_down = (camera->m_limit_rb.y - lock_world_y) / (screen_size->y * (1 - camera->m_trace_line.m_by_x.m_base_y));
        if (camera_scale->y > max_scale_down) camera_scale->y = max_scale_down;
    }

    camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
}

static void ui_sprite_camera_adj_camera_in_limit_trace_x_lock_r(
    ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_vector_2_t camera_pos, ui_vector_2_t camera_scale,
    ui_rect_t screen_rect)
{
    float base_y_r;
    float lock_world_y;

    base_y_r = camera->m_trace_line.m_by_x.m_base_y + camera->m_trace_line.m_by_x.m_dy_dx;

    lock_world_y = camera->m_trace_world_pos.y + (screen_rect->rb.y - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;
    if (lock_world_y < camera->m_limit_lt.y || lock_world_y > camera->m_limit_rb.y) return;

    if (base_y_r > 0) {
        float max_scale_up = (lock_world_y - camera->m_limit_lt.y) / (screen_size->y * base_y_r);
        if (camera_scale->y > max_scale_up) camera_scale->y = max_scale_up;
    }

    if (base_y_r < 1.0f) {
        float max_scale_down = (camera->m_limit_rb.y - lock_world_y) / (screen_size->y * (1 - base_y_r));
        if (camera_scale->y > max_scale_down) camera_scale->y = max_scale_down;
    }

    camera_pos->x = camera->m_limit_rb.x - screen_size->x * camera_scale->x;
    camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
}

static void ui_sprite_camera_adj_camera_in_limit_trace_x(
    ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_vector_2 * camera_pos, ui_vector_2_t camera_scale) {
    ui_rect screen_rect;

    camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
    ui_sprite_camera_screen_world_rect(
        &screen_rect, &g_full_screen_percent, camera, camera_pos, camera_scale);

    if (!ui_sprite_camera_env_have_limit(camera)) return;

    if (camera_scale->x > camera->m_scale_max) camera_scale->x = camera->m_scale_max;
    if (camera_scale->x < camera->m_scale_min) camera_scale->x = camera->m_scale_min;
    if (camera_scale->y > camera->m_scale_max) camera_scale->y = camera->m_scale_max;
    if (camera_scale->y < camera->m_scale_min) camera_scale->y = camera->m_scale_min;

    if (screen_rect.lt.x < camera->m_limit_lt.x) {
        camera_pos->x = camera->m_limit_lt.x;
        camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
        ui_sprite_camera_screen_world_rect(
            &screen_rect, &g_full_screen_percent, camera, camera_pos, camera_scale);

        ui_sprite_camera_adj_camera_in_limit_trace_x_lock_l(camera, screen_size, camera_pos, camera_scale, &screen_rect);
    }
    else if (screen_rect.rb.x > camera->m_limit_rb.x) {
        camera_pos->x -= (screen_rect.rb.x - camera->m_limit_rb.x);
        camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
        ui_sprite_camera_screen_world_rect(
            &screen_rect, &g_full_screen_percent, camera, camera_pos, camera_scale);

        ui_sprite_camera_adj_camera_in_limit_trace_x_lock_r(camera, screen_size, camera_pos, camera_scale, &screen_rect);
    }
    else {
        ui_sprite_camera_adj_camera_in_limit_trace_x_lock_l(camera, screen_size, camera_pos, camera_scale, &screen_rect);

        ui_sprite_camera_screen_world_rect(
            &screen_rect, &g_full_screen_percent, camera, camera_pos, camera_scale);
    
        if (screen_rect.rb.x > camera->m_limit_rb.x) {
            camera_pos->x -= (screen_rect.rb.x - camera->m_limit_rb.x);
            camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
            ui_sprite_camera_screen_world_rect(
                &screen_rect, &g_full_screen_percent, camera, camera_pos, camera_scale);

            ui_sprite_camera_adj_camera_in_limit_trace_x_lock_r(camera, screen_size, camera_pos, camera_scale, &screen_rect);
        }
    }
}

void ui_sprite_camera_env_adj_camera_in_limit(ui_sprite_camera_env_t camera, ui_vector_2 * camera_pos, ui_vector_2_t camera_scale) {
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    
    switch(camera->m_trace_type) {
    case ui_sprite_camera_trace_none:
        ui_sprite_camera_adj_camera_in_limit_no_trace(camera, screen_size, camera_pos, camera_scale);
        break;
    case ui_sprite_camera_trace_by_x:
        ui_sprite_camera_adj_camera_in_limit_trace_x(camera, screen_size, camera_pos, camera_scale);
        break;
    case ui_sprite_camera_trace_by_y:
        break;
    default:
        break;
    }
}
