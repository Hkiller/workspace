#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_log.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/random.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_string_table.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_emitter_mod_i.h"
#include "plugin_particle_obj_emitter_data_i.h"
#include "plugin_particle_obj_emitter_binding_i.h"
#include "plugin_particle_obj_mod_data_i.h"
#include "plugin_particle_obj_particle_i.h"
#include "plugin_particle_obj_plugin_i.h"

static void plugin_particle_obj_emitter_calc_normalized_vtx(
    plugin_particle_obj_emitter_t emitter, UI_PARTICLE_EMITTER const * emitter_data);

static plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_create_i(
    plugin_particle_module_t module, plugin_particle_obj_t obj, plugin_particle_data_emitter_t data)
{
    plugin_particle_obj_emitter_t emitter;
    
    emitter = TAILQ_FIRST(&module->m_free_emitters);
    if (emitter == NULL) {
        emitter = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_obj_emitter));
        if (emitter == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_create: alloc fail!");
            return NULL;
        }
    }
    else {
        module->m_free_emitter_count--;
        TAILQ_REMOVE(&module->m_free_emitters, emitter, m_next);
    }

    bzero(emitter, sizeof(*emitter));
    emitter->m_obj = obj;
    emitter->m_static_data = data;
    emitter->m_use_state = plugin_particle_obj_emitter_use_state_suspend;
    emitter->m_texture_mode = plugin_particle_obj_emitter_texture_mode_basic;
    emitter->m_time_scale = 1.0f;
    emitter->m_name = NULL;
    
    TAILQ_INIT(&emitter->m_mods);
    TAILQ_INIT(&emitter->m_mod_datas);
    TAILQ_INIT(&emitter->m_plugins);
    TAILQ_INIT(&emitter->m_particles);
    TAILQ_INIT(&emitter->m_binding_particles);
    TAILQ_INSERT_TAIL(&obj->m_emitters, emitter, m_next);

    emitter->m_addition_data.m_meta = NULL;
    emitter->m_addition_data.m_data = NULL;
    emitter->m_addition_data.m_size = 0;
    emitter->m_transform = UI_TRANSFORM_IDENTITY;
    emitter->m_have_transform = 0;
    emitter->m_lifecircle = plugin_particle_obj_emitter_lifecircle_basic;

    cpe_assert_float_sane(emitter->m_transform.m_s.x);
    cpe_assert_float_sane(emitter->m_transform.m_s.y);

    return emitter;
}

static int plugin_particle_obj_emitter_init(plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter) {
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    if (emitter_data == NULL) return 0;
    
    if (TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_data_mod_t mod;

        assert(emitter->m_static_data);
        
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);
            if (g_mod_defs[mod->m_data.type].data_init) {
                plugin_particle_obj_mod_data_t mod_data = plugin_particle_obj_mod_data_create(emitter);

                if (mod_data == NULL) {
                    CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_create: create mod data fail!");
                    return -1;
                }
                
                g_mod_defs[mod->m_data.type].data_init(&mod->m_data, mod_data);
            }
        }
    }
    else {
        plugin_particle_obj_emitter_mod_t mod;
    
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);
            if (g_mod_defs[mod->m_data.type].data_init) {
                plugin_particle_obj_mod_data_t mod_data = plugin_particle_obj_mod_data_create(emitter);

                if (mod_data == NULL) {
                    CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_create: create mod data fail!");
                    return -1;
                }
                
                g_mod_defs[mod->m_data.type].data_init(&mod->m_data, mod_data);
            }
        }
    }
        
    if (emitter_data->texture_id) {
        emitter->m_texture = ui_cache_res_find_by_path(
            module->m_cache_mgr,
            plugin_particle_obj_emitter_msg(emitter, emitter_data->texture_id));
    }
    plugin_particle_obj_emitter_texture_cache_init(emitter);

    plugin_particle_obj_emitter_calc_normalized_vtx(emitter, emitter_data);

    emitter->m_use_state = emitter_data->use_state;
    if (emitter->m_use_state != plugin_particle_obj_emitter_use_state_suspend) {
        plugin_particle_obj_emitter_do_active(emitter);
    }

    return 0;
}

plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_create(plugin_particle_obj_t obj, plugin_particle_data_emitter_t data) {
    plugin_particle_module_t module = obj->m_module;
    plugin_particle_obj_emitter_t emitter;
    
    emitter = plugin_particle_obj_emitter_create_i(module, obj, data);
    if (emitter == NULL) return NULL;

    if (plugin_particle_obj_emitter_init(module, emitter) != 0) {
        plugin_particle_obj_emitter_free(emitter);
        return NULL;
    }

    ui_assert_vector_2_sane(&emitter->m_tile_size);
    
    return emitter;
}

plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_clone(plugin_particle_obj_t obj, plugin_particle_obj_emitter_t o) {
    plugin_particle_module_t module = obj->m_module;
    plugin_particle_obj_emitter_t emitter;
    plugin_particle_obj_emitter_mod_t o_mod;
    plugin_particle_obj_plugin_t o_plugin;
    
    emitter = plugin_particle_obj_emitter_create_i(module, obj, o->m_static_data);
    emitter->m_transform = o->m_transform;
    emitter->m_have_transform = o->m_have_transform;

    TAILQ_FOREACH(o_mod, &o->m_mods, m_next) {
        if (plugin_particle_obj_emitter_mod_clone(emitter, o_mod) == NULL) {
            plugin_particle_obj_emitter_free(emitter);
            return NULL;
        }
    }

    if (plugin_particle_obj_emitter_init(module, emitter) != 0) {
        plugin_particle_obj_emitter_free(emitter);
        return NULL;
    }

    if (o->m_addition_data.m_meta) {
        if (plugin_particle_obj_emitter_set_addition_data(emitter, &o->m_addition_data) != 0) {
            plugin_particle_obj_emitter_free(emitter);
            return NULL;
        }
    }

    TAILQ_FOREACH(o_plugin, &o->m_plugins, m_next) {
        plugin_particle_obj_plugin_t plugin = 
            plugin_particle_obj_plugin_create(
                emitter,
                o_plugin->m_ctx, o_plugin->m_data_capacity,
                o_plugin->m_init_fun, o_plugin->m_fini_fun, o_plugin->m_update_fun);
        if (plugin == NULL) {
            plugin_particle_obj_emitter_free(emitter);
            return NULL;
        }
    }

    if (o->m_runtime_data) {
        assert(emitter->m_runtime_data == NULL);
        emitter->m_runtime_data = plugin_particle_obj_emitter_data_create(emitter);
        if (emitter->m_runtime_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_clone: create emitter data fail!");
            plugin_particle_obj_emitter_free(emitter);
            return NULL;
        }
        
        emitter->m_runtime_data->m_data = o->m_runtime_data->m_data;
    }

    ui_assert_vector_2_sane(&emitter->m_tile_size);
    
    return emitter;
}

void plugin_particle_obj_emitter_free(plugin_particle_obj_emitter_t emitter) {
    plugin_particle_obj_t obj = emitter->m_obj;
    plugin_particle_module_t module = obj->m_module;

    if (emitter->m_name) {
        mem_free(module->m_alloc, emitter->m_name);
        emitter->m_name = NULL;
    }

    if (emitter->m_addition_data.m_data) {
        plugin_particle_obj_emitter_set_addition_data(emitter, NULL);
        assert(emitter->m_addition_data.m_data == NULL);
    }

    if (emitter->m_use_state != plugin_particle_obj_emitter_use_state_suspend) {
        plugin_particle_obj_emitter_do_deactive(emitter);
    }

    while(!TAILQ_EMPTY(&emitter->m_particles)) {
        plugin_particle_obj_particle_free(TAILQ_FIRST(&emitter->m_particles));
    }
    assert(emitter->m_particle_count == 0);

    while(!TAILQ_EMPTY(&emitter->m_plugins)) {
        plugin_particle_obj_plugin_free(TAILQ_FIRST(&emitter->m_plugins));
    }

    while(!TAILQ_EMPTY(&emitter->m_binding_particles)) {
        plugin_particle_obj_emitter_binding_free(TAILQ_FIRST(&emitter->m_binding_particles));
    }

    while(!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_free(TAILQ_FIRST(&emitter->m_mods));
    }
    assert(TAILQ_EMPTY(&emitter->m_mods));

    if (emitter->m_runtime_data) {
        plugin_particle_obj_emitter_data_free(emitter->m_runtime_data);
        assert(emitter->m_runtime_data == NULL);
    }
    
    while(!TAILQ_EMPTY(&emitter->m_mod_datas)) {
        plugin_particle_obj_mod_data_free(TAILQ_FIRST(&emitter->m_mod_datas));
    }
    
    TAILQ_REMOVE(&obj->m_emitters, emitter, m_next);

    module->m_free_emitter_count++;
    TAILQ_INSERT_TAIL(&module->m_free_emitters, emitter, m_next);
}

void plugin_particle_obj_emitter_real_free(plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter) {
    module->m_free_emitter_count--;
    TAILQ_REMOVE(&module->m_free_emitters, emitter, m_next);
    mem_free(module->m_alloc, emitter);
}

