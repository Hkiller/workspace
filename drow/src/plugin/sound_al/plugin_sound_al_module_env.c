#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "render/cache/ui_cache_sound.h"
#include "render/cache/ui_cache_sound_buf.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_sound_al_module_i.h"

int plugin_sound_al_module_init_device(plugin_sound_al_module_t module) {
	const char * device_name;

    device_name = (const char*)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	if (device_name == NULL) {
        CPE_ERROR(module->m_em, "plugin_sound_al_module_init_device: get device name fail!");
        return -1;
    }

    assert(module->m_device == NULL);
	module->m_device = alcOpenDevice(device_name);
    if (module->m_device == NULL) {
        CPE_ERROR(module->m_em, "plugin_sound_al_module_init_device: open device %s fail!", device_name);
        return -1;
    }

    CPE_INFO(module->m_em, "plugin_sound_al_module_init_device: open device %s success!", device_name);

    return 0;
}

void plugin_sound_al_module_fini_device(plugin_sound_al_module_t module) {
    assert(module->m_device);
    alcCloseDevice(module->m_device);
    module->m_device = NULL;
}

int plugin_sound_al_module_init_context(plugin_sound_al_module_t module) {
    assert(module->m_context == NULL);
    assert(module->m_device);
	module->m_context = alcCreateContext(module->m_device, NULL);
	if (module->m_context ==  NULL) {
        CPE_INFO(module->m_em, "plugin_sound_al_module_init_context: create context fail!");
        return -1;
    }

	alcMakeContextCurrent(module->m_context);

    CPE_INFO(module->m_em, "plugin_sound_al_module_init_device: create contexts success!");
    
    return 0;
}

void plugin_sound_al_module_fini_context(plugin_sound_al_module_t module) {
	alcMakeContextCurrent(NULL);
    
    assert(module->m_context);
	alcDestroyContext(module->m_context);
    module->m_context = NULL;
}
