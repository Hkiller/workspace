#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_camera_env_i.h"
#include "ui_sprite_camera_module_i.h"

static void ui_sprite_camera_env_clear(ui_sprite_world_res_t world_res, void * ctx);

ui_sprite_camera_env_t
ui_sprite_camera_env_create(ui_sprite_camera_module_t module, ui_sprite_world_t world) {
    ui_sprite_camera_env_t camera;
    ui_sprite_world_res_t world_res = ui_sprite_world_res_create(world, UI_SPRITE_CAMERA_ENV_NAME, sizeof(struct ui_sprite_camera_env));

    camera = ui_sprite_world_res_data(world_res);

    camera->m_module = module;

    camera->m_render = ui_sprite_render_env_find(world);
    if (camera->m_render == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_env_create: no render env!");
        return NULL;
    }
    
    camera->m_limit_lt.x = 0.0f;
    camera->m_limit_lt.y = 0.0f;
    camera->m_limit_rb.x = 0.0f;
    camera->m_limit_rb.y = 0.0f;

    camera->m_max_op_id = 0;
    camera->m_curent_op_id = 0;

    camera->m_trace_type = ui_sprite_camera_trace_none;

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_camera_env_clear, module);

    return camera;
}

static void ui_sprite_camera_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
}

void ui_sprite_camera_env_free(ui_sprite_camera_env_t camera) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_from_data(camera);

    ui_sprite_world_res_free(world_res);
}

ui_sprite_camera_env_t ui_sprite_camera_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_CAMERA_ENV_NAME);
    return world_res ? ui_sprite_world_res_data(world_res) : NULL;
}


ui_vector_2 ui_sprite_camera_env_limit_lt(ui_sprite_camera_env_t camera) {
    return camera->m_limit_lt;
}

ui_vector_2 ui_sprite_camera_env_limit_rb(ui_sprite_camera_env_t camera) {
    return camera->m_limit_rb;
}

static float ui_sprite_camera_env_scale_max_for_limit(ui_sprite_camera_env_t camera) {
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    float scale_max_x;
    float scale_max_y;
    
    scale_max_x = (camera->m_limit_rb.x - camera->m_limit_lt.x) / screen_size->x;
    scale_max_y = (camera->m_limit_rb.y - camera->m_limit_lt.y) / screen_size->y;

    return scale_max_x < scale_max_y ? scale_max_x : scale_max_y;
}

int ui_sprite_camera_env_set_limit(ui_sprite_camera_env_t camera, ui_vector_2 limit_lt, ui_vector_2 limit_rb) {
    float scale_max;

    if (limit_lt.x >= limit_rb.x || limit_lt.y >= limit_rb.y) {
        CPE_ERROR(
            camera->m_module->m_em, "camera set limit: lt=(%f,%f), rb=(%f,%f) is error!",
            limit_lt.x, limit_lt.y, limit_rb.x, limit_rb.y);
        return -1;
    }

    camera->m_limit_lt = limit_lt;
    camera->m_limit_rb = limit_rb;

    scale_max = ui_sprite_camera_env_scale_max_for_limit(camera);
    if (scale_max < camera->m_scale_max) camera->m_scale_max = scale_max;

    return 0;
}

int8_t ui_sprite_camera_env_have_limit(ui_sprite_camera_env_t camera) {
    return camera->m_limit_lt.x < camera->m_limit_rb.x;
}

int ui_sprite_camera_env_set_scale_range(ui_sprite_camera_env_t camera, float scale_min, float scale_max) {
    if (scale_min > 0.0f && scale_max > 0.0f) {
        if (scale_min >= scale_max) {
            CPE_ERROR(camera->m_module->m_em, "camera set scale range: %f ~ %f is error!", scale_min, scale_max);
            return -1;
        }
    }

    if (ui_sprite_camera_env_have_limit(camera)) {
        float limit_max_scale = ui_sprite_camera_env_scale_max_for_limit(camera);
        if (scale_max > limit_max_scale) {
            CPE_ERROR(
                camera->m_module->m_em, "camera set scale range: %f ~ %f is error, max scale for limit is %f!",
                scale_min, scale_max, limit_max_scale);
            return -1;
        }
    }

    camera->m_scale_min = scale_min;
    if (scale_max > 0.0f) camera->m_scale_max = scale_max;

    return 0;
}

float ui_sprite_camera_env_scale_min(ui_sprite_camera_env_t camera) {
    return camera->m_scale_min;
}