plugin_particle_obj_emitter_t
plugin_particle_obj_emitter_find(plugin_particle_obj_t obj, const char * emitter_name) {
    plugin_particle_obj_emitter_t emitter;

    TAILQ_FOREACH(emitter, &obj->m_emitters, m_next) {
        const char * check_emitter_name = plugin_particle_obj_emitter_msg(emitter, plugin_particle_obj_emitter_data_r(emitter)->name_id);
        if (strcmp(check_emitter_name, emitter_name) == 0) return emitter;
    }

    return NULL;
}

plugin_particle_obj_t
plugin_particle_obj_emitter_obj(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_obj;
}

void plugin_particle_obj_emitter_set_lifecircle(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_lifecircle_t lifecircle) {
    emitter->m_lifecircle = lifecircle;
}

plugin_particle_obj_emitter_lifecircle_t
plugin_particle_obj_emitter_lifecircle(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_lifecircle;
}

dr_data_t plugin_particle_obj_emitter_addition_data(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_addition_data.m_meta ? &emitter->m_addition_data : NULL;
}

int plugin_particle_obj_emitter_set_addition_data(plugin_particle_obj_emitter_t emitter, dr_data_t data) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    struct dr_data old_value = emitter->m_addition_data;
    
    if (data == NULL) {
        emitter->m_addition_data.m_meta = NULL;
        emitter->m_addition_data.m_data = NULL;
        emitter->m_addition_data.m_size = 0;
    }
    else {
        emitter->m_addition_data.m_data = mem_alloc(module->m_alloc, data->m_size);
        if (emitter->m_addition_data.m_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_set_addition_data: alloc fail!");
            emitter->m_addition_data = old_value;
            return -1;
        }

        emitter->m_addition_data.m_meta = data->m_meta;
        emitter->m_addition_data.m_size = data->m_size;
        memcpy(emitter->m_addition_data.m_data, data->m_data, data->m_size);
    }

    if (old_value.m_data) {
        mem_free(module->m_alloc, old_value.m_data);
    }

    return 0;
}

plugin_particle_data_emitter_t plugin_particle_obj_emitter_static_data(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_static_data;
}

UI_PARTICLE_EMITTER const * plugin_particle_obj_emitter_data_r(plugin_particle_obj_emitter_t emitter) {
    if (emitter->m_runtime_data) {
        return &emitter->m_runtime_data->m_data;
    }

    if (emitter->m_static_data) {
        return &emitter->m_static_data->m_data;
    }

    return plugin_particle_obj_emitter_data_w(emitter);
}

