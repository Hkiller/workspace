#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_barrage_enable_seq_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_barrage_i.h"

static void ui_sprite_barrage_enable_seq_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);

ui_sprite_barrage_enable_seq_t ui_sprite_barrage_enable_seq_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_barrage_enable_seq_free(ui_sprite_barrage_enable_seq_t enable_seq) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(enable_seq);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_barrage_enable_seq_next(ui_sprite_barrage_enable_seq_t enable_seq) {
    assert(enable_seq->m_curent_node);

    assert(!enable_seq->m_curent_is_enable);
    assert(enable_seq->m_curent_group_name == NULL);
    
    enable_seq->m_curent_node = TAILQ_NEXT(enable_seq->m_curent_node, m_next);
    if (enable_seq->m_curent_node == NULL) {
        enable_seq->m_runing_loop_count++;
        if (enable_seq->m_loop_count == 0
            || enable_seq->m_loop_count < enable_seq->m_runing_loop_count)
        {
            enable_seq->m_curent_node = TAILQ_FIRST(&enable_seq->m_cfg_nodes);
        }
    }
}


static int ui_sprite_barrage_enable_seq_start_curent(
    ui_sprite_fsm_action_t fsm_action, ui_sprite_entity_t entity, ui_sprite_barrage_obj_t barrage_obj)
{
    ui_sprite_barrage_enable_seq_t enable_seq = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_barrage_module_t module = enable_seq->m_module;
    uint16_t loop_count = 1;
    
    assert(enable_seq->m_curent_group_name == NULL);
    assert(!enable_seq->m_curent_is_enable);

    if (enable_seq->m_curent_node->m_cfg_loop_count) {
        if (ui_sprite_fsm_action_check_calc_uint16(&loop_count, enable_seq->m_curent_node->m_cfg_loop_count, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): start current: calc loop count from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), enable_seq->m_curent_node->m_cfg_loop_count);
            return -1;
        }
    }
    
    if (enable_seq->m_curent_node->m_cfg_group_name) {
        enable_seq->m_curent_group_name =
            ui_sprite_fsm_action_check_calc_str_dup(
                module->m_alloc, enable_seq->m_curent_node->m_cfg_group_name, fsm_action, NULL, module->m_em);
        if (enable_seq->m_curent_group_name == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): start current: calc group name from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), enable_seq->m_curent_node->m_cfg_group_name);
            return -1;
        }
    }
    else {
        enable_seq->m_curent_group_name = NULL;
    }
    
    ui_sprite_barrage_obj_enable_barrages(
        barrage_obj,
        enable_seq->m_curent_group_name,
        enable_seq->m_evt,
        loop_count);
        
    enable_seq->m_curent_is_enable = 1;

    return 0;
}

static int ui_sprite_barrage_enable_seq_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_seq_t enable_seq = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;
    const char * cfg_evt;
    
    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter seq: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    cfg_evt = enable_seq->m_cfg_collision_event;
    if (cfg_evt == NULL) {
        cfg_evt = barrage_obj->m_dft_collision_event;
        if (cfg_evt == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage enable emitter seq: no event configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }
    
    if (enable_seq->m_cfg_loop_count) {
        if (ui_sprite_fsm_action_check_calc_uint16(&enable_seq->m_loop_count, enable_seq->m_cfg_loop_count, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage enable: calc loop count from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), enable_seq->m_cfg_loop_count);
            return -1;
        }
    }
    else {
        enable_seq->m_loop_count = 0;
    }
    
    enable_seq->m_evt = ui_sprite_fsm_action_build_event(fsm_action, module->m_alloc, cfg_evt, NULL);
    if (enable_seq->m_evt == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): create event from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), cfg_evt);
        return -1;
    }

    enable_seq->m_curent_is_enable = 0;
    enable_seq->m_runing_loop_count = 0;
    
    ui_sprite_fsm_action_start_update(fsm_action);

    enable_seq->m_curent_node = TAILQ_FIRST(&enable_seq->m_cfg_nodes);
    ui_sprite_barrage_enable_seq_update(fsm_action, ctx, 0.0f);
    return 0;
}

