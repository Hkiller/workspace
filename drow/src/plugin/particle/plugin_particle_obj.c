#include <assert.h>
#include "render/model/ui_data_src.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin_particle_obj_i.h"
#include "plugin_particle_data_i.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_particle_i.h"

plugin_particle_obj_t plugin_particle_obj_create(ui_runtime_module_t module, const char * name, plugin_particle_data_t particle_data) {
    ui_runtime_render_obj_t obj;
    ui_runtime_render_obj_meta_t obj_meta;
    plugin_particle_module_t particle_module;
    plugin_particle_obj_t particle_obj;
    plugin_particle_data_emitter_t emitter_data;

    obj = ui_runtime_render_obj_create_by_type(module, name, "particle");
    if (obj == NULL) return NULL;

    obj_meta = ui_runtime_render_obj_meta(obj);

    particle_module = ui_runtime_render_obj_meta_ctx(obj_meta);
    particle_obj = ui_runtime_render_obj_data(obj);

    TAILQ_FOREACH(emitter_data, &particle_data->m_emitters, m_next_for_particle) {
        plugin_particle_obj_emitter_t emitter = plugin_particle_obj_emitter_create(particle_obj, emitter_data);
        if (emitter == NULL) {
            CPE_ERROR(
                particle_module->m_em, "%s: particle obj init: load emitter fail!",
                plugin_particle_module_name(particle_module));
            goto LOAD_ERROR;
        }
    }

    return particle_obj;

LOAD_ERROR:
    ui_runtime_render_obj_free(obj);
    return NULL;
}

int plugin_particle_obj_init(void * ctx, ui_runtime_render_obj_t obj) {
    plugin_particle_module_t module = ctx;
    plugin_particle_obj_t particle_obj = ui_runtime_render_obj_data(obj);

    particle_obj->m_module = module;
    particle_obj->m_data = NULL;
    particle_obj->m_particle_count = 0;
    particle_obj->m_active_emitter_count = 0;
    particle_obj->m_enable = 1;
    TAILQ_INIT(&particle_obj->m_emitters);

    return 0;
}

int plugin_particle_obj_set(void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url) {
    plugin_particle_module_t module = ctx;
    plugin_particle_obj_t particle_obj = ui_runtime_render_obj_data(obj);
    ui_data_src_t particle_src;
    plugin_particle_data_t particle_data;
    UI_OBJECT_URL_DATA_PARTICAL const * particle_url = &obj_url->data.particle;
    
    particle_src = ui_runtime_module_find_src(module->m_runtime, &particle_url->src, ui_data_src_type_particle);
    if (particle_src == NULL) {
        CPE_ERROR(module->m_em, "%s: particle obj init: find src fail!", plugin_particle_module_name(module));
        return -1;
    }

    particle_data = ui_data_src_product(particle_src);
    if (particle_data == NULL) {
        CPE_ERROR(module->m_em, "%s: particle obj init: data not loaded!", plugin_particle_module_name(module));
        return -1;
    }

    if (plugin_particle_obj_set_data(particle_obj, particle_data) != 0) return -1;

    ui_runtime_render_obj_set_src(obj, particle_src);
        
    return 0;
}

void plugin_particle_obj_free(void * ctx, ui_runtime_render_obj_t obj) {
    plugin_particle_obj_t particle_obj = ui_runtime_render_obj_data(obj);

    while(!TAILQ_EMPTY(&particle_obj->m_emitters)) {
        plugin_particle_obj_emitter_free(TAILQ_FIRST(&particle_obj->m_emitters));
    }
    
    assert(particle_obj->m_particle_count == 0);
    assert(particle_obj->m_active_emitter_count == 0);
}

plugin_particle_data_t plugin_particle_obj_data(plugin_particle_obj_t obj) {
    return obj->m_data;
}

int plugin_particle_obj_set_data(plugin_particle_obj_t particle_obj, plugin_particle_data_t particle_data) {
    plugin_particle_data_emitter_t emitter_data;
    int rv = 0;

    assert(particle_data);
    
    if (particle_obj->m_data != particle_data) {
        while(!TAILQ_EMPTY(&particle_obj->m_emitters)) {
            plugin_particle_obj_emitter_free(TAILQ_FIRST(&particle_obj->m_emitters));
        }
    
        particle_obj->m_data = particle_data;

        TAILQ_FOREACH(emitter_data, &particle_data->m_emitters, m_next_for_particle) {
            plugin_particle_obj_emitter_t emitter = plugin_particle_obj_emitter_create(particle_obj, emitter_data);
            if (emitter == NULL) {
                CPE_ERROR(
                    particle_obj->m_module->m_em, "%s: particle obj init: load emitter fail!",
                    plugin_particle_module_name(particle_obj->m_module));
                rv = -1;
            }
        }
    }

    return rv;
}

uint8_t plugin_particle_obj_is_playing(void * ctx, ui_runtime_render_obj_t obj) {
    plugin_particle_obj_t particle_obj = ui_runtime_render_obj_data(obj);
    return (particle_obj->m_active_emitter_count > 0 || particle_obj->m_particle_count > 0) ? 1 : 0;
}

uint32_t plugin_particle_obj_particle_count(plugin_particle_obj_t obj) {
    return obj->m_particle_count;
}

uint32_t plugin_particle_obj_active_emitter_count(plugin_particle_obj_t obj) {
    return obj->m_active_emitter_count;
}

uint8_t plugin_particle_obj_is_enable(plugin_particle_obj_t obj) {
    return obj->m_enable;
}

void plugin_particle_obj_set_enable(plugin_particle_obj_t obj, uint8_t is_enable) {
    obj->m_enable = is_enable;
}
