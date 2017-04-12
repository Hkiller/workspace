#include <assert.h>
#include <stdio.h>
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_platform.h"
#include "render/utils/ui_vector_2.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin_barrage_env_i.h"
#include "plugin_barrage_data_emitter_i.h"
#include "plugin_barrage_barrage_i.h"
#include "plugin_barrage_emitter_i.h"
#include "plugin_barrage_group_i.h"
#include "plugin_barrage_op_i.h"
#include "plugin_barrage_trigger_op_i.h"

static cpBool plugin_barrage_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
    
plugin_barrage_env_t plugin_barrage_env_create(plugin_barrage_module_t module, plugin_chipmunk_env_t chipmunk_env) {
    plugin_barrage_env_t env;
    uint8_t i;
    cpCollisionHandler * collision_handelr;

    env = (plugin_barrage_env_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_env));
    if (env == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_env_create: create fail!");
        return NULL;
    }

    env->m_module = module;
    env->m_chipmunk_env = chipmunk_env;
    
    if (plugin_chipmunk_env_register_collision_type(&env->m_collision_type, chipmunk_env, "barrage") != 0) {
        CPE_ERROR(module->m_em, "plugin_barrage_env_create: register chipmunk_env fail!");
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    env->m_data_count = 0;

    env->m_fps = 60.0f;
    env->m_frame_spane = 1 / env->m_fps;
    env->m_left_span = 0.0f;
    env->m_cur_frame = 0;

    env->m_near_op_pos = 0;
    for(i = 0; i < CPE_ARRAY_SIZE(env->m_near_ops); ++i) {
        TAILQ_INIT(&env->m_near_ops[i]);
    }
    TAILQ_INIT(&env->m_far_ops);

    env->m_bullet_count = 0;
    env->m_emitter_count = 0;
    env->m_barrage_count = 0;
    env->m_op_count = 0;
    
    collision_handelr = cpSpaceAddWildcardHandler((cpSpace*)plugin_chipmunk_env_space(chipmunk_env), env->m_collision_type);
    assert(collision_handelr);
    collision_handelr->beginFunc = plugin_barrage_env_begin_collision;
    collision_handelr->userData = env;

    TAILQ_INIT(&env->m_groups);
    TAILQ_INIT(&env->m_active_bullets);
    TAILQ_INIT(&env->m_collided_bullets);
    TAILQ_INIT(&env->m_idle_barrages);
    TAILQ_INIT(&env->m_active_barrages);    
    TAILQ_INIT(&env->m_free_barrages);
    TAILQ_INIT(&env->m_free_emitters);
    TAILQ_INIT(&env->m_free_bullets);
    TAILQ_INIT(&env->m_free_trigger_ops);

    return env;
}

void plugin_barrage_env_free(plugin_barrage_env_t env) {
    plugin_barrage_module_t module = env->m_module;

    while(!TAILQ_EMPTY(&env->m_active_bullets)) {
        plugin_barrage_bullet_free(TAILQ_FIRST(&env->m_active_bullets));
    }

    while(!TAILQ_EMPTY(&env->m_collided_bullets)) {
        plugin_barrage_bullet_free(TAILQ_FIRST(&env->m_collided_bullets));
    }

    while(!TAILQ_EMPTY(&env->m_idle_barrages)) {
        plugin_barrage_barrage_free(TAILQ_FIRST(&env->m_idle_barrages));
    }
    while(!TAILQ_EMPTY(&env->m_active_barrages)) {
        plugin_barrage_barrage_free(TAILQ_FIRST(&env->m_active_barrages));
    }

    while(!TAILQ_EMPTY(&env->m_groups)) {
        plugin_barrage_group_free(TAILQ_FIRST(&env->m_groups));
    }

    assert(env->m_barrage_count == 0);
    assert(env->m_bullet_count == 0);
    assert(env->m_emitter_count == 0);
    assert(env->m_op_count == 0);
    assert(env->m_data_count == 0);

    while(!TAILQ_EMPTY(&env->m_free_trigger_ops)) {
        plugin_barrage_trigger_op_real_free(TAILQ_FIRST(&env->m_free_trigger_ops));
    }

    while(!TAILQ_EMPTY(&env->m_free_bullets)) {
        plugin_barrage_bullet_real_free(TAILQ_FIRST(&env->m_free_bullets));
    }

    while(!TAILQ_EMPTY(&env->m_free_emitters)) {
        plugin_barrage_emitter_real_free(TAILQ_FIRST(&env->m_free_emitters));
    }

    while(!TAILQ_EMPTY(&env->m_free_barrages)) {
        plugin_barrage_barrage_real_free(TAILQ_FIRST(&env->m_free_barrages));
    }

    mem_free(module->m_alloc, env);
}

