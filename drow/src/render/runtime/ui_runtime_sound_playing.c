#include <assert.h>
#include "render/cache/ui_cache_sound.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "ui_runtime_sound_playing_i.h"
#include "ui_runtime_sound_backend_i.h"
#include "ui_runtime_sound_res_i.h"

ui_runtime_sound_playing_t
ui_runtime_sound_playing_create(ui_runtime_sound_chanel_t chanel, ui_cache_res_t res, uint8_t is_loop) {
    ui_runtime_module_t module = chanel->m_group->m_module;
    ui_runtime_sound_playing_t playing;
    ui_runtime_sound_res_t sound_res;
    
    if (ui_cache_res_load_state(res) != ui_cache_res_loaded) {
        CPE_ERROR(module->m_em, "ui_runtime_sound_playing_create: res %s not loaded!", ui_cache_res_path(res));
        return NULL;
    }

    sound_res = ui_cache_res_plugin_data(res);
    if (sound_res->m_backend == NULL) {
        ui_cache_sound_buf_t sound_buf;
    
        sound_buf = ui_cache_sound_get_buf(res);
        if (sound_buf == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_sound_playing_create: res %s no sound buf", ui_cache_res_path(res));
            return NULL;
        }
        
        if (chanel->m_backend->m_res_install(chanel->m_backend->m_ctx, sound_res, sound_buf) != 0) {
            CPE_ERROR(
                module->m_em, "ui_runtime_sound_playing_create: res %s install to backend %s fail",
                ui_cache_res_path(res), chanel->m_backend->m_name);
            return NULL;
        }

        sound_res->m_backend = chanel->m_backend;
        ui_cache_res_unload_data(res);
    }
    else if (sound_res->m_backend != chanel->m_backend) {
        CPE_ERROR(
            module->m_em, "ui_runtime_sound_playing_create: res %s is already used in bakcned %s, can`t use in %s again!",
            ui_cache_res_path(res), sound_res->m_backend->m_name, chanel->m_backend->m_name);
        return NULL;
    }
    
    playing = TAILQ_FIRST(&module->m_free_playings);
    if (playing) {
        TAILQ_REMOVE(&module->m_free_playings, playing, m_next_for_module);
    }
    else {
        playing = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_sound_playing));
        if (playing == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_sound_playing_create: alloc fail!");
            return NULL;
        }
    }

    playing->m_chanel = chanel;
    playing->m_id = module->m_sound_playing_max_id + 1;
    playing->m_sound_res = res;
    playing->m_loop = is_loop;
    playing->m_volume = 1.0f;
    playing->m_stop_duration = 0.0f;
    playing->m_stop_worked = 0.0f;

    module->m_sound_playing_max_id++;
    module->m_sound_playing_count++;
    TAILQ_INSERT_TAIL(&module->m_sound_playings, playing, m_next_for_module);
    TAILQ_INSERT_TAIL(&chanel->m_playings, playing, m_next_for_chanel);

    if (module->m_debug) {
        CPE_INFO(
            module->m_em, "sound chanel %s[%d]: playing %d: res %s: create",
            chanel->m_group->m_name, chanel->m_id, playing->m_id,
            ui_cache_res_path(playing->m_sound_res));
    }
    
    return playing;
}

void ui_runtime_sound_playing_free(ui_runtime_sound_playing_t playing) {
    ui_runtime_sound_chanel_t chanel = playing->m_chanel;
    ui_runtime_module_t module = chanel->m_group->m_module;

    if (playing == TAILQ_FIRST(&chanel->m_playings)) {
        ui_runtime_sound_backend_t backend = playing->m_chanel->m_backend;
        backend->m_chanel_stop(backend->m_ctx, playing->m_chanel);
    }

    if (module->m_debug) {
        CPE_INFO(
            module->m_em, "sound chanel %s: playing %d: res %s: free",
            chanel->m_group->m_name, playing->m_id, ui_cache_res_path(playing->m_sound_res));
    }
    
    TAILQ_REMOVE(&chanel->m_playings, playing, m_next_for_chanel);

    module->m_sound_playing_count--;
    TAILQ_REMOVE(&module->m_sound_playings, playing, m_next_for_module);
    
    playing->m_chanel = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_playings, playing, m_next_for_module);
}

ui_runtime_sound_chanel_t ui_runtime_sound_playing_chanel(ui_runtime_sound_playing_t playing) {
    return playing->m_chanel;
}

uint32_t ui_runtime_sound_playing_id(ui_runtime_sound_playing_t playing) {
    return playing->m_id;
}

ui_cache_res_t ui_runtime_sound_playing_res(ui_runtime_sound_playing_t playing) {
    return playing->m_sound_res;
}

uint8_t ui_runtime_sound_playing_is_loop(ui_runtime_sound_playing_t playing) {
    return playing->m_loop;
}

float ui_runtime_sound_playing_volum(ui_runtime_sound_playing_t playing) {
    return playing->m_volume;
}

void ui_runtime_sound_playing_set_volumn(ui_runtime_sound_playing_t playing, float volume) {
    if (playing->m_volume == volume) return;

    playing->m_volume = volume;

    if (playing == TAILQ_FIRST(&playing->m_chanel->m_playings)) {
        ui_runtime_sound_playing_do_set_volume(playing);
    }
}

ui_runtime_sound_playing_t ui_runtime_sound_playing_find_by_id(ui_runtime_module_t module, uint32_t id) {
    ui_runtime_sound_playing_t playing;
    
    TAILQ_FOREACH(playing, &module->m_sound_playings, m_next_for_module) {
        if (playing->m_id == id) return playing;
    }

    return NULL;
}

void ui_runtime_sound_playing_real_free(ui_runtime_sound_playing_t playing) {
    ui_runtime_module_t module = (void*)playing->m_chanel;
    TAILQ_REMOVE(&module->m_free_playings, playing, m_next_for_module);
    mem_free(module->m_alloc, playing);
}

void ui_runtime_sound_playing_do_set_volume(ui_runtime_sound_playing_t playing) {
    ui_runtime_sound_chanel_t chanel = playing->m_chanel;
    
    assert(playing == TAILQ_FIRST(&chanel->m_playings));
    
    if (!playing->m_chanel->m_pause) {
        ui_runtime_sound_backend_t backend = chanel->m_backend;
        backend->m_chanel_set_volumn(
            backend->m_ctx, playing->m_chanel, 
            playing->m_volume
            * chanel->m_group->m_volume
            * chanel->m_group->m_module->m_sound_volume);
    }
}
