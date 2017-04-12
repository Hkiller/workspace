#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_fsm_ins_state_i.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_ins_transition_i.h"
#include "ui_sprite_fsm_action_meta_i.h"
#include "ui_sprite_fsm_action_fsm_i.h"

static ui_sprite_fsm_state_t ui_sprite_fsm_state_create_i(ui_sprite_fsm_ins_t fsm, const char * name) {
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_state_t fsm_state;
    size_t name_len = strlen(name) + 1;

    if (ui_sprite_fsm_state_find_by_name(fsm, name) != NULL) {
        CPE_ERROR(module->m_em, "fsm create state %s: name duplicate", name);
        return NULL;
    }

    fsm_state = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_fsm_state) + name_len);
    if (fsm_state == NULL) {
        CPE_ERROR(module->m_em, "fsm create state %s: alloc fail", name);
        return NULL;
    }

    fsm_state->m_ins = fsm;
    fsm_state->m_id = fsm->m_max_state_id + 1;
    fsm_state->m_name = (const char *)(fsm_state + 1);
    fsm_state->m_return_to = NULL;
    fsm_state->m_enter_event = NULL;

    memcpy(fsm_state + 1, name, name_len);

    TAILQ_INIT(&fsm_state->m_transitions);
    TAILQ_INIT(&fsm_state->m_actions);
    TAILQ_INIT(&fsm_state->m_updating_actions);
    TAILQ_INIT(&fsm_state->m_waiting_actions);
    TAILQ_INIT(&fsm_state->m_runing_actions);
    TAILQ_INIT(&fsm_state->m_done_actions);

    TAILQ_INSERT_TAIL(&fsm->m_states, fsm_state, m_next_for_ins);

    return fsm_state;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_create(ui_sprite_fsm_ins_t fsm, const char * name) {
    ui_sprite_fsm_state_t fsm_state = ui_sprite_fsm_state_create_i(fsm, name);

    if (fsm_state) fsm->m_max_state_id++;

    return fsm_state;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_clone(ui_sprite_fsm_ins_t fsm, ui_sprite_fsm_state_t from) {
    ui_sprite_fsm_state_t to = ui_sprite_fsm_state_create_i(fsm, from->m_name);
    ui_sprite_fsm_action_t from_fsm_action;
    ui_sprite_fsm_transition_t from_fsm_transition;

    if (to == NULL) return NULL;


    fsm->m_max_state_id++;

    TAILQ_FOREACH(from_fsm_action, &from->m_actions, m_next_for_state) {
        ui_sprite_fsm_action_t to_fsm_action = ui_sprite_fsm_action_clone(to, from_fsm_action);
        if (to_fsm_action == NULL) {
            ui_sprite_fsm_state_free(to);
            return NULL;
        }
    }

    TAILQ_FOREACH(from_fsm_transition, &from->m_transitions, m_next_for_state) {
        ui_sprite_fsm_transition_t to_fsm_transition = ui_sprite_fsm_transition_clone(to, from_fsm_transition);
        if (to_fsm_transition == NULL) {
            ui_sprite_fsm_state_free(to);
            return NULL;
        }
    }

    return to;
}

void ui_sprite_fsm_state_free(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;

    if (fsm->m_cur_state == fsm_state) {
        ui_sprite_fsm_state_exit(fsm_state);
        assert(fsm->m_cur_state == NULL);
    }

    assert(fsm_state->m_enter_event == NULL);

    while(!TAILQ_EMPTY(&fsm_state->m_transitions)) {
        ui_sprite_fsm_transition_free(TAILQ_FIRST(&fsm_state->m_transitions));
    }

    while(!TAILQ_EMPTY(&fsm_state->m_actions)) {
        ui_sprite_fsm_action_free(TAILQ_FIRST(&fsm_state->m_actions));
    }
    assert(TAILQ_EMPTY(&fsm_state->m_updating_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_waiting_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_runing_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_done_actions));

    TAILQ_REMOVE(&fsm->m_states, fsm_state, m_next_for_ins);

    if (fsm->m_init_state == fsm_state) fsm->m_init_state = NULL;
    if (fsm->m_init_call_state == fsm_state) fsm->m_init_call_state = NULL;

    mem_free(module->m_alloc, fsm_state);
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_find_by_name(ui_sprite_fsm_ins_t fsm, const char * name) {
    ui_sprite_fsm_state_t fsm_state;

    TAILQ_FOREACH(fsm_state, &fsm->m_states, m_next_for_ins) {
        if (strcmp(fsm_state->m_name, name) == 0) return fsm_state;
    }

    return NULL;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_find_by_id(ui_sprite_fsm_ins_t fsm, uint16_t id) {
    ui_sprite_fsm_state_t fsm_state;

    TAILQ_FOREACH(fsm_state, &fsm->m_states, m_next_for_ins) {
        if (fsm_state->m_id == id) return fsm_state;
    }

    return NULL;
}

ui_sprite_fsm_ins_t ui_sprite_fsm_state_fsm(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_ins;
}

uint16_t ui_sprite_fsm_state_id(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_id;
}

const char * ui_sprite_fsm_state_name(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_name;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_return_to(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_return_to;
}

ui_sprite_event_t ui_sprite_fsm_state_enter_event(ui_sprite_fsm_state_t fsm_state) {
    while(fsm_state->m_enter_event == NULL && fsm_state->m_ins->m_parent) {
        fsm_state = fsm_state->m_ins->m_parent->m_cur_state;
        assert(fsm_state);
    }

    return fsm_state->m_enter_event;
}

int ui_sprite_fsm_state_set_enter_event(ui_sprite_fsm_state_t fsm_state, ui_sprite_event_t evt) {
    ui_sprite_event_t new_evt = NULL;

    if (evt) {
        new_evt = ui_sprite_event_copy(fsm_state->m_ins->m_module->m_alloc, evt);
        if (new_evt == NULL) return -1;
    }
    
    if (fsm_state->m_enter_event) {
        mem_free(fsm_state->m_ins->m_module->m_alloc, fsm_state->m_enter_event);
    }

    fsm_state->m_enter_event = new_evt;
    
    return 0;
}

ui_sprite_event_t ui_sprite_fsm_state_local_enter_event(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_enter_event;
}

int ui_sprite_fsm_state_enter(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_action_t fsm_action;
    ui_sprite_fsm_transition_t fsm_transition;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    assert(fsm->m_cur_state == NULL);

    fsm->m_cur_state = fsm_state;

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: enter", 
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm));
    }

    TAILQ_FOREACH(fsm_action, &fsm_state->m_actions, m_next_for_state) {
        assert(fsm_action->m_runing_state == ui_sprite_fsm_action_state_deactive);

        if (fsm_action->m_follow_to != NULL) {
            ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_waiting);
        }
        else {
            if (fsm_action->m_condition) {
                struct ui_sprite_fsm_addition_source_ctx addition_source_ctx;
                dr_data_source_t data_source = NULL;
                ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &addition_source_ctx);
                
                if (!ui_sprite_entity_calc_bool_with_dft(fsm_action->m_condition, entity, data_source, 0)) {
                    if (ui_sprite_entity_debug(entity) >= 2) {
                        CPE_INFO(
                            module->m_em, "entity %d(%s): %s: action %s(%s): condition[%s] check fail",
                            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                            fsm_action->m_meta->m_name, fsm_action->m_name,
                            fsm_action->m_condition);
                    }
                    ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_done);
                    continue;
                }
            }

            if (ui_sprite_fsm_action_enter(fsm_action) != 0) {
                ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_done);
                continue;
            }

            if (ui_sprite_fsm_action_check_do_enter(fsm_action) != 0) {
                ui_sprite_fsm_action_exit(fsm_action);
                continue;
            }
        }
    }

    TAILQ_FOREACH(fsm_transition, &fsm_state->m_transitions, m_next_for_state) {
        if (ui_sprite_fsm_transition_enter(fsm_transition) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: enter fail", 
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm));
            goto STATE_ENTER_FAIL;
        }
    }

    return 0;

