#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_camera_touch_i.h"
#include "ui_sprite_camera_utils.h"
#include "protocol/ui/sprite_camera/ui_sprite_camera_evt.h"

static void ui_sprite_camera_touch_scale_adj_by_trace_x(
    ui_sprite_camera_touch_t touch, ui_sprite_entity_t entity, 
    ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_vector_2_t camera_pos, ui_vector_2_t camera_scale,
    ui_vector_2_t lock_screen_pos, ui_vector_2_t lock_world_pos);

int ui_sprite_camera_touch_on_scale_2_finter_no_trace(
    ui_sprite_camera_touch_t touch, ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_sprite_entity_t entity,
    ui_vector_2 * target_pos, ui_vector_2_t target_scale,
    UI_SPRITE_EVT_CAMERA_TOUCH_SCALE const * evt_data)
{
    /*ui_sprite_camera_module_t module = touch->m_module;*/
    ui_vector_2 start_world_pos[2];
    ui_vector_2 lock_pos_in_screen;
    ui_vector_2 lock_pos_in_world;
    ui_vector_2 moved_0;
    ui_vector_2 moved_1;
    float cur_screen_distance_x;
    float cur_screen_distance_y;
    float world_distance_x;
    float world_distance_y;

    start_world_pos[0].x = touch->m_init_camera_rect.lt.x
        + (touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x) 
        * (evt_data->start_screen_pos[0].x / screen_size->x);
    start_world_pos[0].y = touch->m_init_camera_rect.lt.y
        + (touch->m_init_camera_rect.rb.y - touch->m_init_camera_rect.lt.y) 
        * (evt_data->start_screen_pos[0].y / screen_size->y);
    start_world_pos[1].x = touch->m_init_camera_rect.lt.x
        + (touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x) 
        * (evt_data->start_screen_pos[1].x / screen_size->x);
    start_world_pos[1].y = touch->m_init_camera_rect.lt.y
        + (touch->m_init_camera_rect.rb.y - touch->m_init_camera_rect.lt.y) 
        * (evt_data->start_screen_pos[1].y / screen_size->y);

    world_distance_x = fabs(start_world_pos[0].x - start_world_pos[1].x);
    world_distance_y = fabs(start_world_pos[0].y - start_world_pos[1].y);
    cur_screen_distance_x = fabs(evt_data->curent_screen_pos[0].x - evt_data->curent_screen_pos[1].x);
    cur_screen_distance_y = fabs(evt_data->curent_screen_pos[0].y - evt_data->curent_screen_pos[1].y);

    if (cur_screen_distance_x > cur_screen_distance_y) {
        if (cur_screen_distance_x < 5) {
            return -1;
        }
        else {
            target_scale->x =  world_distance_x / cur_screen_distance_x;
        }
    }
    else {
        if (cur_screen_distance_y < 5) {
            return -1;
        }
        else {
            target_scale->y = world_distance_y / cur_screen_distance_y;
        }
    }
    
    moved_0.x = fabs(evt_data->curent_screen_pos[0].x - evt_data->start_screen_pos[0].x);
    moved_0.y = fabs(evt_data->curent_screen_pos[0].y - evt_data->start_screen_pos[0].y);
    moved_1.x = fabs(evt_data->curent_screen_pos[1].x - evt_data->start_screen_pos[1].x);
    moved_1.y = fabs(evt_data->curent_screen_pos[1].y - evt_data->start_screen_pos[1].y);
        
    lock_pos_in_screen.x =
        (moved_0.x + moved_1.x < 1)
        ? evt_data->start_screen_pos[0].x
        : (evt_data->start_screen_pos[0].x
           + (evt_data->start_screen_pos[1].x - evt_data->start_screen_pos[1].y) * moved_0.x / (moved_0.x + moved_1.x));

    lock_pos_in_screen.y =
        (moved_0.y + moved_1.y < 1)
        ? evt_data->start_screen_pos[0].y
        : (evt_data->start_screen_pos[0].y
           + (evt_data->start_screen_pos[1].y - evt_data->start_screen_pos[1].y) * moved_0.y / (moved_0.y + moved_1.y));

    lock_pos_in_world.x =
        touch->m_init_camera_rect.lt.x
        + ((touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x) * (lock_pos_in_screen.x / screen_size->x));

    lock_pos_in_world.y =
        touch->m_init_camera_rect.lt.y
        + ((touch->m_init_camera_rect.rb.y - touch->m_init_camera_rect.lt.y) * (lock_pos_in_screen.y / screen_size->y));

    if (ui_sprite_camera_env_have_limit(camera)) {
        
    }

    target_pos->x = lock_pos_in_world.x - (lock_pos_in_screen.x * target_scale->x);
    target_pos->y = lock_pos_in_world.y - (lock_pos_in_screen.y * target_scale->y);
    
    return 0;
}

