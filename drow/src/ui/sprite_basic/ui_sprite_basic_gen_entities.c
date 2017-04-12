#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_gen_entities_i.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_data.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_evt.h"

ui_sprite_basic_gen_entities_t ui_sprite_basic_gen_entities_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_GEN_ENTITIES_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_gen_entities_free(ui_sprite_basic_gen_entities_t gen_entities) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(gen_entities);
    ui_sprite_fsm_action_free(fsm_action);
}

void ui_sprite_basic_gen_entities_set_proto(ui_sprite_basic_gen_entities_t gen_entities, const char * proto) {
    cpe_str_dup(gen_entities->m_proto, sizeof(gen_entities->m_proto), proto);
    * cpe_str_trim_tail(gen_entities->m_proto + strlen(gen_entities->m_proto), gen_entities->m_proto) = 0;
}

const char * ui_sprite_basic_gen_entities_proto(ui_sprite_basic_gen_entities_t gen_entities) {
    return gen_entities->m_proto;
}

int ui_sprite_basic_gen_entities_set_attrs(ui_sprite_basic_gen_entities_t gen_entities, const char * attrs) {
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(gen_entities));
    ui_sprite_basic_module_t module = gen_entities->m_module;

    if (gen_entities->m_attrs) {
        mem_free(module->m_alloc, gen_entities->m_attrs);
        gen_entities->m_attrs = NULL;
    }

    if (attrs) {
        gen_entities->m_attrs = cpe_str_mem_dup(module->m_alloc, attrs);
        if (gen_entities->m_attrs == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: set attrs fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));

            return -1;
        }
    }

    return 0;
}

const char * ui_sprite_basic_gen_entities_attrs(ui_sprite_basic_gen_entities_t gen_entities) {
    return gen_entities->m_attrs;
}

void ui_sprite_basic_gen_entities_set_wait_stop(ui_sprite_basic_gen_entities_t gen_entities, uint8_t wait_stop) {
    gen_entities->m_wait_stop = wait_stop;
}

uint8_t ui_sprite_basic_gen_entities_wait_stop(ui_sprite_basic_gen_entities_t gen_entities) {
    return gen_entities->m_wait_stop;
}

void ui_sprite_basic_gen_entities_set_do_destory(ui_sprite_basic_gen_entities_t gen_entities, uint8_t do_destory) {
    gen_entities->m_do_destory = do_destory;
}

uint8_t ui_sprite_basic_gen_entities_do_destory(ui_sprite_basic_gen_entities_t gen_entities) {
    return gen_entities->m_do_destory;
}

ui_sprite_basic_value_generator_t 
ui_sprite_basic_gen_entities_create_generator(
    ui_sprite_basic_gen_entities_t gen_entities, UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF const * def)
{
    return ui_sprite_basic_value_generator_create(gen_entities->m_module, &gen_entities->m_generators, def);
}

static void ui_sprite_basic_gen_entities_sync_update(ui_sprite_basic_gen_entities_t gen_entities) {
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(gen_entities);

    uint8_t is_stop = 
        /*没有需要等待停止的entity，或者不需要等待entity停止 */
        (gen_entities->m_runing_entity_count == 0 || !gen_entities->m_wait_stop) 
        &&
        /*所有entity已经生成*/
        (gen_entities->m_generated_count >= gen_entities->m_gen_count);

    /* printf( */
    /*     "xxxxx: sync_update: generated=%d, runing=%d, gen=%d, is_stop=%d\n",  */
    /*     gen_entities->m_generated_count, */
    /*     gen_entities->m_runing_entity_count, */
    /*     gen_entities->m_gen_count, */
    /*     is_stop); */

    ui_sprite_fsm_action_sync_update(action, is_stop ? 0 : 1);
}