STATE_ENTER_FAIL:
    TAILQ_FOREACH(fsm_transition, &fsm_state->m_transitions, m_next_for_state) {
        if(fsm_transition->m_handler) {
            ui_sprite_fsm_transition_exit(fsm_transition);
        }
    }

    while(!TAILQ_EMPTY(&fsm_state->m_runing_actions)) {
        ui_sprite_fsm_action_exit(TAILQ_FIRST(&fsm_state->m_runing_actions));
    }

    TAILQ_FOREACH(fsm_action, &fsm_state->m_actions, m_next_for_state) {
        if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_deactive) {
            ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_deactive);
        }
    }

    fsm->m_cur_state = NULL;
    return -1;

    assert(TAILQ_EMPTY(&fsm_state->m_runing_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_waiting_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_updating_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_done_actions));
}

void ui_sprite_fsm_state_exit(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_action_t fsm_action;
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_transition_t fsm_transition;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    assert(fsm->m_cur_state == fsm_state);

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: exit",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm));
    }

    TAILQ_FOREACH(fsm_transition, &fsm_state->m_transitions, m_next_for_state) {
        ui_sprite_fsm_transition_exit(fsm_transition);
    }

    while(!TAILQ_EMPTY(&fsm_state->m_runing_actions)) {
        ui_sprite_fsm_action_exit(TAILQ_LAST(&fsm_state->m_runing_actions, ui_sprite_fsm_action_list));
    }

    TAILQ_FOREACH_REVERSE(fsm_action, &fsm_state->m_actions, ui_sprite_fsm_action_list, m_next_for_state) {
        if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_deactive) {
            ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_deactive);
        }
    }

    if (fsm_state->m_enter_event) {
        mem_free(module->m_alloc, fsm_state->m_enter_event);
        fsm_state->m_enter_event = NULL;
    }

    fsm->m_cur_state = NULL;

    assert(TAILQ_EMPTY(&fsm_state->m_runing_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_waiting_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_updating_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_done_actions));
}