UI_PARTICLE_EMITTER * plugin_particle_obj_emitter_data_w(plugin_particle_obj_emitter_t emitter) {
    if (emitter->m_runtime_data == NULL) {
        plugin_particle_module_t module = emitter->m_obj->m_module;
        
        emitter->m_runtime_data = plugin_particle_obj_emitter_data_create(emitter);
        if (emitter->m_runtime_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_data_w: create emitter data fail!");
            return NULL;
        }

        if (emitter->m_static_data) {
            emitter->m_runtime_data->m_data = emitter->m_static_data->m_data;
        }
        else {
            bzero(&emitter->m_runtime_data->m_data, sizeof(emitter->m_runtime_data->m_data));
            dr_meta_set_defaults(
                &emitter->m_runtime_data->m_data, sizeof(emitter->m_runtime_data->m_data),
                module->m_meta_particle_emitter, DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
        }
    }

    return &emitter->m_runtime_data->m_data;
}

const char * plugin_particle_obj_emitter_name(plugin_particle_obj_emitter_t emitter) {
    if (emitter->m_name) return emitter->m_name;

    if (emitter->m_static_data->m_data.name_id) {
        return plugin_particle_obj_emitter_msg(emitter, emitter->m_static_data->m_data.name_id);
    }

    return "";
}

int plugin_particle_obj_emitter_set_name(plugin_particle_obj_emitter_t emitter, const char * name) {
    plugin_particle_module_t module = emitter->m_obj->m_module;

    if (emitter->m_name) mem_free(module->m_alloc, emitter->m_name);
    
    emitter->m_name = cpe_str_mem_dup(module->m_alloc, name);
    if (emitter->m_name == NULL) {
        CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_set_name: dup str fail!");
        return -1;
    }
    
    return 0;
}

ui_transform_t plugin_particle_obj_emitter_transform(plugin_particle_obj_emitter_t emitter) {
    return &emitter->m_transform;
}

void plugin_particle_obj_emitter_set_transform(plugin_particle_obj_emitter_t emitter, ui_transform_t transform) {
    ui_transform_assert_sane(transform);
    
    emitter->m_transform = *transform;
    emitter->m_have_transform = ui_transform_cmp(transform, &UI_TRANSFORM_IDENTITY) == 0 ? 0 : 1;
}

ui_vector_2_t plugin_particle_obj_emitter_texture_size(plugin_particle_obj_emitter_t emitter) {
    return &emitter->m_texture_size;
}

ui_vector_2_t plugin_particle_obj_emitter_tile_size(plugin_particle_obj_emitter_t emitter) {
    return &emitter->m_tile_size;
}

ui_vector_2_t plugin_particle_obj_emitter_normalized_vtx_4(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_normalized_vtx;
}

uint8_t plugin_particle_obj_emitter_is_closing(plugin_particle_obj_emitter_t emitter) {
    if (emitter->m_use_state != plugin_particle_obj_emitter_use_state_active) return 0;
    return cpe_ba_get(emitter->m_flags, plugin_particle_obj_emitter_state_pendkill) ? 0 : 1;
}

void plugin_particle_obj_emitter_set_close(plugin_particle_obj_emitter_t emitter, uint8_t is_closing) {
    if (emitter->m_use_state != plugin_particle_obj_emitter_use_state_active) return;
    cpe_ba_set(emitter->m_flags, plugin_particle_obj_emitter_state_pendkill, is_closing ? cpe_ba_true : cpe_ba_false);
}

plugin_particle_obj_emitter_use_state_t
plugin_particle_obj_emitter_use_state(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_use_state;
}

void plugin_particle_obj_emitter_set_use_state(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_use_state_t target_state) {
    if (emitter->m_use_state == target_state) return;

    if(target_state == plugin_particle_obj_emitter_use_state_active) {
        if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_passive) {
            plugin_particle_obj_emitter_do_deactive(emitter);
            emitter->m_play_counter = 0;
            emitter->m_use_state = target_state;
            plugin_particle_obj_emitter_do_active(emitter);
        }
        else {
            assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend);
            emitter->m_play_counter = 0;
            emitter->m_use_state = target_state;
            plugin_particle_obj_emitter_do_active(emitter);
        }
    }
    else if(target_state == plugin_particle_obj_emitter_use_state_passive) {
        if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend) {
            emitter->m_play_counter = 0;
            emitter->m_use_state = target_state;
            plugin_particle_obj_emitter_do_active(emitter);
        }
        else {
            assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_active);
            emitter->m_play_counter = 0;
            plugin_particle_obj_emitter_do_deactive(emitter);
            emitter->m_use_state = target_state;
            plugin_particle_obj_emitter_do_active(emitter);
        }
    }
    else {
        assert(target_state == plugin_particle_obj_emitter_use_state_suspend);

        if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_passive) {
            plugin_particle_obj_emitter_do_deactive(emitter);
            emitter->m_use_state = target_state;
        }
        else {
            assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_active);
            plugin_particle_obj_emitter_do_deactive(emitter);
            emitter->m_use_state = target_state;
        }

        while(!TAILQ_EMPTY(&emitter->m_particles)) {
            plugin_particle_obj_particle_free(TAILQ_FIRST(&emitter->m_particles));
        }
        assert(emitter->m_particle_count == 0);
    }
}

float plugin_particle_obj_emitter_real_spawn_rate(plugin_particle_obj_emitter_t emitter) {
    float rate;
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    float addition_rate = 0.0f;
    if (emitter_data->duration > 0.0f && emitter_data->particle_repeat_times > 0) {
        if (emitter_data->particle_repeat_times > 0) {
            addition_rate =
                (((float)(emitter_data->min_extra_brust + emitter_data->max_extra_brust)) / 2.0f)
                /  ((emitter_data->duration / emitter_data->time_scale) * emitter_data->particle_repeat_times);
        }
    }

    rate = addition_rate + emitter_data->spawn_rate;
    /* printf( */
    /*     "xxxxxxx: calc emitter %s: addition_rate=%f, base_rate=%f, result=%f\n", */
    /*     plugin_particle_obj_emitter_name(emitter), addition_rate, emitter_data->spawn_rate, rate); */

    if (emitter_data->max_amount > 0) {
        UI_PARTICLE_MOD const * lifetime_mod;

        if ((lifetime_mod = plugin_particle_obj_emitter_find_mod_r(emitter, ui_particle_mod_type_lifetime_seed))) {
            UI_PARTICLE_MOD_LIFETIME_SEED const * seed = &lifetime_mod->data.lifetime_seed;
            float life_time = (seed->min_base_time + seed->max_base_time) * 0.5f / emitter_data->time_scale;
            float max_rate = (float)emitter_data->max_amount / life_time;

            if (max_rate < rate) {
                /* printf( */
                /*     "xxxxxxx: calc emitter %s: limit by max count: life-time=%f, max-count=%d, max-rate=%f\n", */
                /*     plugin_particle_obj_emitter_name(emitter), life_time, emitter_data->max_amount, max_rate); */
                rate = max_rate;
            }
        }
    }
    
    return rate;
}

