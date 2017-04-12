#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_barrage_clear_bullets_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_barrage_i.h"

ui_sprite_barrage_clear_bullets_t ui_sprite_barrage_clear_bullets_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_barrage_clear_bullets_free(ui_sprite_barrage_clear_bullets_t clear_bullets) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(clear_bullets);
    ui_sprite_fsm_action_free(fsm_action);
}

const char * ui_sprite_barrage_clear_bullets_group_name(ui_sprite_barrage_clear_bullets_t clear_bullets) {
    return clear_bullets->m_group_name;
}

int ui_sprite_barrage_clear_bullets_set_group_name(ui_sprite_barrage_clear_bullets_t clear_bullets, const char * name) {
    if (clear_bullets->m_group_name) {
        mem_free(clear_bullets->m_module->m_alloc, clear_bullets->m_group_name);
    }

    if (name) {
        clear_bullets->m_group_name = cpe_str_mem_dup(clear_bullets->m_module->m_alloc, name);
        if (clear_bullets->m_group_name == NULL) {
            return -1;
        }
    }
    else {
        clear_bullets->m_group_name = NULL;
    }

    return 0;
}

static int ui_sprite_barrage_clear_bullets_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_clear_bullets_t clear_bullets = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage clear bullets: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_barrage_obj_clear_bullets(barrage_obj, clear_bullets->m_group_name);

    return 0;
}

static void ui_sprite_barrage_clear_bullets_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_barrage_clear_bullets_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_clear_bullets_t clear_bullets = ui_sprite_fsm_action_data(fsm_action);
    clear_bullets->m_module = ctx;
    clear_bullets->m_group_name = NULL;
    return 0;
}


static void ui_sprite_barrage_clear_bullets_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_clear_bullets_t clear_bullets = ui_sprite_fsm_action_data(fsm_action);

    if (clear_bullets->m_group_name) {
        mem_free(module->m_alloc, clear_bullets->m_group_name);
        clear_bullets->m_group_name = NULL;
    }
}

static int ui_sprite_barrage_clear_bullets_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_clear_bullets_t to_clear_bullets = ui_sprite_fsm_action_data(to);
    ui_sprite_barrage_clear_bullets_t from_clear_bullets = ui_sprite_fsm_action_data(from);
    
    if (ui_sprite_barrage_clear_bullets_init(to, ctx)) return -1;

    if (from_clear_bullets->m_group_name) {
        to_clear_bullets->m_group_name = cpe_str_mem_dup(module->m_alloc, from_clear_bullets->m_group_name);
        if (to_clear_bullets->m_group_name == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_barrage_clear_bullets_copy: copy str fail");
            return -1; 
        }
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_barrage_clear_bullets_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_clear_bullets_t clear_bullets = ui_sprite_barrage_clear_bullets_create(fsm_state, name);
    
    if (clear_bullets == NULL) {
        CPE_ERROR(module->m_em, "%s: create clear_bullets action: create fail!", ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if (ui_sprite_barrage_clear_bullets_set_group_name(clear_bullets, cfg_get_string(cfg, "group-name", NULL)) != 0) {
        CPE_ERROR(module->m_em, "%s: create clear bullets: set group name fail!", ui_sprite_barrage_module_name(module));
        ui_sprite_barrage_clear_bullets_free(clear_bullets);
        return NULL;
    }
        
    return ui_sprite_fsm_action_from_data(clear_bullets);
}

int ui_sprite_barrage_clear_bullets_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME, sizeof(struct ui_sprite_barrage_clear_bullets));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: barrage clear bullets register: meta create fail",
            ui_sprite_barrage_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_barrage_clear_bullets_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_barrage_clear_bullets_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_barrage_clear_bullets_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_barrage_clear_bullets_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_barrage_clear_bullets_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME, ui_sprite_barrage_clear_bullets_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_clear_bullets_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_barrage_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME);
}

const char * UI_SPRITE_BARRAGE_CLEAR_BULLETS_NAME = "clear-barrage-bullets";
