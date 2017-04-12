#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_spine_play_anim_i.h"
#include "ui_sprite_spine_utils_i.h"

ui_sprite_spine_play_anim_t ui_sprite_spine_play_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_PLAY_ANIM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_play_anim_free(ui_sprite_spine_play_anim_t play_anim) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(play_anim);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_play_anim_set_def(ui_sprite_spine_play_anim_t play_anim, const char * anim_def) {
    
    ui_sprite_spine_module_t module = play_anim->m_module;

    if (play_anim->m_anim_def) mem_free(module->m_alloc, play_anim->m_anim_def);

    if (anim_def) {
        play_anim->m_anim_def = cpe_str_mem_dup(module->m_alloc, cpe_str_trim_head((char*)anim_def));
        if (play_anim->m_anim_def == NULL) return -1;
        * cpe_str_trim_tail(play_anim->m_anim_def + strlen(play_anim->m_anim_def), play_anim->m_anim_def) = 0;
    }
    else {
        play_anim->m_anim_def = NULL;
    }

    return 0;
}

const char * ui_sprite_spine_play_anim_def(ui_sprite_spine_play_anim_t play_anim) {
    return play_anim->m_anim_def;
}

int ui_sprite_spine_play_anim_set_name(ui_sprite_spine_play_anim_t play_anim, const char * anim_name) {
    
    ui_sprite_spine_module_t module = play_anim->m_module;

    if (play_anim->m_anim_name) mem_free(module->m_alloc, play_anim->m_anim_name);

    if (anim_name) {
        play_anim->m_anim_name = cpe_str_mem_dup(module->m_alloc, cpe_str_trim_head((char*)anim_name));
        if (play_anim->m_anim_name == NULL) return -1;
        * cpe_str_trim_tail(play_anim->m_anim_name + strlen(play_anim->m_anim_name), play_anim->m_anim_name) = 0;
    }
    else {
        play_anim->m_anim_name = NULL;
    }

    return 0;
}

const char * ui_sprite_spine_play_anim_name(ui_sprite_spine_play_anim_t play_anim) {
    return play_anim->m_anim_name;
}

static void ui_sprite_spine_play_anim_on_event(void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, spEvent* event) {
    ui_sprite_fsm_action_t fsm_action = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (type == plugin_spine_anim_event_event) {
        ui_sprite_entity_build_and_send_event(entity, event->data->name, NULL);
	}
}

static int ui_sprite_spine_play_anim_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_spine_obj_t spine_obj;
    struct mem_buffer dump_buffer;
    const char * anim_def;

    if (play_anim->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: anim name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    spine_obj = ui_sprite_spine_find_obj(module->m_sprite_render, entity, play_anim->m_anim_name, module->m_em);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: anim %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), play_anim->m_anim_name);
        return -1;
    }

    if (play_anim->m_anim_def == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: anim name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    assert(play_anim->m_anim_group == NULL);
    play_anim->m_anim_group = plugin_spine_obj_anim_group_create(plugin_spine_obj_module(spine_obj));
    if (play_anim->m_anim_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: create anim group!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    mem_buffer_init(&dump_buffer, module->m_alloc);
    anim_def = ui_sprite_fsm_action_check_calc_str(&dump_buffer, play_anim->m_anim_def, fsm_action, NULL, module->m_em);
    if (anim_def == NULL) {
        mem_buffer_clear(&dump_buffer);
        plugin_spine_obj_anim_group_free(play_anim->m_anim_group);
        play_anim->m_anim_group = NULL;
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: calc anim def fail, def=%s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), play_anim->m_anim_def);
        return -1;
    }

    if (plugin_spine_obj_play_anims(spine_obj, anim_def, play_anim->m_anim_group) != 0) {
        mem_buffer_clear(&dump_buffer);
        plugin_spine_obj_anim_group_free_all_anims(play_anim->m_anim_group);
        plugin_spine_obj_anim_group_free(play_anim->m_anim_group);
        play_anim->m_anim_group = NULL;
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: play anims fail, def=%s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), anim_def);
        return -1;
    }

    if (plugin_spine_obj_anim_group_add_track_listener(play_anim->m_anim_group, ui_sprite_spine_play_anim_on_event, fsm_action) != 0) {
        mem_buffer_clear(&dump_buffer);
        plugin_spine_obj_anim_group_free_all_anims(play_anim->m_anim_group);
        plugin_spine_obj_anim_group_free(play_anim->m_anim_group);
        play_anim->m_anim_group = NULL;
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine play anim: add listener fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        if (ui_sprite_fsm_action_start_update(fsm_action) != 0) {
            mem_buffer_clear(&dump_buffer);
            plugin_spine_obj_anim_group_free_all_anims(play_anim->m_anim_group);
            plugin_spine_obj_anim_group_free(play_anim->m_anim_group);
            play_anim->m_anim_group = NULL;
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine play anim: start update fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): spine play anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), anim_def);
    }

    mem_buffer_clear(&dump_buffer);
    
    return 0;
}