int plugin_particle_obj_emitter_reset(plugin_particle_obj_emitter_t emitter) {
    plugin_particle_module_t module = emitter->m_obj->m_module;

    while(!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_free(TAILQ_FIRST(&emitter->m_mods));
    }

    while(!TAILQ_EMPTY(&emitter->m_mod_datas)) {
        plugin_particle_obj_mod_data_free(TAILQ_FIRST(&emitter->m_mod_datas));        
    }

    if(emitter->m_static_data) {
        plugin_particle_data_mod_t mod;
    
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);
            if (g_mod_defs[mod->m_data.type].data_init) {
                plugin_particle_obj_mod_data_t mod_data = plugin_particle_obj_mod_data_create(emitter);

                if (mod_data == NULL) {
                    CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_reset: create mod data fail!");
                    return -1;
                }
                
                g_mod_defs[mod->m_data.type].data_init(&mod->m_data, mod_data);
            }
        }
    }
    
    if (emitter->m_runtime_data) {
        if (emitter->m_static_data) {
            const char * texture_path = plugin_particle_data_emitter_msg(emitter->m_static_data, emitter->m_runtime_data->m_data.texture_id);
            if (emitter->m_texture == NULL || strcmp(ui_cache_res_path(emitter->m_texture), texture_path) != 0) {
                emitter->m_texture = ui_cache_res_find_by_path(emitter->m_obj->m_module->m_cache_mgr, texture_path);
            }
        }
        else {
            emitter->m_texture = NULL;
        }

        plugin_particle_obj_emitter_data_free(emitter->m_runtime_data);
        assert(emitter->m_runtime_data == NULL);
    }
    plugin_particle_obj_emitter_texture_cache_init(emitter);

    return 0;
}

void plugin_particle_obj_emitter_texture_cache_init(plugin_particle_obj_emitter_t emitter) {
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    UI_PARTICLE_MOD const * texture_mod = NULL;

    /*计算贴图方式 */
    emitter->m_texture_mode = plugin_particle_obj_emitter_texture_mode_basic;
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            if (mod->m_data.type == ui_particle_mod_type_texcoord_flipbook_uv
                || mod->m_data.type == ui_particle_mod_type_texcoord_scroll_anim
                || mod->m_data.type == ui_particle_mod_type_texcoord_tile_sub_tex)
            {
                texture_mod = &mod->m_data;
                break;
            }
        }
    }
    else if (emitter->m_static_data) {
        plugin_particle_data_mod_t mod;
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            if (mod->m_data.type == ui_particle_mod_type_texcoord_flipbook_uv
                || mod->m_data.type == ui_particle_mod_type_texcoord_scroll_anim
                || mod->m_data.type == ui_particle_mod_type_texcoord_tile_sub_tex)
            {
                texture_mod = &mod->m_data;
                break;
            }
        }
    }

    if (texture_mod) {
        emitter->m_texture_mode =
            texture_mod->type == ui_particle_mod_type_texcoord_scroll_anim
            ? plugin_particle_obj_emitter_texture_mode_scroll/*plugin_particle_obj_emitter_texture_mode_scroll*/
            : plugin_particle_obj_emitter_texture_mode_tiled;
    }

    /*计算贴图数据 */
    if (emitter_data && emitter->m_texture) {
        uint8_t hasAtlas;

        emitter->m_texture_origin_size.x = emitter_data->texture_w ? emitter_data->texture_w : ui_cache_texture_width(emitter->m_texture);
        emitter->m_texture_origin_size.y = emitter_data->texture_h ? emitter_data->texture_h : ui_cache_texture_height(emitter->m_texture);

        emitter->m_texture_size.x = emitter_data->atlas_w ? emitter_data->atlas_w : emitter->m_texture_origin_size.x;
        emitter->m_texture_size.y = emitter_data->atlas_h ? emitter_data->atlas_h : emitter->m_texture_origin_size.y;

        hasAtlas = ( emitter_data->atlas_x!=0 || emitter_data->atlas_y!=0 || emitter_data->atlas_w!=0 || emitter_data->atlas_h!=0 );
        if (!hasAtlas) {
            emitter->m_tex_coord_start.x = 0.0f;
            emitter->m_tex_coord_start.y = 0.0f;
            emitter->m_tex_coord_scale.x = 1.0f;
            emitter->m_tex_coord_scale.y = 1.0f;
        }
        else {
            float inv_w = 1.0f / (float)ui_cache_texture_width(emitter->m_texture);
            float inv_h = 1.0f / (float)ui_cache_texture_height(emitter->m_texture);

            emitter->m_tex_coord_start.x = ((uint32_t)emitter_data->atlas_x % ui_cache_texture_width(emitter->m_texture))  * inv_w;
            emitter->m_tex_coord_scale.x = cpe_min(emitter->m_texture_size.x, emitter->m_texture_origin_size.x) * inv_w;
            emitter->m_tex_coord_start.y = ((uint32_t)emitter_data->atlas_y % ui_cache_texture_height(emitter->m_texture)) * inv_h;
            emitter->m_tex_coord_scale.y = cpe_min(emitter->m_texture_size.y, emitter->m_texture_origin_size.y) * inv_h;
            assert(emitter->m_tex_coord_start.x >= 0.0f && emitter->m_tex_coord_start.x <= 1.0f);
            assert(emitter->m_tex_coord_start.y >= 0.0f && emitter->m_tex_coord_start.y <= 1.0f);
            assert(emitter->m_tex_coord_scale.x >= 0.0f && emitter->m_tex_coord_scale.x <= 1.0f);
            assert(emitter->m_tex_coord_scale.y >= 0.0f && emitter->m_tex_coord_scale.y <= 1.0f);

        }

        emitter->m_tile_size = emitter->m_texture_size;
        if (emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_tiled) {
            if(emitter_data->tiling_u > 0) {
                emitter->m_tile_size.x /= emitter_data->tiling_u;
            }
            
            if(emitter_data->tiling_v > 0) {
                emitter->m_tile_size.y /= emitter_data->tiling_v;
            }
        }
    }
    else {
        emitter->m_tex_coord_start = UI_VECTOR_2_ZERO;
        emitter->m_tex_coord_scale = UI_VECTOR_2_ZERO;
        emitter->m_texture_origin_size = UI_VECTOR_2_ZERO;
        emitter->m_texture_size = UI_VECTOR_2_ZERO;
        emitter->m_tile_size = UI_VECTOR_2_ZERO;
    }
}

