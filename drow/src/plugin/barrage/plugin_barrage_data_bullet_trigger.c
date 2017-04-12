#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_json.h"
#include "plugin_barrage_data_emitter_i.h"

plugin_barrage_data_bullet_trigger_t
plugin_barrage_data_bullet_trigger_create(plugin_barrage_data_emitter_t emitter) {
    plugin_barrage_module_t module = emitter->m_barrage->m_module;
    plugin_barrage_data_bullet_trigger_t trigger;

    trigger = (plugin_barrage_data_bullet_trigger_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_data_bullet_trigger));
    if (trigger == NULL) {
        CPE_ERROR(module->m_em, "alloc trigger fail!");
        return NULL;
    }

    trigger->m_emitter = emitter;
    bzero(&trigger->m_data, sizeof(trigger->m_data));
    trigger->m_state = plugin_barrage_data_emitter_trigger_queue_noop;
    trigger->m_next = emitter->m_bullet_noop_triggers;
    emitter->m_bullet_noop_triggers = trigger;
    
    emitter->m_bullet_trigger_count++;

    return trigger;
}

static void plugin_barrage_data_bullet_trigger_dequeue(plugin_barrage_data_bullet_trigger_t trigger) {
    plugin_barrage_data_emitter_t emitter = trigger->m_emitter;

    if(trigger->m_state == plugin_barrage_data_emitter_trigger_queue_frame) {
        plugin_barrage_data_bullet_trigger_t pre = NULL;
        plugin_barrage_data_bullet_trigger_t * p = &emitter->m_bullet_frame_triggers_begin;

        while(*p) {
            if (*p == trigger) {
                *p = trigger->m_next;
                trigger->m_next = NULL;
                if (emitter->m_bullet_frame_triggers_last == trigger) {
                    emitter->m_bullet_frame_triggers_last = pre;
                }
                trigger->m_state = plugin_barrage_data_emitter_trigger_queue_none;
                break;
            }
            else {
                pre = *p;
                p = &(*p)->m_next;
            }
        }

    }
    else if(trigger->m_state == plugin_barrage_data_emitter_trigger_queue_check) {
        plugin_barrage_data_bullet_trigger_t * p = &emitter->m_bullet_check_triggers;
        while(*p) {
            if (*p == trigger) {
                *p = trigger->m_next;
                trigger->m_next = NULL;
                trigger->m_state = plugin_barrage_data_emitter_trigger_queue_none;
                break;
            }
            else {
                p = &(*p)->m_next;
            }                
        }
    }
    else if(trigger->m_state == plugin_barrage_data_emitter_trigger_queue_noop) {
        plugin_barrage_data_bullet_trigger_t * p = &emitter->m_bullet_noop_triggers;
        while(*p) {
            if (*p == trigger) {
                *p = trigger->m_next;
                trigger->m_next = NULL;
                trigger->m_state = plugin_barrage_data_emitter_trigger_queue_none;
                break;
            }
            else {
                p = &(*p)->m_next;
            }                
        }
    }

    assert(trigger->m_state == plugin_barrage_data_emitter_trigger_queue_none);
}

void plugin_barrage_data_bullet_trigger_free(plugin_barrage_data_bullet_trigger_t trigger) {
    plugin_barrage_data_emitter_t emitter = trigger->m_emitter;
    plugin_barrage_module_t module = emitter->m_barrage->m_module;

    
    plugin_barrage_data_bullet_trigger_dequeue(trigger);
    assert(trigger->m_next == NULL);

    emitter->m_bullet_trigger_count--;

    mem_free(module->m_alloc, trigger);
}

BARRAGE_EMITTER_BULLET_TRIGGER_INFO *
plugin_barrage_data_bullet_trigger_data(plugin_barrage_data_bullet_trigger_t trigger) {
    return &trigger->m_data;
}