static void ui_sprite_barrage_enable_seq_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_seq_t enable_seq = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    if (enable_seq->m_evt) {
        mem_free(module->m_alloc, enable_seq->m_evt);
        enable_seq->m_evt = NULL;
    }

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (enable_seq->m_curent_is_enable) {
        assert(enable_seq->m_curent_node);
        ui_sprite_barrage_obj_disable_barrages(
            barrage_obj,
            enable_seq->m_curent_group_name,
            0);
        enable_seq->m_curent_is_enable = 0;
    }

    enable_seq->m_curent_node = NULL;

    if (enable_seq->m_curent_group_name) {
        mem_free(module->m_alloc, enable_seq->m_curent_group_name);
        enable_seq->m_curent_group_name = NULL;
    }
}

static void ui_sprite_barrage_enable_seq_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_seq_t enable_seq = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj = NULL;
    ui_sprite_barrage_enable_seq_node_t loop_begin_node;
    uint16_t looped_count;
    
    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): enable emitter seq: no barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    loop_begin_node = enable_seq->m_curent_node;
    looped_count = 0;
    while(enable_seq->m_curent_node) {
        /*当前播放的弹幕还没有开始 */
        if (!enable_seq->m_curent_is_enable) {
            /*开始播放 */
            if (ui_sprite_barrage_enable_seq_start_curent(fsm_action, entity, barrage_obj) != 0) {
                /*下一个 */
                ui_sprite_barrage_enable_seq_next(enable_seq);
            }
        }
        else {
            /*还没有播放完毕，继续等待 */
            if (ui_sprite_barrage_obj_is_barrages_enable(barrage_obj, enable_seq->m_curent_group_name)) break;

            if (enable_seq->m_curent_group_name) {
                mem_free(enable_seq->m_module->m_alloc, enable_seq->m_curent_group_name);
                enable_seq->m_curent_group_name = NULL;
            }
            enable_seq->m_curent_is_enable = 0;
            
            /*下一个 */
            ui_sprite_barrage_enable_seq_next(enable_seq);
        }

        if (loop_begin_node == enable_seq->m_curent_node) {
            looped_count++;
            if (looped_count > 1) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): enable emitter seq: loopned one tick !",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                ui_sprite_fsm_action_stop_update(fsm_action);
                return;
            }
        }
    }

    if (enable_seq->m_curent_node == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_barrage_enable_seq_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_enable_seq_t enable_seq = ui_sprite_fsm_action_data(fsm_action);
    enable_seq->m_module = ctx;
    enable_seq->m_cfg_collision_event = NULL;
    enable_seq->m_cfg_loop_count = NULL;
    TAILQ_INIT(&enable_seq->m_cfg_nodes);
    enable_seq->m_evt = NULL;
    enable_seq->m_loop_count = 0;
    enable_seq->m_runing_loop_count = 0;
    enable_seq->m_curent_node = NULL;
    enable_seq->m_curent_group_name = NULL;
    enable_seq->m_curent_is_enable = 0;
    return 0;
}

static void ui_sprite_barrage_enable_seq_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_enable_seq_t enable_seq = ui_sprite_fsm_action_data(fsm_action);

    assert(enable_seq->m_evt == NULL);
    assert(enable_seq->m_curent_node == NULL);

    if (enable_seq->m_cfg_collision_event) {
        mem_free(enable_seq->m_module->m_alloc, enable_seq->m_cfg_collision_event);
        enable_seq->m_cfg_collision_event = NULL;
    }
    
    if (enable_seq->m_cfg_loop_count) {
        mem_free(enable_seq->m_module->m_alloc, enable_seq->m_cfg_loop_count);
        enable_seq->m_cfg_loop_count = NULL;
    }

    while(!TAILQ_EMPTY(&enable_seq->m_cfg_nodes)) {
        ui_sprite_barrage_enable_seq_node_free(enable_seq, TAILQ_FIRST(&enable_seq->m_cfg_nodes));
    }
}

