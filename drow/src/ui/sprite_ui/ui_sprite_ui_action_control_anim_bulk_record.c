#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_control.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui_sprite_ui_action_control_anim_bulk_record_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_control_anim_bulk_record_t
ui_sprite_ui_action_control_anim_bulk_record_create(
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk,
    float delay_ms, uint32_t loop_count, float loop_delay_ms,
    const char * setup, const char * teardown, const char * init,
    const char * control, const char * condition, const char * anim,
    ui_sprite_ui_action_control_anim_bulk_record_t parent)
{
    ui_sprite_ui_module_t module = control_anim_bulk->m_module;
    ui_sprite_ui_action_control_anim_bulk_record_t record;

    record = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_ui_action_control_anim_bulk_record));
    if (record == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_bulk_record_create: alloc record fail!");
        return NULL;
    }

    record->m_control_anim_bulk = control_anim_bulk;
    record->m_parent = parent;
    record->m_cfg_delay_ms = delay_ms;
    record->m_cfg_loop_count = loop_count;
    record->m_cfg_loop_delay_ms = loop_delay_ms;
    record->m_animation_id = 0;
    record->m_state = ui_sprite_ui_action_control_anim_bulk_record_init;
    record->m_waiting_duration = 0.0f;
    TAILQ_INIT(&record->m_follows);

    if (condition) {
        record->m_cfg_condition = cpe_str_mem_dup_trim(module->m_alloc, condition);
        if (record->m_cfg_condition == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_condition_anim_bulk_record_create: dup condition %s fail!", condition);
            mem_free(module->m_alloc, record);
            return NULL;
        }
    }
    else {
        record->m_cfg_condition = NULL;
    }

    if (setup) {
        record->m_cfg_setup = cpe_str_mem_dup_trim(module->m_alloc, setup);
        if (record->m_cfg_setup == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_setup_anim_bulk_record_create: dup setup %s fail!", setup);
            mem_free(module->m_alloc, record->m_cfg_condition);
            mem_free(module->m_alloc, record);
            return NULL;
        }
    }
    else {
        record->m_cfg_setup = NULL;
    }

    if (teardown) {
        record->m_cfg_teardown = cpe_str_mem_dup_trim(module->m_alloc, teardown);
        if (record->m_cfg_teardown == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_teardown_anim_bulk_record_create: dup teardown %s fail!", teardown);
            mem_free(module->m_alloc, record->m_cfg_condition);
            mem_free(module->m_alloc, record);
            return NULL;
        }
    }
    else {
        record->m_cfg_teardown = NULL;
    }
    
    if (init) {
        record->m_cfg_init = cpe_str_mem_dup_trim(module->m_alloc, init);
        if (record->m_cfg_init == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_action_init_anim_bulk_record_create: dup init %s fail!", init);
            mem_free(module->m_alloc, record->m_cfg_setup);
            mem_free(module->m_alloc, record->m_cfg_condition);
            mem_free(module->m_alloc, record);
            return NULL;
        }
    }
    else {
        record->m_cfg_init = NULL;
    }
    
    record->m_cfg_control = cpe_str_mem_dup_trim(module->m_alloc, control);
    if (record->m_cfg_control == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_bulk_record_create: dup control %s fail!", control);
        mem_free(module->m_alloc, record->m_cfg_init);
        mem_free(module->m_alloc, record->m_cfg_setup);
        mem_free(module->m_alloc, record->m_cfg_condition);
        mem_free(module->m_alloc, record);
        return NULL;
    }

    record->m_cfg_anim = cpe_str_mem_dup_trim(module->m_alloc, anim);
    if (record->m_cfg_anim == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_action_control_anim_bulk_record_create: dup anim %s fail!", anim);
        mem_free(module->m_alloc, record->m_cfg_control);
        mem_free(module->m_alloc, record->m_cfg_init);
        mem_free(module->m_alloc, record->m_cfg_setup);
        mem_free(module->m_alloc, record->m_cfg_condition);
        mem_free(module->m_alloc, record);
        return NULL;
    }

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_follows, record, m_next_for_parent);
    }

    TAILQ_INSERT_TAIL(&control_anim_bulk->m_records, record, m_next_for_bulk);
    
    return record;
}

