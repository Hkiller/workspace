#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_sound_group.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_sound_play_i.h"

ui_sprite_sound_play_t ui_sprite_sound_play_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SOUND_PLAY_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_sound_play_free(ui_sprite_sound_play_t play_bgm) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(play_bgm);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_sound_play_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_sound_module_t module = ctx;
    ui_runtime_sound_group_t group;
    ui_sprite_sound_play_t play_bgm = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (play_bgm->m_res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): sound play: res not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    group = ui_runtime_sound_group_find(module->m_runtime, "bgm");
    if (group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): sound play: group bgm not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_runtime_sound_group_play_by_path(group, play_bgm->m_res, play_bgm->m_loop_count) == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): sound play: res %s play fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), play_bgm->m_res);
        return -1;
    }

    return 0;
}

static void ui_sprite_sound_play_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_sound_play_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_sound_play_t play_bgm = ui_sprite_fsm_action_data(fsm_action);
	bzero(play_bgm, sizeof(*play_bgm));
    play_bgm->m_module = ctx;
    return 0;
}

static void ui_sprite_sound_play_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_sound_module_t module = ctx;
    ui_sprite_sound_play_t play_bgm = ui_sprite_fsm_action_data(fsm_action);

    if(play_bgm->m_res) {
        mem_free(module->m_alloc, play_bgm->m_res);
        play_bgm->m_res = NULL;
    }
}

static int ui_sprite_sound_play_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_sound_module_t module = ctx;
    ui_sprite_sound_play_t to_play_bgm = ui_sprite_fsm_action_data(to);
    ui_sprite_sound_play_t from_play_bgm = ui_sprite_fsm_action_data(from);

    if (ui_sprite_sound_play_init(to, ctx)) return -1;

    if (from_play_bgm->m_res) {
        to_play_bgm->m_res = cpe_str_mem_dup(module->m_alloc, from_play_bgm->m_res);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_sound_play_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_sound_module_t module = ctx;
    ui_sprite_sound_play_t sound_play_bgm = ui_sprite_sound_play_create(fsm_state, name);

    if (sound_play_bgm == NULL) {
        CPE_ERROR(module->m_em, "%s: create sound_play_bgm action: create fail!", ui_sprite_sound_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(sound_play_bgm);
}

int ui_sprite_sound_play_regist(ui_sprite_sound_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SOUND_PLAY_NAME, sizeof(struct ui_sprite_sound_play));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_sound_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_sound_play_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_sound_play_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_sound_play_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_sound_play_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_sound_play_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SOUND_PLAY_NAME, ui_sprite_sound_play_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_sound_play_unregist(ui_sprite_sound_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SOUND_PLAY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_sound_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SOUND_PLAY_NAME = "sound-play";