int plugin_particle_obj_emitter_spawn_at_world(plugin_particle_obj_emitter_t emitter, ui_transform_t trans, uint16_t gen_count) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    UI_PARTICLE_EMITTER const * emitter_data;
    ui_transform trans_buf;
    ui_transform_t effect_trans;
    uint32_t i;
    int rv = 0;
    
    if (plugin_particle_obj_emitter_use_state(emitter) == plugin_particle_obj_emitter_use_state_suspend) {
        CPE_ERROR(module->m_em, "emitter spawn: emitter %s not in use!", plugin_particle_obj_emitter_name(emitter));
        return -1;
    }

    emitter_data = plugin_particle_obj_emitter_data_r(emitter);

    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
        ui_transform_t world_t;
        
        world_t = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj));
        if (world_t) {
            ui_transform world_t_r;

            ui_transform_reverse(&world_t_r, world_t);
            
            trans_buf = *trans;
            ui_transform_adj_by_parent(&trans_buf, &world_t_r);
            effect_trans = &trans_buf;
        }
        else {
            effect_trans = trans;
        }
    }
    else {
        effect_trans = trans;
    }

    if (gen_count == 0) {
        gen_count = emitter_data->min_extra_brust;
        if (emitter_data->max_extra_brust > emitter_data->min_extra_brust) {
            gen_count += cpe_rand_dft(emitter_data->max_extra_brust - emitter_data->min_extra_brust);
        }
    }

    //printf("gen count = %d, pos=(%f,%f), adj-pos=(%f,%f)\n", gen_count, pos->x, pos->y, adj_pos.x, adj_pos.y);

    for(i = 0; i < gen_count; ++i) {
        if (plugin_particle_obj_particle_create_at(emitter, effect_trans) == NULL) {
            rv = -1;
        }
    }

    return rv;
}

int plugin_particle_obj_emitter_spawn_at_local(plugin_particle_obj_emitter_t emitter, ui_transform_t trans, uint16_t gen_count) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    UI_PARTICLE_EMITTER const * emitter_data;
    ui_transform trans_buf;
    ui_transform_t effect_trans;
    uint32_t i;
    int rv = 0;
    
    if (plugin_particle_obj_emitter_use_state(emitter) == plugin_particle_obj_emitter_use_state_suspend) {
        CPE_ERROR(module->m_em, "emitter spawn: emitter %s not in use!", plugin_particle_obj_emitter_name(emitter));
        return -1;
    }
    
    emitter_data = plugin_particle_obj_emitter_data_r(emitter);

    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_WORLD) {
        ui_transform_t world_t = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj));

        if (trans && world_t) {
            trans_buf = *trans;
            ui_transform_adj_by_parent(&trans_buf, world_t);
            effect_trans = &trans_buf;
        }
        else if (world_t) {
            effect_trans = world_t;
        }
        else {
            effect_trans = trans;
        }
    }
    else {
        effect_trans = trans;
    }
    
    if (gen_count == 0) {
        gen_count = emitter_data->min_extra_brust;
        if (emitter_data->max_extra_brust > emitter_data->min_extra_brust) {
            gen_count += cpe_rand_dft(emitter_data->max_extra_brust - emitter_data->min_extra_brust);
        }
    }

    //printf("gen count = %d, pos=(%f,%f), adj-pos=(%f,%f)\n", gen_count, pos->x, pos->y, adj_pos.x, adj_pos.y);

    for(i = 0; i < gen_count; ++i) {
        if (plugin_particle_obj_particle_create_at(emitter, effect_trans) == NULL) {
            rv = -1;
        }
    }

    return rv;
}

