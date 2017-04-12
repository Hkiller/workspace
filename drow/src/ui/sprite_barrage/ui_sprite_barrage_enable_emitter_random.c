#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/weight_selector.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_barrage_enable_emitter_random_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_barrage_i.h"

static void ui_sprite_barrage_enable_emitter_random_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);

ui_sprite_barrage_enable_emitter_random_t ui_sprite_barrage_enable_emitter_random_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_barrage_enable_emitter_random_free(ui_sprite_barrage_enable_emitter_random_t enable_emitter_random) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(enable_emitter_random);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_barrage_enable_emitter_random_select(ui_sprite_barrage_enable_emitter_random_t enable_emitter_random) {
    ui_sprite_barrage_module_t module = enable_emitter_random->m_module;
    struct cpe_weight_selector selector;
    ui_sprite_barrage_enable_emitter_random_node_t node;
    int idx;

    enable_emitter_random->m_curent_node = NULL;
    
    if (enable_emitter_random->m_repeat_count > 0 && enable_emitter_random->m_runing_repeat_count >= enable_emitter_random->m_repeat_count) return;
    if (TAILQ_EMPTY(&enable_emitter_random->m_nodes)) return;

    cpe_weight_selector_init(&selector, module->m_alloc);

    TAILQ_FOREACH(node, &enable_emitter_random->m_nodes, m_next) {
        if (cpe_weight_selector_add_item(&selector, node->m_weight) != 0) {
            CPE_ERROR(module->m_em, "%s: random select: add item fail!", ui_sprite_barrage_module_name(module));
            return;
        }
    }

    idx = cpe_weight_selector_select(&selector, NULL);

    TAILQ_FOREACH(node, &enable_emitter_random->m_nodes, m_next) {
        if (idx == 0) {
            enable_emitter_random->m_curent_node = node;
            break;
        }
        else {
            -- idx;
        }
    }

    assert(enable_emitter_random->m_curent_node);

    cpe_weight_selector_clear(&selector);
}

static int ui_sprite_barrage_enable_emitter_random_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_random_t enable_emitter_random = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    enable_emitter_random->m_curent_node = NULL;
    enable_emitter_random->m_curent_is_enable = 0;
    enable_emitter_random->m_runing_time = 0.0f;
    enable_emitter_random->m_runing_repeat_count = 0;

    ui_sprite_barrage_enable_emitter_random_select(enable_emitter_random);

    if (enable_emitter_random->m_curent_node == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter: no emitter to enable, random select fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_start_update(fsm_action) != 0) return -1;

    ui_sprite_barrage_enable_emitter_random_update(fsm_action, ctx, 0.0f);

    return ui_sprite_fsm_action_is_active(fsm_action) ? 0 : -1;
}

static void ui_sprite_barrage_enable_emitter_random_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_random_t enable_emitter_random = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage enable emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (enable_emitter_random->m_curent_is_enable) {
        assert(enable_emitter_random->m_curent_node);
        ui_sprite_barrage_obj_disable_barrages(
            barrage_obj,
            enable_emitter_random->m_curent_node->m_group_name,
            enable_emitter_random->m_curent_node->m_destory_bullets);
    }

    enable_emitter_random->m_curent_is_enable = 0;
    enable_emitter_random->m_curent_node = NULL;
    enable_emitter_random->m_runing_time = 0.0f;
    enable_emitter_random->m_runing_repeat_count = 0.0f;
}