uint32_t plugin_barrage_env_bullet_count(plugin_barrage_env_t env) {
    return env->m_bullet_count;
}

uint32_t plugin_barrage_env_emitter_count(plugin_barrage_env_t env) {
    return env->m_emitter_count;
}

uint32_t plugin_barrage_env_op_count(plugin_barrage_env_t env) {
    return env->m_op_count;
}

uint32_t plugin_barrage_env_collision_type(plugin_barrage_env_t env) {
    return env->m_collision_type;
}

int plugin_barrage_env_update(plugin_barrage_env_t env, float delta, BARRAGE_RECT const * rect) {
    uint32_t process_frame_count;
    plugin_barrage_bullet_t bullet;
    plugin_barrage_bullet_t bullet_next;
    plugin_barrage_barrage_t barrage;
    plugin_barrage_barrage_t barrage_next;
    plugin_barrage_group_t barrage_group;

    env->m_left_span += delta;

    process_frame_count = (uint32_t)(env->m_left_span / env->m_frame_spane);
    env->m_left_span -= env->m_frame_spane * process_frame_count;

    /*计算 */
    while(process_frame_count > 0) {
        env->m_cur_frame++;
        process_frame_count--;
        env->m_near_op_pos = (env->m_near_op_pos + 1) % PLUGIN_BARRAGE_OP_QUEUE_COUNT;

        /*更新所有子弹 */
        for(bullet = TAILQ_FIRST(&env->m_active_bullets); bullet; bullet = bullet_next) {
            plugin_barrage_data_bullet_trigger_t check_trigger;
            ui_vector_2 pos;
            
            bullet_next = TAILQ_NEXT(bullet, m_next_for_env);

            if (rect) {
                cpBB bullet_bb = cpShapeGetBB(&bullet->m_shape.shape);

                if (rect->lt.x < rect->rb.x && (bullet_bb.r < rect->lt.x || bullet_bb.l > rect->rb.x)) {
                    plugin_barrage_bullet_free(bullet);
                    continue;
                }

                if (rect->lt.y < rect->rb.y && (bullet_bb.b < rect->lt.y || bullet_bb.t > rect->rb.y)) {
                    plugin_barrage_bullet_free(bullet);
                    continue;
                }
            }            
            
            bullet->m_data.frame++;

            /*删除超过生命周期的子弹 */
            if (bullet->m_data.frame > bullet->m_data.life_circle) {
                plugin_barrage_bullet_free(bullet);
                continue;
            }

            pos = *plugin_particle_obj_particle_pos(bullet->m_particle);
            pos.x += bullet->m_data.speed_pair.x;
            pos.y += bullet->m_data.speed_pair.y;
            plugin_particle_obj_particle_set_pos(bullet->m_particle, &pos);
            
            cpBodySetPosition(&bullet->m_body, cpv(pos.x, pos.  y));
            cpBodySetAngle(&bullet->m_body, plugin_particle_obj_particle_spin(bullet->m_particle));

			//CPE_ERROR(env->m_module->m_em, "speed_pair.x =%f, speed_pair.y =%f", bullet->m_data.speed_pair.x, bullet->m_data.speed_pair.y);
            
			/*处理按照frame排序的trigger */
            for(;
                bullet->m_next_trigger && bullet->m_next_trigger->m_data.conditions[0].condition_value <= bullet->m_data.frame;
                bullet->m_next_trigger = bullet->m_next_trigger->m_next)
            {
                plugin_barrage_bullet_trigger_do(bullet, bullet->m_next_trigger);
            }

            /*处理每一帧都需要检查的trigger */
            for(check_trigger = bullet->m_check_triggers; check_trigger; check_trigger = check_trigger->m_next) {
                if (plugin_barrage_bullet_trigger_check(bullet, check_trigger)) {
                    plugin_barrage_bullet_trigger_do(bullet, check_trigger);
                }
            }
        }

        /*更新所有发射器 */
        for(barrage = TAILQ_FIRST(&env->m_active_barrages); barrage; barrage = barrage_next) {
            plugin_barrage_emitter_t emitter;
            
            barrage_next = TAILQ_NEXT(barrage, m_next_for_env);

            barrage->m_data.frame++;
            if (barrage->m_data.frame > barrage->m_frame_loop) {
                if (barrage->m_loop_count > 0) {
                    barrage->m_loop_count--;
                    if (barrage->m_loop_count == 0) {
                        plugin_barrage_barrage_disable(barrage, 0);
                        continue;
                    }
                }

                plugin_barrage_barrage_reset(barrage);
                barrage->m_data.frame = 1;
            }
            assert(barrage->m_data.frame > 0 && barrage->m_data.frame <= barrage->m_frame_loop);

            TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next_for_barrage) {
                plugin_barrage_data_emitter_trigger_t check_trigger;

                if (!emitter->m_is_working) continue;

                if (emitter->m_barrage->m_data.frame > emitter->m_data.frame_complete) {
                    plugin_barrage_emitter_stop(emitter);
                    continue;
                }
                assert(emitter->m_barrage->m_data.frame > 0 && emitter->m_barrage->m_data.frame <= emitter->m_data.frame_complete);
                
                emitter->m_data.emitter.pos.x += emitter->m_data.speed_pair.x;
                emitter->m_data.emitter.pos.y += emitter->m_data.speed_pair.y;

                /*处理按照frame排序的trigger */
                for(;
                    emitter->m_next_trigger && emitter->m_next_trigger->m_data.conditions[0].condition_value <= emitter->m_barrage->m_data.frame;
                    emitter->m_next_trigger = emitter->m_next_trigger->m_next)
                {
                    plugin_barrage_emitter_trigger_do(emitter, emitter->m_next_trigger);
                }

                /*处理每一帧都需要检查的trigger */
                for(check_trigger = emitter->m_data_emitter->m_emitter_check_triggers; check_trigger; check_trigger = check_trigger->m_next) {
                    if (plugin_barrage_emitter_trigger_check(emitter, check_trigger)) {
                        plugin_barrage_emitter_trigger_do(emitter, check_trigger);
                    }
                }
            }
        }

        /*执行所有需要在本帧处理的操作 */
        plugin_barrage_op_execute(env, env->m_near_op_pos);
        if (env->m_near_op_pos == 0) plugin_barrage_op_move_far_to_near(env);
    }

    /*清除所有消弹完成的子弹 */
    for(bullet = TAILQ_FIRST(&env->m_collided_bullets); bullet; bullet = bullet_next) {
        bullet_next = TAILQ_NEXT(bullet, m_next_for_env);
        plugin_barrage_bullet_free(bullet);
    }

    TAILQ_FOREACH(barrage_group, &env->m_groups, m_next_for_env) {
        ui_runtime_render_obj_update(ui_runtime_render_obj_from_data(barrage_group->m_particle_obj), delta);
    }
    
    return 0;
}

