#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_cfg_action_loader_i.h"

ui_sprite_fsm_action_t
ui_sprite_cfg_loader_load_action_from_path(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_fsm_state_t fsm_state, const char * comp_name, const char * src_path)
{
    cfg_t cfg = NULL;

    cfg = ui_sprite_cfg_loader_find_cfg(loader, src_path);
    if (cfg == NULL) {
        CPE_ERROR(loader->m_em, "%s: cfg %s not exist!", ui_sprite_cfg_loader_name(loader), src_path);
        return NULL;
    }

    return ui_sprite_cfg_loader_load_action_from_cfg(loader, fsm_state, comp_name, cfg);
}

ui_sprite_fsm_action_t
ui_sprite_cfg_loader_load_action_from_cfg(
    ui_sprite_cfg_loader_t loader,
    ui_sprite_fsm_state_t fsm_state, const char * comp_name, cfg_t cfg)
{
    ui_sprite_cfg_action_loader_t action_loader;
    ui_sprite_fsm_action_t action;
    const char * str_life_circle;
    ui_sprite_fsm_action_life_circle_t life_circle;
    cfg_t cfg_duration;
    const char * work;
    const char * condition;
    cfg_t cfg_convert;

    action_loader = ui_sprite_cfg_action_loader_find(loader, comp_name);
    if (action_loader == NULL) {
        CPE_ERROR(loader->m_em, "%s: action loader %s not exist!", ui_sprite_cfg_loader_name(loader), comp_name);
        return NULL;
    }

    action = action_loader->m_fun(action_loader->m_ctx, fsm_state, cfg_get_string(cfg, "name", ""), cfg);
    if (action == NULL) {
        if (loader->m_debug) {
            CPE_ERROR(
                loader->m_em, "%s: action %s load from [%s] fail!",
                ui_sprite_cfg_loader_name(loader), comp_name, cfg_dump_inline(cfg, &loader->m_dump_buffer));
        }
        else {
            CPE_ERROR(
                loader->m_em, "%s: action %s load fail!",
                ui_sprite_cfg_loader_name(loader), comp_name);
        }

        return NULL;
    }

    if ((str_life_circle = cfg_get_string(cfg, "life-circle", NULL))) {
        if (strcmp(str_life_circle, "passive") == 0) {
            life_circle = ui_sprite_fsm_action_life_circle_passive;
        }
        else if (strcmp(str_life_circle, "working") == 0) {
            life_circle = ui_sprite_fsm_action_life_circle_working;
        }
        else if (strcmp(str_life_circle, "endless") == 0) {
            life_circle = ui_sprite_fsm_action_life_circle_endless;
        }
        else if (strcmp(str_life_circle, "duration") == 0) {
            life_circle = ui_sprite_fsm_action_life_circle_duration;
        }
        else if (strcmp(str_life_circle, "passive-working") == 0) {
            life_circle = ui_sprite_fsm_action_life_circle_passive_working;
        }
        else {
            CPE_ERROR(
                loader->m_em, "%s: action %s(%s) load fail: life-circle %s unknown!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                str_life_circle);
            ui_sprite_fsm_action_free(action);
            return NULL;
        }
    }
    else {
        life_circle = ui_sprite_fsm_action_life_circle(action);
    }

    if ((cfg_duration = cfg_find_cfg(cfg, "duration"))) {
        if (str_life_circle == NULL) {
            life_circle = ui_sprite_fsm_action_life_circle_duration;
        }

        if (life_circle != ui_sprite_fsm_action_life_circle_duration
            && life_circle != ui_sprite_fsm_action_life_circle_working)
        {
            CPE_ERROR(
                loader->m_em, "%s: action %s(%s) load fail: can`t set duration in life-circie %d!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                life_circle);
            ui_sprite_fsm_action_free(action);
            return NULL;
        }
    }
    else {
        if (life_circle == ui_sprite_fsm_action_life_circle_duration) {
            CPE_ERROR(
                loader->m_em, "%s: action %s(%s) load fail: duration not set!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action));
            ui_sprite_fsm_action_free(action);
            return NULL;
        }
    }

    if (ui_sprite_fsm_action_set_life_circle(action, life_circle) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: action %s(%s) load fail: set life-circle %d fail!",
            ui_sprite_cfg_loader_name(loader),
            ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
            life_circle);
        ui_sprite_fsm_action_free(action);
        return NULL;
    }

    if (cfg_duration) {
        float duration;
        uint8_t setted = 0;
        
        if (cfg_type(cfg_duration) == CPE_CFG_TYPE_STRING) {
            const char * str_duration = cfg_as_string(cfg_duration, NULL);
            assert(str_duration);

            if (str_duration[0] == ':') {
                if (ui_sprite_fsm_action_set_duration_calc(action, str_duration + 1) != 0) {
                    CPE_ERROR(
                        loader->m_em, "%s: action %s(%s) load fail: set duration calc %s fail!",
                        ui_sprite_cfg_loader_name(loader),
                        ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                        str_duration);
                    ui_sprite_fsm_action_free(action);
                    return NULL;
                }

                setted = 1;
            }
            else {
                char * endptr = NULL;
                duration = strtof(str_duration, &endptr);
                if (endptr == NULL || *endptr != 0) {
                    CPE_ERROR(
                        loader->m_em, "%s: action %s(%s) load fail: set duration %s (format not float) fail!",
                        ui_sprite_cfg_loader_name(loader),
                        ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                        str_duration);
                    ui_sprite_fsm_action_free(action);
                    return NULL;
                }
            }
        }
        else {
            duration = cfg_as_float(cfg_duration, -1.0f);
        }

        if (!setted) {
            if (ui_sprite_fsm_action_set_duration(action, duration) != 0) {
                CPE_ERROR(
                    loader->m_em, "%s: action %s(%s) load fail: set duration %f fail!",
                    ui_sprite_cfg_loader_name(loader),
                    ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                    duration);
                ui_sprite_fsm_action_free(action);
                return NULL;
            }
        }
    }

    ui_sprite_fsm_action_set_apply_enter_evt(
        action,
        cfg_get_uint8(cfg, "apply-enter-evt", ui_sprite_fsm_action_apply_enter_evt(action)));

    if ((work = cfg_get_string(cfg, "work", NULL))) {
        if (ui_sprite_fsm_action_set_work(action, work) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: action %s(%s) load fail: set enter %s fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                work);
            ui_sprite_fsm_action_free(action);
            return NULL;
        }
    }

    if ((condition = cfg_get_string(cfg, "condition", NULL))) {
        if (ui_sprite_fsm_action_set_condition(action, condition) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: action %s(%s) load fail: set enter %s fail!",
                ui_sprite_cfg_loader_name(loader),
                ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action),
                condition);
            ui_sprite_fsm_action_free(action);
            return NULL;
        }
    }

    if ((cfg_convert = cfg_find_cfg(cfg, "convertors"))) {
        struct cfg_it child_it;
        cfg_t child_cfg;

        cfg_it_init(&child_it, cfg_convert);
        while((child_cfg = cfg_it_next(&child_it))) {
            const char * event = cfg_get_string(child_cfg, "event", NULL);
            const char * condition = cfg_get_string(child_cfg, "condition", NULL);
            const char * convert_to = cfg_get_string(child_cfg, "convert-to", NULL);
            ui_sprite_fsm_convertor_t convertor;

            if (event == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: action %s(%s) load fail: load convertor: event not configured!",
                    ui_sprite_cfg_loader_name(loader),
                    ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action));
                ui_sprite_fsm_action_free(action);
                return NULL;
            }

            if (convert_to == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: action %s(%s) load fail: load convertor: event %s: convert-to not configured!",
                    ui_sprite_cfg_loader_name(loader),
                    ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action), event);
                ui_sprite_fsm_action_free(action);
                return NULL;
            }

            convertor = ui_sprite_fsm_convertor_create(action, event, condition, convert_to);
            if (convertor == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: action %s(%s) load fail: load convertor: event %s: create convertor fail!",
                    ui_sprite_cfg_loader_name(loader),
                    ui_sprite_fsm_action_type_name(action), ui_sprite_fsm_action_name(action), event);
                ui_sprite_fsm_action_free(action);
                return NULL;
            }
        }
    }

    return action;
}

