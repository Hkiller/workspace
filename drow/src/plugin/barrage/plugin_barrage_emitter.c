#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_log.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin_barrage_emitter_i.h"
#include "plugin_barrage_group_i.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_bullet_proto_i.h"
#include "plugin_barrage_trigger_op_i.h"
#include "plugin_barrage_utils_i.h"

static void plugin_barrage_emitter_reset_data(plugin_barrage_emitter_t emitter);

plugin_barrage_emitter_t
plugin_barrage_emitter_create(
    plugin_barrage_barrage_t barrage,
    plugin_barrage_data_emitter_t data_emitter,
    plugin_barrage_data_emitter_flip_type_t flip_type)
{
    plugin_barrage_env_t env = barrage->m_group->m_env;
    plugin_barrage_module_t module = env->m_module;
    plugin_barrage_emitter_t emitter;
    plugin_barrage_bullet_proto_t bullet_proto;

    if (barrage->m_frame_loop != 0) {
        if (barrage->m_frame_loop != data_emitter->m_data.frame_loop) {
            CPE_ERROR(
                module->m_em, "plugin_barrage_emitter_create: barrage frame loop is %d, emitter loop frame is %d, mismatch!",
                barrage->m_frame_loop, data_emitter->m_data.frame_loop);
            return NULL;
        }
    }
    
    bullet_proto = plugin_barrage_bullet_proto_find(barrage->m_group, data_emitter->m_data.bullet.proto);
    if (bullet_proto == NULL) {
        bullet_proto = plugin_barrage_bullet_proto_create(barrage->m_group, data_emitter->m_data.bullet.proto);
        if (bullet_proto == NULL) {
            CPE_ERROR(module->m_em, "plugin_barrage_emitter_create: bullet proto %s create fail!", data_emitter->m_data.bullet.proto);
            return NULL;
        }
    }

    emitter = TAILQ_FIRST(&env->m_free_emitters);
    if (emitter) {
        TAILQ_REMOVE(&env->m_free_emitters, emitter, m_next_for_barrage);
    }
    else {
        emitter = (plugin_barrage_emitter_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_emitter));
        if (emitter == NULL) {
            CPE_ERROR(module->m_em, "create emitter alloc fail!");
            return NULL;
        }
    }
    
    emitter->m_barrage = barrage;
    emitter->m_data_emitter = data_emitter;
    emitter->m_bullet_proto = bullet_proto;
    emitter->m_next_trigger = NULL;
    
    bzero(&emitter->m_emitter_op, sizeof(emitter->m_emitter_op));
    
    bzero(&emitter->m_data, sizeof(emitter->m_data));
    
    plugin_barrage_emitter_reset_data(emitter);

    emitter->m_is_working = 0;
    emitter->m_flip_type = flip_type;

    env->m_emitter_count++;
    TAILQ_INIT(&emitter->m_bullets);
    TAILQ_INIT(&emitter->m_trigger_ops);

    if (barrage->m_frame_loop == 0) {
        barrage->m_frame_loop = data_emitter->m_data.frame_loop;
    }
    
    TAILQ_INSERT_TAIL(&barrage->m_emitters, emitter, m_next_for_barrage);
    
    return emitter;
}

void plugin_barrage_emitter_free(plugin_barrage_emitter_t emitter) {
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;

    while(!TAILQ_EMPTY(&emitter->m_bullets)) {
        plugin_barrage_bullet_remove_emitter(TAILQ_FIRST(&emitter->m_bullets));
    }

    if (emitter->m_is_working) {
        plugin_barrage_emitter_stop(emitter);
        assert(!emitter->m_is_working);
    }
    assert(emitter->m_emitter_op.m_op_fun == NULL);
    assert(TAILQ_EMPTY(&emitter->m_trigger_ops));

    env->m_emitter_count--;

    TAILQ_REMOVE(&emitter->m_barrage->m_emitters, emitter, m_next_for_barrage);

    if (TAILQ_EMPTY(&emitter->m_barrage->m_emitters)) {
        emitter->m_barrage->m_frame_loop = 0;
    }

    emitter->m_barrage = (plugin_barrage_barrage_t)env;
    TAILQ_INSERT_TAIL(&env->m_free_emitters, emitter, m_next_for_barrage);
}

void plugin_barrage_emitter_real_free(plugin_barrage_emitter_t emitter) {
    plugin_barrage_env_t env = (plugin_barrage_env_t)emitter->m_barrage;
    TAILQ_REMOVE(&env->m_free_emitters, emitter, m_next_for_barrage);
    mem_free(env->m_module->m_alloc, emitter);
}
    