static void ui_sprite_spine_play_anim_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);

    if (!plugin_spine_obj_anim_group_is_playing(play_anim->m_anim_group)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_spine_play_anim_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);

    if (play_anim->m_anim_group) {
        plugin_spine_obj_anim_group_free_all_anims(play_anim->m_anim_group);
        plugin_spine_obj_anim_group_free(play_anim->m_anim_group);
        play_anim->m_anim_group = NULL;
    }
}

static int ui_sprite_spine_play_anim_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);
    play_anim->m_module = ctx;
    play_anim->m_anim_name = NULL;
	play_anim->m_anim_def = NULL;
    play_anim->m_anim_group = NULL;
    return 0;
}

static void ui_sprite_spine_play_anim_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t play_anim = ui_sprite_fsm_action_data(fsm_action);

    assert(play_anim->m_anim_group == NULL);

    if (play_anim->m_anim_def) {
        mem_free(module->m_alloc, play_anim->m_anim_def);
        play_anim->m_anim_def = NULL;
    }

    if (play_anim->m_anim_name) {
        mem_free(module->m_alloc, play_anim->m_anim_name);
        play_anim->m_anim_name = NULL;
    }
}

static int ui_sprite_spine_play_anim_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t to_play_anim = ui_sprite_fsm_action_data(to);
    ui_sprite_spine_play_anim_t from_play_anim = ui_sprite_fsm_action_data(from);

    if (ui_sprite_spine_play_anim_init(to, ctx)) return -1;

    if (from_play_anim->m_anim_def) {
        to_play_anim->m_anim_def = cpe_str_mem_dup(module->m_alloc, from_play_anim->m_anim_def);
    }

    if (from_play_anim->m_anim_name) {
        to_play_anim->m_anim_name = cpe_str_mem_dup(module->m_alloc, from_play_anim->m_anim_name);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_spine_play_anim_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_play_anim_t spine_play_anim = ui_sprite_spine_play_anim_create(fsm_state, name);
    const char * str_value;

    if (spine_play_anim == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine_play_anim action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-def", NULL))) {
        if (ui_sprite_spine_play_anim_set_def(spine_play_anim, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_play_anim action: set anim-def %s fail",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_play_anim_free(spine_play_anim);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine_play_anim action: anim-def not configured",
            ui_sprite_spine_module_name(module));
        ui_sprite_spine_play_anim_free(spine_play_anim);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_play_anim_set_name(spine_play_anim, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_play_anim action: set anim-name %s fail",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_play_anim_free(spine_play_anim);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine_play_anim action: anim-name not configured",
            ui_sprite_spine_module_name(module));
        ui_sprite_spine_play_anim_free(spine_play_anim);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(spine_play_anim);
}

int ui_sprite_spine_play_anim_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_PLAY_ANIM_NAME, sizeof(struct ui_sprite_spine_play_anim));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_play_anim_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_play_anim_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_play_anim_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_play_anim_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_play_anim_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_play_anim_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_PLAY_ANIM_NAME, ui_sprite_spine_play_anim_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_play_anim_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_PLAY_ANIM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_PLAY_ANIM_NAME = "spine-play-anim";

