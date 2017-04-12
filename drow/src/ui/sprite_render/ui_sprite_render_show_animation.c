#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_render_show_animation_i.h"
#include "ui_sprite_render_module_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_anim_i.h"

ui_sprite_render_show_animation_t
ui_sprite_render_show_animation_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_show_animation_free(ui_sprite_render_show_animation_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

const char * ui_sprite_render_show_animation_group(ui_sprite_render_show_animation_t show_animation) {
    return show_animation->m_cfg_group;
}

void ui_sprite_render_show_animation_set_group(ui_sprite_render_show_animation_t show_animation, const char * group) {
    ui_sprite_render_module_t module = show_animation->m_module;
    
    if (show_animation->m_cfg_group) {
        mem_free(module->m_alloc, show_animation->m_cfg_group);
    }

    if (group) {
        show_animation->m_cfg_group = cpe_str_mem_dup_trim(module->m_alloc, group);
    }
    else {
        show_animation->m_cfg_group = NULL;
    }
}

const char * ui_sprite_render_show_animation_res(ui_sprite_render_show_animation_t show_animation) {
    return show_animation->m_cfg_res;
}

void ui_sprite_render_show_animation_set_res(ui_sprite_render_show_animation_t show_animation, const char * res) {
    ui_sprite_render_module_t module = show_animation->m_module;
    
    if (show_animation->m_cfg_res) {
        mem_free(module->m_alloc, show_animation->m_cfg_res);
    }

    if (res) {
        show_animation->m_cfg_res = cpe_str_mem_dup_trim(module->m_alloc, res);
    }
    else {
        show_animation->m_cfg_res = NULL;
    }
}

void ui_sprite_render_show_animation_set_name(ui_sprite_render_show_animation_t show_animation, const char * name) {
    ui_sprite_render_module_t module = show_animation->m_module;
    
    if (show_animation->m_cfg_name) {
        mem_free(module->m_alloc, show_animation->m_cfg_name);
    }

    if (name) {
        show_animation->m_cfg_name = cpe_str_mem_dup_trim(module->m_alloc, name);
    }
    else {
        show_animation->m_cfg_name = NULL;
    }
}

int ui_sprite_render_show_animation_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_render_sch_t anim_sch = ui_sprite_render_sch_find(entity);
    struct mem_buffer buffer_res;
    const char * res;
    struct mem_buffer buffer_name;
    const char * name;
    ui_sprite_render_anim_t anim;

    if (anim_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): no anim_sch, can`t start anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_cfg_res);
        return -1;
    }

    if (show_animation->m_anim_id != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show animation enter: already started!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (show_animation->m_cfg_res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show animation enter: res not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    mem_buffer_init(&buffer_res, module->m_alloc);
    res = ui_sprite_fsm_action_check_calc_str(&buffer_res, show_animation->m_cfg_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): calc anim url from %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_cfg_res);
        mem_buffer_clear(&buffer_res);
        return -1;
    }

    mem_buffer_init(&buffer_name, module->m_alloc);
    if (show_animation->m_cfg_name) {
        name = ui_sprite_fsm_action_check_calc_str(&buffer_name, show_animation->m_cfg_name, fsm_action, NULL, module->m_em);
        if (name == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): calc anim name from %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_cfg_name);
            mem_buffer_clear(&buffer_res);
            mem_buffer_clear(&buffer_name);
            return -1;
        }
    }
    else {
        name = NULL;
    }

    anim = ui_sprite_render_sch_start_anim(anim_sch, show_animation->m_cfg_group, res, name);
    if (anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): start anim %s(name=%s) fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res, name);
        mem_buffer_clear(&buffer_res);
        mem_buffer_clear(&buffer_name);
        return -1;
    }

    mem_buffer_clear(&buffer_res);
    mem_buffer_clear(&buffer_name);
    
    show_animation->m_anim_id = ui_sprite_render_anim_id(anim);
    
    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

void ui_sprite_render_show_animation_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_render_sch_t anim_sch = ui_sprite_render_sch_find(entity);

    if (anim_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): no anim_sch, can`t stop anim %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_cfg_res);
        return;
    }

    if (show_animation->m_anim_id != 0) {
        ui_sprite_render_anim_t anim = ui_sprite_render_anim_find_by_id(anim_sch, show_animation->m_anim_id);
        if (anim) ui_sprite_render_anim_free(anim);
        show_animation->m_anim_id = 0;
    }
}