int ui_sprite_cfg_loader_add_action_loader(ui_sprite_cfg_loader_t loader, const char * name, ui_sprite_cfg_load_action_fun_t fun, void * ctx) {
    ui_sprite_cfg_action_loader_t action = ui_sprite_cfg_action_loader_create(loader, name, ctx, fun);

    if (action == NULL) {
        CPE_ERROR(loader->m_em, "add action loader %s fail!", name);
        return -1;
    }

    return 0;
}

int ui_sprite_cfg_loader_remove_action_loader(ui_sprite_cfg_loader_t loader, const char * name) {
    ui_sprite_cfg_action_loader_t action = ui_sprite_cfg_action_loader_find(loader, name);
    if (action == NULL) return -1;

    ui_sprite_cfg_action_loader_free(action);
    return 0;
}

/*impl*/
ui_sprite_cfg_action_loader_t
ui_sprite_cfg_action_loader_create(
    ui_sprite_cfg_loader_t loader, const char * name,
    void * ctx, ui_sprite_cfg_load_action_fun_t fun)
{
    ui_sprite_cfg_action_loader_t comp;
    size_t name_len = strlen(name) + 1;

    comp = mem_alloc(loader->m_alloc, sizeof(struct ui_sprite_cfg_action_loader) + name_len);
    if (comp == NULL) {
        CPE_ERROR(loader->m_em, "create ui_sprite_cfg_action_loader_loader: alloc fail!");
        return NULL;
    }

    memcpy(comp + 1, name, name_len);

    comp->m_loader = loader;
    comp->m_name = (char *)(comp + 1);
    comp->m_fun = fun;
    comp->m_ctx = ctx;

    cpe_hash_entry_init(&comp->m_hh_for_loader);
    if (cpe_hash_table_insert_unique(&loader->m_action_loaders, comp) != 0) {
        CPE_ERROR(loader->m_em, "create ui_sprite_cfg_action_loader: name %s duplicate!", name);
        mem_free(loader->m_alloc, comp);
        return NULL;
    }

    return comp;
}

