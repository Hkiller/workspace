#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_token.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/cfg/cfg_calc.h"
#include "cpe/cfg/cfg_read.h"

xtoken_t cfg_create_token(xcomputer_t computer, cfg_t cfg) {
    void * data = cfg_data(cfg);
    return data ? dr_create_token_from_ctype(computer, cfg_type(cfg), data) : NULL;
}

xtoken_t cfg_calc_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em) {
    cfg_calc_context_t ctx = input_ctx;
    cfg_calc_context_t c;
    cfg_t result_cfg;
    
    if (strcmp(attr_name, ".") == 0) {
        result_cfg = NULL;
        for(c = ctx; c; c = c->m_next) {
            if(cfg_type(c->m_cfg) != CPE_CFG_TYPE_STRUCT && cfg_type(c->m_cfg) != CPE_CFG_TYPE_SEQUENCE) {
                result_cfg = c->m_cfg;
                
                /* struct mem_buffer buffer; */
                /* mem_buffer_init(&buffer, NULL); */
                /* printf("xxx: found %s\n", cfg_dump(result_cfg, &buffer, 0, 4)); */
                /* mem_buffer_clear(&buffer); */
                
                break;
            }                
        }
    }
    else {
        for(c = ctx; c; c = c->m_next) {
            result_cfg = cfg_find_cfg(c->m_cfg, attr_name);
            if (result_cfg) break;
        }
    }

    if (result_cfg == NULL) {
        return dr_create_token_from_ctype(computer, CPE_DR_TYPE_STRING, (void *)"");
    }
    else {
        /* struct mem_buffer buffer; */
        /* mem_buffer_init(&buffer, NULL); */
        /* printf("xxx: %s\n", cfg_dump(result_cfg, &buffer, 0, 4)); */
        /* mem_buffer_clear(&buffer); */
    
        return cfg_create_token(computer, result_cfg);
    }
}

uint8_t cfg_calc_bool_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int8_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    uint8_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_bool(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int8_t cfg_calc_int8_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int8_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint8_t cfg_calc_uint8_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint8_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint8_t)r;
}

int16_t cfg_calc_int16_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int16_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint16_t cfg_calc_uint16_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint16_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint16_t)r;
}

int32_t cfg_calc_int32_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int32_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint32_t cfg_calc_uint32_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint32_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int64_t cfg_calc_int64_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, int64_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint64_t cfg_calc_uint64_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, uint64_t dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    uint64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

float cfg_calc_float_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, float dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (float)r;
}

double cfg_calc_double_with_dft(xcomputer_t computer, const char * def, cfg_calc_context_t ctx, double dft) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

const char * cfg_calc_str_with_dft(
    xcomputer_t computer, mem_buffer_t buffer, const char * def, cfg_calc_context_t ctx, const char * dft)
{
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

char * cfg_calc_str_with_dft_dup(
    xcomputer_t computer, mem_allocrator_t alloc, const char * def, cfg_calc_context_t ctx, const char * dft)
{
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

int cfg_try_calc_bool(xcomputer_t computer, uint8_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_bool(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int cfg_try_calc_int8(xcomputer_t computer, int8_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

int cfg_try_calc_uint8(xcomputer_t computer, uint8_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

int cfg_try_calc_int16(xcomputer_t computer, int16_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

int cfg_try_calc_uint16(xcomputer_t computer, uint16_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

int cfg_try_calc_int32(xcomputer_t computer, int32_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int cfg_try_calc_uint32(xcomputer_t computer, uint32_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int cfg_try_calc_int64(xcomputer_t computer, int64_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int cfg_try_calc_uint64(xcomputer_t computer, uint64_t * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int cfg_try_calc_float(xcomputer_t computer, float * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_float(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int cfg_try_calc_double(xcomputer_t computer, double * result, const char * def, cfg_calc_context_t ctx, error_monitor_t em) {
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_double(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

const char * cfg_try_calc_str(
    xcomputer_t computer, mem_buffer_t buffer, const char * def, cfg_calc_context_t ctx, error_monitor_t em)
{
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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

char * cfg_try_calc_str_dup(
    xcomputer_t computer, mem_allocrator_t alloc, const char * def, cfg_calc_context_t ctx, error_monitor_t em)
{
    struct xcomputer_args calc_args = { ctx, cfg_calc_find_value };
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