static int ui_sprite_basic_gen_entities_do_gen(
    ui_sprite_fsm_action_t fsm_action, ui_sprite_basic_gen_entities_t gen_entities, uint16_t require_count)
{
    ui_sprite_basic_module_t module = gen_entities->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_entity_t gened_entity;
    const char * proto = NULL;
    
    while(gen_entities->m_generated_count < require_count) {
        if (gen_entities->m_wait_stop || gen_entities->m_do_destory) {
            if (gen_entities->m_runing_entity_count >= gen_entities->m_runing_entity_capacity) {
                uint16_t new_capacity = gen_entities->m_runing_entity_capacity < 16 ? 16 : gen_entities->m_runing_entity_capacity * 2;
                uint16_t * new_runing_entities = mem_alloc(module->m_alloc, sizeof(uint16_t) * new_capacity);
                if (new_runing_entities == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): gen entity: alloc new runing entity buf fail, capacity=%d!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), new_capacity);
                    return -1;
                }

                if (gen_entities->m_runing_entities) {
                    memcpy(new_runing_entities, gen_entities->m_runing_entities, sizeof(uint16_t) * gen_entities->m_runing_entity_count);
                    mem_free(module->m_alloc, gen_entities->m_runing_entities);
                }

                gen_entities->m_runing_entities = new_runing_entities;
                gen_entities->m_runing_entity_capacity = new_capacity;
            }
        }

        if (proto == NULL) {
            proto = ui_sprite_fsm_action_check_calc_str(
                &module->m_tmp_buff, gen_entities->m_proto, fsm_action, NULL, module->m_em);
            if (proto == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): gen entity: create entity from proto %s fail, calc value fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), gen_entities->m_proto);
                return -1;
            }
        }
        
        gened_entity = ui_sprite_entity_create(world, "", proto);
        if (gened_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: create entity from proto %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), proto);
            return -1;
        }

        if (gen_entities->m_attrs) {
            struct dr_data_source data_source_buf[64];
            dr_data_source_t data_source = data_source_buf;
            dr_data_source_t * next_data_source;
            UI_SPRITE_BASIC_GEN_ENTITIES_DATA gen_data;
            struct ui_sprite_fsm_addition_source_ctx action_source_ctx;

            gen_data.creator_id = ui_sprite_entity_id(entity);
            cpe_str_dup(gen_data.creator_name, sizeof(gen_data.creator_name), ui_sprite_entity_name(entity));

            data_source->m_data.m_meta = module->m_meta_gen_entities_data;
            data_source->m_data.m_data = &gen_data;
            data_source->m_data.m_size = sizeof(gen_data);
            data_source->m_next = NULL;

            ui_sprite_fsm_action_append_addition_source(fsm_action, &data_source, &action_source_ctx);

            next_data_source = &data_source->m_next;
            while(*next_data_source) {
                next_data_source = &(*next_data_source)->m_next;
            }
            *next_data_source = ui_sprite_entity_build_data_source(entity, data_source_buf + 1, sizeof(data_source_buf) - 1);

            if (ui_sprite_entity_bulk_set_attrs(gened_entity, gen_entities->m_attrs, data_source) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): gen entity: entity %d() set attrs %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_entity_id(gened_entity),
                    gen_entities->m_attrs);
                ui_sprite_entity_free(gened_entity);
                return -1;
            }
        }

        if (ui_sprite_basic_value_generator_supply_all(&gen_entities->m_generators, gened_entity, NULL) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: entity %d() process generators fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_entity_id(gened_entity));
            ui_sprite_entity_free(gened_entity);
            return -1;
        }

        if (ui_sprite_entity_enter(gened_entity) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: entity %d() enter fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_entity_id(gened_entity));
            ui_sprite_entity_free(gened_entity);
            return -1;
        }

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): gen entity: proto %s: generate entity %d()",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                proto,
                ui_sprite_entity_id(gened_entity));
        }

        if (gen_entities->m_wait_stop || gen_entities->m_do_destory) {
            gen_entities->m_runing_entities[gen_entities->m_runing_entity_count++] = ui_sprite_entity_id(gened_entity);
        }

        gen_entities->m_generated_count++;
    }

    return 0;
}

