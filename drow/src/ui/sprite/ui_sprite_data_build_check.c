#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_data_build_i.h"
#include "ui_sprite_entity_i.h"

int ui_sprite_entity_check_calc_bool(uint8_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_bool(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_bool(r, def);
    }
}

int ui_sprite_entity_check_calc_int8(int8_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_int8(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_int8(r, def);
    }
}

int ui_sprite_entity_check_calc_uint8(uint8_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_uint8(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_uint8(r, def);
    }
}

int ui_sprite_entity_check_calc_int16(int16_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_int16(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_int16(r, def);
    }
}

int ui_sprite_entity_check_calc_uint16(uint16_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_uint16(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_uint16(r, def);
    }
}

int ui_sprite_entity_check_calc_int32(int32_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_int32(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_int32(r, def);
    }
}

int ui_sprite_entity_check_calc_uint32(uint32_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_uint32(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_uint32(r, def);
    }
}

int ui_sprite_entity_check_calc_int64(int64_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_int64(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_int64(r, def);
    }
}

int ui_sprite_entity_check_calc_uint64(uint64_t * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_uint64(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_uint64(r, def);
    }
}

int ui_sprite_entity_check_calc_float(float * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_float(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_float(r, def);
    }
}

int ui_sprite_entity_check_calc_double(double * r, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_double(r, def + 1, entity, data_source, em);
    }
    else {
        return cpe_str_to_double(r, def);
    }    
}

const char * ui_sprite_entity_check_calc_str(mem_buffer_t buffer, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_str(buffer, def + 1, entity, data_source, em);
    }
    else {
        return def;
    }    
}

int ui_sprite_entity_check_calc_entity_id(
    uint32_t * r,
    const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em)
{
    if (def[0] == ':') {
        return ui_sprite_entity_try_calc_entity_id(r, def + 1, entity, data_source, em);
    }
    else {
        uint32_t entity_id;
        if (cpe_str_to_uint32(&entity_id, def) == 0) {
            *r = entity_id;
        }
        else {
            ui_sprite_world_t world = ui_sprite_entity_world(entity);
            ui_sprite_entity_t entity = ui_sprite_entity_find_by_name(world, def);
            if (entity == NULL) {
                CPE_ERROR(em, "ui_sprite_entity_check_calc_entity_id: entity %s not exist!", def);
                return -1;
            }
            else {
                *r = ui_sprite_entity_id(entity);
            }
        }
    }

    return 0;
}