float ui_sprite_camera_env_scale_max(ui_sprite_camera_env_t camera) {
    return camera->m_scale_max;
}

int ui_sprite_camera_env_set_trace(
    ui_sprite_camera_env_t camera, enum ui_sprite_camera_trace_type type,
    ui_vector_2 screen_pos, ui_vector_2 world_pos_a, ui_vector_2 world_pos_b)
{
    if (type != ui_sprite_camera_trace_by_x && type != ui_sprite_camera_trace_by_y) {
        CPE_ERROR(camera->m_module->m_em, "camera set trace: trace type %d not support", type);
        return -1;
    }

    if (type == ui_sprite_camera_trace_by_x) {
        if ((world_pos_b.x - world_pos_a.x) < 1.0f) {
            CPE_ERROR(
                camera->m_module->m_em, "camera set trace: trace by x, diff in x too small, a=(%f,%f), b=(%f,%f)",
                world_pos_a.x, world_pos_a.y, world_pos_b.x, world_pos_b.y);
            return -1;
        }

        camera->m_trace_line.m_by_x.m_dy_dx = (world_pos_b.y - world_pos_a.y) / (world_pos_b.x - world_pos_a.x);
        camera->m_trace_line.m_by_x.m_base_y = screen_pos.y - screen_pos.x * camera->m_trace_line.m_by_x.m_dy_dx;

        /* printf( */
        /*     "set trace: input=(%f,%f)-(%f,%f), base: %f-%f, dy_dx=%f\n", */
        /*     world_pos_a.x, world_pos_a.y, world_pos_b.x, world_pos_b.y, */
        /*     camera->m_trace_line.m_by_x.m_base_y,  */
        /*     camera->m_trace_line.m_by_x.m_base_y + camera->m_trace_line.m_by_x.m_dy_dx, */
        /*     camera->m_trace_line.m_by_x.m_dy_dx); */

    }
    else if (type == ui_sprite_camera_trace_by_y) {
        if ((world_pos_b.y - world_pos_a.y) < 1.0f) {
            CPE_ERROR(
                camera->m_module->m_em, "camera set trace: trace by y, diff in y too small, a=(%f,%f), b=(%f,%f)",
                world_pos_a.x, world_pos_a.y, world_pos_b.x, world_pos_b.y);
            return -1;
        }

        camera->m_trace_line.m_by_y.m_dx_dy = (world_pos_b.x - world_pos_a.x) / (world_pos_b.y - world_pos_a.y);
        camera->m_trace_line.m_by_y.m_base_x = screen_pos.x - screen_pos.y * camera->m_trace_line.m_by_y.m_dx_dy;
    }

    camera->m_trace_type = type;
    camera->m_trace_screen_pos = screen_pos;
    camera->m_trace_world_pos = world_pos_a;

    return 0;
}

void ui_sprite_camera_env_remove_trace(ui_sprite_camera_env_t camera) {
    camera->m_trace_type = ui_sprite_camera_trace_none;
}

uint32_t ui_sprite_camera_env_start_op(ui_sprite_camera_env_t camera) {
    camera->m_curent_op_id = ++camera->m_max_op_id;
    return camera->m_curent_op_id;
}

void ui_sprite_camera_env_stop_op(ui_sprite_camera_env_t camera, uint32_t op_id) {
    if (camera->m_curent_op_id == op_id) {
        camera->m_curent_op_id = 0;
    }
}

