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
#include "ui_sprite_spine_schedule_state_i.h"
#include "ui_sprite_spine_utils_i.h"

static void ui_sprite_spine_schedule_state_exit(ui_sprite_fsm_action_t fsm_action, void * ctx);
static void ui_sprite_spine_schedule_state_on_anim_event(
    void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, struct spEvent* event);

ui_sprite_spine_schedule_state_t ui_sprite_spine_schedule_state_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_SCHEDULE_STATE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_schedule_state_free(ui_sprite_spine_schedule_state_t schedule_state) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(schedule_state);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_schedule_state_set_part(ui_sprite_spine_schedule_state_t schedule_state, const char * part) {
    ui_sprite_spine_module_t module = schedule_state->m_module;

    if (schedule_state->m_cfg_part) mem_free(module->m_alloc, schedule_state->m_part);

    if (part) {
        schedule_state->m_cfg_part = cpe_str_mem_dup_trim(module->m_alloc, part);
        if (schedule_state->m_cfg_part == NULL) return -1;
    }
    else {
        schedule_state->m_cfg_part = NULL;
    }

    return 0;
}

int ui_sprite_spine_schedule_state_set_loop_count(ui_sprite_spine_schedule_state_t schedule_state, const char * loop_count) {
    ui_sprite_spine_module_t module = schedule_state->m_module;

    if (schedule_state->m_cfg_loop_count) mem_free(module->m_alloc, schedule_state->m_cfg_loop_count);

    if (loop_count) {
        schedule_state->m_cfg_loop_count = cpe_str_mem_dup_trim(module->m_alloc, loop_count);
        if (schedule_state->m_cfg_loop_count == NULL) return -1;
    }
    else {
        schedule_state->m_cfg_loop_count = NULL;
    }

    return 0;
}

int ui_sprite_spine_schedule_state_add_node(
    ui_sprite_spine_schedule_state_t schedule_state, const char * state, const char * loop_count)
{
    ui_sprite_spine_module_t module = schedule_state->m_module;
    ui_sprite_spine_schedule_state_node_t node;

    node = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_spine_schedule_state_node));
    if (node == NULL) return -1;

    node->m_cfg_state = cpe_str_mem_dup_trim(module->m_alloc, state);
    if (node->m_cfg_state == NULL) {
        mem_free(module->m_alloc, node);
        return -1;
    }

    if (loop_count) {
        node->m_cfg_loop_count = cpe_str_mem_dup_trim(module->m_alloc, loop_count);
        if (node->m_cfg_loop_count == NULL) {
            mem_free(module->m_alloc, node->m_cfg_state);
            mem_free(module->m_alloc, node);
            return -1;
        }
    }
    else {
        node->m_cfg_loop_count = NULL;
    }
    
    node->m_state = NULL;
    node->m_loop_count = 0;
    node->m_runing_count = 0;
    node->m_started = 0;
    TAILQ_INSERT_TAIL(&schedule_state->m_nodes, node, m_next);
    
    return 0;
}

static int ui_sprite_spine_schedule_state_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_schedule_state_t schedule_state = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_spine_obj_part_t obj_part;
    ui_sprite_spine_schedule_state_node_t node;
    
    if (TAILQ_EMPTY(&schedule_state->m_nodes)) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: no any node!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (schedule_state->m_cfg_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: part not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    schedule_state->m_part =
        ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, schedule_state->m_cfg_part, fsm_action, NULL, module->m_em);
    if (schedule_state->m_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: calc part %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), schedule_state->m_cfg_part);
        goto ENTER_FAIL;
    }

    obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, schedule_state->m_part, module->m_em);
    if (obj_part == NULL) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: part %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), schedule_state->m_part);
        }
        return 0;
    }

    schedule_state->m_loop_count = 0;
    if (schedule_state->m_cfg_loop_count) {
        if (ui_sprite_fsm_action_check_calc_uint16(
                &schedule_state->m_loop_count, schedule_state->m_cfg_loop_count, fsm_action, NULL, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: calc loop-count %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), schedule_state->m_cfg_loop_count);
            goto ENTER_FAIL;
        }
    }

    TAILQ_FOREACH(node, &schedule_state->m_nodes, m_next) {
        assert(node->m_state == NULL);

        node->m_state = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, node->m_cfg_state, fsm_action, NULL, module->m_em);
        if (node->m_state == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: calc node state %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), node->m_cfg_state);
            goto ENTER_FAIL;
        }

        node->m_loop_count = 1;
        if (node->m_cfg_loop_count) {
            if (ui_sprite_fsm_action_check_calc_uint16(&node->m_loop_count, node->m_cfg_loop_count, fsm_action, NULL, module->m_em) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: calc node loop-count %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), node->m_cfg_loop_count);
                goto ENTER_FAIL;
            }
        }

        node->m_runing_count = 0;
        node->m_started = 0;
    }

    schedule_state->m_cur_node = TAILQ_FIRST(&schedule_state->m_nodes);
    assert(schedule_state->m_cur_node);
    schedule_state->m_cur_node->m_started = 0;

    if (plugin_spine_obj_part_switch_or_set_to_state_by_name(obj_part, schedule_state->m_cur_node->m_state, 0) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: switch to state %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), schedule_state->m_cur_node->m_state);
        goto ENTER_FAIL;
    }
    schedule_state->m_cur_node->m_started = 1;

    if (plugin_spine_obj_add_listener(plugin_spine_obj_part_obj(obj_part), ui_sprite_spine_schedule_state_on_anim_event, schedule_state) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: calc node loop-count %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), node->m_cfg_loop_count);
        goto ENTER_FAIL;
    }

    
    ui_sprite_fsm_action_start_update(fsm_action);
    
    return 0;

