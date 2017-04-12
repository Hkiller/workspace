#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_token.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui_sprite_entity_i.h"
#include "ui_sprite_data_build_i.h"

uint8_t ui_sprite_entity_calc_bool_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int8_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    uint8_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_bool(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int8_t ui_sprite_entity_calc_int8_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int8_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint8_t ui_sprite_entity_calc_uint8_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, uint8_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint8_t)r;
}

int16_t ui_sprite_entity_calc_int16_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int16_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint16_t ui_sprite_entity_calc_uint16_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, uint16_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint16_t)r;
}

int32_t ui_sprite_entity_calc_int32_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int32_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint32_t ui_sprite_entity_calc_uint32_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, uint32_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int64_t ui_sprite_entity_calc_int64_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int64_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint64_t ui_sprite_entity_calc_uint64_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, uint64_t dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    uint64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

float ui_sprite_entity_calc_float_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, float dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (float)r;
}

double ui_sprite_entity_calc_double_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, double dft) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

const char * ui_sprite_entity_calc_str_with_dft(
    mem_buffer_t buffer, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, const char * dft)
{
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    switch(xtoken_data_type(value)) {
    case xtoken_data_none:
        xcomputer_free_token(computer, value);
        return dft;
    case xtoken_data_int: {
        char buf[23];
        char * r;
        int64_t rv;

        if (xtoken_try_to_int64(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return dft;
        }

        snprintf(buf, sizeof(buf), FMT_INT64_T, rv);
        mem_buffer_clear_data(buffer);
        r = mem_buffer_strdup(buffer, buf);

        xcomputer_free_token(computer, value);
        return r ? r : dft;
    }
    case xtoken_data_double: {
        char buf[23];
        char * r;
        double rv;

        if (xtoken_try_to_double(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return dft;
        }

        snprintf(buf, sizeof(buf), "%f", rv);
        mem_buffer_clear_data(buffer);
        r = mem_buffer_strdup(buffer, buf);
        xcomputer_free_token(computer, value);
        return r ? r : dft;
    }
    case xtoken_data_str: {
        const char * r = xtoken_try_to_str(value);
        if (r) {
            r = mem_buffer_strdup(buffer, r);
        }
        xcomputer_free_token(computer, value);
        return r ? r : dft;
    }
    default:
        xcomputer_free_token(computer, value);
        return dft; 
    }
}

char * ui_sprite_entity_calc_str_with_dft_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, const char * dft)
{
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return cpe_str_mem_dup(alloc, dft);

    switch(xtoken_data_type(value)) {
    case xtoken_data_none:
        xcomputer_free_token(computer, value);
        return cpe_str_mem_dup(alloc, dft);
    case xtoken_data_int: {
        char buf[23];
        int64_t rv;

        if (xtoken_try_to_int64(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return cpe_str_mem_dup(alloc, dft);
        }

        snprintf(buf, sizeof(buf), FMT_INT64_T, rv);
        xcomputer_free_token(computer, value);

        return cpe_str_mem_dup(alloc, buf);
    }
    case xtoken_data_double: {
        char buf[23];
        double rv;

        if (xtoken_try_to_double(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return cpe_str_mem_dup(alloc, dft);
        }

        snprintf(buf, sizeof(buf), "%f", rv);
        xcomputer_free_token(computer, value);

        return cpe_str_mem_dup(alloc, buf);
    }
    case xtoken_data_str: {
        const char * r = xtoken_try_to_str(value);
        char * r2;
        
        r2 = r ? cpe_str_mem_dup(alloc, r) : NULL;

        xcomputer_free_token(computer, value);

        return r2;
    }
    default:
        xcomputer_free_token(computer, value);
        return cpe_str_mem_dup(alloc, dft); 
    }
}

int ui_sprite_entity_try_calc_bool(uint8_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_bool(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_int8(int8_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;
    int32_t rv;
    
    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_uint8(uint8_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;
    uint32_t rv;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_int16(int16_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;
    int32_t rv;
    
    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_uint16(uint16_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;
    uint32_t rv;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, &rv);
    *result = rv;

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_int32(int32_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_uint32(uint32_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_int64(int64_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_uint64(uint64_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_float(float * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_float(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int ui_sprite_entity_try_calc_double(double * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_double(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

const char * ui_sprite_entity_try_calc_str(
    mem_buffer_t buffer, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em)
{
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return NULL;

    switch(xtoken_data_type(value)) {
    case xtoken_data_none:
        xcomputer_free_token(computer, value);
        return NULL;
    case xtoken_data_int: {
        char buf[23];
        int64_t rv;

        if (xtoken_try_to_int64(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return NULL;
        }

        xcomputer_free_token(computer, value);

        snprintf(buf, sizeof(buf), FMT_INT64_T, rv);
        mem_buffer_clear_data(buffer);
        return mem_buffer_strdup(buffer, buf);
    }
    case xtoken_data_double: {
        char buf[23];
        double rv;

        if (xtoken_try_to_double(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return NULL;
        }

        xcomputer_free_token(computer, value);

        snprintf(buf, sizeof(buf), "%f", rv);
        mem_buffer_clear_data(buffer);
        return mem_buffer_strdup(buffer, buf);
    }
    case xtoken_data_str: {
        const char * r = xtoken_try_to_str(value);
        if (r) {
            r = mem_buffer_strdup(buffer, r);
        }
        xcomputer_free_token(computer, value);
        return r;
    }
    default:
        xcomputer_free_token(computer, value);
        return NULL; 
    }
}

char * ui_sprite_entity_try_calc_str_dup(
    mem_allocrator_t alloc, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em)
{
    struct ui_sprite_data_find_ctx ctx = { entity->m_world, entity, data_source };
    struct xcomputer_args calc_args = { &ctx, ui_sprite_data_find_value };
    xcomputer_t computer = entity->m_world->m_repo->m_computer;
    xtoken_t value;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return NULL;

    switch(xtoken_data_type(value)) {
    case xtoken_data_none:
        xcomputer_free_token(computer, value);
        return NULL;
    case xtoken_data_int: {
        char buf[23];
        int64_t rv;

        if (xtoken_try_to_int64(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return NULL;
        }

        xcomputer_free_token(computer, value);

        snprintf(buf, sizeof(buf), FMT_INT64_T, rv);
        return cpe_str_mem_dup(alloc, buf);
    }
    case xtoken_data_double: {
        char buf[23];
        double rv;

        if (xtoken_try_to_double(value, &rv) != 0) {
            xcomputer_free_token(computer, value);
            return NULL;
        }

        xcomputer_free_token(computer, value);

        snprintf(buf, sizeof(buf), "%f", rv);
        return cpe_str_mem_dup(alloc, buf);
    }
    case xtoken_data_str: {
        const char * r = xtoken_try_to_str(value);
        char * r2;

        r2 = r ? cpe_str_mem_dup(alloc, r) : NULL;

        xcomputer_free_token(computer, value);

        return r2;
    }
    default:
        xcomputer_free_token(computer, value);
        return NULL; 
    }
}

int ui_sprite_entity_try_calc_entity_id(
    uint32_t * r,
    const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em)
{
    uint32_t entity_id;
    if (ui_sprite_entity_try_calc_uint32(&entity_id, def, entity, data_source, em) == 0) {
        *r = entity_id;
    }
    else {
        const char * entity_name;
        struct mem_buffer buffer;
        mem_buffer_init(&buffer, NULL);
        entity_name = ui_sprite_entity_try_calc_str(&buffer, def, entity, data_source, em);
        if (entity_name) {
            ui_sprite_world_t world = ui_sprite_entity_world(entity);
            ui_sprite_entity_t entity = ui_sprite_entity_find_by_name(world, entity_name);
            if (entity == NULL) {
                CPE_ERROR(em, "ui_sprite_entity_check_calc_entity_id : entity %s not exist!", entity_name);
                return -1;
            }
            else {
                *r = ui_sprite_entity_id(entity);
            }
        }
        else {
            CPE_ERROR(em, "ui_sprite_entity_check_calc_entity_id: calc entity %s fail!", def);
            return -1;
        }
    }

    return 0;
}