uint16_t ui_sprite_fsm_state_check_actions(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_action_t action;
    uint16_t life_circle_action_count = 0;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    /*检查已经完成的action，退出 */
    for(action = TAILQ_FIRST(&fsm_state->m_runing_actions);
        action != TAILQ_END(&fsm_state->m_runing_actions);
        )
    {
        ui_sprite_fsm_action_t next = TAILQ_NEXT(action, m_next_for_work);

        switch(action->m_life_circle) {
        case ui_sprite_fsm_action_life_circle_passive:
            break;
        case ui_sprite_fsm_action_life_circle_working:
            if (action->m_duration > 0.0f && action->m_runing_time > action->m_duration) {
                ui_sprite_fsm_action_exit(action);
            }
            else if (action->m_is_update) {
                life_circle_action_count++;
            }
            else {
                ui_sprite_fsm_action_exit(action);
            }
            break;
        case ui_sprite_fsm_action_life_circle_endless:
            life_circle_action_count++;
            break;
        case ui_sprite_fsm_action_life_circle_duration:
            if (action->m_runing_time < action->m_duration) {
                life_circle_action_count++;
            }
            else {
                ui_sprite_fsm_action_exit(action);
            }
            break;
        case ui_sprite_fsm_action_life_circle_passive_working:
            if (action->m_is_update) {
                life_circle_action_count++;
            }
            break;
        default:
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s): life-circle %d unknown",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                action->m_meta->m_name, action->m_name, action->m_life_circle);
            ui_sprite_fsm_action_exit(action);
            break;
        }

        action = next;
    }

    /*检查所有刚刚完成的action，有follow则启动 */
    while(!TAILQ_EMPTY(&fsm_state->m_done_actions)) {
        ui_sprite_fsm_action_t follow_action;

        action = TAILQ_FIRST(&fsm_state->m_done_actions);
        ui_sprite_fsm_action_set_runing_state(action, ui_sprite_fsm_action_state_deactive);

        TAILQ_FOREACH(follow_action, &action->m_followers, m_next_for_follow) {
            if (follow_action->m_runing_state != ui_sprite_fsm_action_state_waiting) continue;

            if (follow_action->m_condition) {
                dr_data_source_t data_source = NULL;
                struct ui_sprite_fsm_addition_source_ctx addition_source_ctx;
                ui_sprite_fsm_action_append_addition_source(follow_action, &data_source, &addition_source_ctx);

                if (!ui_sprite_entity_calc_bool_with_dft(follow_action->m_condition, entity, data_source, 0)) {
                    if (ui_sprite_entity_debug(entity) >= 2) {
                        CPE_INFO(
                            module->m_em, "entity %d(%s): %s: action %s(%s): condition[%s] check fail", 
                            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                            follow_action->m_meta->m_name, follow_action->m_name,
                            follow_action->m_condition);
                    }
                    ui_sprite_fsm_action_set_runing_state(follow_action, ui_sprite_fsm_action_state_done);
                    continue;
                }
            }

            if (ui_sprite_fsm_action_enter(follow_action) != 0) {
                ui_sprite_fsm_action_set_runing_state(follow_action, ui_sprite_fsm_action_state_done);
                continue;
            }

            if (ui_sprite_fsm_action_check_do_enter(follow_action) != 0) {
                ui_sprite_fsm_action_exit(follow_action);
                continue;
            }

            if (follow_action->m_life_circle == ui_sprite_fsm_action_life_circle_working && !follow_action->m_is_update) {
                ui_sprite_fsm_action_exit(follow_action);
                continue;
            }

            life_circle_action_count++;
        }
    }

    return life_circle_action_count;
}