int ui_sprite_camera_touch_on_scale_2_finter_trace_by_x(
    ui_sprite_camera_touch_t touch, ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_sprite_entity_t entity,
    ui_vector_2_t target_pos, ui_vector_2_t target_scale,
    UI_SPRITE_EVT_CAMERA_TOUCH_SCALE const * evt_data)
{
    ui_sprite_camera_module_t module = touch->m_module;
    uint8_t l_pos = 0;
    uint8_t r_pos = 1;
    float world_x_l;
    float world_x_r;
    ui_vector_2 lock_pos_in_screen;
    ui_vector_2 lock_pos_in_world;

    if (fabs(evt_data->start_screen_pos[0].x - evt_data->start_screen_pos[1].x) < 5) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: screen pos too close in x, %f and %f!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            evt_data->start_screen_pos[0].x, evt_data->start_screen_pos[1].x);
        return -1;
    }

    if (evt_data->start_screen_pos[0].x < evt_data->start_screen_pos[1].x) {
        l_pos = 0; r_pos = 1;
    }
    else {
        l_pos = 1; r_pos = 0;
    }

    if (evt_data->curent_screen_pos[l_pos].x >= evt_data->curent_screen_pos[r_pos].x) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: finger swiped!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    world_x_l = touch->m_init_camera_rect.lt.x
        + (touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x) 
        * (evt_data->start_screen_pos[l_pos].x / screen_size->x);
        
    world_x_r = touch->m_init_camera_rect.lt.x
        + (touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x)
        * (evt_data->start_screen_pos[r_pos].x / screen_size->x);

    assert(world_x_r > world_x_l);

    target_scale->x = (world_x_r - world_x_l) / (evt_data->curent_screen_pos[r_pos].x - evt_data->curent_screen_pos[l_pos].x);

    target_pos->x = world_x_l - (evt_data->curent_screen_pos[l_pos].x * target_scale->x);

    lock_pos_in_screen.x = evt_data->curent_screen_pos[l_pos].x / screen_size->x;
    lock_pos_in_screen.y = evt_data->curent_screen_pos[l_pos].y / screen_size->y;

    lock_pos_in_world.x = world_x_l;
    lock_pos_in_world.y = touch->m_init_camera_rect.lt.y
        + (touch->m_init_camera_rect.rb.y - touch->m_init_camera_rect.lt.y) 
        * (evt_data->start_screen_pos[l_pos].y / screen_size->y);

    if (ui_sprite_camera_env_have_limit(camera)) {
        ui_sprite_camera_touch_scale_adj_by_trace_x(
            touch, entity, camera, screen_size, target_pos, target_scale, &lock_pos_in_screen, &lock_pos_in_world);
    }

    return 0;
}

void ui_sprite_camera_touch_on_scale(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_camera_touch_t touch = ctx;
    ui_sprite_camera_module_t module = touch->m_module;
    UI_SPRITE_EVT_CAMERA_TOUCH_SCALE const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_vector_2 target_pos;
    ui_vector_2 target_scale;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (evt_data->finger_count == 2) {
        switch(camera->m_trace_type) {
        case ui_sprite_camera_trace_none:
            if (ui_sprite_camera_touch_on_scale_2_finter_no_trace(
                    touch, camera, screen_size, entity,
                    &target_pos, &target_scale, evt_data)
                != 0)
            {
                return;
            }
            break;
        case ui_sprite_camera_trace_by_x:
            if (ui_sprite_camera_touch_on_scale_2_finter_trace_by_x(
                    touch, camera, screen_size, entity,
                    &target_pos, &target_scale, evt_data)
                != 0)
            {
                return;
            }
            break;
        case ui_sprite_camera_trace_by_y:
            assert(0);
            return;
        default:
            break;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: not support scale by finger %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->finger_count);
        return;
    }

    touch->m_state = ui_sprite_camera_touch_state_move;

    ui_sprite_camera_updator_set_max_speed(&touch->m_updator, evt_data->max_speed);

    ui_sprite_camera_updator_set_camera(&touch->m_updator, camera, target_pos, &target_scale);
}

