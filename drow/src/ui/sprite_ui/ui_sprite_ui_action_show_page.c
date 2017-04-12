#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_ui_action_show_page_i.h"
#include "ui_sprite_ui_env_i.h"
#include "ui_sprite_ui_phase_i.h"

ui_sprite_ui_action_show_page_t ui_sprite_ui_action_show_page_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_show_page_free(ui_sprite_ui_action_show_page_t action_show_page) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_show_page);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_show_page_set_page_name(ui_sprite_ui_action_show_page_t action_show_page, const char * page_name) {
    assert(page_name);

    if (action_show_page->m_page_name) {
        mem_free(action_show_page->m_module->m_alloc, action_show_page->m_page_name);
        action_show_page->m_page_name = NULL;
    }

    action_show_page->m_page_name = cpe_str_mem_dup(action_show_page->m_module->m_alloc, page_name);
    
    return 0;
}

const char * ui_sprite_ui_action_show_page_name(ui_sprite_ui_action_show_page_t action_show_page) {
    return action_show_page->m_page_name;
}

int ui_sprite_ui_action_show_page_set_before_page_name(ui_sprite_ui_action_show_page_t action_show_page, const char * before_page_name) {
    if (action_show_page->m_before_page_name) {
        mem_free(action_show_page->m_module->m_alloc, action_show_page->m_before_page_name);
    }

    if(before_page_name) {
        action_show_page->m_before_page_name = cpe_str_mem_dup(action_show_page->m_module->m_alloc, before_page_name);
        if (action_show_page->m_before_page_name == NULL) {
            return -1;
        }
    }
    else {
        action_show_page->m_before_page_name = NULL;
    }
    
    return 0;
}

const char * ui_sprite_ui_action_show_before_page_name(ui_sprite_ui_action_show_page_t action_show_page) {
    return action_show_page->m_before_page_name;
}

plugin_ui_state_node_t
ui_sprite_ui_action_find_cur_state(ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action) {
    plugin_ui_phase_node_t b_phase;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);    

    b_phase = plugin_ui_phase_node_current(module->m_env->m_env);
    if (b_phase == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show page: ui env no current phase!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return NULL;
    }

    if (entity != module->m_env->m_entity) {
        plugin_ui_state_node_t cur_state;
        
        cur_state = plugin_ui_state_node_current(b_phase);
        if (cur_state == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): show page: ui env no current state!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return NULL;
        }

        return cur_state;
    }
    else {
        ui_sprite_ui_phase_t phase;
        ui_sprite_fsm_state_t cur_state = ui_sprite_fsm_action_state(fsm_action);
        ui_sprite_fsm_ins_t fsm_ins = ui_sprite_fsm_state_fsm(cur_state);
        ui_sprite_fsm_ins_t p_fsm_ins = ui_sprite_fsm_parent(fsm_ins);
        ui_sprite_fsm_state_t p_state = ui_sprite_fsm_current_state(p_fsm_ins);
        ui_sprite_fsm_action_t state_action;
        int level;
        plugin_ui_state_node_t state_node;
        plugin_ui_phase_t b_phase_data = plugin_ui_phase_node_current_phase(b_phase);
        
        if (b_phase_data == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): show page: cur phase not work!!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return NULL;
        }

        phase = plugin_ui_phase_data(b_phase_data);

        while(p_state != phase->m_phase_state) {
            cur_state = p_state;
            fsm_ins = p_fsm_ins;
            p_fsm_ins = ui_sprite_fsm_parent(fsm_ins);
            p_state = ui_sprite_fsm_current_state(p_fsm_ins);
            assert(p_state);
        }

        state_action = ui_sprite_fsm_action_from_data(ui_sprite_fsm_state_fsm(cur_state));
        if (sscanf(ui_sprite_fsm_action_name(state_action), "L%d", &level) != 1) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): show page: get state level from name %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(state_action));
            return NULL;
        }

        state_node = plugin_ui_state_node_find_by_level(b_phase, level);
        if (state_node == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): show page: no state at level %d!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), level);
            return NULL;
        }

        return state_node;
    }
}

static void ui_sprite_ui_action_show_page_on_hide(void *ctx, plugin_ui_page_t page, plugin_ui_state_node_t state_node) {
    ui_sprite_ui_action_show_page_t action_show_page = ctx;
    
    assert(action_show_page->m_page);
    assert(action_show_page->m_show_in_state);

    action_show_page->m_page = NULL;
    action_show_page->m_show_in_state = NULL;
    
    ui_sprite_fsm_action_stop_update(ui_sprite_fsm_action_from_data(action_show_page));
}

