#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui_sprite_camera_updator.h"

void ui_sprite_camera_updator_set_max_speed(ui_sprite_camera_updator_t updator, float max_speed) {
    updator->m_max_speed = max_speed;
}

void ui_sprite_camera_updator_stop(ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera) {
    if (updator->m_curent_op_id) {
        if (camera) ui_sprite_camera_env_stop_op(camera, updator->m_curent_op_id);
        updator->m_curent_op_id = 0;
    }

    updator->m_duration = 0.0f;
}

static void ui_sprite_camera_updator_update_data(
    ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_transform_t camera_transform)
{
    float diff_lt;
    float diff_rb;

    ui_transform_get_pos_2(camera_transform, &updator->m_origin_rect.lt);
    updator->m_origin_rect.rb.x = updator->m_origin_rect.lt.x + screen_size->x * camera_transform->m_s.x;
    updator->m_origin_rect.rb.y = updator->m_origin_rect.lt.y + screen_size->y * camera_transform->m_s.y;

    diff_lt = cpe_math_distance(
        updator->m_origin_rect.lt.x, updator->m_origin_rect.lt.y,
        updator->m_target_rect.lt.x, updator->m_target_rect.lt.y);

    diff_rb = cpe_math_distance(
        updator->m_origin_rect.rb.x, updator->m_origin_rect.rb.y,
        updator->m_target_rect.rb.x, updator->m_target_rect.rb.y);

    updator->m_duration =
        diff_lt > diff_rb
        ? diff_lt / updator->m_max_speed
        : diff_rb / updator->m_max_speed;

    updator->m_runing_time = 0.0f;
}

void ui_sprite_camera_updator_set_camera(
    ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera, ui_vector_2 pos, ui_vector_2_t scale)
{
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
    ui_rect target_rect;

    if (updator->m_max_speed == 0.0f) {
        ui_transform new_transform = *camera_transform;
        ui_vector_3 s = UI_VECTOR_3_INITLIZER(scale->x, scale->y, 1.0f);
        
        assert(updator->m_curent_op_id == 0);

        ui_transform_set_scale(&new_transform, &s);
        ui_transform_set_pos_2(&new_transform, &pos);
        
        updator->m_curent_op_id = ui_sprite_camera_env_start_op(camera);
        ui_sprite_render_env_set_transform(camera->m_render, &new_transform);
        ui_sprite_camera_env_stop_op(camera, updator->m_curent_op_id);
        updator->m_curent_op_id = 0;

        return;
    }

    target_rect.lt = pos;
    target_rect.rb.x = pos.x + screen_size->x * scale->x;
    target_rect.rb.y = pos.y + screen_size->y * scale->y;

    if (updator->m_duration > 0.0f && ui_sprite_2d_rect_eq(&target_rect, &updator->m_target_rect, 0.1f)) {
        return;
    }

    updator->m_target_rect = target_rect;
    ui_sprite_camera_updator_update_data(updator, camera, screen_size, camera_transform);

    if (updator->m_duration == 0.0f) {
        ui_sprite_camera_updator_stop(updator, camera);
        return;
    }

    if (updator->m_curent_op_id == 0) {
        updator->m_curent_op_id = ui_sprite_camera_env_start_op(camera);
    }

    assert(updator->m_curent_op_id == camera->m_curent_op_id);
}

void ui_sprite_camera_updator_update(ui_sprite_camera_updator_t updator, ui_sprite_camera_env_t camera, float delta) {
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_transform_t camera_transform = ui_sprite_render_env_transform(camera->m_render);
    ui_transform new_transform = *camera_transform;
    ui_vector_3 s;
    float percent;
    ui_vector_2 pos_lt;
    ui_vector_2 pos_rb;
    float scale;

    assert(screen_size->x > 0);
    assert(screen_size->y > 0);

    if (!ui_sprite_camera_updator_is_runing(updator)) return;

    assert(updator->m_duration > 0.0f);

    if (updator->m_curent_op_id != camera->m_curent_op_id) {
        if (camera->m_curent_op_id != 0) return;
        updator->m_curent_op_id = ui_sprite_camera_env_start_op(camera);
        ui_sprite_camera_updator_update_data(updator, camera, screen_size, camera_transform);

        if (updator->m_duration == 0.0f) {
            ui_sprite_camera_updator_stop(updator, camera);
            return;
        }
    }

    updator->m_runing_time += delta;

    percent = 
        updator->m_runing_time > updator->m_duration
        ? 1.0f
        : (updator->m_runing_time / updator->m_duration);

    percent = ui_percent_decorator_decorate(&updator->m_decorator, percent);

    pos_lt.x = updator->m_origin_rect.lt.x + (updator->m_target_rect.lt.x - updator->m_origin_rect.lt.x) * percent;
    pos_lt.y = updator->m_origin_rect.lt.y + (updator->m_target_rect.lt.y - updator->m_origin_rect.lt.y) * percent;

    pos_rb.x = updator->m_origin_rect.rb.x + (updator->m_target_rect.rb.x - updator->m_origin_rect.rb.x) * percent;
    pos_rb.y = updator->m_origin_rect.rb.y + (updator->m_target_rect.rb.y - updator->m_origin_rect.rb.y) * percent;

    scale = fabs(pos_rb.x - pos_lt.x) / screen_size->x;

    /* /\* printf("xxxxx: duration = %f, percent=%f, pos-in-world=(%f,%f), pos-in-screen=(%f,%f), " *\/ */
    /* /\*        "origin-pos=(%f,%f), camera-pos=(%f,%f), camera-scale=%f\n", *\/ */
    /* /\*        move->m_duration, percent, move->m_pos_in_world.x, move->m_pos_in_world.y, *\/ */
    /* /\*        move->m_pos_in_screen.x, move->m_pos_in_screen.y, *\/ */
    /* /\*        move->m_camera_orig_pos.x, move->m_camera_orig_pos.y, *\/ */
    /* /\*        target_camera_pos.x, target_camera_pos.y, target_camera_scale); *\/ */

    s.x = s.y = scale; s.z = 1.0f;
    ui_transform_set_scale(&new_transform, &s);
    ui_transform_set_pos_2(&new_transform, &pos_lt);
    ui_sprite_render_env_set_transform(camera->m_render, &new_transform);

    if (percent >= 1.0f) ui_sprite_camera_updator_stop(updator, camera);
}

uint8_t ui_sprite_camera_updator_is_runing(ui_sprite_camera_updator_t updator) {
    return updator->m_curent_op_id != 0;
}
