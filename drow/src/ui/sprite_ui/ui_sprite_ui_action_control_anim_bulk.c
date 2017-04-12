#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui_sprite_ui_action_control_anim_bulk_i.h"
#include "ui_sprite_ui_action_control_anim_bulk_record_i.h"
#include "ui_sprite_ui_env_i.h"

static void ui_sprite_ui_action_control_anim_bulk_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);

ui_sprite_ui_action_control_anim_bulk_t ui_sprite_ui_action_control_anim_bulk_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_control_anim_bulk_free(ui_sprite_ui_action_control_anim_bulk_t action_control_anim_bulk) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_control_anim_bulk);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_ui_action_control_anim_bulk_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ui_action_control_anim_bulk_record_t record;

    /*全局设置 */
    TAILQ_FOREACH(record, &control_anim_bulk->m_records, m_next_for_bulk) {
        if (ui_sprite_ui_action_control_anim_bulk_record_setup(module, entity, fsm_action, record) != 0) goto ENTER_FAIL;
    }
    
    TAILQ_FOREACH(record, &control_anim_bulk->m_records, m_next_for_bulk) {
        if (record->m_parent == NULL) {
            ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_runing);
        }
        else {
            ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_waiting);
        }
    }

    ui_sprite_fsm_action_start_update(fsm_action);

    ui_sprite_ui_action_control_anim_bulk_update(fsm_action, ctx, 0.0f);
    
    return 0;

ENTER_FAIL:    
    TAILQ_FOREACH(record, &control_anim_bulk->m_records, m_next_for_bulk) {
        if (record->m_animation_id) {
            ui_sprite_ui_action_control_anim_bulk_record_exit(module, record);
        }
    }

    return -1;
}

static void ui_sprite_ui_action_control_anim_bulk_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ui_action_control_anim_bulk_record_t record;

    /*全局设置 */
    TAILQ_FOREACH(record, &control_anim_bulk->m_runing_records, m_next_for_state) {
        if (record->m_animation_id == 0) {
            record->m_waiting_duration += delta_s;
            if (record->m_waiting_duration >= record->m_cfg_delay_ms) {
                ui_sprite_ui_action_control_anim_bulk_record_enter(module, entity, fsm_action, record);
                if (record->m_animation_id == 0) {
                    ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_init);
                }
            }
        }
        else {
            plugin_ui_animation_t anim = plugin_ui_animation_find(module->m_env->m_env, record->m_animation_id);
            if (anim == NULL) {
                record->m_animation_id = 0;
                ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_init);
            }
            else {
                if (!plugin_ui_animation_have_visiable_control(anim)) {
                    plugin_ui_animation_free(anim);
                    record->m_animation_id = 0;
                    ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_init);
                }
            }
        }
    }

    TAILQ_FOREACH(record, &control_anim_bulk->m_waiting_records, m_next_for_state) {
        assert(record->m_parent);
        if (record->m_parent->m_state == ui_sprite_ui_action_control_anim_bulk_record_init) {
            ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_runing);
            if (record->m_cfg_delay_ms == 0.0f) {
                ui_sprite_ui_action_control_anim_bulk_record_enter(module, entity, fsm_action, record);
                if (record->m_animation_id == 0) {
                    ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_init);
                }
            }
        }
    }
    
    if (TAILQ_EMPTY(&control_anim_bulk->m_runing_records) && TAILQ_EMPTY(&control_anim_bulk->m_waiting_records)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_ui_action_control_anim_bulk_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ui_action_control_anim_bulk_record_t record;

    TAILQ_FOREACH(record, &control_anim_bulk->m_records, m_next_for_bulk) {
        ui_sprite_ui_action_control_anim_bulk_record_teardown(module, entity, fsm_action, record);
        ui_sprite_ui_action_control_anim_bulk_record_exit(module, record);
    }
}

static int ui_sprite_ui_action_control_anim_bulk_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = ui_sprite_fsm_action_data(fsm_action);
    control_anim_bulk->m_module = ctx;
    TAILQ_INIT(&control_anim_bulk->m_records);
    TAILQ_INIT(&control_anim_bulk->m_runing_records);
    TAILQ_INIT(&control_anim_bulk->m_waiting_records);
    return 0;
}

static void ui_sprite_ui_action_control_anim_bulk_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = ui_sprite_fsm_action_data(fsm_action);

    while(!TAILQ_EMPTY(&control_anim_bulk->m_records)) {
        ui_sprite_ui_action_control_anim_bulk_record_free(TAILQ_FIRST(&control_anim_bulk->m_records));
    }
}

