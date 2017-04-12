#include <assert.h>
#include "render/cache/ui_cache_sound.h"
#include "render/cache/ui_cache_sound_buf.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_sound_backend.h"
#include "render/runtime/ui_runtime_sound_chanel.h"
#include "render/runtime/ui_runtime_sound_res.h"
#include "plugin_sound_al_module_i.h"

static ui_runtime_sound_chanel_state_t plugin_sound_al_chanel_get_state(void * ctx, ui_runtime_sound_chanel_t chanel);

static int plugin_sound_al_res_install(void * ctx, ui_runtime_sound_res_t res, ui_cache_sound_buf_t sound_buf) {
    plugin_sound_al_module_t module = ctx;
    ALuint * buffer = ui_runtime_sound_res_data(res);
	ALenum format;
    ALenum rv;

    *buffer = 0;
    
    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_res_install: current context not active");
        return -1;
    }
    
    switch(ui_cache_sound_buf_data_format(sound_buf)) {
    case ui_cache_sound_data_format_mono8:
        format = AL_FORMAT_MONO8;
        break;
    case ui_cache_sound_data_format_mono16:
        format = AL_FORMAT_MONO16;
        break;
    case ui_cache_sound_data_format_stereo8:
        format = AL_FORMAT_STEREO8;
        break;
    case ui_cache_sound_data_format_stereo16:
        format = AL_FORMAT_STEREO16;
        break;
    default:
        CPE_ERROR(
            module->m_em, "plugin_sound_al_res_install: with not support format %d", ui_cache_sound_buf_data_format(sound_buf));
        return -1;
    }
    
	alGenBuffers(1, buffer);
    if ((rv = alGetError())) {
        CPE_ERROR(module->m_em, "plugin_sound_al_module_on_res_loaded: gen buf fail, rv=%d (%s)", rv, plugin_sound_al_module_error_str(rv));
        *buffer = 0;
        return -1;
    }

	alBufferData(
        *buffer, format,
        ui_cache_sound_buf_data(sound_buf),
        ui_cache_sound_buf_data_size(sound_buf),
        ui_cache_sound_buf_freq(sound_buf));
	if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_res_install: sound %d buff data fail, rv=%d (%s)",
            *buffer, rv, plugin_sound_al_module_error_str(rv));
        alDeleteBuffers(1, buffer);
        *buffer = 0;
        return -1;
    }

    return 0;
}

static void plugin_sound_al_res_uninstall(void * ctx, ui_runtime_sound_res_t res) {
    plugin_sound_al_module_t module = ctx;
    ALuint * buffer = ui_runtime_sound_res_data(res);
    ALenum rv;

    alDeleteBuffers(1, buffer);
	if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_module_on_res_unload: sound %d delete buf fail, rv=%d (%s)",
            *buffer, rv, plugin_sound_al_module_error_str(rv));
        return;
    }
}

static void plugin_sound_al_chanel_fini_source(plugin_sound_al_module_t module, ALuint* source) {
    alDeleteSources(1, source);
    *source = 0;
}

static int plugin_sound_al_chanel_init_source(plugin_sound_al_module_t module, ALuint* source) {
    ALenum rv;

    alGenSources(1, source);
    if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel: gen source fail, rv=%d, (%s)!",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }

    return 0;
}

static int plugin_sound_al_chanel_init(void * ctx, ui_runtime_sound_chanel_t chanel) {
    ALuint* source = ui_runtime_sound_chanel_data(chanel);
    *source = 0;
    return 0;
}

static void plugin_sound_al_chanel_fini(void * ctx, ui_runtime_sound_chanel_t chanel) {
    ALuint* source = ui_runtime_sound_chanel_data(chanel);
    if(*source) plugin_sound_al_chanel_fini_source(ctx, source);
}

static int plugin_sound_al_chanel_pause(void * ctx, ui_runtime_sound_chanel_t chanel) {
    plugin_sound_al_module_t module = ctx;
    ALuint* source = ui_runtime_sound_chanel_data(chanel);
    ALenum rv;

    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_chanel_pause: current context not active");
        return -1;
    }
    
    if (*source == 0) return 0;

    alSourcePause(*source);
    if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_pause: source pause fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }

    return 0;
}

static int plugin_sound_al_chanel_resume(void * ctx, ui_runtime_sound_chanel_t chanel) {
    plugin_sound_al_module_t module = ctx;
    ALuint* source = ui_runtime_sound_chanel_data(chanel);
    ALenum rv;

    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_chanel_resume: current context not active");
        return -1;
    }
    
    if (*source == 0) return 0;

    alSourcePlay(*source);
    if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_pause: source play fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }

    return 0;
}