void ui_sprite_fsm_state_process_complete(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;

    /*没有活动的action了，则退出state */
    if (fsm_state->m_return_to && fsm_state->m_return_to->m_enter_event == NULL && fsm_state->m_enter_event != NULL) {
        fsm_state->m_return_to->m_enter_event =
            ui_sprite_event_copy(module->m_alloc, fsm_state->m_enter_event);
    }
    ui_sprite_fsm_state_exit(fsm_state);

    /*回退调用堆栈 */
    while(fsm_state->m_return_to) {
        ui_sprite_fsm_state_t return_state = fsm_state->m_return_to;
        fsm_state->m_return_to = NULL;
        fsm_state = return_state;
 
        if (ui_sprite_fsm_state_enter(fsm_state) == 0) return;

        if (fsm_state->m_return_to && fsm_state->m_return_to->m_enter_event) {
            fsm_state->m_return_to->m_enter_event = fsm_state->m_enter_event;
            fsm_state->m_enter_event = NULL;
        }
        else {
            mem_free(module->m_alloc, fsm_state->m_enter_event);
            fsm_state->m_enter_event = NULL;
        }
    }

    assert(fsm->m_cur_state == NULL);
    if (fsm->m_parent) {
        TAILQ_REMOVE(&fsm->m_parent->m_childs, fsm, m_next_for_parent);
    }
}

uint16_t ui_sprite_fsm_state_count_left_actions(ui_sprite_fsm_state_t fsm_state) {
    uint16_t count = 0;
    ui_sprite_fsm_action_t action;

    /*检查已经完成的action，退出 */
    TAILQ_FOREACH(action, &fsm_state->m_runing_actions, m_next_for_work) {
        switch(action->m_life_circle) {
        case ui_sprite_fsm_action_life_circle_working:
            if (action->m_is_update) count++;
            break;
        case ui_sprite_fsm_action_life_circle_endless:
            count++;
            break;
        case ui_sprite_fsm_action_life_circle_duration:
            if (action->m_runing_time < action->m_duration) count++;
            break;
        case ui_sprite_fsm_action_life_circle_passive_working:
            if (action->m_is_update) count++;
            break;
        default:
            break;
        }
    }

    /*检查所有刚刚完成的action，有follow则启动 */
    TAILQ_FOREACH(action, &fsm_state->m_waiting_actions, m_next_for_state) {
        count++;
    }

    return count;
}

void ui_sprite_fsm_state_update(ui_sprite_fsm_state_t fsm_state, float delta) {
    ui_sprite_fsm_action_t action;

    /*所有需要更新的action先更新 */
    for(action = TAILQ_FIRST(&fsm_state->m_updating_actions);
        action != TAILQ_END(&fsm_state->m_updating_actions);
        )
    {
        ui_sprite_fsm_action_t next = TAILQ_NEXT(action, m_next_for_update);

        assert(action->m_meta->m_update_fun);
        action->m_meta->m_update_fun(action, action->m_meta->m_update_fun_ctx, delta);

        action = next;
    }

    TAILQ_FOREACH(action, &fsm_state->m_runing_actions, m_next_for_work) {
        action->m_runing_time += delta;
    }

    if (ui_sprite_fsm_state_check_actions(fsm_state) != 0) return;

    ui_sprite_fsm_state_process_complete(fsm_state);
}