void ui_sprite_render_show_animation_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_render_sch_t anim_sch = ui_sprite_render_sch_find(entity);
    ui_sprite_render_anim_t anim;
    
    if (anim_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): no anim_sch, anim %s stop update!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_cfg_res);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    anim = ui_sprite_render_anim_find_by_id(anim_sch, show_animation->m_anim_id);
    if (anim == NULL || !ui_sprite_render_anim_is_runing(anim)) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): anim %s stoped",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), show_animation->m_cfg_res);
        }
        
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

int ui_sprite_render_show_animation_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);

    show_animation->m_module = ctx;
    show_animation->m_anim_id = 0;
    show_animation->m_cfg_res = NULL;
    show_animation->m_cfg_group = NULL;
    show_animation->m_cfg_name = NULL;

    return 0;
}

int ui_sprite_render_show_animation_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_show_animation_t to_show_animation = ui_sprite_fsm_action_data(to);
    ui_sprite_render_show_animation_t from_show_animation = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_show_animation_init(to, ctx) != 0) return -1;

    if (from_show_animation->m_cfg_res) {
        to_show_animation->m_cfg_res = cpe_str_mem_dup(module->m_alloc, from_show_animation->m_cfg_res);
    }

    if (from_show_animation->m_cfg_group) {
        to_show_animation->m_cfg_group = cpe_str_mem_dup(module->m_alloc, from_show_animation->m_cfg_group);
    }

    if (from_show_animation->m_cfg_name) {
        to_show_animation->m_cfg_name = cpe_str_mem_dup(module->m_alloc, from_show_animation->m_cfg_name);
    }
    
    return 0;
}

void ui_sprite_render_show_animation_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_show_animation_t show_animation = ui_sprite_fsm_action_data(fsm_action);

    if (show_animation->m_cfg_res) {
        mem_free(module->m_alloc, show_animation->m_cfg_res);
        show_animation->m_cfg_res = NULL;
    }

    if (show_animation->m_cfg_group) {
        mem_free(module->m_alloc, show_animation->m_cfg_group);
        show_animation->m_cfg_group = NULL;
    }

    if (show_animation->m_cfg_name) {
        mem_free(module->m_alloc, show_animation->m_cfg_name);
        show_animation->m_cfg_name = NULL;
    }
}

static ui_sprite_fsm_action_t ui_sprite_render_show_animation_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_show_animation_t show_animation = ui_sprite_render_show_animation_create(fsm_state, name);
    const char * animation_res;

    if (show_animation == NULL) {
        CPE_ERROR(module->m_em, "%s: create show-animation action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((animation_res = cfg_get_string(cfg, "anim-res", NULL)) == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create show-animation action: anim-res not configured!",
            ui_sprite_render_module_name(module));
        ui_sprite_render_show_animation_free(show_animation);
        return NULL;
    }
    ui_sprite_render_show_animation_set_res(show_animation, animation_res);

    ui_sprite_render_show_animation_set_group(show_animation, cfg_get_string(cfg, "group", NULL));
    ui_sprite_render_show_animation_set_name(show_animation, cfg_get_string(cfg, "anim-name", NULL));

    return ui_sprite_fsm_action_from_data(show_animation);
}

int ui_sprite_render_show_animation_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME, sizeof(struct ui_sprite_render_show_animation));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_show_animation_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_show_animation_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_show_animation_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_show_animation_exit, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_render_show_animation_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_show_animation_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(
                module->m_loader, UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME, ui_sprite_render_show_animation_load, module) != 0)
        {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_render_show_animation_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_render_module_name(module), UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME);
    }
}

const char * UI_SPRITE_RENDER_SHOW_ANIMATION_TYPE_NAME = "show-animation";