static void ui_sprite_barrage_enable_emitter_random_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_random_t enable_emitter_random = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj = NULL;
    
    enable_emitter_random->m_runing_time += delta_s;

    while(enable_emitter_random->m_curent_node) {
        /*当前播放的弹幕还没有开始 */
        if (!enable_emitter_random->m_curent_is_enable) {
            ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
            const char * cfg_evt;
            ui_sprite_event_t evt;
            
            /*还没有开始，继续等待 */
            if (enable_emitter_random->m_runing_time < enable_emitter_random->m_curent_node->m_delay) break;

            enable_emitter_random->m_runing_time -= enable_emitter_random->m_curent_node->m_delay;

            /*直接已经过了播放周期 */
            if (enable_emitter_random->m_runing_time >= enable_emitter_random->m_curent_node->m_duration) {
                enable_emitter_random->m_runing_time -= enable_emitter_random->m_curent_node->m_duration;
                ui_sprite_barrage_enable_emitter_random_select(enable_emitter_random);
                continue;
            }

            /*开始播放 */
            if (barrage_obj == NULL) {
                barrage_obj = ui_sprite_barrage_obj_find(entity);
                if (barrage_obj == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): enable emitter random: no barrage obj!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                    ui_sprite_fsm_action_stop_update(fsm_action);
                    return;
                }
            }

    
            cfg_evt = enable_emitter_random->m_curent_node->m_collision_event;
            if (cfg_evt == NULL) {
                cfg_evt = barrage_obj->m_dft_collision_event;
                if (cfg_evt == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): enable emitter random: no event configured!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                    ui_sprite_fsm_action_stop_update(fsm_action);
                    return;
                }
            }

            evt = ui_sprite_fsm_action_build_event(fsm_action, module->m_alloc, enable_emitter_random->m_curent_node->m_collision_event, NULL);
            if (evt == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): barrage enable emitter %s: create event from %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    enable_emitter_random->m_curent_node->m_group_name,
                    enable_emitter_random->m_curent_node->m_collision_event);
                ui_sprite_fsm_action_stop_update(fsm_action);
                return;
            }

            ui_sprite_barrage_obj_enable_barrages(
                barrage_obj,
                enable_emitter_random->m_curent_node->m_group_name,
                evt,
                enable_emitter_random->m_curent_node->m_loop_count);
            enable_emitter_random->m_curent_is_enable = 1;

            mem_free(module->m_alloc, evt);
        }
        else {
            /*还没有播放完毕，继续等待 */
            if (enable_emitter_random->m_runing_time < enable_emitter_random->m_curent_node->m_duration) break;

            /*已经播放完毕，停止 */
            if (barrage_obj == NULL) {
                ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
                barrage_obj = ui_sprite_barrage_obj_find(entity);
                if (barrage_obj == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): enable emitter random: no barrage obj!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                    ui_sprite_fsm_action_stop_update(fsm_action);
                    return;
                }
            }

            ui_sprite_barrage_obj_disable_barrages(
                barrage_obj,
                enable_emitter_random->m_curent_node->m_group_name,
                enable_emitter_random->m_curent_node->m_destory_bullets);
            enable_emitter_random->m_curent_is_enable = 0;

            /*准备下一个播放 */
            enable_emitter_random->m_runing_time -= enable_emitter_random->m_curent_node->m_duration;
            ui_sprite_barrage_enable_emitter_random_select(enable_emitter_random);
        }
    }

    if (enable_emitter_random->m_curent_node == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_barrage_enable_emitter_random_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_enable_emitter_random_t enable_emitter_random = ui_sprite_fsm_action_data(fsm_action);
    enable_emitter_random->m_module = ctx;
	enable_emitter_random->m_repeat_count = 0;
    TAILQ_INIT(&enable_emitter_random->m_nodes);
    enable_emitter_random->m_curent_is_enable = 0;
    enable_emitter_random->m_curent_node = NULL;
    enable_emitter_random->m_runing_time = 0.0f;
    enable_emitter_random->m_runing_repeat_count = 0.0f;
    return 0;
}

static void ui_sprite_barrage_enable_emitter_random_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_enable_emitter_random_t enable_emitter_random = ui_sprite_fsm_action_data(fsm_action);

    assert(enable_emitter_random->m_curent_is_enable == 0);
    assert(enable_emitter_random->m_curent_node == NULL);
    assert(enable_emitter_random->m_runing_time == 0.0f);
    assert(enable_emitter_random->m_runing_repeat_count == 0.0f);

    while(!TAILQ_EMPTY(&enable_emitter_random->m_nodes)) {
        ui_sprite_barrage_enable_emitter_random_node_free(enable_emitter_random, TAILQ_FIRST(&enable_emitter_random->m_nodes));
    }
}

