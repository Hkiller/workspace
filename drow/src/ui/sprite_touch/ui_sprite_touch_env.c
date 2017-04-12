#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_touch_env_i.h"
#include "ui_sprite_touch_trace_i.h"
#include "ui_sprite_touch_responser_i.h"
#include "ui_sprite_touch_touchable_i.h"

static int ui_sprite_touch_env_process_touch(
    void * ctx, ui_sprite_render_env_t render_env,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t world_pt);

static void ui_sprite_touch_env_clear(ui_sprite_world_res_t world_res, void * ctx);

ui_sprite_touch_env_t
ui_sprite_touch_env_create(ui_sprite_touch_mgr_t mgr, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_touch_env_t env;
    ui_sprite_render_env_t render_env;

    render_env = ui_sprite_render_env_find(world);
    if (render_env == NULL) {
        CPE_ERROR(mgr->m_em, "create touch env: no render env!");
        return NULL;
    }
    
    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_TOUCH_ENV_NAME, sizeof(struct ui_sprite_touch_env));

    if (world_res == NULL) {
        CPE_ERROR(mgr->m_em, "create render env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_touch_env_t)ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_mgr = mgr;

    TAILQ_INIT(&env->m_touch_traces);
    TAILQ_INIT(&env->m_active_responsers);
    TAILQ_INIT(&env->m_waiting_responsers);

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_touch_env_clear, NULL);

    if (ui_sprite_render_env_add_touch_processor(render_env, ui_sprite_touch_env_process_touch, env) != 0) {
        CPE_ERROR(mgr->m_em, "create render env: add touch processor fail!");
        ui_sprite_touch_env_free(world);
        return NULL;
    }
    
    return env;
}

static void ui_sprite_touch_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    //ui_sprite_world_t world = ui_sprite_world_res_world(world_res);
    ui_sprite_touch_env_t env = (ui_sprite_touch_env_t)ui_sprite_world_res_data(world_res);

    assert(TAILQ_EMPTY(&env->m_active_responsers));
    assert(TAILQ_EMPTY(&env->m_waiting_responsers));

    ui_sprite_touch_trace_free_all(env);
}

