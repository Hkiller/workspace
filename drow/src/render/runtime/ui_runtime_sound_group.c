#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/cache/ui_cache_res.h"
#include "ui_runtime_sound_group_i.h"
#include "ui_runtime_sound_chanel_i.h"
#include "ui_runtime_sound_playing_i.h"
#include "ui_runtime_sound_backend_i.h"

ui_runtime_sound_group_t
ui_runtime_sound_group_create(ui_runtime_module_t module, const char * name, ui_runtime_sound_type_t sound_type) {
    ui_runtime_sound_group_t group;

    assert(name);
    
    group = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_sound_group));
    if (group == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_sound_group_create: alloc fail!");
        return NULL;
    }

    group->m_module = module;
    group->m_enable = 1;
    group->m_volume = 1.0f;
    group->m_sound_type = sound_type;
    group->m_schedule_type = ui_runtime_sound_group_schedule_none;
    
    cpe_str_dup(group->m_name, sizeof(group->m_name), name);
    TAILQ_INIT(&group->m_chanels);
    TAILQ_INSERT_TAIL(&module->m_sound_groups, group, m_next);
    
    return group;
}

void ui_runtime_sound_group_free(ui_runtime_sound_group_t group) {
    ui_runtime_module_t module = group->m_module;

    while(!TAILQ_EMPTY(&group->m_chanels)) {
        ui_runtime_sound_chanel_free(TAILQ_FIRST(&group->m_chanels));
    }

    TAILQ_REMOVE(&module->m_sound_groups, group, m_next);

    mem_free(module->m_alloc, group);
}

ui_runtime_sound_group_t
ui_runtime_sound_group_find(ui_runtime_module_t module, const char * name) {
    ui_runtime_sound_group_t group;

    TAILQ_FOREACH(group, &module->m_sound_groups, m_next) {
        if (group->m_name[0] && strcmp(group->m_name, name) == 0) {
            return group;
        }
    }

    return NULL;
}

uint8_t ui_runtime_sound_group_is_enable(ui_runtime_sound_group_t group) {
    return group->m_enable;
}

void ui_runtime_sound_group_set_enable(ui_runtime_sound_group_t group, uint8_t enable) {
    if (enable) enable = 1;
    
    if (group->m_enable == enable) return;

    group->m_enable = enable;

    if (!group->m_enable) {
        while(!TAILQ_EMPTY(&group->m_chanels)) {
            ui_runtime_sound_chanel_free(TAILQ_FIRST(&group->m_chanels));
        }
    }
}

ui_runtime_sound_playing_t
ui_runtime_sound_group_play(ui_runtime_sound_group_t group, ui_cache_res_t res, uint8_t is_loop) {
    ui_runtime_module_t module = group->m_module;
    ui_runtime_sound_chanel_t chanel;

    if (!group->m_enable) {
        CPE_ERROR(module->m_em, "ui_runtime_sound_group_play: group %s is not enable!", group->m_name);
        return NULL;
    }

    switch(group->m_schedule_type) {
    case ui_runtime_sound_group_schedule_none:
        TAILQ_FOREACH(chanel, &group->m_chanels, m_next_for_group) {
            if (TAILQ_EMPTY(&chanel->m_playings)) {
                break;
            }
        }
        break;
    case ui_runtime_sound_group_schedule_queue:
        chanel = TAILQ_FIRST(&group->m_chanels);
        if (chanel) {
            ui_runtime_sound_playing_t playing = TAILQ_FIRST(&chanel->m_playings);
            if (playing) {
                ui_runtime_sound_playing_free(playing);
                //playing->m_stop_duration = 1.0f;
            }
        }
        break;
    default:
        CPE_ERROR(module->m_em, "ui_runtime_sound_group_play: group %s type %d unknown!", group->m_name, group->m_schedule_type);
        return NULL;
    }

    if (chanel == NULL) {
        ui_runtime_sound_backend_t backend;
        
        backend = ui_runtime_sound_backend_find(module, group->m_sound_type);
        if (backend == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_sound_group_play: backend for sound type %d not exist!", group->m_sound_type);
            return NULL;
        }
        
        chanel = ui_runtime_sound_chanel_create(group, backend);
        if (chanel == NULL) {
            CPE_ERROR(module->m_em, "ui_runtime_sound_group_play: create chanel fail!");
            return NULL;
        }
    }

    assert(chanel);

    return ui_runtime_sound_playing_create(chanel, res, is_loop);
}

ui_runtime_sound_playing_t
ui_runtime_sound_group_play_by_path(ui_runtime_sound_group_t group, const char * res_path, uint8_t is_loop) {
    ui_cache_res_t sound_res = ui_cache_res_find_by_path(group->m_module->m_cache_mgr, res_path);
    if (sound_res == NULL) {
        CPE_ERROR(group->m_module->m_em, "ui_runtime_sound_group_play_by_path: res %s not exist!", res_path);
        return NULL;
    }

    return ui_runtime_sound_group_play(group, sound_res, is_loop);
}

float ui_runtime_sound_group_volum(ui_runtime_sound_group_t group) {
    return group->m_volume;
}

void ui_runtime_sound_group_set_volumn(ui_runtime_sound_group_t group, float volume) {
    ui_runtime_sound_chanel_t chanel;
    if (group->m_volume == volume) return;

    group->m_volume = volume;

    TAILQ_FOREACH(chanel, &group->m_chanels, m_next_for_group) {
        ui_runtime_sound_playing_t playing = TAILQ_FIRST(&chanel->m_playings);
        if (playing) {
            ui_runtime_sound_playing_do_set_volume(playing);            
        }
    }
}

ui_runtime_sound_group_schedule_type_t
ui_runtime_sound_group_schedule_type(ui_runtime_sound_group_t group) {
    return group->m_schedule_type;
}

void ui_runtime_sound_group_set_schedule_type(ui_runtime_sound_group_t group, ui_runtime_sound_group_schedule_type_t schedule_type) {
    group->m_schedule_type = schedule_type;
}