static void ui_sprite_camera_touch_scale_adj_by_trace_x(
    ui_sprite_camera_touch_t touch, ui_sprite_entity_t entity, 
    ui_sprite_camera_env_t camera, ui_vector_2_t screen_size, ui_vector_2_t camera_pos, ui_vector_2_t camera_scale,
    ui_vector_2_t lock_screen_pos, ui_vector_2_t lock_world_pos)
{
    ui_sprite_camera_module_t module = touch->m_module;
    ui_rect screen_rect;

    camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);
    lock_screen_pos->y = camera->m_trace_line.m_by_x.m_base_y + camera->m_trace_line.m_by_x.m_dy_dx * lock_screen_pos->x;
    lock_world_pos->y = camera->m_trace_world_pos.y + (lock_world_pos->x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;

    camera_scale->x = cpe_limit_in_range(camera_scale->x, camera->m_scale_min, camera->m_scale_max);
    camera_scale->y = cpe_limit_in_range(camera_scale->y, camera->m_scale_min, camera->m_scale_max);
    
    ui_sprite_camera_screen_world_rect(
        &screen_rect, &g_full_screen_percent, camera, camera_pos, camera_scale);

    if (lock_world_pos->y > camera->m_limit_lt.y && lock_screen_pos->y > 0.0f) {
        float scale_up = (lock_world_pos->y - camera->m_limit_lt.y) / (screen_size->y * lock_screen_pos->y);
        if (scale_up < camera->m_scale_min) scale_up = camera->m_scale_min;
        if (camera_scale->y > scale_up) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: limit by up: %f ==> %f"
                    "track=(%f-%f), init-screen=(%f,%f)-(%f,%f), limit=(%f,%f)-(%f,%f), lock-world-pos=(%f,%f), lock-screen-pos=(%f,%f)",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    camera_scale->y, scale_up,
                    camera->m_trace_line.m_by_x.m_base_y, camera->m_trace_line.m_by_x.m_dy_dx,
                    touch->m_init_camera_rect.lt.x, touch->m_init_camera_rect.lt.y, touch->m_init_camera_rect.rb.x, touch->m_init_camera_rect.rb.y,
                    camera->m_limit_lt.x, camera->m_limit_lt.y, camera->m_limit_rb.x, camera->m_limit_rb.y,
                    lock_world_pos->x, lock_world_pos->y, lock_screen_pos->x, lock_screen_pos->y);
            }

            camera_scale->y = scale_up;
            camera_pos->x = lock_world_pos->x - screen_size->x * lock_screen_pos->x * camera_scale->y;
        }
    }

    if (lock_world_pos->y < camera->m_limit_rb.y && lock_screen_pos->y < 1.0f) {
        float scale_down = (camera->m_limit_rb.y - lock_world_pos->y) / (screen_size->y * (1 - lock_screen_pos->y));
        if (scale_down < camera->m_scale_min) scale_down = camera->m_scale_min;
        if (camera_scale->y > scale_down) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: limit by down: %f ==> %f"
                    "track=(%f-%f), init-screen=(%f,%f)-(%f,%f), limit=(%f,%f)-(%f,%f), lock-world-pos=(%f,%f), lock-screen-pos=(%f,%f)",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    camera_scale->y, scale_down,
                    camera->m_trace_line.m_by_x.m_base_y, camera->m_trace_line.m_by_x.m_dy_dx,
                    touch->m_init_camera_rect.lt.x, touch->m_init_camera_rect.lt.y, touch->m_init_camera_rect.rb.x, touch->m_init_camera_rect.rb.y,
                    camera->m_limit_lt.x, camera->m_limit_lt.y, camera->m_limit_rb.x, camera->m_limit_rb.y,
                    lock_world_pos->x, lock_world_pos->y, lock_screen_pos->x, lock_screen_pos->y);
            }

            camera_scale->y = scale_down;
            camera_pos->x = lock_world_pos->x - screen_size->x * lock_screen_pos->x * camera_scale->y;
        }
    }
    camera_pos->y = ui_sprite_camera_env_trace_x2y(camera, camera_pos->x, camera_scale);

    if (camera_pos->x < camera->m_limit_lt.x) {
        camera_pos->x = camera->m_limit_lt.x;
    }
    else if ((camera_pos->x + screen_size->x * camera_scale->x) > camera->m_limit_rb.x) {
        camera_pos->x = camera->m_limit_rb.x - screen_size->x * camera_scale->x;
    }
}

