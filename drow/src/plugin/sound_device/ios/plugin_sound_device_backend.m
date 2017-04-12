#include "render/cache/ui_cache_sound.h"
#include "render/cache/ui_cache_sound_buf.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_sound_backend.h"
#include "render/runtime/ui_runtime_sound_chanel.h"
#include "render/runtime/ui_runtime_sound_res.h"
#include "plugin_sound_device_backend.h"

int plugin_sound_device_module_init_backend(plugin_sound_device_module_t module) {
    if (module->m_runtime) {
        ui_runtime_sound_backend_t backend =
            ui_runtime_sound_backend_create(
                module->m_runtime, "ios",
                module,
                plugin_sound_device_res_install,
                plugin_sound_device_res_uninstall,
                0,
                plugin_sound_device_chanel_init,
                plugin_sound_device_chanel_fini,
                plugin_sound_device_chanel_play,
                plugin_sound_device_chanel_stop,
                plugin_sound_device_chanel_pause,
                plugin_sound_device_chanel_resume,
                plugin_sound_device_chanel_get_state,
                plugin_sound_device_chanel_set_volumn);
        if (backend == NULL) {
            CPE_ERROR(module->m_em, "plugin_sound_device_module_init_backend: create sound backend fail!");
            return -1;
        }
    }

    return 0;
}

void plugin_sound_device_module_fini_backend(plugin_sound_device_module_t module) {
    if (module->m_runtime) {
        ui_runtime_sound_backend_t backend = ui_runtime_sound_backend_find_by_name(module->m_runtime, "ios");
        if (backend) {
            ui_runtime_sound_backend_free(backend);
        }
    }
}
