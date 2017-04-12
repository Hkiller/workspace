#include <assert.h>
#include "cpe/pal/pal_limits.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_token.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_calc.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data_entry.h"
#include "plugin_ui_page_calc_i.h"
#include "plugin_ui_page_slot_i.h"

struct ui_sprite_page_data_find_ctx {
    plugin_ui_env_t m_env;
    plugin_ui_page_t m_page;
    dr_data_source_t m_data_source;
};

xtoken_t plugin_ui_page_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx * ctx = input_ctx;
    struct dr_data_entry from_attr_buf;
    dr_data_entry_t from_attr;
    plugin_ui_page_slot_t slot;
    
    while (attr_name[0] == '[') {
        char name_buf[64];
        const char * page_name = cpe_str_trim_head((char*)attr_name + 1);
        const char * page_name_end = strchr(page_name, ']');
        size_t page_name_len;

        if (page_name_end == NULL) {
            CPE_ERROR(em, "plugin_ui_page_find_value: %s format error!", attr_name);
            return NULL;
        }

        page_name_len = page_name_end - page_name;
        if ((page_name_len + 1) > CPE_ARRAY_SIZE(name_buf)) {
            CPE_ERROR(em, "plugin_ui_page_find_value: page name len %d overflow!", (int)page_name_len);
            return NULL;
        }

        memcpy(name_buf, page_name, page_name_len);
        name_buf[page_name_len] = 0;

        if (name_buf[0] == '@') {
            xtoken_t v = plugin_ui_page_find_value(input_ctx, computer, name_buf + 1, em);
            if (v == NULL) {
                CPE_ERROR(em, "plugin_ui_page_find_value: %s: calc inner arg %s fail!", attr_name, name_buf + 1);
                return NULL;
            }

            switch(xtoken_data_type(v)) {
            case xtoken_data_str: {
                const char * str_v = xtoken_try_to_str(v);
                assert(str_v);

                if (str_v[0] == 0) {
                    xcomputer_free_token(computer, v);
                    return NULL;
                }

                ctx->m_page = plugin_ui_page_find(ctx->m_env, str_v);
                if (ctx->m_page == NULL) {
                    CPE_ERROR(em, "plugin_ui_page_find_value: %s: from arg %s: page %s not exist!", attr_name, name_buf + 1, str_v);
                    xcomputer_free_token(computer, v);
                    return NULL;
                }
                break;
            }
            default:
                CPE_ERROR(
                    em, "plugin_ui_page_find_value: %s: calc inner arg %s: result type %d fail!",
                    attr_name, name_buf + 1, xtoken_data_type(v));
                xcomputer_free_token(computer, v);
                return NULL;
            }

            xcomputer_free_token(computer, v);
        }
        else {
            ctx->m_page = plugin_ui_page_find(ctx->m_env, name_buf);
            if (ctx->m_page == NULL) {
                CPE_ERROR(em, "plugin_ui_page_find_value: page %s not exist!", name_buf);
                return NULL;
            }
        }

        attr_name = cpe_str_trim_head((char*)page_name_end + 1);
    }

    if ((from_attr = dr_data_entry_search_in_source(&from_attr_buf, ctx->m_data_source, attr_name))) {
        return dr_create_token_from_entry(ctx->m_env->m_module->m_computer, from_attr);
    }

    if (ctx->m_page->m_page_data) {
        int data_start_pos;
        from_attr_buf.m_entry = dr_meta_find_entry_by_path_ex(ctx->m_page->m_page_data_meta, attr_name, &data_start_pos);
        if (from_attr_buf.m_entry) {
            from_attr_buf.m_data = ((char*)ctx->m_page->m_page_data) + data_start_pos;
            from_attr_buf.m_size = dr_entry_element_size(from_attr_buf.m_entry);
            return dr_create_token_from_entry(ctx->m_env->m_module->m_computer, from_attr);
        }
    }
    
    if ((slot = plugin_ui_page_slot_find(ctx->m_page, attr_name))) {
        return dr_create_token_from_value(ctx->m_env->m_module->m_computer, &slot->m_value);
    }
    
    CPE_ERROR(em, "plugin_ui_page_find_value: %s: entry not exist!", attr_name);
    return NULL;
}

xtoken_t plugin_ui_page_calc_i(plugin_ui_page_t page, const char * def, dr_data_source_t data_source) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    return xcomputer_compute(page->m_env->m_module->m_computer, def, &calc_args);
}

uint8_t plugin_ui_page_calc_bool_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, int8_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    uint8_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_bool(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int8_t plugin_ui_page_calc_int8_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, int8_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint8_t plugin_ui_page_calc_uint8_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, uint8_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint8_t)r;
}

int16_t plugin_ui_page_calc_int16_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, int16_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint16_t plugin_ui_page_calc_uint16_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, uint16_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (uint16_t)r;
}

int32_t plugin_ui_page_calc_int32_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, int32_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint32_t plugin_ui_page_calc_uint32_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, uint32_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    uint32_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint32(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

int64_t plugin_ui_page_calc_int64_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, int64_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_int64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

uint64_t plugin_ui_page_calc_uint64_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, uint64_t dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    uint64_t r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_uint64(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

float plugin_ui_page_calc_float_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, float dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return (float)r;
}

double plugin_ui_page_calc_double_with_dft(const char * def, plugin_ui_page_t page, dr_data_source_t data_source, double dft) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    double r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return dft;

    if (xtoken_try_to_double(value, &r) != 0) r = dft;

    xcomputer_free_token(computer, value);

    return r;
}

const char * plugin_ui_page_calc_str_with_dft(
    mem_buffer_t buffer, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, const char * dft)
{
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
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

int plugin_ui_page_try_calc_bool(uint8_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_bool(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int plugin_ui_page_try_calc_int8(int8_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
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

int plugin_ui_page_try_calc_uint8(uint8_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
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

int plugin_ui_page_try_calc_int16(int16_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
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

int plugin_ui_page_try_calc_uint16(uint16_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
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

int plugin_ui_page_try_calc_int32(int32_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int plugin_ui_page_try_calc_uint32(uint32_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint32(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int plugin_ui_page_try_calc_int64(int64_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_int64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int plugin_ui_page_try_calc_uint64(uint64_t * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_uint64(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int plugin_ui_page_try_calc_float(float * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_float(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

int plugin_ui_page_try_calc_double(double * result, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em) {
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
    xtoken_t value;
    int r;

    value = xcomputer_compute(computer, def, &calc_args);
    if (value == NULL) return -1;

    r = xtoken_try_to_double(value, result);

    xcomputer_free_token(computer, value);

    return r;
}

const char * plugin_ui_page_try_calc_str(
    mem_buffer_t buffer, const char * def, plugin_ui_page_t page, dr_data_source_t data_source, error_monitor_t em)
{
    struct ui_sprite_page_data_find_ctx ctx = { page->m_env, page, data_source };
    struct xcomputer_args calc_args = { &ctx, plugin_ui_page_find_value };
    xcomputer_t computer = page->m_env->m_module->m_computer;
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