int plugin_particle_obj_emitter_spawn(plugin_particle_obj_emitter_t emitter, uint16_t gen_count) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    UI_PARTICLE_EMITTER const * emitter_data;
    uint32_t i;
    int rv = 0;
    
    if (plugin_particle_obj_emitter_use_state(emitter) == plugin_particle_obj_emitter_use_state_suspend) {
        CPE_ERROR(module->m_em, "emitter spawn: emitter %s not in use!", plugin_particle_obj_emitter_name(emitter));
        return -1;
    }
    
    emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    
    if (gen_count == 0) {
        gen_count = emitter_data->min_extra_brust;
        if (emitter_data->max_extra_brust > emitter_data->min_extra_brust) {
            gen_count += cpe_rand_dft(emitter_data->max_extra_brust - emitter_data->min_extra_brust);
        }
    }

    for(i = 0; i < gen_count; ++i) {
        if (plugin_particle_obj_particle_create(emitter) == NULL) {
            rv = -1;
        }
    }

    return rv;
}

int8_t plugin_particle_obj_emitter_mod_index(plugin_particle_obj_emitter_t emitter, UI_PARTICLE_MOD const * mod) {
    int8_t r = 0;
    
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t check_mod;
            
        TAILQ_FOREACH(check_mod, &emitter->m_mods, m_next) {
            if (&check_mod->m_data == mod) return r;
            ++r;
        }
    }
    else if (emitter->m_static_data) {
        plugin_particle_data_mod_t check_mod;
            
        TAILQ_FOREACH(check_mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            if (&check_mod->m_data == mod) return r;
            ++r;
        }
    }

    return -1;
}

void plugin_particle_obj_emitter_do_active(plugin_particle_obj_emitter_t emitter) {
    assert(emitter->m_use_state != plugin_particle_obj_emitter_use_state_suspend);
    cpe_ba_set(emitter->m_flags, plugin_particle_obj_emitter_state_waitstop, cpe_ba_false);
    cpe_ba_set(emitter->m_flags, plugin_particle_obj_emitter_state_pendkill, cpe_ba_false);

    /*初始化所有mod */
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t mod;
        plugin_particle_obj_mod_data_t cur_mod_data;
    
        cur_mod_data = TAILQ_FIRST(&emitter->m_mod_datas);
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);
            if (g_mod_defs[mod->m_data.type].data_init) {
                g_mod_defs[mod->m_data.type].data_init(&mod->m_data, cur_mod_data);
                cur_mod_data = TAILQ_NEXT(cur_mod_data, m_next);
            }
        }
    }
    else {
        plugin_particle_data_mod_t mod;
        plugin_particle_obj_mod_data_t cur_mod_data;
    
        cur_mod_data = TAILQ_FIRST(&emitter->m_mod_datas);
        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);
            if (g_mod_defs[mod->m_data.type].data_init) {
                g_mod_defs[mod->m_data.type].data_init(&mod->m_data, cur_mod_data);
                cur_mod_data = TAILQ_NEXT(cur_mod_data, m_next);
            }
        }
    }
    
    plugin_particle_obj_emitter_runtime_init(&emitter->m_runtime);
    emitter->m_play_counter++;

    if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_active) {
        emitter->m_obj->m_active_emitter_count++;
        emitter->m_obj->m_module->m_active_emitter_count++;
    }
}

void plugin_particle_obj_emitter_do_deactive(plugin_particle_obj_emitter_t emitter) {
    UI_PARTICLE_EMITTER const * emitter_data;

    assert(emitter->m_use_state != plugin_particle_obj_emitter_use_state_suspend);

    if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_active) {
        emitter->m_obj->m_active_emitter_count--;
        emitter->m_obj->m_module->m_active_emitter_count--;

        emitter_data = plugin_particle_obj_emitter_data_r(emitter);
        if (emitter_data->on_emitter_end_id) {
            ui_runtime_render_obj_send_event(
                ui_runtime_render_obj_from_data(emitter->m_obj),
                plugin_particle_obj_emitter_msg(emitter, emitter_data->on_emitter_end_id));
        }
    }
}

