#include <assert.h>
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/barrage/plugin_barrage_data_emitter.h"
#include "plugin_barrage_manip_i.h"

ui_ed_obj_t plugin_barrage_emitter_emitter_ed_obj_create(ui_ed_obj_t parent) {
    plugin_barrage_data_barrage_t barrage;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_barrage_data_emitter_t emitter;
    ui_ed_obj_t obj;

    barrage = ui_ed_src_product(ed_src);
    assert(barrage);

    emitter = plugin_barrage_data_emitter_create(barrage);
    if (emitter == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent, ui_ed_obj_type_barrage_emitter,
        emitter, plugin_barrage_data_emitter_data(emitter), sizeof(*plugin_barrage_data_emitter_data(emitter)));
    if (obj == NULL) {
        plugin_barrage_data_emitter_free(emitter);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_barrage_emitter_emitter_trigger_ed_obj_create(ui_ed_obj_t parent) {
    plugin_barrage_data_emitter_t emitter;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_barrage_data_emitter_trigger_t trigger;
    ui_ed_obj_t obj;

    emitter = ui_ed_obj_product(parent);
    assert(emitter);

    trigger = plugin_barrage_data_emitter_trigger_create(emitter);
    if (trigger == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_barrage_emitter_trigger,
        trigger, plugin_barrage_data_emitter_trigger_data(trigger), sizeof(*plugin_barrage_data_emitter_trigger_data(trigger)));
    if (obj == NULL) {
        plugin_barrage_data_emitter_trigger_free(trigger);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_barrage_bullet_trigger_ed_obj_create(ui_ed_obj_t parent) {
    plugin_barrage_data_emitter_t emitter;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_barrage_data_bullet_trigger_t trigger;
    ui_ed_obj_t obj;

    emitter = ui_ed_obj_product(parent);
    assert(emitter);

    trigger = plugin_barrage_data_bullet_trigger_create(emitter);
    if (trigger == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_barrage_bullet_trigger,
        trigger, plugin_barrage_data_bullet_trigger_data(trigger), sizeof(*plugin_barrage_data_bullet_trigger_data(trigger)));
    if (obj == NULL) {
        plugin_barrage_data_bullet_trigger_free(trigger);
        return NULL;
    }

    return obj;
}

int plugin_barrage_emitter_ed_src_load(ui_ed_src_t src) {
    ui_ed_obj_t src_obj = ui_ed_src_root_obj(src);
    plugin_barrage_data_barrage_t barrage = ui_ed_src_product(src);
    ui_ed_obj_t obj;
    struct plugin_barrage_data_emitter_it emitter_it;
    plugin_barrage_data_emitter_t emitter;

    assert(barrage);

    obj = ui_ed_obj_create_i(
        src, src_obj,
        ui_ed_obj_type_barrage,
        barrage,
        plugin_barrage_data_barrage_data(barrage),
        sizeof(*plugin_barrage_data_barrage_data(barrage)));
    if (obj == NULL) {
        return -1;
    }

    plugin_barrage_data_barrage_emitters(&emitter_it, barrage);
    while((emitter = plugin_barrage_data_emitter_it_next(&emitter_it))) {
        struct plugin_barrage_data_emitter_trigger_it emitter_trigger_it;
        plugin_barrage_data_emitter_trigger_t emitter_trigger;
        struct plugin_barrage_data_bullet_trigger_it bullet_trigger_it;
        plugin_barrage_data_bullet_trigger_t bullet_trigger;
        ui_ed_obj_t emitter_obj;

        emitter_obj = ui_ed_obj_create_i(
                src, obj,
                ui_ed_obj_type_barrage_emitter,
                emitter,
                plugin_barrage_data_emitter_data(emitter),
                sizeof(*plugin_barrage_data_emitter_data(emitter)));
        if (emitter_obj == NULL) {
            return -1;
        }

        plugin_barrage_data_emitter_triggers(&emitter_trigger_it, emitter);
        while((emitter_trigger = plugin_barrage_data_emitter_trigger_it_next(&emitter_trigger_it))) {
            ui_ed_obj_t trigger_obj = ui_ed_obj_create_i(
                src, emitter_obj,
                ui_ed_obj_type_barrage_emitter_trigger,
                emitter_trigger, plugin_barrage_data_emitter_trigger_data(emitter_trigger),
                sizeof(*plugin_barrage_data_emitter_trigger_data(emitter_trigger)));
            if (trigger_obj == NULL) {
                return -1;
            }
        }

        plugin_barrage_data_bullet_triggers(&bullet_trigger_it, emitter);
        while((bullet_trigger = plugin_barrage_data_bullet_trigger_it_next(&bullet_trigger_it))) {
            ui_ed_obj_t trigger_obj = ui_ed_obj_create_i(
                src, emitter_obj,
                ui_ed_obj_type_barrage_bullet_trigger,
                bullet_trigger, plugin_barrage_data_bullet_trigger_data(bullet_trigger),
                sizeof(*plugin_barrage_data_bullet_trigger_data(bullet_trigger)));
            if (trigger_obj == NULL) {
                return -1;
            }
        }
    }

    return 0;
}