int plugin_barrage_data_bullet_trigger_update(plugin_barrage_data_bullet_trigger_t trigger) {
    plugin_barrage_data_emitter_t emitter = trigger->m_emitter;

    plugin_barrage_data_bullet_trigger_dequeue(trigger);

    assert(trigger->m_next == NULL);

    if(trigger->m_data.condition_count == 1
       && trigger->m_data.conditions[0].condition_type == barrage_emitter_emitter_value_frame
       && trigger->m_data.conditions[0].condition_op == barrage_emitter_condition_op_eq)
    {
        if (emitter->m_bullet_frame_triggers_last == NULL) {
            assert(emitter->m_bullet_frame_triggers_begin == NULL);
            emitter->m_bullet_frame_triggers_begin = trigger;
            emitter->m_bullet_frame_triggers_last = trigger;
            trigger->m_state = plugin_barrage_data_emitter_trigger_queue_frame;
        }
        else {
            assert(emitter->m_bullet_frame_triggers_last->m_next == NULL);

            if (trigger->m_data.conditions[0].condition_value >=
                emitter->m_bullet_frame_triggers_last->m_data.conditions[0].condition_value)
            {
                emitter->m_bullet_frame_triggers_last->m_next = trigger;
                emitter->m_bullet_frame_triggers_last = trigger;
                trigger->m_state = plugin_barrage_data_emitter_trigger_queue_frame;
            }
            else {
                plugin_barrage_data_bullet_trigger_t * p;

                for(p = &emitter->m_bullet_frame_triggers_begin; *p; p = &(*p)->m_next) {
                    if (trigger->m_data.conditions[0].condition_value <
                        (*p)->m_data.conditions[0].condition_value)
                    {
                        trigger->m_next = *p;
                        *p = trigger;
                        trigger->m_state = plugin_barrage_data_emitter_trigger_queue_frame;
                        break;
                    }
                }
            }
        }
    }
    else if (trigger->m_data.condition_count > 0) {
        trigger->m_next = emitter->m_bullet_check_triggers;
        emitter->m_bullet_check_triggers = trigger;
        trigger->m_state = plugin_barrage_data_emitter_trigger_queue_check;
    }
    else {
        trigger->m_next = emitter->m_bullet_noop_triggers;
        emitter->m_bullet_noop_triggers = trigger;
        trigger->m_state = plugin_barrage_data_emitter_trigger_queue_noop;
    }

    return 0;
}

struct plugin_barrage_data_bullet_trigger_it_data {
    plugin_barrage_data_bullet_trigger_t m_frame_triggers;
    plugin_barrage_data_bullet_trigger_t m_check_triggers;
    plugin_barrage_data_bullet_trigger_t m_noop_triggers;
};

static plugin_barrage_data_bullet_trigger_t plugin_barrage_data_bullet_triggers_next(struct plugin_barrage_data_bullet_trigger_it * it) {
    struct plugin_barrage_data_bullet_trigger_it_data * data = (struct plugin_barrage_data_bullet_trigger_it_data *)(it->m_data);
    plugin_barrage_data_bullet_trigger_t r;

    if (data->m_frame_triggers) {
        r = data->m_frame_triggers;
        data->m_frame_triggers = r->m_next;
        return r;
    }

    if (data->m_check_triggers) {
        r = data->m_check_triggers;
        data->m_check_triggers = r->m_next;
        return r;
    }

    if (data->m_noop_triggers) {
        r = data->m_noop_triggers;
        data->m_noop_triggers = r->m_next;
        return r;
    }

    return NULL;
}

void plugin_barrage_data_bullet_triggers(plugin_barrage_data_bullet_trigger_it_t it, plugin_barrage_data_emitter_t emitter) {
    struct plugin_barrage_data_bullet_trigger_it_data * data = (struct plugin_barrage_data_bullet_trigger_it_data *)(it->m_data);
    data->m_frame_triggers = emitter->m_bullet_frame_triggers_begin;
    data->m_check_triggers = emitter->m_bullet_check_triggers;
    data->m_noop_triggers = emitter->m_bullet_noop_triggers;
    it->next = plugin_barrage_data_bullet_triggers_next;
}