float plugin_particle_obj_emitter_time_scale(plugin_particle_obj_emitter_t emitter) {
    return emitter->m_time_scale;
}

void plugin_particle_obj_emitter_set_tile_scale(plugin_particle_obj_emitter_t emitter, float time_scale) {
    emitter->m_time_scale = time_scale;
}

static void plugin_particle_obj_emitter_calc_normalized_vtx(
    plugin_particle_obj_emitter_t emitter, UI_PARTICLE_EMITTER const * emitter_data)
{
    ui_vector_2 bias;
    switch(emitter_data->origin) {
    case ui_pos_policy_top_left:
        bias.x = 0.5f, bias.y = 0.5f;
        break;
    case ui_pos_policy_top_center:
        bias.x = 0.0f, bias.y = 0.5f;
        break;
    case ui_pos_policy_top_right:
        bias.x = -0.5f, bias.y = 0.5f;
        break;
    case ui_pos_policy_center_left:
        bias.x = 0.5f, bias.y = 0.0f;
        break;
    case ui_pos_policy_center_right:
        bias.x = -0.5f, bias.y = 0.0f;
        break;
    case ui_pos_policy_bottom_left:
        bias.x = 0.5f, bias.y = -0.5f;
        break;
    case ui_pos_policy_bottom_center:
        bias.x = 0.0f, bias.y = -0.5f;
        break;
    case ui_pos_policy_bottom_right:
        bias.x = -0.5f, bias.y = -0.5f;
        break;
    default:
        bias.x = -0.0f, bias.y = -0.0f;
        break;
    }

    emitter->m_normalized_vtx[0].x = -0.5f + bias.x;
    emitter->m_normalized_vtx[0].y =  0.5f + bias.y;

    emitter->m_normalized_vtx[1].x = -0.5f + bias.x;
    emitter->m_normalized_vtx[1].y = -0.5f + bias.y;

    emitter->m_normalized_vtx[2].x =  0.5f + bias.x;
    emitter->m_normalized_vtx[2].y = -0.5f + bias.y;

    emitter->m_normalized_vtx[3].x =  0.5f + bias.x;
    emitter->m_normalized_vtx[3].y =  0.5f + bias.y;
}

static plugin_particle_obj_emitter_t plugin_particle_obj_emitter_next(struct plugin_particle_obj_emitter_it * it) {
    plugin_particle_obj_emitter_t * data = (plugin_particle_obj_emitter_t *)(it->m_data);
    plugin_particle_obj_emitter_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_particle_obj_emitters(plugin_particle_obj_emitter_it_t emitter_it, plugin_particle_obj_t obj) {
    *(plugin_particle_obj_emitter_t *)(emitter_it->m_data) = TAILQ_FIRST(&obj->m_emitters);
    emitter_it->next = plugin_particle_obj_emitter_next;
}

void plugin_particle_obj_emitter_runtime_init(plugin_particle_obj_emitter_runtime_t runtime) {
    runtime->m_elapsed_time = 0.0f;
    runtime->m_emit_counter = 0.0f;
	runtime->m_is_first_shoot = 1;
    runtime->m_is_first_frame = 1;
}

float plugin_particle_obj_emitter_curve_sample(plugin_particle_obj_emitter_t emitter, uint16_t curve_id, float key) {
    plugin_particle_data_curve_t curve;
    
    if (curve_id == 0) return 0.0f;
    
    curve = plugin_particle_data_curve_find(emitter->m_static_data->m_particle, curve_id);
    return curve ? plugin_particle_data_curve_sample(curve, key) : 0.0f;
}

const char * plugin_particle_obj_emitter_msg(plugin_particle_obj_emitter_t emitter, uint32_t msg) {
    return emitter->m_static_data ? plugin_particle_data_emitter_msg(emitter->m_static_data, msg) : "";
}

const char * plugin_particle_obj_emitter_texture(plugin_particle_obj_emitter_t emitter) {
    return plugin_particle_obj_emitter_msg(emitter, plugin_particle_obj_emitter_data_r(emitter)->texture_id);
}

const char * plugin_particle_obj_emitter_user_text(plugin_particle_obj_emitter_t emitter) {
    return plugin_particle_obj_emitter_msg(emitter, plugin_particle_obj_emitter_data_r(emitter)->user_text_id);
}

const char * plugin_particle_obj_emitter_group(plugin_particle_obj_emitter_t emitter) {
    return plugin_particle_obj_emitter_msg(emitter, plugin_particle_obj_emitter_data_r(emitter)->group_id);
}

const char * plugin_particle_obj_emitter_dead_anim(plugin_particle_obj_emitter_t emitter) {
    return plugin_particle_obj_emitter_msg(emitter, plugin_particle_obj_emitter_data_r(emitter)->dead_anim_id);
}