void plugin_barrage_emitter_reset_data(plugin_barrage_emitter_t emitter) {
    emitter->m_data.emitter.pos.x = plugin_barrage_calc_float(&emitter->m_data_emitter->m_data.emitter.pos.x);
    emitter->m_data.emitter.pos.y = plugin_barrage_calc_float(&emitter->m_data_emitter->m_data.emitter.pos.y);
    emitter->m_data.emitter.speed = plugin_barrage_calc_float(&emitter->m_data_emitter->m_data.emitter.speed);
    emitter->m_data.emitter.speed_angle = plugin_barrage_calc_float(&emitter->m_data_emitter->m_data.emitter.speed_angle);
    emitter->m_data.emitter.acceleration = plugin_barrage_calc_float(&emitter->m_data_emitter->m_data.emitter.acceleration);
    emitter->m_data.emitter.acceleration_angle = plugin_barrage_calc_float(&emitter->m_data_emitter->m_data.emitter.acceleration_angle);
    emitter->m_data.emitter.emitter_pos_radius = emitter->m_data_emitter->m_data.emitter.emitter_pos_radius;
    emitter->m_data.emitter.emitter_pos_angle = emitter->m_data_emitter->m_data.emitter.emitter_pos_angle;
    emitter->m_data.emitter.emitter_count = emitter->m_data_emitter->m_data.emitter.emitter_count;
    emitter->m_data.emitter.emitter_span = emitter->m_data_emitter->m_data.emitter.emitter_span;
    emitter->m_data.emitter.emitter_angle = emitter->m_data_emitter->m_data.emitter.emitter_angle;
    emitter->m_data.emitter.emitter_angle_range = emitter->m_data_emitter->m_data.emitter.emitter_angle_range;

    emitter->m_data.next_emitter_span = plugin_barrage_calc_uint8(&emitter->m_data.emitter.emitter_span);
    emitter->m_data.next_emitter_count = plugin_barrage_calc_uint8(&emitter->m_data.emitter.emitter_count);
    
    cpe_str_dup(emitter->m_data.bullet.proto, sizeof(emitter->m_data.bullet.proto), emitter->m_data_emitter->m_data.bullet.proto);
    emitter->m_data.bullet.life_circle = emitter->m_data_emitter->m_data.bullet.life_circle;
    plugin_barrage_calc_pair(&emitter->m_data.bullet.scale, &emitter->m_data_emitter->m_data.bullet.scale);
    emitter->m_data.bullet.angle = emitter->m_data_emitter->m_data.bullet.angle;
    emitter->m_data.bullet.color = emitter->m_data_emitter->m_data.bullet.color;
    emitter->m_data.bullet.speed = emitter->m_data_emitter->m_data.bullet.speed;
    emitter->m_data.bullet.acceleration = emitter->m_data_emitter->m_data.bullet.acceleration;
    emitter->m_data.bullet.acceleration_angle = emitter->m_data_emitter->m_data.bullet.acceleration_angle;
    emitter->m_data.bullet.angle_to_speed = emitter->m_data_emitter->m_data.bullet.angle_to_speed;
    emitter->m_data.bullet.x_rate = emitter->m_data_emitter->m_data.bullet.x_rate;
    emitter->m_data.bullet.y_rate = emitter->m_data_emitter->m_data.bullet.y_rate;
}
    