static void ui_sprite_basic_gen_entities_on_gen_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_basic_gen_entities_t gen_entities = ctx;
    ui_sprite_basic_module_t module = gen_entities->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    UI_SPRITE_EVT_BASIC_GEN_ENTITIES const * evt_data = evt->data;

    gen_entities->m_generated_count = 0;
    gen_entities->m_generated_duration = 0.0f;
    gen_entities->m_gen_count = evt_data->generate_count;
    gen_entities->m_gen_duration = evt_data->generate_duration;

    if (!TAILQ_EMPTY(&gen_entities->m_generators)) {
        struct ui_sprite_fsm_addition_source_ctx action_source_ctx;
        dr_data_source_t data_source = NULL;

        ui_sprite_fsm_action_append_addition_source(action, &data_source, &action_source_ctx);

        if (ui_sprite_basic_value_generator_init_all(&gen_entities->m_generators, entity, data_source) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: on-gen-event: init generators fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }
    }

    if (gen_entities->m_gen_duration <= 0.0f) {
        if (ui_sprite_basic_gen_entities_do_gen(action, gen_entities, gen_entities->m_gen_count) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: on-gen-event: gen entities fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_fsm_action_sync_update(action, 0);
            return;
        }
    }

    ui_sprite_basic_gen_entities_sync_update(gen_entities);
}

static int ui_sprite_basic_gen_entities_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (gen_entities->m_proto[0] == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen entity: proto not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    gen_entities->m_generated_duration = 0.0f;
    gen_entities->m_generated_count = 0;
    gen_entities->m_runing_entity_count = 0;

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self, 
            "ui_sprite_evt_basic_gen_entities", ui_sprite_basic_gen_entities_on_gen_event, gen_entities) != 0)
    {
        CPE_ERROR(module->m_em, "camera ctrl enter: add eventer handler fail!");
        return -1;
    }

    return 0;
}

static void ui_sprite_basic_gen_entities_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);

    if (gen_entities->m_do_destory) {
        uint16_t i;
        for(i = 0; i < gen_entities->m_runing_entity_count; ++i) {
            ui_sprite_entity_t gened_entity = ui_sprite_entity_find_by_id(world, gen_entities->m_runing_entities[i]);
            if (gened_entity) {
                if (ui_sprite_entity_debug(entity)) {
                    CPE_INFO(
                        module->m_em, "entity %d(%s): gen entity: destory entity %d(%s)!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                        ui_sprite_entity_id(gened_entity), ui_sprite_entity_name(gened_entity));
                }

                ui_sprite_entity_set_destory(gened_entity);
            }
        }
    }

    gen_entities->m_runing_entity_count = 0;
}

static int ui_sprite_basic_gen_entities_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_fsm_action_data(fsm_action);
    bzero(gen_entities, sizeof(*gen_entities));
    gen_entities->m_module = ctx;

    TAILQ_INIT(&gen_entities->m_generators);

    return 0;
}

static void ui_sprite_basic_gen_entities_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_fsm_action_data(fsm_action);

    assert(gen_entities->m_runing_entity_count == 0);

    if (gen_entities->m_runing_entities) {
        mem_free(module->m_alloc, gen_entities->m_runing_entities);
        gen_entities->m_runing_entities = NULL;
        gen_entities->m_runing_entity_count = 0;
        gen_entities->m_runing_entity_capacity = 0;
    }

    if (gen_entities->m_attrs) {
        mem_free(module->m_alloc, gen_entities->m_attrs);
        gen_entities->m_attrs = NULL;
    }

    ui_sprite_basic_value_generator_free_all(&gen_entities->m_generators);
}

