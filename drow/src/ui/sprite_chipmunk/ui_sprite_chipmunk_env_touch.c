#include <assert.h>
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_chipmunk_env_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_touch_trace_i.h"
#include "ui_sprite_chipmunk_touch_binding_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_env_touch_query_ctx {
    ui_sprite_chipmunk_env_t env;
    ui_sprite_chipmunk_touch_trace_t trace;
    int rv;
    uint32_t processed_count;
};

static void ui_sprite_chipmunk_env_touch_on_shape(cpShape *shape, cpVect point, cpFloat distance, cpVect gradient, void *data) {
    struct ui_sprite_chipmunk_env_touch_query_ctx * ctx = (struct ui_sprite_chipmunk_env_touch_query_ctx *)data;
    ui_sprite_chipmunk_obj_body_t body;
    ui_sprite_chipmunk_touch_responser_t responser;
    uint8_t processed = 0;

    if (ctx->rv != 0) return;
    if (shape->type != ctx->env->m_collision_type) return;

    body = (ui_sprite_chipmunk_obj_body_t)shape->userData;

    TAILQ_FOREACH(responser, &body->m_obj->m_responsers, m_next_for_obj) {
        ui_sprite_chipmunk_touch_binding_t binding;

        if (responser->m_is_cur_processed) continue;
        
        binding = ui_sprite_chipmunk_touch_binding_find(responser, ctx->trace);
        if (binding == NULL) {
            binding = ui_sprite_chipmunk_touch_binding_create(responser, ctx->trace, body);
            if (binding == NULL) {
                ctx->rv = -1;
                return;
            }

            binding->m_start_world_pt = ctx->trace->m_last_pt;
            binding->m_pre_world_pt = ctx->trace->m_last_pt;
            binding->m_cur_world_pt = ctx->trace->m_last_pt;
            ui_sprite_chipmunk_touch_responser_on_begin(responser);
        }
        else {
            binding->m_pre_world_pt = binding->m_cur_world_pt;
            binding->m_cur_world_pt = ctx->trace->m_last_pt;
            ui_sprite_chipmunk_touch_responser_on_move(responser);
        }

        responser->m_is_cur_processed = 1;
        processed = 1;
    }

    if (processed) ctx->processed_count++;
}

static int ui_sprite_chipmunk_env_touch_process(
    void * ctx, ui_sprite_render_env_t render_env,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t logic_pt)
{
    ui_sprite_chipmunk_env_t env = (ui_sprite_chipmunk_env_t)ctx;
    ui_sprite_chipmunk_module_t module = env->m_module;
    ui_sprite_chipmunk_touch_trace_t trace;
    ui_sprite_chipmunk_touch_binding_t binding;
    ui_sprite_chipmunk_touch_responser_t responser;
    
    trace = ui_sprite_chipmunk_touch_trace_find(env, track_id);
    
    switch(evt) {
    case ui_runtime_touch_begin:
    case ui_runtime_touch_move: {
        struct ui_sprite_chipmunk_env_touch_query_ctx query_ctx;
        if (trace == NULL) {
            trace = ui_sprite_chipmunk_touch_trace_create(env, track_id, logic_pt);
            if (trace == NULL) {
                CPE_ERROR(module->m_em, "%s: touch begin: create trace fail!", ui_sprite_chipmunk_module_name(module));
                return -1;
            }
        }
        else {
            TAILQ_FOREACH(binding, &trace->m_bindings, m_next_for_trace) {
                binding->m_responser->m_is_cur_processed = 0;
            }
        }
        
        trace->m_last_pt = *logic_pt;
        
        query_ctx.env = env;
        query_ctx.trace = trace;
        query_ctx.rv = 0;
        query_ctx.processed_count = 0;
        
        cpSpacePointQuery(
            (cpSpace *)plugin_chipmunk_env_space(env->m_env),
            cpv(logic_pt->x, logic_pt->y), 1.0,
            env->m_touch_filter, ui_sprite_chipmunk_env_touch_on_shape, &query_ctx);

        for(binding = TAILQ_FIRST(&trace->m_bindings); binding; ) {
            ui_sprite_chipmunk_touch_binding_t next = TAILQ_NEXT(binding, m_next_for_trace);

            if (!binding->m_responser->m_is_cur_processed) {
                if (binding->m_responser->m_is_grab) {
                    ui_sprite_chipmunk_touch_responser_on_move(binding->m_responser);
                }
                else {
                    responser = binding->m_responser;
                    ui_sprite_chipmunk_touch_binding_free(binding);
                    ui_sprite_chipmunk_touch_responser_on_end(responser);
                }
            }

            binding = next;
        }
        
        if (query_ctx.rv != 0) return query_ctx.rv;
        return query_ctx.processed_count;
    }
    case ui_runtime_touch_end:
        trace = ui_sprite_chipmunk_touch_trace_find(env, track_id);
        if (trace == NULL) return 0;

        trace->m_is_ending = 1;

        while(!TAILQ_EMPTY(&trace->m_bindings)) {
            binding = TAILQ_FIRST(&trace->m_bindings);
            responser = binding->m_responser;
            ui_sprite_chipmunk_touch_binding_free(binding);
            ui_sprite_chipmunk_touch_responser_on_end(responser);
        }

        ui_sprite_chipmunk_touch_trace_free(env, trace);
        return 1;
    default:
        CPE_ERROR(module->m_em, "%s: touch env process touch: unknown event %d", ui_sprite_chipmunk_module_name(module), evt);
        return -1;
    }
    
    return 0;
}
    

int ui_sprite_chipmunk_env_set_process_touch(ui_sprite_chipmunk_env_t env, uint8_t is_process_touch) {
    ui_sprite_render_env_t render_env;
    
    if (is_process_touch) is_process_touch = 1;

    if (env->m_process_touch == is_process_touch) return 0;

    render_env = ui_sprite_render_env_find(ui_sprite_world_res_world(ui_sprite_world_res_from_data(env)));
    if (render_env == NULL) {
        CPE_ERROR(env->m_module->m_em, "chipmunk env: set process touch: no render env!");
        return -1;
    }
    
    if (is_process_touch) {
        if (ui_sprite_render_env_add_touch_processor(render_env, ui_sprite_chipmunk_env_touch_process, env) != 0) {
            CPE_ERROR(env->m_module->m_em, "chipmunk env: set process touch: register to render env fail!");
            return -1;
        }
    }
    else {
        ui_sprite_render_env_remove_touch_processor(render_env, ui_sprite_chipmunk_env_touch_process, env);
    }

    env->m_process_touch = is_process_touch;
    return 0;
}
    
uint8_t sprite_chipmunk_env_is_process_touch(ui_sprite_chipmunk_env_t env) {
    return env->m_process_touch;
}


#ifdef __cplusplus
}
#endif