static int ui_sprite_barrage_enable_seq_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_seq_t to_enable_seq = ui_sprite_fsm_action_data(to);
    ui_sprite_barrage_enable_seq_t from_enable_seq = ui_sprite_fsm_action_data(from);
    ui_sprite_barrage_enable_seq_node_t from_node;
    
    if (ui_sprite_barrage_enable_seq_init(to, ctx)) return -1;

    to_enable_seq->m_cfg_collision_event =
        from_enable_seq->m_cfg_collision_event
        ? cpe_str_mem_dup(module->m_alloc, from_enable_seq->m_cfg_collision_event)
        : NULL;
    
    to_enable_seq->m_cfg_loop_count =
        from_enable_seq->m_cfg_loop_count
        ? cpe_str_mem_dup(module->m_alloc, from_enable_seq->m_cfg_loop_count)
        : NULL;
    
    to_enable_seq->m_loop_count = from_enable_seq->m_loop_count;
    TAILQ_FOREACH(from_node, &from_enable_seq->m_cfg_nodes, m_next) {
        ui_sprite_barrage_enable_seq_node_t to_node = ui_sprite_barrage_enable_seq_node_create(to_enable_seq);
        if (to_node == NULL) {
            CPE_ERROR(module->m_em, "%s: copy enable_seq action: create node fail!", ui_sprite_barrage_module_name(module));
        }

        to_node->m_cfg_group_name = from_node->m_cfg_group_name ? cpe_str_mem_dup(module->m_alloc, from_node->m_cfg_group_name) : NULL;
        to_node->m_cfg_loop_count = from_node->m_cfg_loop_count ? cpe_str_mem_dup(module->m_alloc, from_node->m_cfg_loop_count) : NULL;
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_barrage_enable_seq_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_seq_t enable_seq;
    struct cfg_it emitter_cfg_it;
    cfg_t emitter_cfg;
    const char * str_value;

    enable_seq = ui_sprite_barrage_enable_seq_create(fsm_state, name);
    if (enable_seq == NULL) {
        CPE_ERROR(module->m_em, "%s: create enable_seq action: create fail!", ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "loop-count", NULL))) {
        enable_seq->m_cfg_loop_count = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "collision-event", NULL))) {
        enable_seq->m_cfg_collision_event = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    }
    
    cfg_it_init(&emitter_cfg_it, cfg_find_cfg(cfg, "emitters"));
    while((emitter_cfg = cfg_it_next(&emitter_cfg_it))) {
        ui_sprite_barrage_enable_seq_node_t node;

        node = ui_sprite_barrage_enable_seq_node_create(enable_seq);
        if (node == NULL) {
            CPE_ERROR(module->m_em, "%s: create enable_seq action: node create fail!", ui_sprite_barrage_module_name(module));
            ui_sprite_barrage_enable_seq_free(enable_seq);
            return NULL;
        }

        if ((str_value = cfg_get_string(emitter_cfg, "group-name", NULL))) {
            node->m_cfg_group_name = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        }

        if ((str_value = cfg_get_string(emitter_cfg, "loop-count", NULL))) {
            node->m_cfg_loop_count = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        }
    }
    
    return ui_sprite_fsm_action_from_data(enable_seq);
}

int ui_sprite_barrage_enable_seq_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME, sizeof(struct ui_sprite_barrage_enable_seq));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: barrage enable emitter register: meta create fail",
            ui_sprite_barrage_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_barrage_enable_seq_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_barrage_enable_seq_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_barrage_enable_seq_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_barrage_enable_seq_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_barrage_enable_seq_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_barrage_enable_seq_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME, ui_sprite_barrage_enable_seq_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_enable_seq_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_barrage_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME);
}

const char * UI_SPRITE_BARRAGE_ENABLE_SEQ_NAME = "barrage-enable-seq";

ui_sprite_barrage_enable_seq_node_t ui_sprite_barrage_enable_seq_node_create(ui_sprite_barrage_enable_seq_t seq) {
    ui_sprite_barrage_module_t module = seq->m_module;
    ui_sprite_barrage_enable_seq_node_t node;

    node = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_barrage_enable_seq_node));
    if (node == NULL) {
        CPE_ERROR(module->m_em, "%s: alloc  ui_sprite_barrage_enable_seq_node fail!", ui_sprite_barrage_module_name(module));
        return NULL; 
    }

    node->m_cfg_group_name = NULL;
    node->m_cfg_loop_count = NULL;
    TAILQ_INSERT_TAIL(&seq->m_cfg_nodes, node, m_next);
    return node;
}

void ui_sprite_barrage_enable_seq_node_free(ui_sprite_barrage_enable_seq_t seq, ui_sprite_barrage_enable_seq_node_t node) {
    ui_sprite_barrage_module_t module = seq->m_module;

    if (node->m_cfg_group_name) mem_free(module->m_alloc, node->m_cfg_group_name);
    if (node->m_cfg_loop_count) mem_free(module->m_alloc, node->m_cfg_loop_count);
    
    TAILQ_REMOVE(&seq->m_cfg_nodes, node, m_next);
    mem_free(module->m_alloc, node);
}