float ui_sprite_camera_env_trace_x2y(ui_sprite_camera_env_t camera, float camera_x, ui_vector_2_t scale) {
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_transform_t camera_trans = ui_sprite_render_env_transform(camera->m_render);
    float line_pos_y;

    assert(camera->m_trace_type == ui_sprite_camera_trace_by_x);

    line_pos_y = camera->m_trace_world_pos.y + (camera_x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;

    return line_pos_y - screen_size->y * camera_trans->m_s.y * camera->m_trace_screen_pos.y;
}

float ui_sprite_camera_env_trace_y2x(ui_sprite_camera_env_t camera, float camera_y, ui_vector_2_t scale) {
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    ui_transform_t camera_trans = ui_sprite_render_env_transform(camera->m_render);
    float line_pos_x;

    assert(camera->m_trace_type == ui_sprite_camera_trace_by_y);

    line_pos_x = camera->m_trace_world_pos.x + (camera_y - camera->m_trace_world_pos.y) * camera->m_trace_line.m_by_y.m_dx_dy;

    return line_pos_x - screen_size->x * camera_trans->m_s.x * camera->m_trace_screen_pos.x;
}

float ui_sprite_camera_env_screen_x2y_lock_x(ui_sprite_camera_env_t camera, float screen_x, ui_vector_2 world_pos, ui_vector_2_t scale) {
    ui_vector_2_t screen_size = ui_sprite_render_env_size(camera->m_render);
    float lock_pos_x;
    float lock_pos_y;

    assert(camera->m_trace_type == ui_sprite_camera_trace_by_x);

    lock_pos_x = world_pos.x - screen_size->x * (screen_x - camera->m_trace_screen_pos.x) * scale->x;
    lock_pos_y = camera->m_trace_world_pos.y + (lock_pos_x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;

    if (fabs(world_pos.x - lock_pos_x) < 0.49) {
        return camera->m_trace_screen_pos.y + (world_pos.y - lock_pos_y) / screen_size->y / scale->y;
    }
    else {
        float d = (world_pos.y - lock_pos_y) / (world_pos.x - lock_pos_x);

        return camera->m_trace_screen_pos.y + (screen_x - camera->m_trace_screen_pos.x) * d;
    }
}

static ui_sprite_world_res_t ui_sprite_camera_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_env_t camera;
    ui_sprite_camera_module_t camera_module;
    const char * camera_module_name;
    cfg_t limit_cfg;

    camera_module_name = cfg_get_string(cfg, "module", NULL);

    camera_module = ui_sprite_camera_module_find_nc(ui_sprite_world_app(world), camera_module_name);
    if (camera_module == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create camera resource: camera module %s not exist!",
            ui_sprite_camera_module_name(module), camera_module_name ? camera_module_name : "default");
        return NULL;
    }

    camera = ui_sprite_camera_env_create(camera_module, world);
    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create camera resource: create camera fail!",
            ui_sprite_camera_module_name(module));
        return NULL;
    }

    if ((limit_cfg = cfg_find_cfg(cfg, "limit"))) {
        ui_vector_2 limit_lt = ui_sprite_camera_env_limit_lt(camera);
        ui_vector_2 limit_rb = ui_sprite_camera_env_limit_rb(camera);

        limit_lt.x = cfg_get_float(limit_cfg, "lt.x", limit_lt.x);
        limit_lt.y = cfg_get_float(limit_cfg, "lt.y", limit_lt.y);
        limit_rb.x = cfg_get_float(limit_cfg, "rb.x", limit_rb.x);
        limit_rb.y = cfg_get_float(limit_cfg, "rb.y", limit_rb.y);

        if (ui_sprite_camera_env_set_limit(camera, limit_lt, limit_rb) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create camera resource: set camera limit fail!",
                ui_sprite_camera_module_name(module));
            return NULL;
        }
    }
    
    ui_sprite_camera_env_set_scale_range(
        camera, 
        cfg_get_float(cfg, "scale.min", ui_sprite_camera_env_scale_min(camera)),
        cfg_get_float(cfg, "scale.max", ui_sprite_camera_env_scale_min(camera)));

    return ui_sprite_world_res_from_data(camera);
}

int ui_sprite_camera_env_pos_of_entity(ui_vector_2 * pos, ui_sprite_world_t world, uint32_t entity_id, const char * entity_name, uint8_t pos_of_entity) {
    ui_sprite_entity_t entity;
    ui_sprite_2d_transform_t transform;

    if (entity_id) {
        entity = ui_sprite_entity_find_by_id(world, entity_id);
        if (entity == NULL) return -1;
    }
    else {
        entity = ui_sprite_entity_find_by_name(world, entity_name);
        if (entity == NULL) return -1;
    }

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) return -1;

    *pos = ui_sprite_2d_transform_world_pos(transform, pos_of_entity, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    return 0;
}

int ui_sprite_camera_env_regist(ui_sprite_camera_module_t module) {
    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_resource_loader(module->m_loader, UI_SPRITE_CAMERA_ENV_NAME, ui_sprite_camera_env_load, module) != 0) {
            CPE_ERROR(
                module->m_em, "%s: add default resource loader %s fail!", ui_sprite_camera_module_name(module),
                UI_SPRITE_CAMERA_ENV_NAME);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_env_unregist(ui_sprite_camera_module_t module) {
    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_CAMERA_ENV_NAME);
    }
}

const char * UI_SPRITE_CAMERA_ENV_NAME = "AnimationCamera";