void plugin_barrage_emitter_reset(plugin_barrage_emitter_t emitter) {
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;

    if (emitter->m_emitter_op.m_op_fun != NULL) {
        plugin_barrage_op_dequeue(env, &emitter->m_emitter_op);
    }

    plugin_barrage_emitter_reset_data(emitter);
    emitter->m_data.frame_start = plugin_barrage_calc_uint16(&emitter->m_data_emitter->m_data.frame_start);
    emitter->m_data.frame_complete = emitter->m_data.frame_start + plugin_barrage_calc_uint16(&emitter->m_data_emitter->m_data.frame_duration) - 1;
    emitter->m_next_trigger = emitter->m_data_emitter->m_emitter_frame_triggers_begin;

    if (emitter->m_data.emitter.emitter_angle.type == barrage_emitter_value_value) {
        if (emitter->m_flip_type == plugin_barrage_data_emitter_flip_type_x || emitter->m_flip_type == plugin_barrage_data_emitter_flip_type_xy) {
            emitter->m_data.emitter.emitter_angle.data.value.base = cpe_math_angle_regular(180.0f - emitter->m_data.emitter.emitter_angle.data.value.base);
        }

        if (emitter->m_flip_type == plugin_barrage_data_emitter_flip_type_y || emitter->m_flip_type == plugin_barrage_data_emitter_flip_type_xy) {
            emitter->m_data.emitter.emitter_angle.data.value.base = cpe_math_angle_regular(- emitter->m_data.emitter.emitter_angle.data.value.base);
        }
    }

    if (emitter->m_data.next_emitter_span > 0 && emitter->m_data.next_emitter_count > 0) {
        uint32_t emitter_span =
            emitter->m_barrage->m_emitter_adj > 0.0f
            ? (emitter->m_data.next_emitter_span / emitter->m_barrage->m_emitter_adj)
            : 1;
        if (emitter_span < 1) emitter_span = 1;

        emitter->m_emitter_op.m_op_frame = env->m_cur_frame + emitter->m_data.frame_start + emitter_span - 1;
        emitter->m_emitter_op.m_op_fun = plugin_barrage_emitter_trigger_once;
        emitter->m_emitter_op.m_op_ctx = emitter;
        plugin_barrage_op_enqueue(env, &emitter->m_emitter_op);
    }
}

BARRAGE_EMITTER const * plugin_barrage_emitter_data(plugin_barrage_emitter_t emitter) {
    return &emitter->m_data;
}
    
uint8_t plugin_barrage_emitter_is_working(plugin_barrage_emitter_t emitter) {
    return emitter->m_is_working;
}

void plugin_barrage_emitter_start(plugin_barrage_emitter_t emitter) {
    assert(emitter->m_barrage->m_is_enable);
    if (emitter->m_is_working) return;

    assert(emitter->m_emitter_op.m_op_fun == NULL);

    emitter->m_is_working = 1;

    plugin_barrage_emitter_reset(emitter);
}

void plugin_barrage_emitter_stop(plugin_barrage_emitter_t emitter) {
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;

    if (!emitter->m_is_working) return;

    if (emitter->m_emitter_op.m_op_fun != NULL) {
        plugin_barrage_op_dequeue(env, &emitter->m_emitter_op);
    }
    plugin_barrage_trigger_op_free_all(env, &emitter->m_trigger_ops);
    assert(TAILQ_EMPTY(&emitter->m_trigger_ops));

    emitter->m_is_working = 0;
}

void plugin_barrage_emitter_clear_bullets(plugin_barrage_emitter_t emitter) {
    const char * dead_anim;
    plugin_particle_obj_emitter_t dead_emitter;
    
    if (TAILQ_EMPTY(&emitter->m_bullets)) return;

    dead_anim = plugin_particle_obj_emitter_dead_anim(emitter->m_bullet_proto->m_emitter);
    if (dead_anim[0]) {
        dead_emitter = plugin_particle_obj_emitter_find(emitter->m_barrage->m_group->m_particle_obj, dead_anim);
        if (dead_emitter == NULL) {
            CPE_ERROR(
                emitter->m_barrage->m_group->m_env->m_module->m_em,
                "plugin_barrage_emitter_clear_bullets: dead emitter %s not exist!", dead_anim);
        }
    }

    while(!TAILQ_EMPTY(&emitter->m_bullets)) {
        plugin_barrage_bullet_t bullet = TAILQ_FIRST(&emitter->m_bullets);
        
        if (dead_emitter) {
            ui_transform trans;
            plugin_particle_obj_particle_calc_base_transform(bullet->m_particle, &trans);
            plugin_particle_obj_emitter_spawn_at_world(dead_emitter, &trans, 0);
        }
        
        plugin_barrage_bullet_free(bullet);
    }
}

static plugin_barrage_bullet_t plugin_barrage_emitter_bullets_next(struct plugin_barrage_bullet_it * it) {
    plugin_barrage_bullet_t * data = (plugin_barrage_bullet_t *)(it->m_data);
    plugin_barrage_bullet_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_emitter);

    return r;
}

void plugin_barrage_emitter_bullets(plugin_barrage_bullet_it_t it, plugin_barrage_emitter_t emitter) {
    *(plugin_barrage_bullet_t *)(it->m_data) = TAILQ_FIRST(&emitter->m_bullets);
    it->next = plugin_barrage_emitter_bullets_next;
}