ENTER_FAIL:
    ui_sprite_spine_schedule_state_exit(fsm_action, ctx);
    return -1;
}

static void ui_sprite_spine_schedule_state_on_anim_event(
    void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, struct spEvent* event)
{
    ui_sprite_spine_schedule_state_t schedule_state = ctx;
    ui_sprite_spine_module_t module = schedule_state->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(schedule_state));
    plugin_spine_obj_part_t obj_part;

    if (type != plugin_spine_anim_event_loop) return;
    if (schedule_state->m_cur_node == NULL) return;
    
    obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, schedule_state->m_part, module->m_em);
    if (obj_part == NULL) return;

    if (plugin_spine_obj_part_state_anim(obj_part) != anim) return;
    if (plugin_spine_obj_part_track(obj_part) != plugin_spine_obj_anim_track(anim)) return;

    schedule_state->m_cur_node->m_runing_count++;
    if (schedule_state->m_cur_node->m_runing_count >= schedule_state->m_cur_node->m_loop_count) {
        schedule_state->m_cur_node = TAILQ_NEXT(schedule_state->m_cur_node, m_next);
    }
}

static void ui_sprite_spine_schedule_state_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_module_t module =  ctx;
    ui_sprite_spine_schedule_state_t schedule_state = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_spine_obj_part_t obj_part;

    /*对象不存在了，自动停止 */
    obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, schedule_state->m_part, module->m_em);
    if (obj_part == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    /*一次整体循环完成，停止或者启动下一次循环 */
    if (schedule_state->m_cur_node == NULL) {
        ui_sprite_spine_schedule_state_node_t node;

        schedule_state->m_runing_count++;
        if (schedule_state->m_loop_count > 0 && schedule_state->m_runing_count >= schedule_state->m_loop_count) {
            ui_sprite_fsm_action_stop_update(fsm_action);
            return;
        }

        /*重置所有节点 */
        TAILQ_FOREACH(node, &schedule_state->m_nodes, m_next) {
            node->m_runing_count = 0;
            node->m_started = 0;
        }

        schedule_state->m_cur_node = TAILQ_FIRST(&schedule_state->m_nodes);
    }
        
    /*启动一个节点 */
    if (!schedule_state->m_cur_node->m_started) {
        if (plugin_spine_obj_part_switch_or_set_to_state_by_name(obj_part, schedule_state->m_cur_node->m_state, 1) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): ui_sprite_spine_schedule_state: switch to state %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), schedule_state->m_cur_node->m_state);
            
        }
        schedule_state->m_cur_node->m_started = 1;
    }
}

static void ui_sprite_spine_schedule_state_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_schedule_state_t schedule_state = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_spine_schedule_state_node_t node;

    if (schedule_state->m_part) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
        plugin_spine_obj_part_t obj_part;
        
        obj_part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, schedule_state->m_part, module->m_em);
        if (obj_part) {
            plugin_spine_obj_remove_listener(plugin_spine_obj_part_obj(obj_part), schedule_state);
        }
        
        mem_free(module->m_alloc, schedule_state->m_part);
        schedule_state->m_part = NULL;
    }

    schedule_state->m_runing_count = 0;
    schedule_state->m_loop_count = 0;

    TAILQ_FOREACH(node, &schedule_state->m_nodes, m_next) {
        if (node->m_state) {
            mem_free(module->m_alloc, node->m_state);
            node->m_state = NULL;
        }

        node->m_loop_count = 0;
        node->m_runing_count = 0;
    }

    schedule_state->m_cur_node = NULL;
}

static int ui_sprite_spine_schedule_state_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_schedule_state_t schedule_state = ui_sprite_fsm_action_data(fsm_action);
    schedule_state->m_module = ctx;
    schedule_state->m_cfg_part = NULL;
    schedule_state->m_cfg_loop_count = NULL;
    TAILQ_INIT(&schedule_state->m_nodes);

    schedule_state->m_part = NULL;
    schedule_state->m_loop_count = 0;
    schedule_state->m_runing_count = 0;
    schedule_state->m_cur_node = NULL;
    
    return 0;
}

