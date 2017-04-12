#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_json.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_particle_obj_emitter_mod_i.h"
#include "plugin_particle_obj_i.h"
#include "plugin_particle_obj_mod_data_i.h"

plugin_particle_obj_emitter_mod_t
plugin_particle_obj_emitter_mod_create(plugin_particle_obj_emitter_t emitter) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    plugin_particle_obj_emitter_mod_t mod;

    mod = TAILQ_FIRST(&module->m_free_emitter_mods);
    if (mod == NULL) {
        mod = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_obj_emitter_mod));
        if (mod == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_mod_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&module->m_free_emitter_mods, mod, m_next);
    }

    mod->m_emitter = emitter;
    TAILQ_INSERT_TAIL(&emitter->m_mods, mod, m_next);
    
    return mod;
}

plugin_particle_obj_emitter_mod_t
plugin_particle_obj_emitter_mod_clone(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_mod_t o) {
    plugin_particle_obj_emitter_mod_t mod = plugin_particle_obj_emitter_mod_create(emitter);

    if (mod == NULL) return NULL;

    mod->m_data = o->m_data;

    return mod;
}

void plugin_particle_obj_emitter_mod_free(plugin_particle_obj_emitter_mod_t mod) {
    plugin_particle_obj_emitter_t emitter = mod->m_emitter;
    plugin_particle_module_t module = emitter->m_obj->m_module;

    TAILQ_REMOVE(&emitter->m_mods, mod, m_next);
    
    TAILQ_INSERT_TAIL(&module->m_free_emitter_mods, mod, m_next);
}

void plugin_particle_obj_emitter_mod_real_free(plugin_particle_module_t module, plugin_particle_obj_emitter_mod_t mod) {
    TAILQ_REMOVE(&module->m_free_emitter_mods, mod, m_next);
    mem_free(module->m_alloc, mod);
}

UI_PARTICLE_MOD const * plugin_particle_obj_emitter_find_mod_r(plugin_particle_obj_emitter_t emitter, uint8_t mod_type) {
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            if (mod->m_data.type == mod_type) return &mod->m_data;
        }
    }
    else if (emitter->m_static_data) {
        plugin_particle_data_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            if (mod->m_data.type == mod_type) return &mod->m_data;
        }
    }

    return NULL;
}

UI_PARTICLE_MOD * plugin_particle_obj_emitter_find_mod_w(plugin_particle_obj_emitter_t emitter, uint8_t mod_type) {
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            if (mod->m_data.type == mod_type) return &mod->m_data;
        }
        return NULL;
    }
    else if (emitter->m_static_data) {
        plugin_particle_data_mod_t mod;
        UI_PARTICLE_MOD * r = NULL;

        if (plugin_particle_obj_emitter_find_mod_r(emitter, mod_type) == NULL) return NULL;
        
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            plugin_particle_obj_emitter_mod_t obj_mod = plugin_particle_obj_emitter_mod_create(emitter);

            if (obj_mod == NULL) {
                while(!TAILQ_EMPTY(&emitter->m_mods)) {
                    plugin_particle_obj_emitter_mod_free(TAILQ_FIRST(&emitter->m_mods));
                }
            }
            
            obj_mod->m_data = mod->m_data;
            if (obj_mod->m_data.type == mod_type) r = &obj_mod->m_data;
        }
        return r;
    }

    return NULL;
}