static int plugin_sound_al_chanel_play(void * ctx, ui_runtime_sound_chanel_t chanel, ui_runtime_sound_res_t res, float volumn, uint8_t loop) {
    plugin_sound_al_module_t module = ctx;
    ALuint * buffer = ui_runtime_sound_res_data(res);
    ALuint * source = ui_runtime_sound_chanel_data(chanel);
    ALenum rv;

    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_chanel_play: current context not active");
        return -1;
    }
    
    assert(alIsBuffer(*buffer));

    if(*source) plugin_sound_al_chanel_fini_source(module, source);
    if (plugin_sound_al_chanel_init_source(module, source) != 0) return -1;
    
    alSourcei(*source, AL_BUFFER, *buffer);
    if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_play: source set buffer fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }
                
    alSourcef(*source, AL_GAIN, volumn);
	if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_play: source set volumn fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }
    
    alSourcei(*source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
	if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_play: source set loop fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }
    
    alSourcePlay(*source);
    if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_play: source play fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }

    /* CPE_ERROR( */
    /*     module->m_em, "xxxxxxxxxxxxx: source=%d, state=%d", *source, plugin_sound_al_chanel_get_state(ctx, chanel)); */
    
    return 0;
}

static void plugin_sound_al_chanel_stop(void * ctx, ui_runtime_sound_chanel_t chanel) {
    plugin_sound_al_module_t module = ctx;
    ALuint * source = ui_runtime_sound_chanel_data(chanel);

    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_chanel_play: current context not active");
        return;
    }

    if (source) plugin_sound_al_chanel_fini_source(module, source);
}

static int plugin_sound_al_chanel_set_volumn(void * ctx, ui_runtime_sound_chanel_t chanel, float volumn) {
    plugin_sound_al_module_t module = ctx;
    ALuint* source = ui_runtime_sound_chanel_data(chanel);
    ALenum rv;

    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_chanel_set_volumn: current context not active");
        return -1;
    }
    
    if (*source == 0) return 0;

    alSourcef(*source, AL_GAIN, volumn);
	if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_al_chanel_set_volumn: get source state fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return -1;
    }

    return 0;
}

static ui_runtime_sound_chanel_state_t plugin_sound_al_chanel_get_state(void * ctx, ui_runtime_sound_chanel_t chanel) {
    plugin_sound_al_module_t module = ctx;
    ALuint* source = ui_runtime_sound_chanel_data(chanel);
    ALenum rv;
    ALint cur_state;

    if(alcGetCurrentContext() != module->m_context) {
        CPE_ERROR(module->m_em, "plugin_sound_al_chanel_get_state: current context not active");
        return ui_runtime_sound_chanel_initial;
    }
    
    if (*source == 0) return ui_runtime_sound_chanel_initial;
    
	alGetSourcei(*source, AL_SOURCE_STATE, &cur_state);
	if ((rv = alGetError())) {
        CPE_ERROR(
            module->m_em, "plugin_sound_chanel_sync_pause: get source state fail, rv=%d (%s)",
            rv, plugin_sound_al_module_error_str(rv));
        return ui_runtime_sound_chanel_initial;
    }

    switch(cur_state) {
    case AL_INITIAL:
        return ui_runtime_sound_chanel_initial;
    case AL_PLAYING:
        return ui_runtime_sound_chanel_playing;
    case AL_PAUSED:
        return ui_runtime_sound_chanel_paused;
    case AL_STOPPED:
        return ui_runtime_sound_chanel_stopped;
    default:
        CPE_ERROR(module->m_em, "plugin_sound_chanel_sync_pause: unknown state %d", cur_state);
        return ui_runtime_sound_chanel_initial;
    }
}

int plugin_sound_al_module_init_sound_backend(plugin_sound_al_module_t module) {
    if (module->m_runtime) {
        ui_runtime_sound_backend_t backend =
            ui_runtime_sound_backend_create(
                module->m_runtime, "al",
                module,
                plugin_sound_al_res_install,
                plugin_sound_al_res_uninstall,
                sizeof(ALuint),
                plugin_sound_al_chanel_init,
                plugin_sound_al_chanel_fini,
                plugin_sound_al_chanel_play,
                plugin_sound_al_chanel_stop,
                plugin_sound_al_chanel_pause,
                plugin_sound_al_chanel_resume,
                plugin_sound_al_chanel_get_state,
                plugin_sound_al_chanel_set_volumn);
        if (backend == NULL) {
            CPE_ERROR(module->m_em, "plugin_sound_al_module_init_backend: create sound backend fail!");
            return -1;
        }
    }

    return 0;
}

void plugin_sound_al_module_fini_sound_backend(plugin_sound_al_module_t module) {
    if (module->m_runtime) {
        ui_runtime_sound_backend_t backend = ui_runtime_sound_backend_find_by_name(module->m_runtime, "al");
        if (backend) {
            ui_runtime_sound_backend_free(backend);
        }
    }

}
