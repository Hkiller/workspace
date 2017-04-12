#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui_runtime_sound_chanel_i.h"
#include "ui_runtime_sound_playing_i.h"
#include "ui_runtime_sound_backend_i.h"

ui_runtime_sound_chanel_t
ui_runtime_sound_chanel_create(ui_runtime_sound_group_t group, ui_runtime_sound_backend_t backend) {
    ui_runtime_module_t module = group->m_module;
    ui_runtime_sound_chanel_t chanel;

    chanel = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_sound_chanel) + backend->m_chanel_capacity);
    if (chanel == NULL) {
        CPE_ERROR(module->m_em, "ui_runtime_sound_chanel_create: alloc fail!");
        return NULL;
    }

    chanel->m_group = group;
    chanel->m_backend = backend;
    chanel->m_id = group->m_chanel_max_id + 1;
    TAILQ_INIT(&chanel->m_playings);

    if (backend->m_chanel_init(backend->m_ctx, chanel) != 0) {
        CPE_ERROR(module->m_em, "ui_runtime_sound_chanel_create: backend init chanel fail!");
        mem_free(module->m_alloc, chanel);
        return NULL;
    }
        
    module->m_sound_chanel_count++;
    group->m_chanel_max_id++;
    group->m_chanel_count++;
    TAILQ_INSERT_TAIL(&group->m_chanels, chanel, m_next_for_group);
    TAILQ_INSERT_TAIL(&backend->m_chanels, chanel, m_next_for_backend);
    
    return chanel;
}

void ui_runtime_sound_chanel_free(ui_runtime_sound_chanel_t chanel) {
    ui_runtime_sound_group_t group = chanel->m_group;
    ui_runtime_module_t module = group->m_module;

    while(!TAILQ_EMPTY(&chanel->m_playings)) {
        ui_runtime_sound_playing_free(TAILQ_FIRST(&chanel->m_playings));
    }

    chanel->m_backend->m_chanel_fini(chanel->m_backend->m_ctx, chanel);
    
    module->m_sound_chanel_count--;
    group->m_chanel_count--;

    TAILQ_REMOVE(&chanel->m_backend->m_chanels, chanel, m_next_for_backend);
    
    chanel->m_group = (void*)module;
    TAILQ_REMOVE(&group->m_chanels, chanel, m_next_for_group);
    mem_free(module->m_alloc, chanel);
}

void ui_runtime_sound_chanel_sync_pause(ui_runtime_sound_chanel_t chanel) {
    ui_runtime_module_t module = chanel->m_group->m_module;
    uint8_t need_pause = ui_runtime_module_sound_is_pause(module);
    ui_runtime_sound_chanel_state_t cur_state;

    if (need_pause == chanel->m_pause) return;

    chanel->m_pause = need_pause;
    
    if (TAILQ_EMPTY(&chanel->m_playings)) return;

    cur_state = ui_runtime_sound_chanel_state(chanel);

    if (cur_state == ui_runtime_sound_chanel_playing && need_pause) {
        if (chanel->m_backend->m_chanel_pause(chanel->m_backend->m_ctx, chanel) != 0) {
            CPE_ERROR(module->m_em, "ui_runtime_sound_chanel_sync_pause: suspend fail");
            ui_runtime_sound_chanel_free(chanel);
            return;
        }
    }
    else if (cur_state == ui_runtime_sound_chanel_paused && !need_pause) {
        if (chanel->m_backend->m_chanel_resume(chanel->m_backend->m_ctx, chanel) != 0) {
            CPE_ERROR(module->m_em, "ui_runtime_sound_chanel_sync_pause: resume fail");
            ui_runtime_sound_chanel_free(chanel);
            return;
        }
    }
}

ui_runtime_sound_chanel_state_t ui_runtime_sound_chanel_state(ui_runtime_sound_chanel_t chanel) {
    return chanel->m_backend->m_chanel_get_state(chanel->m_backend->m_ctx, chanel);
}

const char * ui_runtime_sound_chanel_state_str(ui_runtime_sound_chanel_t chanel) {
    ui_runtime_sound_chanel_state_t state = ui_runtime_sound_chanel_state(chanel);
    switch(state) {
    case ui_runtime_sound_chanel_initial:
        return "initial";
    case ui_runtime_sound_chanel_playing:
        return "playing";
    case ui_runtime_sound_chanel_paused:
        return "paused";
    case ui_runtime_sound_chanel_stopped:
        return "stoped";
    default:
        return "unknown";
    }
}

void * ui_runtime_sound_chanel_data(ui_runtime_sound_chanel_t chanel) {
    return chanel + 1;
}