void ui_sprite_cfg_action_loader_free(ui_sprite_cfg_action_loader_t comp) {
    ui_sprite_cfg_loader_t loader = comp->m_loader;

    cpe_hash_table_remove_by_ins(&loader->m_action_loaders, comp);

    mem_free(loader->m_alloc, comp);
}

ui_sprite_cfg_action_loader_t ui_sprite_cfg_action_loader_find(ui_sprite_cfg_loader_t loader, const char * name) {
    struct ui_sprite_cfg_action_loader key;
    key.m_name = name;
    return cpe_hash_table_find(&loader->m_action_loaders, &key);
}

void ui_sprite_cfg_action_loader_free_all(ui_sprite_cfg_loader_t loader) {
    struct cpe_hash_it action_loader_it;
    ui_sprite_cfg_action_loader_t action_loader;

    cpe_hash_it_init(&action_loader_it, &loader->m_action_loaders);

    action_loader = cpe_hash_it_next(&action_loader_it);
    while (action_loader) {
        ui_sprite_cfg_action_loader_t next = cpe_hash_it_next(&action_loader_it);
        ui_sprite_cfg_action_loader_free(action_loader);
        action_loader = next;
    }
}

uint32_t ui_sprite_cfg_action_loader_hash(const ui_sprite_cfg_action_loader_t loader) {
    return cpe_hash_str(loader->m_name, strlen(loader->m_name));
}

int ui_sprite_cfg_action_loader_eq(const ui_sprite_cfg_action_loader_t l, const ui_sprite_cfg_action_loader_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
