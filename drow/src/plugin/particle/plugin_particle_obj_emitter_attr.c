#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data_value.h"
#include "render/cache/ui_cache_res.h"
#include "plugin_particle_obj_emitter_i.h"

static int plugin_particle_obj_emitter_set_emitter_attr(
    plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter, const char * attr_name, dr_value_t input_value)
{
    struct dr_data data;
    struct dr_value obj_value;
    
    data.m_meta = module->m_meta_particle_emitter;
    data.m_data = plugin_particle_obj_emitter_data_w(emitter);
    data.m_size = sizeof(UI_PARTICLE_EMITTER);

    if (dr_value_find_by_path(&data, attr_name, &obj_value) == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_set_attr: emitter %s: no attr %s",
            plugin_particle_obj_emitter_name(emitter), attr_name);
        return -1;
    }

    if (dr_value_set_from_value(&obj_value, input_value, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_set_attr: emitter %s: attr %s set value fail",
            plugin_particle_obj_emitter_name(emitter), attr_name);
        return -1;
    }


    if (strcmp(attr_name, "texture") == 0) {
        emitter->m_texture = ui_cache_res_find_by_path(module->m_cache_mgr, obj_value.m_data);
        plugin_particle_obj_emitter_texture_cache_init(emitter);
    }
    else if (strcmp(attr_name, "origin") == 0
             || cpe_str_start_with(attr_name, "atlas_"))
    {
        plugin_particle_obj_emitter_texture_cache_init(emitter);
    }
    
    return 0;
}

static int plugin_particle_obj_emitter_get_emitter_attr(
    plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter, const char * attr_name, dr_value_t obj_value)
{
    struct dr_data data;

    data.m_meta = module->m_meta_particle_emitter;
    data.m_data = (void*)plugin_particle_obj_emitter_data_r(emitter);
    data.m_size = sizeof(UI_PARTICLE_EMITTER);

    if (dr_value_find_by_path(&data, attr_name, obj_value) == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_get_attr: emitter %s: no attr %s",
            plugin_particle_obj_emitter_name(emitter), attr_name);
        return -1;
    }
    
    return 0;
}

static int plugin_particle_obj_emitter_set_mod_attr(
    plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter,
    const char * mod_name, uint8_t mod_type, LPDRMETA mod_meta, 
    const char * attr_name, dr_value_t input_value)
{
    struct dr_data data;
    struct dr_value obj_value;
    UI_PARTICLE_MOD * mod_data;

    mod_data = plugin_particle_obj_emitter_check_create_mod(emitter, mod_type);
    if (mod_data == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_get_attr: emitter %s: mod %s: check create fail!",
            plugin_particle_obj_emitter_name(emitter), mod_name);
        return -1;
    }

    data.m_meta = mod_meta;
    data.m_data = (void*)&mod_data->data;
    data.m_size = sizeof(UI_PARTICLE_MOD_DATA);

    if (dr_value_find_by_path(&data, attr_name, &obj_value) == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_get_attr: emitter %s: mod %s: no attr %s",
            plugin_particle_obj_emitter_name(emitter), mod_name, attr_name);
        return -1;
    }

    if (dr_value_set_from_value(&obj_value, input_value, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_set_attr: emitter %s: mod %s: attr %s set value fail",
            plugin_particle_obj_emitter_name(emitter), mod_name, attr_name);
        return -1;
    }
    
    return 0;
}

static int plugin_particle_obj_emitter_get_mod_attr(
    plugin_particle_module_t module, plugin_particle_obj_emitter_t emitter,
    const char * mod_name, uint8_t mod_type, LPDRMETA mod_meta,
    const char * attr_name, dr_value_t obj_value)
{
    struct dr_data data;
    UI_PARTICLE_MOD const * mod_data;

    mod_data = plugin_particle_obj_emitter_find_mod_r(emitter, mod_type);
    if (mod_data == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_get_attr: emitter %s: mod %s: mod not exist",
            plugin_particle_obj_emitter_name(emitter), mod_name);
        return -1;
    }
            
    data.m_meta = mod_meta;
    data.m_data = (void*)&mod_data->data;
    data.m_size = sizeof(UI_PARTICLE_MOD_DATA);

    if (dr_value_find_by_path(&data, attr_name, obj_value) == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_get_attr: emitter %s: mod %s: no attr %s",
            plugin_particle_obj_emitter_name(emitter), mod_name, attr_name);
        return -1;
    }

    return 0;
}

int plugin_particle_obj_emitter_set_attr(plugin_particle_obj_emitter_t emitter, const char * attr_name, dr_value_t input_value) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    const char * sep = strchr(attr_name, '.');

    if (sep == NULL) {
        return plugin_particle_obj_emitter_set_emitter_attr(module, emitter, attr_name, input_value);
    }
    else {
        char * mod_name;
        LPDRMETAENTRY mod_entry;
        
        mem_buffer_clear_data(&module->m_dump_buffer);
                              
        mod_name = mem_buffer_strdup_len(&module->m_dump_buffer, attr_name, sep - attr_name);
        attr_name = sep + 1;
        
        mod_entry = dr_meta_find_entry_by_name(module->m_meta_particle_mod_data, mod_name);
        if (mod_entry == NULL) {
            return plugin_particle_obj_emitter_set_emitter_attr(module, emitter, attr_name, input_value);
        }
        else {
            return plugin_particle_obj_emitter_set_mod_attr(
                module, emitter,
                mod_name, dr_entry_id(mod_entry), dr_entry_ref_meta(mod_entry),
                attr_name, input_value);
        }

        return 0;
    }
}

int plugin_particle_obj_emitter_get_attr(plugin_particle_obj_emitter_t emitter, const char * attr_name, dr_value_t output_value) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    struct dr_value obj_value;
    const char * sep = strchr(attr_name, '.');
    
    if (sep == NULL) {
        if (plugin_particle_obj_emitter_get_emitter_attr(module, emitter, attr_name, &obj_value) != 0) return -1;
    }
    else {
        char * mod_name;
        LPDRMETAENTRY mod_entry;
        
        mem_buffer_clear_data(&module->m_dump_buffer);
                              
        mod_name = mem_buffer_strdup_len(&module->m_dump_buffer, attr_name, sep - attr_name);
        attr_name = sep + 1;
        
        mod_entry = dr_meta_find_entry_by_name(module->m_meta_particle_mod_data, mod_name);
        if (mod_entry == NULL) {
            if (plugin_particle_obj_emitter_get_emitter_attr(module, emitter, attr_name, &obj_value) != 0) return -1;
        }
        else {
            if (plugin_particle_obj_emitter_get_mod_attr(
                    module, emitter,
                    mod_name, dr_entry_id(mod_entry), dr_entry_ref_meta(mod_entry),
                    attr_name, &obj_value)
                != 0)
            {
                return -1;
            }
        }
    }

    if (dr_value_set_from_value(output_value, &obj_value, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "plugin_particle_obj_emitter_get_attr: emitter %s: attr %s set to output fail",
            plugin_particle_obj_emitter_name(emitter), attr_name);
        return -1;
    }

    return 0;
}

int plugin_particle_obj_emitter_set_attr_by_str(plugin_particle_obj_emitter_t emitter, const char * attr_name, const char * attr_value) {
    struct dr_value v;

    v.m_type = CPE_DR_TYPE_STRING;
    v.m_meta = NULL;
    v.m_data = (void*)attr_value;
    v.m_size = strlen(attr_value) + 1;

    return plugin_particle_obj_emitter_set_attr(emitter, attr_name, &v);
}
