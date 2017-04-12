#include <assert.h>
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_manip_i.h"

int plugin_particle_ed_src_load(ui_ed_src_t ed_src) {
    plugin_particle_data_t particle;
    struct plugin_particle_data_emitter_it emitter_it;
    plugin_particle_data_emitter_t emitter;

    particle = ui_data_src_product(ui_ed_src_data(ed_src));
    assert(particle);

    plugin_particle_data_emitters(&emitter_it, particle);
    while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
        ui_ed_obj_t emitter_obj;
        struct plugin_particle_data_mod_it mod_it;
        plugin_particle_data_mod_t mod;

        emitter_obj = ui_ed_obj_create_i(
            ed_src, ui_ed_src_root_obj(ed_src),
            ui_ed_obj_type_particle_emitter,
            emitter, plugin_particle_data_emitter_data(emitter), sizeof(*plugin_particle_data_emitter_data(emitter)));
        if (emitter_obj == NULL) return -1;

        plugin_particle_data_emitter_mods(&mod_it, emitter);
        while((mod = plugin_particle_data_mod_it_next(&mod_it))) {
            ui_ed_obj_t obj =
                ui_ed_obj_create_i(
                    ed_src, emitter_obj,
                    ui_ed_obj_type_particle_mod,
                    mod, plugin_particle_data_mod_data(mod), sizeof(*plugin_particle_data_mod_data(mod)));
            if (obj == NULL) return -1;
        }
    }
    
    return 0;
}

ui_ed_obj_t ui_ed_obj_create_particle_emitter(ui_ed_obj_t parent) {
    plugin_particle_data_t particle;
    plugin_particle_data_emitter_t particle_emitter;
    ui_ed_obj_t obj;

    particle = ui_ed_src_product(ui_ed_obj_src(parent));
    assert(particle);

    particle_emitter = plugin_particle_data_emitter_create(particle);
    if (particle_emitter == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ui_ed_obj_src(parent), parent,
        ui_ed_obj_type_particle_emitter,
        particle_emitter, plugin_particle_data_emitter_data(particle_emitter), sizeof(*plugin_particle_data_emitter_data(particle_emitter)));
    if (obj == NULL) {
        plugin_particle_data_emitter_free(particle_emitter);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t ui_ed_obj_create_particle_mod(ui_ed_obj_t parent) {
    plugin_particle_data_emitter_t emitter;
    plugin_particle_data_mod_t mod;
    ui_ed_obj_t obj;

    emitter = ui_ed_obj_product(parent);
    assert(emitter);

    mod = plugin_particle_data_mod_create(emitter);
    if (mod == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ui_ed_obj_src(parent), parent,
        ui_ed_obj_type_particle_mod,
        mod, plugin_particle_data_mod_data(mod), sizeof(*plugin_particle_data_mod_data(mod)));
    if (obj == NULL) {
        plugin_particle_data_mod_free(mod);
        return NULL;
    }

    return obj;
}