static int ui_sprite_ui_action_show_page_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_page_t action_show_page = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ui_env_t env;
    plugin_ui_state_node_t cur_state;
    plugin_ui_page_t page;
    struct ui_sprite_fsm_addition_source_ctx source_ctx;
    dr_data_source_t data_source = NULL;

    assert(action_show_page->m_page == NULL);
    assert(action_show_page->m_show_in_state == NULL);

    if (action_show_page->m_page_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show page: no page name configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    env = module->m_env;
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show page: no ui env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    cur_state = ui_sprite_ui_action_find_cur_state(module, fsm_action);
    if (cur_state == NULL) {
        return -1;
    }

    page = plugin_ui_page_find(env->m_env, action_show_page->m_page_name);
    if (page == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show page: page %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_show_page->m_page_name);
        return -1;
    }

    ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &source_ctx);

    if (plugin_ui_page_show_in_state(
            page, cur_state, data_source, action_show_page->m_before_page_name,
            ui_sprite_ui_action_show_page_on_hide, action_show_page)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): show page: show %s in state fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_show_page->m_page_name);
        return -1;
    }

    if (action_show_page->m_force_change) {
        plugin_ui_page_set_changed(page, 1);
    }

    action_show_page->m_page = page;
    action_show_page->m_show_in_state = cur_state;
    
    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

static void ui_sprite_ui_action_show_page_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
}

static void ui_sprite_ui_action_show_page_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_show_page_t action_show_page = ui_sprite_fsm_action_data(fsm_action);

    if (action_show_page->m_page) {
        assert(action_show_page->m_show_in_state);

        plugin_ui_page_clear_hide_monitor(action_show_page->m_page, ctx);
        plugin_ui_page_hide_in_state(action_show_page->m_page, action_show_page->m_show_in_state);

        action_show_page->m_page = NULL;
        action_show_page->m_show_in_state = NULL;
    }
}

static int ui_sprite_ui_action_show_page_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_show_page_t action_show_page = ui_sprite_fsm_action_data(fsm_action);
    action_show_page->m_module = ctx;
    action_show_page->m_page_name = NULL;
    action_show_page->m_before_page_name = NULL;
    action_show_page->m_force_change = 0;
    action_show_page->m_page = NULL;
    action_show_page->m_show_in_state = NULL;
    return 0;
}

static void ui_sprite_ui_action_show_page_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;
    ui_sprite_ui_action_show_page_t action_show_page = ui_sprite_fsm_action_data(fsm_action);

    assert(action_show_page->m_page == NULL);
    assert(action_show_page->m_show_in_state == NULL);
    
    if (action_show_page->m_page_name) {
        mem_free(modue->m_alloc, action_show_page->m_page_name);
        action_show_page->m_page_name = NULL;
    }

    if (action_show_page->m_before_page_name) {
        mem_free(modue->m_alloc, action_show_page->m_before_page_name);
        action_show_page->m_before_page_name = NULL;
    }
}

static int ui_sprite_ui_action_show_page_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t modue = ctx;    
	ui_sprite_ui_action_show_page_t to_action_show_page = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_show_page_t from_action_show_page = ui_sprite_fsm_action_data(from);

	if (ui_sprite_ui_action_show_page_init(to, ctx)) return -1;

    to_action_show_page->m_force_change = from_action_show_page->m_force_change;

    if (from_action_show_page->m_page_name) {
        to_action_show_page->m_page_name = cpe_str_mem_dup(modue->m_alloc, from_action_show_page->m_page_name);
    }
    
    if (from_action_show_page->m_before_page_name) {
        to_action_show_page->m_before_page_name = cpe_str_mem_dup(modue->m_alloc, from_action_show_page->m_before_page_name);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_ui_action_show_page_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_show_page_t action_show_page = ui_sprite_ui_action_show_page_create(fsm_state, name);
    const char * page_name;
    
    if (action_show_page == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_show_page action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    page_name = cfg_get_string(cfg, "page", NULL);
    if (page_name == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_show_page action: page not configured!", ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_show_page_free(action_show_page);
        return NULL;
    }

    if (ui_sprite_ui_action_show_page_set_page_name(action_show_page, page_name) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create action_show_page action: set page name %s fail!",
            ui_sprite_ui_module_name(module), page_name);
        ui_sprite_ui_action_show_page_free(action_show_page);
        return NULL;
    }

    if (ui_sprite_ui_action_show_page_set_before_page_name(action_show_page, cfg_get_string(cfg, "before-page", NULL)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create action_show_page action: set before page name fail!",
            ui_sprite_ui_module_name(module));
        ui_sprite_ui_action_show_page_free(action_show_page);
        return NULL;
    }

    action_show_page->m_force_change = cfg_get_uint8(cfg, "force-change", action_show_page->m_force_change);

    return ui_sprite_fsm_action_from_data(action_show_page);
}

int ui_sprite_ui_action_show_page_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME, sizeof(struct ui_sprite_ui_action_show_page));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_show_page_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_show_page_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_show_page_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_show_page_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_show_page_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_show_page_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME, ui_sprite_ui_action_show_page_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_action_show_page_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME);
}

const char * UI_SPRITE_UI_ACTION_SHOW_PAGE_NAME = "ui-show-page";