static int ui_sprite_basic_gen_entities_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_gen_entities_t to_gen_entities = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_gen_entities_t from_gen_entities = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_gen_entities_init(to, ctx)) return -1;

    cpe_str_dup(to_gen_entities->m_proto, sizeof(to_gen_entities->m_proto), from_gen_entities->m_proto);
    to_gen_entities->m_gen_count = from_gen_entities->m_gen_count;
    to_gen_entities->m_gen_duration = from_gen_entities->m_gen_duration;
    to_gen_entities->m_wait_stop = from_gen_entities->m_wait_stop;
    to_gen_entities->m_do_destory = from_gen_entities->m_do_destory;

    if (from_gen_entities->m_attrs) {
        to_gen_entities->m_attrs = cpe_str_mem_dup(module->m_alloc, from_gen_entities->m_attrs);
        if (to_gen_entities->m_attrs == NULL) {
            ui_sprite_entity_t to_entity = ui_sprite_fsm_action_to_entity(to);

            CPE_ERROR(
                module->m_em, "entity %d(%s): clone gen_entities: copy attrs fail!",
                ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity));

            ui_sprite_basic_gen_entities_clear(to, ctx);
            return -1;
        }
    }

    if (ui_sprite_basic_value_generator_clone_all(
            module, &to_gen_entities->m_generators, &from_gen_entities->m_generators) != 0)
    {
        ui_sprite_entity_t to_entity = ui_sprite_fsm_action_to_entity(to);

        CPE_ERROR(
            module->m_em, "entity %d(%s): clone gen_entities: copy generators fail!",
            ui_sprite_entity_id(to_entity), ui_sprite_entity_name(to_entity));

        ui_sprite_basic_gen_entities_clear(to, ctx);
        return -1;
    }

    return 0;
}

static void ui_sprite_basic_gen_entities_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);

    gen_entities->m_generated_duration += delta;
    if (gen_entities->m_generated_count < gen_entities->m_gen_count) {
        float percent = 
            gen_entities->m_generated_duration >= gen_entities->m_gen_duration
            ? 1.0f
            : gen_entities->m_generated_duration / gen_entities->m_gen_duration;
        uint16_t require_gen_count = gen_entities->m_gen_count * gen_entities->m_gen_count * percent;

        if (ui_sprite_basic_gen_entities_do_gen(fsm_action, gen_entities, require_gen_count) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): gen entity: update: gen entities fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_fsm_action_sync_update(fsm_action, 0);
            return;
        }
    }

    if (gen_entities->m_wait_stop) {
        uint16_t i;

        for(i = 0; i < gen_entities->m_runing_entity_count;) {
            ui_sprite_entity_t check_entity = 
                ui_sprite_entity_find_by_id(world, gen_entities->m_runing_entities[i]);
            if (check_entity == NULL) {
                if (ui_sprite_entity_debug(entity)) {
                    CPE_INFO(
                        module->m_em, "entity %d(%s): gen entity: find entity %d() stoped, runing-entitiy-count=%d!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                        gen_entities->m_runing_entities[i], gen_entities->m_runing_entity_count - 1);
                }

                memmove(
                    gen_entities->m_runing_entities + i, gen_entities->m_runing_entities + i + 1,
                    sizeof(gen_entities->m_runing_entities[0]) * (gen_entities->m_runing_entity_count - i - 1));
                --gen_entities->m_runing_entity_count;
            }
            else {
                ++i;
            }
        }
    }

    ui_sprite_basic_gen_entities_sync_update(gen_entities);
}

int ui_sprite_basic_gen_entities_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_BASIC_GEN_ENTITIES_NAME, sizeof(struct ui_sprite_basic_gen_entities));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_gen_entities_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_gen_entities_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_gen_entities_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_gen_entities_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_gen_entities_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_basic_gen_entities_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_GEN_ENTITIES_NAME, ui_sprite_basic_gen_entities_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_basic_gen_entities_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_GEN_ENTITIES_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_GEN_ENTITIES_NAME);
    }
}

const char * UI_SPRITE_BASIC_GEN_ENTITIES_NAME = "gen-entities";