static int ui_sprite_ui_action_control_anim_bulk_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	ui_sprite_ui_action_control_anim_bulk_t to_control_anim_bulk = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_control_anim_bulk_t from_control_anim_bulk = ui_sprite_fsm_action_data(from);
    ui_sprite_ui_action_control_anim_bulk_record_t from_record;
    
	if (ui_sprite_ui_action_control_anim_bulk_init(to, ctx)) return -1;

    TAILQ_FOREACH(from_record, &from_control_anim_bulk->m_records, m_next_for_bulk) {
        if (from_record->m_parent == NULL) {
            if (ui_sprite_ui_action_control_anim_bulk_record_clone(to_control_anim_bulk, NULL, from_record) == NULL) {
                ui_sprite_ui_action_control_anim_bulk_clear(to, ctx);
                return -1;
            }
        }
    }
    
    return 0;
}

static ui_sprite_ui_action_control_anim_bulk_record_t
ui_sprite_ui_action_control_anim_bulk_record_load(
    ui_sprite_ui_module_t module, ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk, ui_sprite_ui_action_control_anim_bulk_record_t parent, cfg_t cfg)
{
    ui_sprite_ui_action_control_anim_bulk_record_t record;
    const char * control = cfg_get_string(cfg, "control", NULL);
    const char * condition = cfg_get_string(cfg, "condition", NULL);
    const char * setup = cfg_get_string(cfg, "setup", NULL);
    const char * teardown = cfg_get_string(cfg, "teardown", NULL);
    const char * init = cfg_get_string(cfg, "init", NULL);
    const char * anim = cfg_get_string(cfg, "anim", NULL);
    float delay_ms;
    uint32_t loop_count;
    float loop_delay_ms;
    struct cfg_it child_it;
    cfg_t child_cfg;

    delay_ms = cfg_get_float(cfg, "delay", 0.0f);
    if (delay_ms == 0.0f) {
        delay_ms = ((float)cfg_get_uint32(cfg, "delay-frame", 0)) / plugin_ui_module_cfg_fps(module->m_ui_module);
    }

    if (cfg_try_get_uint32(cfg, "loop-count", &loop_count) == 0) {
        loop_delay_ms = cfg_get_float(cfg, "loop-delay", 0.0f);
        if (loop_delay_ms == 0.0f) {
            loop_delay_ms = ((float)cfg_get_uint32(cfg, "loop-delay-frame", 0)) / plugin_ui_module_cfg_fps(module->m_ui_module);
        }
    }
    else {
        loop_count = 1;
        loop_delay_ms = 0.0f;
    }
    
    if (control == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_bulk_record_load: control not configured!");
        return NULL;
    }

    if (anim == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_bulk_record_load: anim not configured!");
        return NULL;
    }

    record = ui_sprite_ui_action_control_anim_bulk_record_create(
        control_anim_bulk, delay_ms, loop_count, loop_delay_ms, setup, teardown, init,
        control, condition, anim, parent);
    if (record == NULL) return NULL;

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "follows"));
    while((child_cfg = cfg_it_next(&child_it))) {
        if (ui_sprite_ui_action_control_anim_bulk_record_load(module, control_anim_bulk, record, child_cfg) == NULL) {
            ui_sprite_ui_action_control_anim_bulk_record_free(record);
            return NULL;
        }
    }

    return record;
}

static ui_sprite_fsm_action_t ui_sprite_ui_action_control_anim_bulk_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = ui_sprite_ui_action_control_anim_bulk_create(fsm_state, name);
    struct cfg_it child_it;
    cfg_t child_cfg;
    
    if (control_anim_bulk == NULL) {
        CPE_ERROR(module->m_em, "%s: create action_control_anim_bulk action: create fail!", ui_sprite_ui_module_name(module));
        return NULL;
    }

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "animations"));
    while((child_cfg = cfg_it_next(&child_it))) {
        if (ui_sprite_ui_action_control_anim_bulk_record_load(module, control_anim_bulk, NULL, child_cfg) == NULL) {
            ui_sprite_ui_action_control_anim_bulk_free(control_anim_bulk);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(control_anim_bulk);
}

int ui_sprite_ui_action_control_anim_bulk_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME, sizeof(struct ui_sprite_ui_action_control_anim_bulk));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create action %s: meta create fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_control_anim_bulk_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_control_anim_bulk_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_control_anim_bulk_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_control_anim_bulk_copy, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_control_anim_bulk_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_control_anim_bulk_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME, ui_sprite_ui_action_control_anim_bulk_load, module) != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create action_control_anim_bulk action: add loader %s fail!",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME);
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }
    
    return 0;
}

void ui_sprite_ui_action_control_anim_bulk_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME);
    
    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_UI_ACTION_CONTROL_ANIM_BULK_NAME = "ui-control-anim-bulk";
