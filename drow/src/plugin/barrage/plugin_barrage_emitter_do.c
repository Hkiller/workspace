#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin_barrage_emitter_i.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_bullet_proto_i.h"
#include "plugin_barrage_data_emitter_i.h"
#include "plugin_barrage_trigger_op_i.h"
#include "plugin_barrage_utils_i.h"

void plugin_barrage_emitter_emit(plugin_barrage_emitter_t emitter) {
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;
    plugin_barrage_module_t module = env->m_module;
    BARRAGE_EMITTER const * data_emitter = &emitter->m_data;
    uint8_t i;
    float emitter_radians_step;
    float emitter_angle;
    float emitter_radians;
    float pos_radians_step;
    float pos_angle;
    float pos_radians;
    BARRAGE_EMITTER_BULLET_DATA const * emitter_bullet_data;
    ui_vector_2_t texture_size = NULL;
    ui_vector_2 base_size;
    uint8_t emitter_count;

    assert(emitter->m_data.next_emitter_count > 0);

    emitter_count = emitter->m_data.next_emitter_count;

    emitter->m_data.next_emitter_count = plugin_barrage_calc_uint8(&emitter->m_data.emitter.emitter_count);

    assert(emitter->m_barrage->m_data.frame > 0);

    /*准备数据 */
    /*    发射角度 */
    emitter_radians_step =
        cpe_math_angle_to_radians(plugin_barrage_calc_float(&emitter->m_data.emitter.emitter_angle_range))
        / emitter_count;

    switch(emitter->m_data.emitter.emitter_angle.type) {
    case barrage_emitter_value_value:
        emitter_angle = emitter->m_barrage->m_angle
            + data_emitter->world_angle
            + plugin_barrage_calc_float(&emitter->m_data.emitter.emitter_angle.data.value);
        break;
    case barrage_emitter_value_calc:
        if (plugin_barrage_calc_value(&emitter_angle, emitter->m_data.emitter.emitter_angle.data.calc_type, emitter, NULL, env->m_module->m_em) != 0) {
            emitter_angle = emitter->m_barrage->m_angle + data_emitter->world_angle;
        }
        break;
    default:
        CPE_ERROR(env->m_module->m_em, "plugin_barrage_emitter_emit: unknown value type %d", emitter->m_data.emitter.emitter_angle.type);
        emitter_angle = emitter->m_barrage->m_angle + data_emitter->world_angle;
        break;
    }

    emitter_radians =
        cpe_math_radians_regular(
            cpe_math_angle_to_radians(emitter_angle)
            - emitter_radians_step * (emitter_count - 1) / 2.0f);

    /*    发射点角度 */
    pos_radians_step = cpe_math_angle_to_radians(360.0f) / emitter_count;

    pos_angle = emitter->m_barrage->m_angle
        + data_emitter->world_angle
        + plugin_barrage_calc_float(&emitter->m_data.emitter.emitter_pos_angle);

    pos_radians =
        cpe_math_radians_regular(
            cpe_math_angle_to_radians(pos_angle)
            - pos_radians_step * (emitter_count - 1) / 2.0f);

    /**/
    emitter_bullet_data = &emitter->m_data.bullet;

    /*根据数量创建子弹 */
    for(i = 0; i < emitter_count; ++i) {
        plugin_barrage_bullet_t bullet;
        BARRAGE_BULLET * bullet_data;
        float emitter_pos_radius;
        ui_vector_2 pos;

        if (texture_size == NULL) {
            texture_size = plugin_particle_obj_emitter_texture_size(emitter->m_bullet_proto->m_emitter);
            assert(texture_size);
            base_size.x = emitter_bullet_data->scale.x * texture_size->x;
            base_size.y = emitter_bullet_data->scale.y * texture_size->y;

            switch(emitter->m_flip_type) {
            case plugin_barrage_data_emitter_flip_type_x:
                base_size.x *= -1.0f;
                break;
            case plugin_barrage_data_emitter_flip_type_y:
                base_size.y *= -1.0f;
                break;
            case plugin_barrage_data_emitter_flip_type_xy:
                base_size.x *= -1.0f;
                base_size.y *= -1.0f;
                break;
            default:
                break;
            }

            /* printf( */
            /*     "xxx: texture-size=(%f,%f), scale=(%f,%f), base-size=(%f,%f)\n", */
            /*     texture_size->x, texture_size->y, */
            /*     emitter_bullet_data->scale.x, emitter_bullet_data->scale.y, */
            /*     base_size.x, base_size.y); */
        }

        /*创建bullet */
        bullet = plugin_barrage_bullet_create(
            emitter,
            emitter->m_data_emitter->m_bullet_frame_triggers_begin,
            emitter->m_data_emitter->m_bullet_check_triggers);
        if (bullet == NULL) {
            CPE_ERROR(module->m_em, "generate bullet fail!");
            return;
        }
        bullet_data = &bullet->m_data;

        bullet_data->frame = 0;

        plugin_particle_obj_particle_set_spin_init(
            bullet->m_particle,
            cpe_math_angle_to_radians(plugin_barrage_calc_float(&emitter_bullet_data->angle)));
        plugin_particle_obj_particle_set_base_size(bullet->m_particle, &base_size);
		bullet_data->color = emitter_bullet_data->color;
		bullet_data->acceleration = plugin_barrage_calc_float(&emitter_bullet_data->acceleration);
		bullet_data->acceleration_angle = plugin_barrage_calc_float(&emitter_bullet_data->acceleration_angle);
		bullet_data->x_rate = plugin_barrage_calc_float(&emitter_bullet_data->x_rate);
		bullet_data->y_rate = plugin_barrage_calc_float(&emitter_bullet_data->y_rate);
		bullet_data->life_circle = plugin_barrage_calc_uint16(&emitter_bullet_data->life_circle);
		bullet_data->angle_to_speed = emitter_bullet_data->angle_to_speed;

        /*计算子弹发射位置 */
		pos.x = emitter->m_barrage->m_pos.x + data_emitter->world_pos.x + data_emitter->emitter.pos.x;
		pos.y = emitter->m_barrage->m_pos.y + data_emitter->world_pos.y + data_emitter->emitter.pos.y;

        emitter_pos_radius = plugin_barrage_calc_float(&data_emitter->emitter.emitter_pos_radius);
		pos.x += emitter_pos_radius * cpe_cos_radians(pos_radians);
		pos.y += emitter_pos_radius * cpe_sin_radians(pos_radians);

        cpBodySetPosition(&bullet->m_body, cpv(pos.x, pos.y));
        plugin_particle_obj_particle_set_pos(bullet->m_particle, &pos);
        
		bullet_data->speed = plugin_barrage_calc_float(&emitter_bullet_data->speed);
		bullet_data->speed_angle_rad = emitter_radians;

        /*计算子弹的速度 */

		plugin_barrage_bullet_update_speed_angle(bullet);
        plugin_barrage_bullet_update_shape(bullet);

        emitter_radians = cpe_math_radians_regular(emitter_radians + emitter_radians_step);
        pos_radians = cpe_math_radians_regular(pos_radians + pos_radians_step);
    }
}
    
void plugin_barrage_emitter_trigger_once(void * ctx) {
    plugin_barrage_emitter_t emitter = (plugin_barrage_emitter_t)ctx;
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;
    uint8_t emitter_span;

    assert(emitter->m_data.next_emitter_span > 0);

    emitter->m_data.next_emitter_span = plugin_barrage_calc_uint8(&emitter->m_data.emitter.emitter_span);

    emitter_span = emitter->m_barrage->m_emitter_adj > 0.0f
        ? (emitter->m_data.next_emitter_span / emitter->m_barrage->m_emitter_adj)
        : 1;
    if (emitter_span < 1) emitter_span = 1;
    
    /*构造下一次子弹发射的触发器 */
    if (emitter->m_barrage->m_data.frame + emitter_span <= emitter->m_data.frame_complete) {
        emitter->m_emitter_op.m_op_frame = env->m_cur_frame + emitter_span;
        emitter->m_emitter_op.m_op_fun = plugin_barrage_emitter_trigger_once;
        emitter->m_emitter_op.m_op_ctx = emitter;
        plugin_barrage_op_enqueue(env, &emitter->m_emitter_op);
    }

    plugin_barrage_emitter_emit(emitter);
}
