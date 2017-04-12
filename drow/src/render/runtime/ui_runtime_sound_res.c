#include "cpe/pal/pal_strings.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "render/cache/ui_cache_sound.h"
#include "render/cache/ui_cache_sound_buf.h"
#include "ui_runtime_sound_res_i.h"
#include "ui_runtime_sound_backend_i.h"

void * ui_runtime_sound_res_data(ui_runtime_sound_res_t res) {
    return res->m_data;
}

static int ui_runtime_module_sound_on_res_loaded(void * ctx, ui_cache_res_t res) {
    ui_runtime_module_t module = ctx;
    ui_cache_sound_buf_t sound_buf;
    struct ui_runtime_sound_res * sound_res;
    
    sound_buf = ui_cache_sound_get_buf(res);
    if (sound_buf == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_module_sound_on_res_loaded: res %s no sound buf", ui_cache_res_path(res));
        return -1;
    }

    sound_res = (struct ui_runtime_sound_res * )ui_cache_res_plugin_data(res);
    bzero(sound_res, sizeof(*sound_res));
    
    if (module->m_debug) {
        ui_cache_sound_data_format_t format = ui_cache_sound_buf_data_format(sound_buf);
        
        CPE_INFO(
            module->m_em, "ui_runtime_module_sound_on_res_loaded: sound %s load success, format=%s, freq=%d, size=%d",
            ui_cache_res_path(res),
            (format == ui_cache_sound_data_format_mono8 ? "MONO8"
             : format == ui_cache_sound_data_format_mono16 ? "MONO16"
             : format == ui_cache_sound_data_format_stereo8 ? "STEREO8"
             : format == ui_cache_sound_data_format_stereo16 ? "STEREO16"
             : "???"),
            ui_cache_sound_buf_freq(sound_buf),
            ui_cache_sound_buf_data_size(sound_buf));
    }
    
    return 0;
}


static void ui_runtime_module_sound_on_res_unload(void * ctx, ui_cache_res_t res, uint8_t is_external_unload) {
    ui_runtime_module_t module = ctx;
    struct ui_runtime_sound_res * sound_res;

    ui_runtime_module_sound_stop_by_res(module, res);

    sound_res = (struct ui_runtime_sound_res * )ui_cache_res_plugin_data(res);
    if (sound_res->m_backend) {
        sound_res->m_backend->m_res_uninstall(sound_res->m_backend->m_ctx, sound_res);
        sound_res->m_backend = NULL;
    }

    if (module->m_debug) {
        CPE_INFO(module->m_em, "ui_runtime_module_sound_on_res_loaded: sound %s unload success", ui_cache_res_path(res));
    }
}

int ui_runtime_module_init_sound_res_plugin(ui_runtime_module_t module) {
    ui_cache_res_plugin_t plugin = 
        ui_cache_res_plugin_create(
            module->m_cache_mgr, ui_cache_res_type_sound,
            "sound", sizeof(struct ui_runtime_sound_res), module,
            ui_runtime_module_sound_on_res_loaded,
            ui_runtime_module_sound_on_res_unload,
            NULL);
    if (plugin == NULL) {
        CPE_ERROR(module->m_em, "plugin_sound_al_module_init_res_plugin: create plugin fail!");
        return -1;
    }
    
    return 0;
}

void ui_runtime_module_fini_sound_res_plugin(ui_runtime_module_t module) {
    ui_cache_res_plugin_t plugin = ui_cache_res_plugin_find_by_ctx(module->m_cache_mgr, module);
    if (plugin == NULL) {
        CPE_ERROR(module->m_em, "plugin_sound_al_module_fini_res_plugin: plugin not exist!");
    }
    else {
        ui_cache_res_plugin_free(plugin);
    }
}

