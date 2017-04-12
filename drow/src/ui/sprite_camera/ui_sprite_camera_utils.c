#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "ui_sprite_camera_utils.h"

enum ui_sprite_camera_limit_check_result
ui_sprite_camera_check_in_area(
    ui_sprite_camera_env_t camera, ui_vector_2 const * camera_pos, ui_vector_2_t camera_scale)
{
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    float delta = 0.1f * cpe_min(camera_scale->x, camera_scale->y);

    float screen_x = screen_size->x * camera_scale->x;
    float screen_y = screen_size->y * camera_scale->y;

    if (camera_pos->x < camera->m_limit_lt.x || (camera_pos->x + screen_x) > camera->m_limit_rb.x
        || camera_pos->y < camera->m_limit_lt.y || (camera_pos->y + screen_y) > camera->m_limit_rb.y)
    {
        return ui_sprite_camera_limit_check_result_bigger;
    }

    if (fabs(camera_pos->x - camera->m_limit_lt.x) < delta
        || fabs(camera_pos->x + screen_x - camera->m_limit_rb.x) < delta
        || fabs(camera_pos->y - camera->m_limit_lt.y) < delta
        || fabs(camera_pos->y + screen_y - camera->m_limit_lt.y) < delta)
    {
        return ui_sprite_camera_limit_check_result_ok;
    }

    return ui_sprite_camera_limit_check_result_small;
}

float ui_sprite_camera_max_scale_from_point(
    ui_sprite_camera_env_t camera, ui_vector_2 world_pos, ui_vector_2 screen_pos)
{
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    float r = 0;
    float tmp;

    assert(ui_sprite_camera_env_have_limit(camera));
    assert(screen_pos.x >= 0.0f && screen_pos.x <= 1.0f);
    assert(screen_pos.y >= 0.0f && screen_pos.y <= 1.0f);
    assert(world_pos.x >= camera->m_limit_lt.x && world_pos.x <= camera->m_limit_rb.x);
    assert(world_pos.y >= camera->m_limit_lt.y && world_pos.y <= camera->m_limit_rb.y);

    tmp = screen_pos.x > 0.0f
        ? (world_pos.x - camera->m_limit_lt.x) / (screen_size->x * screen_pos.x)
        : 0.0f;
    if (tmp > 0.0f) r = (r == 0.0f) ? tmp : (tmp < r ? tmp : r);

    tmp = screen_pos.x < 1.0f
        ? (camera->m_limit_rb.x - world_pos.x) / (screen_size->x * (1.0f - screen_pos.x))
        : 0.0f;
    if (tmp > 0.0f) r = (r == 0.0f) ? tmp : (tmp < r ? tmp : r);

    tmp = screen_pos.y > 0.0f
        ? (world_pos.y - camera->m_limit_lt.y) / (screen_size->y * screen_pos.y)
        : 0.0f;
    if (tmp > 0.0f) r = (r == 0.0f) ? tmp : (tmp < r ? tmp : r);

    tmp = screen_pos.y < 1.0f
        ? (camera->m_limit_rb.y - world_pos.y) / (screen_size->y * (1.0f - screen_pos.y))
        : 0.0f;

    assert(r > 0.0f);

    return r;
}

ui_vector_2 ui_sprite_camera_calc_pos(
   ui_sprite_camera_env_t camera, ui_vector_2 world_pos, ui_vector_2 screen_pos, ui_vector_2_t camera_scale)
{
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_vector_2 r;

    switch(camera->m_trace_type) {
    case ui_sprite_camera_trace_none:
        r.x = world_pos.x - screen_size->x * camera_scale->x * screen_pos.x;
        r.y = world_pos.y - screen_size->y * camera_scale->y * screen_pos.y;
        return r;
    case ui_sprite_camera_trace_by_x:
        r.x = world_pos.x - screen_size->x * camera_scale->x * screen_pos.x;
        r.y = ui_sprite_camera_env_trace_x2y(camera, r.x, camera_scale);
        return r;
    case ui_sprite_camera_trace_by_y:
        r.y = world_pos.y - screen_size->y * camera_scale->x * screen_pos.y;
        r.x = ui_sprite_camera_env_trace_y2x(camera, r.y, camera_scale);
        return r;
    default:
        assert(0);
        r.x = world_pos.x - screen_size->x * camera_scale->x * screen_pos.x;
        r.y = world_pos.y - screen_size->y * camera_scale->y * screen_pos.y;
        return r;
    }
}

void ui_sprite_camera_screen_world_rect(
    ui_rect * screen_world, ui_rect const * screen_percent, 
    ui_sprite_camera_env_t camera, ui_vector_2 const * camera_pos, ui_vector_2_t camera_scale)
{
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    assert(camera_scale->x > 0.0f);
    assert(camera_scale->y > 0.0f);    

    screen_world->lt.x = camera_pos->x + screen_percent->lt.x * screen_size->x * camera_scale->x;
    screen_world->lt.y = camera_pos->y + screen_percent->lt.y * screen_size->y * camera_scale->y;
    screen_world->rb.x = camera_pos->x + screen_percent->rb.x * screen_size->x * camera_scale->x;
    screen_world->rb.y = camera_pos->y + screen_percent->rb.y * screen_size->y * camera_scale->y;
}

ui_rect g_full_screen_percent = UI_RECT_INITLIZER( 0.0f, 0.0f, 1.0f, 1.0f );