void ui_sprite_touch_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_TOUCH_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_touch_env_t ui_sprite_touch_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_TOUCH_ENV_NAME);
    return world_res ? (ui_sprite_touch_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

void ui_sprite_touch_env_set_debug(ui_sprite_touch_env_t env, uint8_t debug) {
    env->m_debug = debug;
}

static ui_vector_2
ui_sprite_touch_logic_to_world(ui_sprite_touch_touchable_t touchable, ui_vector_2_t screen_pt) {
    ui_sprite_entity_t entity;
    ui_sprite_render_env_t render_env;

    entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    render_env = ui_sprite_render_env_find(ui_sprite_entity_world(entity));
    if (render_env) {
        return ui_sprite_render_env_screen_to_world(render_env, screen_pt);
    }
    else {
        return *screen_pt;
    }
}

static void ui_sprite_touch_env_cancel_other_active(ui_sprite_touch_env_t env, ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_responser_t check_responser;
    for(check_responser = TAILQ_FIRST(&env->m_active_responsers);
        check_responser != TAILQ_END(&env->m_active_responsers);
        )
    {
        ui_sprite_touch_responser_t next = TAILQ_NEXT(check_responser, m_next_for_env);

        if (check_responser != responser) {
            ui_sprite_touch_responser_cancel(check_responser);
        }

        check_responser = next;
    }

    assert(TAILQ_FIRST(&env->m_active_responsers) == responser);
    assert(TAILQ_NEXT(responser, m_next_for_env) == TAILQ_END(&env->m_active_responsers));
}

void ui_sprite_touch_process_trace_begin(ui_sprite_touch_env_t env, ui_sprite_touch_trace_t trace, ui_vector_2_t screen_pt, ui_vector_2_t world_pt) {
    ui_sprite_touch_responser_t responser;
    struct ui_sprite_touch_active_responser_info responser_info;

    ui_sprite_touch_env_calc_active_responser_info(env, &responser_info);
    if (responser_info.m_grab_active_responser) {
        return;
    }

    for(responser = TAILQ_FIRST(&env->m_waiting_responsers);
        responser != TAILQ_END(&env->m_waiting_responsers);
        )
    {
        ui_sprite_touch_responser_t next = TAILQ_NEXT(responser, m_next_for_env);
        ui_vector_2 world_pt = ui_sprite_touch_logic_to_world(responser->m_touchable, screen_pt);

        if (responser_info.m_have_binding_responser && responser_info.m_binding_z > responser->m_z) {
            break;
        }
        
        if (ui_sprite_touch_touchable_is_point_in(responser->m_touchable, world_pt)) {
            ui_sprite_touch_responser_binding_t binding;

            binding = ui_sprite_touch_responser_bind_tracer(responser, trace);
            if (binding == NULL) continue;

            binding->m_start_screen_pt = *screen_pt;
            binding->m_start_world_pt = world_pt;
            binding->m_pre_screen_pt = *screen_pt;
            binding->m_pre_world_pt = world_pt;
            binding->m_cur_screen_pt = *screen_pt;
            binding->m_cur_world_pt = world_pt;

            ui_sprite_touch_responser_on_begin(responser);

            if (responser->m_is_start && responser->m_is_grab) {
                ui_sprite_touch_env_cancel_other_active(env, responser);
                return;
            }

            if (responser_info.m_have_binding_responser == 0) {
                responser_info.m_have_binding_responser = 1;
                responser_info.m_binding_z = responser->m_z;
            }
            else {
                assert(responser_info.m_binding_z == responser->m_z);
            }
        }
        
        responser = next;
    }
}

void ui_sprite_touch_env_calc_active_responser_info(ui_sprite_touch_env_t env, struct ui_sprite_touch_active_responser_info * info) {
    ui_sprite_touch_responser_t responser;
    info->m_grab_active_responser = NULL;
    info->m_have_binding_responser = 0;
    info->m_have_binding_responser = 0.0f;

    TAILQ_FOREACH(responser, &env->m_active_responsers, m_next_for_env) {
        if (!TAILQ_EMPTY(&responser->m_bindings)) {
            if (responser->m_is_start && responser->m_is_grab) {
                info->m_grab_active_responser = responser;
                assert(info->m_have_binding_responser == 0);
                info->m_have_binding_responser = 1;
                info->m_binding_z = responser->m_z;
                break;
            }
            else {
                assert(info->m_grab_active_responser == NULL);
                if (info->m_have_binding_responser == 0) {
                    info->m_have_binding_responser = 1;
                    info->m_binding_z = responser->m_z;
                }
                else {
                    assert(info->m_binding_z == responser->m_z);
                }
            }
        }
    }
}

static void ui_sprite_touch_process_trace_update_one(ui_sprite_touch_responser_binding_t binding, ui_vector_2_t screen_pt, ui_vector_2_t world_pt) {
    ui_sprite_touch_responser_t  responser = binding->m_responser;

    binding->m_pre_screen_pt = binding->m_cur_screen_pt;
    binding->m_pre_world_pt = binding->m_cur_world_pt;
    binding->m_cur_screen_pt = *screen_pt;
    binding->m_cur_world_pt = *world_pt;
        
    if (responser->m_is_capture || ui_sprite_touch_touchable_is_point_in(responser->m_touchable, *world_pt)) {
        ui_sprite_touch_responser_on_move(responser);
    }
    else {
        ui_sprite_touch_responser_on_end(responser);
        ui_sprite_touch_responser_unbind_tracer(binding);
    }
}

static void ui_sprite_touch_process_trace_update(ui_sprite_touch_env_t env, ui_sprite_touch_trace_t trace, ui_vector_2_t screen_pt, ui_vector_2_t world_pt) {
    struct ui_sprite_touch_active_responser_info responser_info;
    ui_sprite_touch_responser_binding_t binding;

    ui_sprite_touch_env_calc_active_responser_info(env, &responser_info);
    if (responser_info.m_grab_active_responser) {
        binding = ui_sprite_touch_responser_binding_find(responser_info.m_grab_active_responser, trace);
        if (binding == NULL) return;
        ui_sprite_touch_process_trace_update_one(binding, screen_pt, world_pt);
        return;
    }

    for(binding = TAILQ_FIRST(&trace->m_bindings);
        binding != TAILQ_END(&trace->m_bindings);
        )
    {
        ui_sprite_touch_responser_t responser;
        ui_sprite_touch_responser_binding_t next;

        next = TAILQ_NEXT(binding, m_next_for_trace);

        responser = binding->m_responser;
        ui_sprite_touch_process_trace_update_one(binding, screen_pt, world_pt);

        if (responser->m_is_start && responser->m_is_grab) {
            ui_sprite_touch_env_cancel_other_active(env, binding->m_responser);
            return;
        }
        else {
            binding = next;
        }
    }
}

static int ui_sprite_touch_env_process_touch(
    void * ctx, ui_sprite_render_env_t render_env,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t world_pt)
{
    ui_sprite_touch_env_t env = ctx;
    ui_sprite_touch_mgr_t mgr = env->m_mgr;
    ui_sprite_touch_trace_t trace;
    
    switch(evt) {
    case ui_runtime_touch_begin:
        trace = ui_sprite_touch_trace_find(env, track_id);
        if (trace == NULL) {
            trace = ui_sprite_touch_trace_create(env, track_id, screen_pt, world_pt);
            if (trace == NULL) {
                CPE_ERROR(env->m_mgr->m_em, "%s: touch begin: create trace fail!", ui_sprite_touch_mgr_name(env->m_mgr));
                return -1;
            }
            ui_sprite_touch_process_trace_begin(env, trace, screen_pt, world_pt);
        }
        else {
            trace->m_last_screen_pt = *screen_pt;
            trace->m_last_world_pt = *world_pt;
            ui_sprite_touch_process_trace_update(env, trace, screen_pt, world_pt);
        }

        return 1;
    case ui_runtime_touch_move:
        trace = ui_sprite_touch_trace_find(env, track_id);
        if (trace == NULL) {
            trace = ui_sprite_touch_trace_create(env, track_id, screen_pt, world_pt);
            if (trace == NULL) {
                CPE_ERROR(env->m_mgr->m_em, "%s: touch begin: create trace fail!", ui_sprite_touch_mgr_name(env->m_mgr));
                return -1;
            }
            ui_sprite_touch_process_trace_begin(env, trace, screen_pt, world_pt);
        }
        else {
            ui_sprite_touch_process_trace_update(env, trace, screen_pt, world_pt);
        }
        return 1;
    case ui_runtime_touch_end:
        trace = ui_sprite_touch_trace_find(env, track_id);
        if (trace == NULL) return 0;

        trace->m_is_ending = 1;

        ui_sprite_touch_process_trace_update(env, trace, screen_pt, world_pt);
        while(!TAILQ_EMPTY(&trace->m_bindings)) {
            ui_sprite_touch_responser_binding_t binding = TAILQ_FIRST(&trace->m_bindings);
            ui_sprite_touch_responser_t responser;
            assert(binding);

            responser = binding->m_responser;

            ui_sprite_touch_responser_on_end(responser);
            ui_sprite_touch_responser_unbind_tracer(binding);
        }

        ui_sprite_touch_trace_free(env, trace);
        return 1;
    default:
        CPE_ERROR(mgr->m_em, "%s: touch env process touch: unknown event %d", ui_sprite_touch_mgr_name(mgr), evt);
        return -1;
    }
}

int ui_sprite_touch_env_regist(ui_sprite_touch_mgr_t mgr) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            mgr->m_loader, UI_SPRITE_TOUCH_ENV_NAME, ui_sprite_touch_env_res_load, mgr)
        != 0)
    {
        CPE_ERROR(
            mgr->m_em, "%s: %s register: register loader fail",
            ui_sprite_touch_mgr_name(mgr), UI_SPRITE_TOUCH_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_touch_env_unregist(ui_sprite_touch_mgr_t mgr) {
    if (ui_sprite_cfg_loader_remove_resource_loader(mgr->m_loader, UI_SPRITE_TOUCH_ENV_NAME) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_touch_mgr_name(mgr), UI_SPRITE_TOUCH_ENV_NAME);
    }
}

const char * UI_SPRITE_TOUCH_ENV_NAME = "TouchEnv";