static ui_sprite_fsm_state_t ui_sprite_fsm_ins_states_next(ui_sprite_fsm_state_it_t it) {
    ui_sprite_fsm_state_t * data = (ui_sprite_fsm_state_t *)(it->m_data);
    ui_sprite_fsm_state_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_ins);

    return r;
}

void ui_sprite_fsm_ins_states(ui_sprite_fsm_state_it_t it, ui_sprite_fsm_ins_t ins) {
    *(ui_sprite_fsm_state_t *)(it->m_data) = TAILQ_FIRST(&ins->m_states);
    it->next = ui_sprite_fsm_ins_states_next;
}

void ui_sprite_fsm_state_append_addition_source(
    ui_sprite_fsm_state_t fsm_state,
    dr_data_source_t * data_source,
    struct ui_sprite_fsm_addition_source_ctx * ctx)
{
    ui_sprite_fsm_module_t module = fsm_state->m_ins->m_module;
    uint8_t append_source_pos = 0;
    dr_data_source_t * insert_pos;

    ctx->m_last_source = data_source;
    while(*ctx->m_last_source) {
        ctx->m_last_source = &(*ctx->m_last_source)->m_next;
    }
    insert_pos = ctx->m_last_source;
        
    /*处理所有state路径上的事件 */
    while(fsm_state) {
        if (append_source_pos + 2 >= CPE_ARRAY_SIZE(ctx->m_append_sources)) break;

        if (fsm_state->m_enter_event) {
            /*append enter_evt common*/
            ctx->m_enter_event.enter_evt_from = fsm_state->m_enter_event->from_entity_id;
            cpe_str_dup(ctx->m_enter_event.enter_evt_name, sizeof(ctx->m_enter_event.enter_evt_name), dr_meta_name(fsm_state->m_enter_event->meta));

            ctx->m_append_sources[append_source_pos].m_data.m_meta = module->m_meta_action_enter_event;
            ctx->m_append_sources[append_source_pos].m_data.m_data = &ctx->m_enter_event;
            ctx->m_append_sources[append_source_pos].m_data.m_size = sizeof(ctx->m_enter_event);
            ctx->m_append_sources[append_source_pos].m_next = NULL;

            *insert_pos = &ctx->m_append_sources[append_source_pos++];
            insert_pos = &((*insert_pos)->m_next);

            /*append enter_evt data*/
            ctx->m_append_sources[append_source_pos].m_data.m_meta = fsm_state->m_enter_event->meta;
            ctx->m_append_sources[append_source_pos].m_data.m_data = (void*)fsm_state->m_enter_event->data;
            ctx->m_append_sources[append_source_pos].m_data.m_size = fsm_state->m_enter_event->size;
            ctx->m_append_sources[append_source_pos].m_next = NULL;

            *insert_pos = &ctx->m_append_sources[append_source_pos++];
            insert_pos = &((*insert_pos)->m_next);
        }

        if (fsm_state->m_ins->m_parent) {
            ui_sprite_fsm_action_fsm_t action_fsm = (ui_sprite_fsm_action_fsm_t)fsm_state->m_ins;
            if (action_fsm->m_data.m_meta) {
                if (append_source_pos + 1 >= CPE_ARRAY_SIZE(ctx->m_append_sources)) break;
                
                /*append enter_evt data*/
                ctx->m_append_sources[append_source_pos].m_data = action_fsm->m_data;
                ctx->m_append_sources[append_source_pos].m_next = NULL;

                *insert_pos = &ctx->m_append_sources[append_source_pos++];
                insert_pos = &((*insert_pos)->m_next);
            }
            
            fsm_state = fsm_state->m_ins->m_parent->m_cur_state;
        }
        else {
            break;
        }
    }
}
