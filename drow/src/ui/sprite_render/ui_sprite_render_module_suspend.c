#include <assert.h>
#include "plugin/app_env/plugin_app_env_module.h"
#include "protocol/plugin/app_env/app_env_pro.h"
#include "ui_sprite_render_module_i.h"

static int ui_sprite_render_on_suspend(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    ui_sprite_render_module_t module = ctx;
    APP_ENV_SUSPEND const * req = req_data; 
    uint8_t is_suspend = req->is_suspend;
    
    if (is_suspend) is_suspend = 1;

    if (module->m_app_pause == is_suspend) return 0;

    module->m_app_pause = is_suspend;

    return 0;
}

int ui_sprite_render_suspend_monitor_regist(ui_sprite_render_module_t module) {
    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(module->m_app, NULL);
    if (app_env == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_suspend_monitor_regist: app env not exist!");
        return -1;
    }
    
    module->m_suspend_monitor = 
        plugin_app_env_monitor_create(app_env, "app_env_suspend", module, ui_sprite_render_on_suspend, NULL);
    if (module->m_suspend_monitor == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_suspend_monitor_regist: create suspend fail!");
        return -1;
    }

    return 0;
}

void ui_sprite_render_suspend_monitor_unregist(ui_sprite_render_module_t module) {
    assert(module->m_suspend_monitor);
    plugin_app_env_monitor_free(module->m_suspend_monitor);
    module->m_suspend_monitor = NULL;
}