void ui_sprite_ui_action_control_anim_bulk_record_free(ui_sprite_ui_action_control_anim_bulk_record_t record) {
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk = record->m_control_anim_bulk;
    ui_sprite_ui_module_t module = control_anim_bulk->m_module;

    assert(record->m_animation_id == 0);

    assert(record->m_cfg_anim);
    mem_free(module->m_alloc, record->m_cfg_anim);
    record->m_cfg_anim = NULL;

    ui_sprite_ui_action_control_anim_bulk_record_set_state(module, record, ui_sprite_ui_action_control_anim_bulk_record_init);

    if(record->m_cfg_condition) {
        mem_free(module->m_alloc, record->m_cfg_condition);
        record->m_cfg_condition = NULL;
    }

    if(record->m_cfg_setup) {
        mem_free(module->m_alloc, record->m_cfg_setup);
        record->m_cfg_setup = NULL;
    }

    if(record->m_cfg_teardown) {
        mem_free(module->m_alloc, record->m_cfg_teardown);
        record->m_cfg_teardown = NULL;
    }
    
    if(record->m_cfg_init) {
        mem_free(module->m_alloc, record->m_cfg_init);
        record->m_cfg_init = NULL;
    }
    
    assert(record->m_cfg_control);
    mem_free(module->m_alloc, record->m_cfg_control);
    record->m_cfg_control = NULL;

    if (record->m_parent) {
        TAILQ_REMOVE(&record->m_parent->m_follows, record, m_next_for_parent);
    }

    TAILQ_REMOVE(&control_anim_bulk->m_records, record, m_next_for_bulk);

    mem_free(module->m_alloc, record);
}

int ui_sprite_ui_action_control_anim_bulk_record_setup(
    ui_sprite_ui_module_t module, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_control_anim_bulk_record_t record)
{
    const char * control_path;
    plugin_ui_control_t control;
    const char * setup_attrs;
    
    if (record->m_cfg_setup == NULL) return 0;
    
    control_path = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), record->m_cfg_control, fsm_action, NULL, module->m_em);
    if (control_path == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: setup: calc control name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), record->m_cfg_control);
        return -1;
    }

    control = ui_sprite_ui_find_control_from_action(module, fsm_action, control_path);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_path);
        return -1;
    }

    setup_attrs = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), record->m_cfg_setup, fsm_action, NULL, module->m_em);
    if (setup_attrs == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s calc setup from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            plugin_ui_control_name(control), record->m_cfg_setup);
        return -1;
    }
    
    if (plugin_ui_control_bulk_set_attrs(control, setup_attrs) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s setup from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            plugin_ui_control_name(control), setup_attrs);
        return -1;
    }

    return 0;
}

int ui_sprite_ui_action_control_anim_bulk_record_teardown(
    ui_sprite_ui_module_t module, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_control_anim_bulk_record_t record)
{
    const char * control_path;
    plugin_ui_control_t control;
    const char * teardown_attrs;
    
    if (record->m_cfg_teardown == NULL) return 0;
    
    control_path = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), record->m_cfg_control, fsm_action, NULL, module->m_em);
    if (control_path == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: teardown: calc control name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), record->m_cfg_control);
        return -1;
    }

    control = ui_sprite_ui_find_control_from_action(module, fsm_action, control_path);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_path);
        return -1;
    }

    teardown_attrs = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), record->m_cfg_teardown, fsm_action, NULL, module->m_em);
    if (teardown_attrs == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s calc teardown from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            plugin_ui_control_name(control), record->m_cfg_teardown);
        return -1;
    }
    
    if (plugin_ui_control_bulk_set_attrs(control, teardown_attrs) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s teardown from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            plugin_ui_control_name(control), teardown_attrs);
        return -1;
    }

    return 0;
}

int ui_sprite_ui_action_control_anim_bulk_record_enter(
    ui_sprite_ui_module_t module, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action,
    ui_sprite_ui_action_control_anim_bulk_record_t record)
{
    const char * control_path;
    plugin_ui_control_t control;
    char * str_anim;
    plugin_ui_animation_t animation;
    uint8_t need_process = 1;
    
    assert(record->m_animation_id == 0);
    
    control_path = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), record->m_cfg_control, fsm_action, NULL, module->m_em);
    if (control_path == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: enter: calc control name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), record->m_cfg_control);
        return -1;
    }

    control = ui_sprite_ui_find_control_from_action(module, fsm_action, control_path);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: control %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_path);
        return -1;
    }

    if (record->m_cfg_init) {
        const char * init_attrs = ui_sprite_fsm_action_check_calc_str(
            gd_app_tmp_buffer(module->m_app), record->m_cfg_init, fsm_action, NULL, module->m_em);
        if (plugin_ui_control_bulk_set_attrs(control, init_attrs) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim-control-bulk: record: control %s init from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                plugin_ui_control_name(control), init_attrs);
            return -1;
        }
    }

    if (record->m_cfg_condition) {
        dr_data_source_t data_source = NULL;
        struct dr_data_source ds_buf;
        plugin_ui_page_t page = plugin_ui_control_page(control);

        if (plugin_ui_page_data(page)) {
            ds_buf.m_data.m_data = plugin_ui_page_data(page);
            ds_buf.m_data.m_meta = plugin_ui_page_data_meta(page);
            ds_buf.m_data.m_size = plugin_ui_page_data_size(page);
            ds_buf.m_next = NULL;
            data_source = &ds_buf;
        }
        
        if (ui_sprite_fsm_action_check_calc_bool(&need_process, record->m_cfg_condition, fsm_action, &ds_buf, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim-control-bulk: record: control %s calc condition from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                plugin_ui_control_name(control), record->m_cfg_condition);
            return -1;
        }
    }

    if (!need_process) return 0;
    
    str_anim = cpe_str_mem_dup(module->m_alloc, record->m_cfg_anim);
    if (str_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: anim calc from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), record->m_cfg_anim);
        return -1;
    }

    animation = plugin_ui_control_create_animation(control, str_anim);
    if (animation == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: record: anim %s create fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_anim);
        mem_free(module->m_alloc, str_anim);
        return -1;
    }
    mem_free(module->m_alloc, str_anim);

    if (record->m_cfg_loop_count != 1) {
        if (plugin_ui_animation_set_loop(animation, record->m_cfg_loop_count, record->m_cfg_loop_delay_ms) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim-control-bulk: set loop fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            plugin_ui_animation_free(animation);
            return -1;
        }
    }

    record->m_animation_id = plugin_ui_animation_id(animation);
    
    /*如果没有延时和父动画，则直接启动，不用等到后续自动启动 */
    if (plugin_ui_animation_start(animation) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control-bulk: anim %s start fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_anim);
        plugin_ui_animation_free(animation);
        assert(record->m_animation_id == 0);
        return -1;
    }

    return 0;
}