UI_PARTICLE_MOD * plugin_particle_obj_emitter_check_create_mod(plugin_particle_obj_emitter_t emitter, uint8_t mod_type) {
    UI_PARTICLE_MOD * r = NULL;
    
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            if (mod->m_data.type == mod_type) {
                r = &mod->m_data;
                break;
            }
        }
    }
    else if (emitter->m_static_data) {
        plugin_particle_data_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            plugin_particle_obj_emitter_mod_t obj_mod = plugin_particle_obj_emitter_mod_create(emitter);
            if (obj_mod == NULL){
                while(!TAILQ_EMPTY(&emitter->m_mods)) {
                    plugin_particle_obj_emitter_mod_free(TAILQ_FIRST(&emitter->m_mods));
                }
                return NULL;
            }
            
            obj_mod->m_data = mod->m_data;
            if (obj_mod->m_data.type == mod_type) r = &obj_mod->m_data;
        }
    }

    if (r == NULL) {
        LPDRMETAENTRY data_entry;
        plugin_particle_obj_emitter_mod_t obj_mod;

        data_entry = dr_meta_find_entry_by_id(emitter->m_obj->m_module->m_meta_particle_mod_data, mod_type);
        if (data_entry == NULL) {
            CPE_ERROR(emitter->m_obj->m_module->m_em, "plugin_particle_obj_emitter_check_create_mod: mode type %d unknown(no meta)!", mod_type);
            return NULL;
        }

        obj_mod = plugin_particle_obj_emitter_mod_create(emitter);
        if (obj_mod == NULL) return NULL;

        obj_mod->m_data.type = mod_type;
        bzero(&obj_mod->m_data.data, sizeof(obj_mod->m_data.data));

        dr_meta_set_defaults(
            &obj_mod->m_data.data, sizeof(obj_mod->m_data.data),
            dr_entry_ref_meta(data_entry), DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);

        /* printf( */
        /*     "xxxxxx: create mod %s\n", */
        /*     dr_json_dump_inline( */
        /*         &emitter->m_obj->m_module->m_dump_buffer, */
        /*         &obj_mod->m_data.data, */
        /*         sizeof(obj_mod->m_data.data), */
        /*         dr_entry_ref_meta(data_entry))); */
        
        r = &obj_mod->m_data;

        assert(mod_type >= UI_PARTICLE_MOD_TYPE_MIN && mod_type < UI_PARTICLE_MOD_TYPE_MAX);
        if (g_mod_defs[mod_type].data_init) {
            plugin_particle_obj_mod_data_t mod_data = plugin_particle_obj_mod_data_create(emitter);
            if (mod_data == NULL) {
                plugin_particle_obj_emitter_mod_free(obj_mod);
                return NULL;
            }
                
            g_mod_defs[mod_type].data_init(&obj_mod->m_data, mod_data);
        }
    }

    return r;
}

int plugin_particle_obj_emitter_set_mod_track_location(plugin_particle_obj_emitter_t emitter, uint8_t mod_type, ui_vector_2_t i_pos) {
    UI_PARTICLE_MOD * w;
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    UI_PARTICLE_MOD const * r = plugin_particle_obj_emitter_find_mod_r(emitter, mod_type);
    ui_vector_2_t pos;
    ui_vector_2 pos_buf;
    
    if (mod_type != ui_particle_mod_type_velocity_attract
        && mod_type != ui_particle_mod_type_accel_attract)
    {
        return -1;
    }
    
    if (r == NULL) return -1;

    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        w = (UI_PARTICLE_MOD * )r;
    }
    else {
        w = plugin_particle_obj_emitter_find_mod_w(emitter, mod_type);
        if (w == NULL) return -1;
    }

    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
        ui_transform_t world_t = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj));
        if (world_t) {
            pos = &pos_buf;
            pos_buf = *i_pos;
            ui_transform_inline_reverse_adj_vector_2(world_t, &pos_buf);
        }
        else {
            pos = i_pos;
        }
    }
    else {
        pos = i_pos;
    }
    
    if (mod_type == ui_particle_mod_type_velocity_attract) {
        w->data.velocity_attract.attract_location.value[0] = pos->x;
        w->data.velocity_attract.attract_location.value[1] = pos->y;
    }
    else {
        assert(mod_type == ui_particle_mod_type_accel_attract);
        w->data.accel_attract.location.value[0] = pos->x;
        w->data.accel_attract.location.value[1] = pos->y;
    }
    
    return 0;
}