static void ui_sprite_spine_schedule_state_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_schedule_state_t schedule_state = ui_sprite_fsm_action_data(fsm_action);

    assert(schedule_state->m_part == NULL);
    assert(schedule_state->m_loop_count == 0);

    if (schedule_state->m_cfg_part) {
        mem_free(module->m_alloc, schedule_state->m_cfg_part);
        schedule_state->m_cfg_part = NULL;
    }

    if (schedule_state->m_cfg_loop_count) {
        mem_free(module->m_alloc, schedule_state->m_cfg_loop_count);
        schedule_state->m_cfg_loop_count = NULL;
    }

    while(!TAILQ_EMPTY(&schedule_state->m_nodes)) {
        ui_sprite_spine_schedule_state_node_t node = TAILQ_FIRST(&schedule_state->m_nodes);

        assert(node->m_state == NULL);

        if (node->m_cfg_state) {
            mem_free(module->m_alloc, node->m_cfg_state);
            node->m_cfg_state = NULL;
        }

        if (node->m_cfg_loop_count) {
            mem_free(module->m_alloc, node->m_cfg_loop_count);
            node->m_cfg_loop_count = NULL;
        }
        
        TAILQ_REMOVE(&schedule_state->m_nodes, node, m_next);
        mem_free(module->m_alloc, node);
    }
}

static int ui_sprite_spine_schedule_state_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_schedule_state_t to_schedule_state = ui_sprite_fsm_action_data(to);
    ui_sprite_spine_schedule_state_t from_schedule_state = ui_sprite_fsm_action_data(from);
    ui_sprite_spine_schedule_state_node_t from_node;

    if (ui_sprite_spine_schedule_state_init(to, ctx)) return -1;

    if (from_schedule_state->m_cfg_part) {
        to_schedule_state->m_cfg_part = cpe_str_mem_dup(module->m_alloc, from_schedule_state->m_cfg_part);
        if (to_schedule_state->m_cfg_part == NULL) {
            ui_sprite_spine_schedule_state_clear(to, ctx);
            return -1;
        }
    }

    if (from_schedule_state->m_cfg_loop_count) {
        to_schedule_state->m_cfg_loop_count = cpe_str_mem_dup(module->m_alloc, from_schedule_state->m_cfg_loop_count);
        if (to_schedule_state->m_cfg_loop_count == NULL) {
            ui_sprite_spine_schedule_state_clear(to, ctx);
            return -1;
        }            
    }

    TAILQ_FOREACH(from_node, &from_schedule_state->m_nodes, m_next) {
        if (ui_sprite_spine_schedule_state_add_node(
                to_schedule_state, from_node->m_cfg_state, from_node->m_cfg_loop_count) != 0)
        {
            ui_sprite_spine_schedule_state_clear(to, ctx);
            return -1;
        }
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_spine_schedule_state_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_schedule_state_t spine_schedule_state = ui_sprite_spine_schedule_state_create(fsm_state, name);
    struct cfg_it child_it;
    cfg_t child;
    const char * str_value;

    if (spine_schedule_state == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine_schedule_state action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "part", NULL))) {
        if (ui_sprite_spine_schedule_state_set_part(spine_schedule_state, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_schedule_state action: set part %s fail",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_schedule_state_free(spine_schedule_state);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine_schedule_state action: part not configured",
            ui_sprite_spine_module_name(module));
        ui_sprite_spine_schedule_state_free(spine_schedule_state);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "loop-count", NULL))) {
        if (ui_sprite_spine_schedule_state_set_loop_count(spine_schedule_state, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_schedule_state action: set loop-count %s fail",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_schedule_state_free(spine_schedule_state);
            return NULL;
        }
    }

    cfg_it_init(&child_it, cfg_find_cfg(cfg, "nodes"));
    while((child = cfg_it_next(&child_it))) {
        const char * state = cfg_get_string(child, "state", NULL);
        const char * loop_count = cfg_get_string(child, "loop-count", NULL);

        if (state == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create spine_schedule_state action: node.state not configured",
                ui_sprite_spine_module_name(module));
            ui_sprite_spine_schedule_state_free(spine_schedule_state);
            return NULL;
        }

        if (ui_sprite_spine_schedule_state_add_node(spine_schedule_state, state, loop_count) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine_schedule_state action: add node fail!",
                ui_sprite_spine_module_name(module));
            ui_sprite_spine_schedule_state_free(spine_schedule_state);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(spine_schedule_state);
}

int ui_sprite_spine_schedule_state_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_SCHEDULE_STATE_NAME, sizeof(struct ui_sprite_spine_schedule_state));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_schedule_state_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_schedule_state_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_schedule_state_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_schedule_state_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_schedule_state_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_schedule_state_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_SCHEDULE_STATE_NAME, ui_sprite_spine_schedule_state_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_schedule_state_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_SCHEDULE_STATE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_SCHEDULE_STATE_NAME = "spine-schedule-state";