static plugin_barrage_bullet_t plugin_barrage_env_bullets_next(struct plugin_barrage_bullet_it * it) {
    plugin_barrage_bullet_t * data = (plugin_barrage_bullet_t *)(it->m_data);
    plugin_barrage_bullet_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_env);

    return r;
}

void plugin_barrage_env_bullets(plugin_barrage_bullet_it_t it, plugin_barrage_env_t env) {
    *(plugin_barrage_bullet_t *)(it->m_data) = TAILQ_FIRST(&env->m_active_bullets);
    it->next = plugin_barrage_env_bullets_next;
}

float plugin_barrage_env_fps(plugin_barrage_env_t env) {
    return env->m_fps;
}

void plugin_barrage_env_set_fps(plugin_barrage_env_t env, float fps) {
    env->m_fps = fps;
    env->m_frame_spane = 1.0f / env->m_fps;
}

static void plugin_barrage_env_post_step_remove_bullet(cpSpace *space, cpShape *shape, cpDataPointer userData) {
    plugin_barrage_bullet_on_collided((plugin_barrage_bullet_t)cpShapeGetUserData(shape));
}
    
cpBool plugin_barrage_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
    plugin_barrage_env_t env = (plugin_barrage_env_t)userData;
           
    if (cpShapeGetCollisionType(arb->a) == env->m_collision_type) {
        plugin_barrage_bullet_t bullet = (plugin_barrage_bullet_t)cpShapeGetUserData(arb->a);
        if (arb->b->filter.categories & bullet->m_show_dead_anim_mask) {
            plugin_barrage_bullet_show_dead_anim(bullet);
        }
        
        cpSpaceAddPostStepCallback(space, (cpPostStepFunc)plugin_barrage_env_post_step_remove_bullet, (void*)arb->a, userData);
    }
    
    if (cpShapeGetCollisionType(arb->b) == env->m_collision_type) {
        plugin_barrage_bullet_t bullet = (plugin_barrage_bullet_t)cpShapeGetUserData(arb->b);
        if (arb->a->filter.categories & bullet->m_show_dead_anim_mask) {
            plugin_barrage_bullet_show_dead_anim(bullet);
        }
        
        cpSpaceAddPostStepCallback(space, (cpPostStepFunc)plugin_barrage_env_post_step_remove_bullet, (void*)arb->b, userData);
    }

    return 1;
}
