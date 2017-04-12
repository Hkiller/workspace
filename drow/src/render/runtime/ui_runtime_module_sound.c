#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "ui_runtime_module_i.h"
#include "ui_runtime_sound_group_i.h"
#include "ui_runtime_sound_chanel_i.h"
#include "ui_runtime_sound_playing_i.h"
#include "ui_runtime_sound_backend_i.h"

uint8_t ui_runtime_module_sound_is_pause(ui_runtime_module_t module) {
    return module->m_runing_level != ui_runtime_runing || module->m_sound_logic_pause;
}
    
void ui_runtime_module_sound_set_pause(ui_runtime_module_t module, uint8_t is_pause) {
    if (is_pause) is_pause = 1;

    if (module->m_sound_logic_pause == is_pause) return;

    module->m_sound_logic_pause = is_pause;
    ui_runtime_module_sound_sync_suspend(module);
}

void ui_runtime_module_sound_sync_suspend(ui_runtime_module_t module) {
    ui_runtime_sound_group_t group;
    ui_runtime_sound_chanel_t chanel, chanel_next;

    TAILQ_FOREACH(group, &module->m_sound_groups, m_next) {
        for(chanel = TAILQ_FIRST(&group->m_chanels); chanel; chanel = chanel_next) {
            chanel_next = TAILQ_NEXT(chanel, m_next_for_group);
            ui_runtime_sound_chanel_sync_pause(chanel);
        }
    }
}

float ui_runtime_module_sound_volum(ui_runtime_module_t module) {
    return module->m_sound_volume;
}

void ui_runtime_module_sound_set_volumn(ui_runtime_module_t module, float volume) {
    ui_runtime_sound_group_t group;
    ui_runtime_sound_chanel_t chanel;
    
    if (module->m_sound_volume == volume) return;

    TAILQ_FOREACH(group, &module->m_sound_groups, m_next) {
        TAILQ_FOREACH(chanel, &group->m_chanels, m_next_for_group) {
            ui_runtime_sound_playing_t playing = TAILQ_FIRST(&chanel->m_playings);
            if (playing) {
                ui_runtime_sound_playing_do_set_volume(playing);            
            }
        }
    }
}

ui_runtime_sound_playing_t
ui_runtime_module_sound_play_by_res(ui_runtime_module_t module, const char * group_name, ui_cache_res_t res, uint8_t is_loop) {
    ui_runtime_sound_group_t group;
    ui_runtime_sound_playing_t playing;
    
    group = ui_runtime_sound_group_find(module, group_name);
    if (group == NULL) {
        CPE_ERROR(module->m_em, "sound: play %s@%s: group not exist!", ui_cache_res_path(res), group_name);
        return NULL;
    }

    if (ui_cache_res_type(res) != ui_cache_res_type_sound) {
        CPE_ERROR(module->m_em, "sound: play %s@%s: res is not sound!", ui_cache_res_path(res), group_name);
        return NULL;
    }

    if (ui_cache_res_load_state(res) != ui_cache_res_loaded) {
        CPE_ERROR(module->m_em, "sound: play %s@%s: res is not loaded!", ui_cache_res_path(res), group_name);
        return NULL;
    }

    playing = ui_runtime_sound_group_play(group, res, is_loop);
    if (playing == NULL) {
        CPE_ERROR(module->m_em, "sound: play %s@%s: start fail!", ui_cache_res_path(res), group_name);
        return NULL;
    }

    return playing;
}

ui_runtime_sound_playing_t
ui_runtime_module_sound_play_by_res_path(ui_runtime_module_t module, const char * group_name, const char * res_path, uint8_t is_loop) {
    ui_cache_res_t res;
    
    res = ui_cache_res_find_by_path(module->m_cache_mgr, res_path);
    if (res == NULL) {
        CPE_ERROR(module->m_em, "sound: play %s@%s: res not exist!", res_path, group_name);
        return NULL;
    }

    return ui_runtime_module_sound_play_by_res(module, group_name, res, is_loop);
}