static int ui_sprite_barrage_enable_emitter_random_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_random_t to_enable_emitter_random = ui_sprite_fsm_action_data(to);
    ui_sprite_barrage_enable_emitter_random_t from_enable_emitter_random = ui_sprite_fsm_action_data(from);
    ui_sprite_barrage_enable_emitter_random_node_t from_node;
    
    if (ui_sprite_barrage_enable_emitter_random_init(to, ctx)) return -1;

    to_enable_emitter_random->m_repeat_count = from_enable_emitter_random->m_repeat_count;
    TAILQ_FOREACH(from_node, &from_enable_emitter_random->m_nodes, m_next) {
        ui_sprite_barrage_enable_emitter_random_node_t to_node = ui_sprite_barrage_enable_emitter_random_node_create(to_enable_emitter_random);
        if (to_node == NULL) {
            CPE_ERROR(module->m_em, "%s: copy enable_emitter_random action: create node fail!", ui_sprite_barrage_module_name(module));
        }

        to_node->m_delay = from_node->m_delay;
        to_node->m_duration = from_node->m_duration;
        to_node->m_weight = from_node->m_weight;
        cpe_str_dup(to_node->m_group_name, sizeof(to_node->m_group_name), from_node->m_group_name);
        cpe_str_dup(to_node->m_collision_event, sizeof(to_node->m_collision_event), from_node->m_collision_event);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_barrage_enable_emitter_random_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_enable_emitter_random_t enable_emitter_random = ui_sprite_barrage_enable_emitter_random_create(fsm_state, name);
    struct cfg_it emitter_cfg_it;
    cfg_t emitter_cfg;
    
    if (enable_emitter_random == NULL) {
        CPE_ERROR(module->m_em, "%s: create enable_emitter_random action: create fail!", ui_sprite_barrage_module_name(module));
        return NULL;
    }

    enable_emitter_random->m_repeat_count = cfg_get_uint8(cfg, "repeat-count", 0);

    cfg_it_init(&emitter_cfg_it, cfg_find_cfg(cfg, "emitters"));
    while((emitter_cfg = cfg_it_next(&emitter_cfg_it))) {
        ui_sprite_barrage_enable_emitter_random_node_t node = ui_sprite_barrage_enable_emitter_random_node_create(enable_emitter_random);
        const char * group_name;
        const char * collision_event;
        
        if (node == NULL) {
            CPE_ERROR(module->m_em, "%s: create enable_emitter_random action: node create fail!", ui_sprite_barrage_module_name(module));
            ui_sprite_barrage_enable_emitter_random_free(enable_emitter_random);
            return NULL;
        }

        node->m_delay = cfg_get_float(emitter_cfg, "delay", 0.0f);
        if (node->m_delay < 0.0f) {
            CPE_ERROR(module->m_em, "%s: create enable_emitter_random action: delay %f error!", ui_sprite_barrage_module_name(module), node->m_delay);
            ui_sprite_barrage_enable_emitter_random_free(enable_emitter_random);
            return NULL;
        }

        node->m_duration = cfg_get_float(emitter_cfg, "duration", 0.0f);
        if (node->m_duration <= 0.0f) {
            CPE_ERROR(module->m_em, "%s: create enable_emitter_random action: duration %f error!", ui_sprite_barrage_module_name(module), node->m_duration);
            ui_sprite_barrage_enable_emitter_random_free(enable_emitter_random);
            return NULL;
        }

        node->m_weight = cfg_get_uint32(emitter_cfg, "weight", 0);
        if (node->m_weight <= 0) {
            CPE_ERROR(module->m_em, "%s: create enable_emitter_random action: weight %d error!", ui_sprite_barrage_module_name(module), node->m_weight);
            ui_sprite_barrage_enable_emitter_random_free(enable_emitter_random);
            return NULL;
        }

        group_name = cfg_get_string(emitter_cfg, "group-name", NULL);
        if (group_name == NULL) {
            CPE_ERROR(module->m_em, "%s: create enable_emitter_random action: group name not configured!", ui_sprite_barrage_module_name(module));
            ui_sprite_barrage_enable_emitter_random_free(enable_emitter_random);
            return NULL;
        }
        cpe_str_dup(node->m_group_name, sizeof(node->m_group_name), group_name);

        if ((collision_event = cfg_get_string(emitter_cfg, "collision-event", NULL))) {
            cpe_str_dup_trim(node->m_collision_event, sizeof(node->m_collision_event), collision_event);
        }

        node->m_loop_count = cfg_get_uint8(emitter_cfg, "loop-count", 1);
        node->m_destory_bullets = cfg_get_uint8(emitter_cfg, "destory-bullets", 0);
    }
    
    return ui_sprite_fsm_action_from_data(enable_emitter_random);
}

int ui_sprite_barrage_enable_emitter_random_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME, sizeof(struct ui_sprite_barrage_enable_emitter_random));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: barrage enable emitter register: meta create fail",
            ui_sprite_barrage_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_barrage_enable_emitter_random_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_barrage_enable_emitter_random_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_barrage_enable_emitter_random_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_barrage_enable_emitter_random_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_barrage_enable_emitter_random_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_barrage_enable_emitter_random_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME, ui_sprite_barrage_enable_emitter_random_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_enable_emitter_random_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_barrage_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME);
}

const char * UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_NAME = "enable-barrage-emitter-random";

ui_sprite_barrage_enable_emitter_random_node_t ui_sprite_barrage_enable_emitter_random_node_create(ui_sprite_barrage_enable_emitter_random_t random) {
    ui_sprite_barrage_module_t module = random->m_module;
    ui_sprite_barrage_enable_emitter_random_node_t node;

    node = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_barrage_enable_emitter_random_node));
    if (node == NULL) {
        CPE_ERROR(module->m_em, "%s: alloc  ui_sprite_barrage_enable_emitter_random_node fail!", ui_sprite_barrage_module_name(module));
        return NULL; 
    }

    bzero(node, sizeof(*node));
    TAILQ_INSERT_TAIL(&random->m_nodes, node, m_next);
    return node;
}

void ui_sprite_barrage_enable_emitter_random_node_free(ui_sprite_barrage_enable_emitter_random_t random, ui_sprite_barrage_enable_emitter_random_node_t node) {
    ui_sprite_barrage_module_t module = random->m_module;
    TAILQ_REMOVE(&random->m_nodes, node, m_next);
    mem_free(module->m_alloc, node);
}