void ui_sprite_ui_action_control_anim_bulk_record_exit(ui_sprite_ui_module_t module, ui_sprite_ui_action_control_anim_bulk_record_t record) {
    if (record->m_animation_id) {
        plugin_ui_animation_t animation = plugin_ui_animation_find(module->m_env->m_env, record->m_animation_id);
        if (animation) {
            plugin_ui_animation_free(animation);
        }
        record->m_animation_id = 0;
    }
}
    
ui_sprite_ui_action_control_anim_bulk_record_t
ui_sprite_ui_action_control_anim_bulk_record_clone(
    ui_sprite_ui_action_control_anim_bulk_t control_anim_bulk,
    ui_sprite_ui_action_control_anim_bulk_record_t parent, ui_sprite_ui_action_control_anim_bulk_record_t from)
{
    ui_sprite_ui_action_control_anim_bulk_record_t record;
    ui_sprite_ui_action_control_anim_bulk_record_t follow;
    
    record = ui_sprite_ui_action_control_anim_bulk_record_create(
        control_anim_bulk, from->m_cfg_delay_ms, from->m_cfg_loop_count, from->m_cfg_loop_delay_ms,
        from->m_cfg_setup, from->m_cfg_teardown, from->m_cfg_init,
        from->m_cfg_control, from->m_cfg_condition,
        from->m_cfg_anim, parent);
    if (record == NULL) return NULL;

    TAILQ_FOREACH(follow, &from->m_follows, m_next_for_parent) {
        if (ui_sprite_ui_action_control_anim_bulk_record_clone(control_anim_bulk, record, follow) == NULL) {
            ui_sprite_ui_action_control_anim_bulk_record_free_tree(record);
            return NULL;
        }
    }

    return record;
}

void ui_sprite_ui_action_control_anim_bulk_record_free_tree(ui_sprite_ui_action_control_anim_bulk_record_t record) {
    while(!TAILQ_EMPTY(&record->m_follows)) {
        ui_sprite_ui_action_control_anim_bulk_record_free_tree(TAILQ_FIRST(&record->m_follows));
    }
}

void ui_sprite_ui_action_control_anim_bulk_record_set_state(
    ui_sprite_ui_module_t module,
    ui_sprite_ui_action_control_anim_bulk_record_t record,
    enum ui_sprite_ui_action_control_anim_bulk_record_state state)
{
    if (record->m_state == state) return;

    switch(record->m_state) {
    case ui_sprite_ui_action_control_anim_bulk_record_init:
        break;
    case ui_sprite_ui_action_control_anim_bulk_record_runing:
        TAILQ_REMOVE(&record->m_control_anim_bulk->m_runing_records, record, m_next_for_state);
        break;
    case ui_sprite_ui_action_control_anim_bulk_record_waiting:
        TAILQ_REMOVE(&record->m_control_anim_bulk->m_waiting_records, record, m_next_for_state);
        break;
    }

    record->m_state = state;
    
    switch(record->m_state) {
    case ui_sprite_ui_action_control_anim_bulk_record_init:
        break;
    case ui_sprite_ui_action_control_anim_bulk_record_runing:
            record->m_waiting_duration = 0.0f;
        TAILQ_INSERT_TAIL(&record->m_control_anim_bulk->m_runing_records, record, m_next_for_state);
        break;
    case ui_sprite_ui_action_control_anim_bulk_record_waiting:
        TAILQ_INSERT_TAIL(&record->m_control_anim_bulk->m_waiting_records, record, m_next_for_state);
        break;
    }
}
