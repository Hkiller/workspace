#import <AudioToolbox/AudioToolbox.h>
#include "../plugin_sound_al_module_i.h"

static void plugin_sound_al_module_on_interruption(void * data, uint32_t state) {
    plugin_sound_al_module_t module = data;
    if (state == kAudioSessionBeginInterruption) {
        CPE_INFO(module->m_em, "plugin_sound_al_module_on_interruption: begin interruption, swith to no context!");
        alcMakeContextCurrent(NULL);
    } 
    else if (state == kAudioSessionEndInterruption) {
        alcMakeContextCurrent(module->m_context);
        CPE_INFO(module->m_em, "plugin_sound_al_module_on_interruption: end interruption, restore!");
    }
} 

int plugin_sound_al_module_init_backend(plugin_sound_al_module_t module) {
    AudioSessionInitialize(NULL, NULL, plugin_sound_al_module_on_interruption, (void*)module);
    return 0;
}

void plugin_sound_al_module_fini_backend(plugin_sound_al_module_t module) {
    
}

