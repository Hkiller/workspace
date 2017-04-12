#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_gen_entities_i.h"
#include "ui_sprite_basic_value_generator_i.h"

ui_sprite_fsm_action_t ui_sprite_basic_gen_entities_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_basic_gen_entities_create(fsm_state, name);
    const char * proto;
    const char * attrs;
    cfg_t generators_cfg;

    if (gen_entities == NULL) {
        CPE_ERROR(module->m_em, "%s: create gen_entities action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if ((proto = cfg_get_string(cfg, "proto", NULL)) == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create gen_entities action: set proto fail",
            ui_sprite_basic_module_name(module));
        ui_sprite_basic_gen_entities_free(gen_entities);
        return NULL;
    }

    ui_sprite_basic_gen_entities_set_proto(gen_entities, proto);
    ui_sprite_basic_gen_entities_set_wait_stop(gen_entities, cfg_get_uint8(cfg, "wait-stop", 0));
    ui_sprite_basic_gen_entities_set_do_destory(gen_entities, cfg_get_uint8(cfg, "do-destory", 0));

    if ((attrs = cfg_get_string(cfg, "attributes", NULL))) {
        if (ui_sprite_basic_gen_entities_set_attrs(gen_entities, attrs) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create gen_entities action: set attributes %s fail",
                ui_sprite_basic_module_name(module), attrs);
            ui_sprite_basic_gen_entities_free(gen_entities);
            return NULL;
        }
    }

    if ((generators_cfg = cfg_find_cfg(cfg, "generators"))) {
        struct cfg_it generator_it;
        cfg_t generator_cfg;

        cfg_it_init(&generator_it, generators_cfg);
        while((generator_cfg = cfg_it_next(&generator_it))) {
            UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF def;
            if (ui_sprite_basic_value_generator_load(module, &def, generator_cfg) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: create gen_entities action: load generator def fail",
                    ui_sprite_basic_module_name(module));
                ui_sprite_basic_gen_entities_free(gen_entities);
                return NULL;
            }

            if (ui_sprite_basic_gen_entities_create_generator(gen_entities, &def) == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: create gen_entities action: create generator fail",
                    ui_sprite_basic_module_name(module));
                ui_sprite_basic_gen_entities_free(gen_entities);
                return NULL;
            }
        }
    }

    return ui_sprite_fsm_action_from_data(gen_entities);
}

