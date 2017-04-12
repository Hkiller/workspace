#include "render/runtime/ui_runtime_render_obj.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_render_env_transform_monitor_i.h"
#include "ui_sprite_render_obj_world_i.h"

ui_sprite_render_env_transform_monitor_t
ui_sprite_render_env_transform_monitor_create(
    ui_sprite_render_env_t env, ui_sprite_render_env_transform_monitor_fun_t process_fun, void * process_ctx)
{
    ui_sprite_render_env_transform_monitor_t processor;

    processor = mem_alloc(env->m_module->m_alloc, sizeof(struct ui_sprite_render_env_transform_monitor));
    if (processor == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "%s: ui_sprite_render_env_transform_monitor_create: alloc fail!",
            ui_sprite_render_module_name(env->m_module));
        return NULL;
    }

    processor->m_env = env;
    processor->m_process_fun = process_fun;
    processor->m_process_ctx = process_ctx;

    TAILQ_INSERT_TAIL(&env->m_transform_monitors, processor, m_next);
    
    return processor;
}

void ui_sprite_render_env_transform_monitor_free(ui_sprite_render_env_transform_monitor_t processor) {
    ui_sprite_render_env_t env = processor->m_env;
    TAILQ_REMOVE(&env->m_transform_monitors, processor, m_next);
    mem_free(env->m_module->m_alloc, processor);
}
