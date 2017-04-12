#include "render/runtime/ui_runtime_render_obj.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_render_env_touch_processor_i.h"
#include "ui_sprite_render_obj_world_i.h"

ui_sprite_render_env_touch_processor_t
ui_sprite_render_env_touch_processor_create(
    ui_sprite_render_env_t env, ui_sprite_render_env_touch_process_fun_t process_fun, void * process_ctx)
{
    ui_sprite_render_env_touch_processor_t processor;

    processor = mem_alloc(env->m_module->m_alloc, sizeof(struct ui_sprite_render_env_touch_processor));
    if (processor == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "%s: ui_sprite_render_env_touch_processor_create: alloc fail!",
            ui_sprite_render_module_name(env->m_module));
        return NULL;
    }

    processor->m_env = env;
    processor->m_process_fun = process_fun;
    processor->m_process_ctx = process_ctx;

    TAILQ_INSERT_TAIL(&env->m_touch_processors, processor, m_next);
    
    return processor;
}

void ui_sprite_render_env_touch_processor_free(ui_sprite_render_env_touch_processor_t processor) {
    ui_sprite_render_env_t env = processor->m_env;
    TAILQ_REMOVE(&env->m_touch_processors, processor, m_next);
    mem_free(env->m_module->m_alloc, processor);
}

void ui_sprite_render_env_touch_process(
    void * ctx, ui_runtime_render_obj_t render_obj,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t logic_pt)
{
    ui_sprite_render_obj_world_t obj = ui_runtime_render_obj_data(render_obj);
    ui_sprite_render_env_t env;
    ui_sprite_render_env_touch_processor_t processor;
    ui_vector_2 world_pt;
    
    env = obj->m_env;
    if (env == NULL) return;

    if (TAILQ_EMPTY(&env->m_touch_processors)) return;

    world_pt = ui_sprite_render_env_screen_to_world(env, logic_pt);

    /* CPE_ERROR(env->m_module->m_em, "xxxxx: screen_pt=(%f,%f), logic_pt=(%f,%f), world_pt=(%f,%f)\n", screen_pt->x, screen_pt->y, logic_pt->x, logic_pt->y, world_pt.x, world_pt.y); */
    /* CPE_ERROR(env->m_module->m_em, "xxxxx: env.base-transform=(%f,%f)\n", env->m_base_transform.m_s.x, env->m_base_transform.m_s.y); */

    TAILQ_FOREACH(processor, &env->m_touch_processors, m_next) {
        int rv = processor->m_process_fun(processor->m_process_ctx, env, track_id, evt, screen_pt, &world_pt);
        if (rv > 0) break;
    }
}