void ui_runtime_module_sound_stop_by_res(ui_runtime_module_t module, ui_cache_res_t res) {
    ui_runtime_sound_playing_t playing, playing_next;

    for(playing = TAILQ_FIRST(&module->m_sound_playings); playing; playing = playing_next) {
        playing_next = TAILQ_NEXT(playing, m_next_for_module);
        if (playing->m_sound_res == res) {
            if (module->m_debug) {
                CPE_INFO(
                    module->m_em, "ui_runtime_module_on_res_unload: sound %s auto stop playing %d",
                    ui_cache_res_path(res), playing->m_id);
            }

            ui_runtime_sound_playing_free(playing);
        }
    }
}

static ptr_int_t ui_runtime_module_sound_tick(void * ctx, ptr_int_t arg, float delta_s) {
    ui_runtime_module_t module = ctx;
    ui_runtime_sound_group_t group;
    ui_runtime_sound_chanel_t chanel, chanel_next;
    ui_runtime_sound_chanel_state_t cur_state;

    if (ui_runtime_module_sound_is_pause(module)) return 0;

    TAILQ_FOREACH(group, &module->m_sound_groups, m_next) {
        for(chanel = TAILQ_FIRST(&group->m_chanels); chanel; chanel = chanel_next) {
            ui_runtime_sound_backend_t backend = chanel->m_backend;
            ui_runtime_sound_playing_t playing;

            chanel_next = TAILQ_NEXT(chanel, m_next_for_group);

            playing = TAILQ_FIRST(&chanel->m_playings);
            if (playing == NULL) {
                ui_runtime_sound_chanel_free(chanel);
                continue;
            }

            cur_state = ui_runtime_sound_chanel_state(chanel);

            switch(cur_state) {
            case ui_runtime_sound_chanel_paused:
                if (backend->m_chanel_resume(backend->m_ctx, chanel) != 0) {
                    CPE_ERROR(module->m_em, "ui_runtime_module_sound_tick: set res fail");
                    ui_runtime_sound_playing_free(playing);
                    chanel_next = chanel;
                    continue;
                }
                /*goto next*/;
            case ui_runtime_sound_chanel_initial: {
                if (backend->m_chanel_play(
                        backend->m_ctx, chanel,
                        ui_cache_res_plugin_data(playing->m_sound_res),
                        playing->m_volume * chanel->m_group->m_volume * module->m_sound_volume,
                        playing->m_loop)
                    != 0)
                {
                    CPE_ERROR(module->m_em, "ui_runtime_module_sound_tick: set res fail");
                    ui_runtime_sound_playing_free(playing);
                    chanel_next = chanel;
                    continue;
                }
                
                /* assert( */
                /*     ui_runtime_sound_chanel_state(chanel) == ui_runtime_sound_chanel_playing */
                /*     || ui_runtime_sound_chanel_state(chanel) == ui_runtime_sound_chanel_stopped); */
                
                if (module->m_debug) {
                    CPE_INFO(
                        module->m_em, "sound chanel %s[%d]: playing %d: res %s: begin play (%s)!",
                        chanel->m_group->m_name, chanel->m_id, playing->m_id, ui_cache_res_path(playing->m_sound_res), 
                        playing->m_loop ? "loop" : "not loop");
                }
                
                break;
            }
            case ui_runtime_sound_chanel_playing:
                if (playing->m_stop_duration > 0.0f) {
                    //playing->m_stop_worked +=
                }
                break;
            case ui_runtime_sound_chanel_stopped:
                if (module->m_debug) {
                    CPE_INFO(
                        module->m_em, "sound chanel %s: playing %d: res %s: stoped",
                        chanel->m_group->m_name, playing->m_id, ui_cache_res_path(playing->m_sound_res));
                }
                
                ui_runtime_sound_playing_free(playing);
                chanel_next = chanel;
                continue;
            default: 
                CPE_ERROR(module->m_em, "ui_runtime_module_sound_tick: unlnown source %d", cur_state);
                ui_runtime_sound_chanel_free(chanel);
                continue;
            }
        }
    }
    
    return 0;
}

int ui_runtime_module_init_sound_updator(ui_runtime_module_t module) {
    if (gd_app_tick_add(module->m_app, ui_runtime_module_sound_tick, module, 0) != 0) {
        CPE_ERROR(module->m_em, "ui_runtime_module_init_updator: add app tick fail!");
        return -1;
    }

    return 0;
}
    
void ui_runtime_module_fini_sound_updator(ui_runtime_module_t module) {
    gd_app_tick_remove(module->m_app, ui_runtime_module_sound_tick, module);
}

