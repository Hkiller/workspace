#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src_res.h"
#include "render/model/ui_data_evt_collector.h"
#include "plugin_particle_data_i.h"

plugin_particle_data_t plugin_particle_data_create(plugin_particle_module_t module, ui_data_src_t src) {
    plugin_particle_data_t particle;

    if (ui_data_src_type(src) != ui_data_src_type_particle) {
        CPE_ERROR(
            module->m_em, "create particle at %s: src not particle!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create particle at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    particle = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_data));
    if (particle == NULL) {
        CPE_ERROR(
            module->m_em, "create particle at %s: alloc fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (cpe_hash_table_init(
            &particle->m_curves,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_particle_data_curve_hash,
            (cpe_hash_eq_t) plugin_particle_data_curve_eq,
            CPE_HASH_OBJ2ENTRY(plugin_particle_data_curve, m_hh),
            -1) != 0)
    {
        CPE_ERROR(
            module->m_em, "create particle at %s: init hash fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        mem_free(module->m_alloc, particle);
        return NULL;
    }
    
    particle->m_module = module;
    particle->m_src = src;
    particle->m_curve_max_id = 0;
    particle->m_emitter_count = 0;
    TAILQ_INIT(&particle->m_emitters);

    ui_data_src_set_product(src, particle);
    return particle;
}

void plugin_particle_data_free(plugin_particle_data_t particle) {
    plugin_particle_module_t module = particle->m_module;

    assert(ui_data_src_product(particle->m_src) == particle);

    while(!TAILQ_EMPTY(&particle->m_emitters)) {
        plugin_particle_data_emitter_free(TAILQ_FIRST(&particle->m_emitters));
    }
    assert(particle->m_emitter_count == 0);

    plugin_particle_data_curve_free_all(particle);
    cpe_hash_table_fini(&particle->m_curves);

    mem_free(module->m_alloc, particle);
}

uint32_t plugin_particle_data_emitter_count(plugin_particle_data_t particle) {
    return particle->m_emitter_count;
}

static int plugin_particle_data_collect_res_from_evt(
    plugin_particle_module_t module, ui_data_src_t src, plugin_particle_data_emitter_t emitter, UI_PARTICLE_EMITTER * particle_data, uint32_t event_def_id)
{
    const char * sep;
    const char * event_def = plugin_particle_data_emitter_msg(emitter, event_def_id);

    if (event_def[0] == '[') {
        sep = strchr(event_def + 1, ']');
        if (sep == NULL) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, NULL);
            CPE_ERROR(
                module->m_em, "collect: particle %s emitter %s: event %s format error!",
                ui_data_src_path_dump(&buffer, src),
                plugin_particle_data_emitter_msg(emitter, particle_data->name_id),
                event_def);
            mem_buffer_clear(&buffer);
            return -1;
        }

        event_def = cpe_str_trim_head((char*)sep + 1);
    }

    sep = strchr(event_def, ':');
    if (sep == NULL) {
        if (ui_data_src_collect_res_from_event(src, event_def, "") != 0) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, NULL);
            CPE_ERROR(
                module->m_em, "collect: particle %s emitter %s: event %s process fail!",
                ui_data_src_path_dump(&buffer, src),
                plugin_particle_data_emitter_msg(emitter, particle_data->name_id),
                event_def);
            mem_buffer_clear(&buffer);
            return -1;
        }
    }
    else {
        char event_name[64];
        const char * event_value = cpe_str_trim_head((char*)sep + 1);
        
        cpe_str_dup_range_trim(event_name, sizeof(event_name), event_def, sep);
        
        if (ui_data_src_collect_res_from_event(src, event_name, event_value) != 0) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, NULL);
            CPE_ERROR(
                module->m_em, "collect: particle %s emitter %s: event %s:%s process fail!",
                ui_data_src_path_dump(&buffer, src),
                plugin_particle_data_emitter_msg(emitter, particle_data->name_id),
                event_name, event_value);
            mem_buffer_clear(&buffer);
            return -1;
        }
    }
    
    return 0;
}

int plugin_particle_data_update_using(ui_data_src_t src) {
    plugin_particle_module_t module;
    plugin_particle_data_t particle;
    plugin_particle_data_emitter_t emitter;
    uint32_t emitter_pos;
    int rv = 0;
    
    particle = ui_data_src_product(src);
    assert(particle);
    module = particle->m_module;
    
    emitter_pos = 0;

    TAILQ_FOREACH(emitter, &particle->m_emitters, m_next_for_particle) {
        UI_PARTICLE_EMITTER * particle_data = &emitter->m_data;

        if (!particle_data->is_render) continue;
        
        if (particle_data->texture_id == 0) {
            struct mem_buffer buffer;
            mem_buffer_init(&buffer, NULL);
            CPE_ERROR(
                module->m_em, "collect: particle %s emitter %d res path empty!",
                ui_data_src_path_dump(&buffer, src), emitter_pos);
            mem_buffer_clear(&buffer);
            rv = -1;
        }
        else if (ui_data_src_res_create_by_path(src, plugin_particle_data_emitter_msg(emitter, particle_data->texture_id)) == NULL) {
            rv = -1;
        }

        if (particle_data->on_emitter_begin_id
            && plugin_particle_data_collect_res_from_evt(module, src, emitter, particle_data, particle_data->on_emitter_begin_id) != 0)
        {
            rv = -1;
        }

        if (particle_data->on_emitter_end_id
            && plugin_particle_data_collect_res_from_evt(module, src, emitter, particle_data, particle_data->on_emitter_end_id) != 0)
        {
            rv = -1;
        }
        
        if (particle_data->on_particle_begin_id
            && plugin_particle_data_collect_res_from_evt(module, src, emitter, particle_data, particle_data->on_particle_begin_id) != 0)
        {
            rv = -1;
        }

        if (particle_data->on_particle_end_id
            && plugin_particle_data_collect_res_from_evt(module, src, emitter, particle_data, particle_data->on_particle_end_id) != 0)
        {
            rv = -1;
        }

        emitter_pos++;
    }

    return rv;
}
