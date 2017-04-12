#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "cpe/dr/dr_data_entry.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_calc.h"
#include "gd/app/app_context.h"
#include "app_attr_formula_i.h"
#include "app_attr_attr_binding_i.h"

static void app_attr_formula_on_arg(void * ctx, xtoken_t arg);
static xtoken_t app_attr_formula_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em);

app_attr_formula_t
app_attr_formula_create(app_attr_request_t request, const char * name, const char * def) {
    app_attr_module_t module = request->m_module;
    app_attr_formula_t formula;
    
    formula = TAILQ_FIRST(&module->m_free_formulas);
    if (formula) {
        TAILQ_REMOVE(&module->m_free_formulas, formula, m_next_for_request);
    }
    else {
        formula = mem_calloc(module->m_alloc, sizeof(struct app_attr_formula));
        if (formula == NULL) {
            CPE_ERROR(module->m_em, "ad: create formula: alloc fail!");
            return NULL;
        }
    }

    formula->m_request = request;
    cpe_str_dup(formula->m_name, sizeof(formula->m_name), name);
    formula->m_def = cpe_str_mem_dup(module->m_alloc, def);
    if (formula->m_def == NULL) {
        CPE_ERROR(module->m_em, "ad: create formula: alloc def %s fail!", def);
        formula->m_request = (app_attr_request_t)module;
        TAILQ_INSERT_TAIL(&module->m_free_formulas, formula, m_next_for_request);
        return NULL;
    }
    formula->m_result = NULL;
    
    TAILQ_INSERT_TAIL(&request->m_formulas, formula, m_next_for_request);

    if (xcomputer_visit_args(module->m_computer, def, formula, app_attr_formula_on_arg) != 0) {
        app_attr_formula_free(formula);
        return NULL;
    }
    
    return formula;
}

void app_attr_formula_free(app_attr_formula_t formula) {
    app_attr_module_t module = formula->m_request->m_module;

    if (formula->m_result) {
        xcomputer_free_token(module->m_computer, formula->m_result);
    }
    
    mem_free(module->m_alloc, formula->m_def);
    
    TAILQ_REMOVE(&formula->m_request->m_formulas, formula, m_next_for_request);

    formula->m_request = (app_attr_request_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_formulas, formula, m_next_for_request);
}

void app_attr_formula_real_free(app_attr_formula_t formula) {
    app_attr_module_t module = (app_attr_module_t)formula->m_request;

    TAILQ_REMOVE(&module->m_free_formulas, formula, m_next_for_request);
    mem_free(module->m_alloc, formula);
}

app_attr_request_t app_attr_formula_request(app_attr_formula_t formula) {
    return formula->m_request;
}

const char * app_attr_formula_name(app_attr_formula_t formula) {
    return formula->m_name;
}

app_attr_formula_t app_attr_formula_find(app_attr_request_t request, const char * name) {
    app_attr_formula_t formula;

    TAILQ_FOREACH(formula, &request->m_formulas, m_next_for_request) {
        if (strcmp(formula->m_name, name) == 0) return formula;
    }

    return NULL;
}

struct app_attr_formula_calc_ctx {
    app_attr_formula_t m_formula;
    dr_data_source_t m_addition_attrs;
};

int app_attr_formula_calc(app_attr_formula_t formula, dr_data_source_t addition_attrs) {
    app_attr_module_t module = formula->m_request->m_module;
    struct app_attr_formula_calc_ctx ctx = { formula, addition_attrs };
    struct xcomputer_args calc_args = { &ctx, app_attr_formula_find_value };
    xtoken_t value;

    value = xcomputer_compute(module->m_computer, formula->m_def, &calc_args);
    if (value == NULL) {
        return -1;
    }

    if (formula->m_result) {
        xcomputer_free_token(module->m_computer, formula->m_result);
    }

    formula->m_result = value;
    
    return 0;
}

int app_attr_formula_try_to_int8(app_attr_formula_t formula, int8_t * r) {
    int32_t buf;
    
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    if (xtoken_try_to_int32(formula->m_result, &buf) != 0) return -1;

    *r = (int8_t)buf;
    return 0;
}

int app_attr_formula_try_to_uint8(app_attr_formula_t formula, uint8_t * r) {
    uint32_t buf;
    
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    if (xtoken_try_to_uint32(formula->m_result, &buf) != 0) return -1;

    *r = (uint8_t)buf;
    return 0;
}

int app_attr_formula_try_to_int16(app_attr_formula_t formula, int16_t * r) {
    int32_t buf;
    
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    if (xtoken_try_to_int32(formula->m_result, &buf) != 0) return -1;

    *r = (int16_t)buf;
    return 0;
}

int app_attr_formula_try_to_uint16(app_attr_formula_t formula, uint16_t * r) {
    uint32_t buf;
    
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    if (xtoken_try_to_uint32(formula->m_result, &buf) != 0) return -1;

    *r = (uint16_t)buf;
    return 0;
}

int app_attr_formula_try_to_int32(app_attr_formula_t formula, int32_t * r) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    return xtoken_try_to_int32(formula->m_result, r);
}

int app_attr_formula_try_to_uint32(app_attr_formula_t formula, uint32_t * r) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    return xtoken_try_to_uint32(formula->m_result, r);
}

int app_attr_formula_try_to_int64(app_attr_formula_t formula, int64_t * r) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    return xtoken_try_to_int64(formula->m_result, r);
}

int app_attr_formula_try_to_uint64(app_attr_formula_t formula, uint64_t * r) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    return xtoken_try_to_uint64(formula->m_result, r);
}

int app_attr_formula_try_to_float(app_attr_formula_t formula, float * r) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    return xtoken_try_to_float(formula->m_result, r);
}

int app_attr_formula_try_to_double(app_attr_formula_t formula, double * r) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return -1;
    }

    return xtoken_try_to_double(formula->m_result, r);
}

const char * app_attr_formula_try_to_string(app_attr_formula_t formula, mem_buffer_t buffer) {
    if (formula->m_result == NULL) {
        if (app_attr_formula_calc(formula, NULL) != 0) return NULL;
    }

    return xtoken_to_str_with_buffer(formula->m_result, buffer);
}

int8_t app_attr_formula_to_int8(app_attr_formula_t formula, int8_t dft) {
    int8_t r;
    return app_attr_formula_try_to_int8(formula, &r) == 0 ? r : dft;
}

uint8_t app_attr_formula_to_uint8(app_attr_formula_t formula, uint8_t dft) {
    uint8_t r;
    return app_attr_formula_try_to_uint8(formula, &r) == 0 ? r : dft;
}

int16_t app_attr_formula_to_int16(app_attr_formula_t formula, int16_t dft) {
    int16_t r;
    return app_attr_formula_try_to_int16(formula, &r) == 0 ? r : dft;
}

uint16_t app_attr_formula_to_uint16(app_attr_formula_t formula, uint16_t dft) {
    uint16_t r;
    return app_attr_formula_try_to_uint16(formula, &r) == 0 ? r : dft;
}

int32_t app_attr_formula_to_int32(app_attr_formula_t formula, int32_t dft) {
    int32_t r;
    return app_attr_formula_try_to_int32(formula, &r) == 0 ? r : dft;
}

uint32_t app_attr_formula_to_uint32(app_attr_formula_t formula, uint32_t dft) {
    uint32_t r;
    return app_attr_formula_try_to_uint32(formula, &r) == 0 ? r : dft;
}

int64_t app_attr_formula_to_int64(app_attr_formula_t formula, int64_t dft) {
    int64_t r;
    return app_attr_formula_try_to_int64(formula, &r) == 0 ? r : dft;
}

uint64_t app_attr_formula_to_uint64(app_attr_formula_t formula, uint64_t dft) {
    uint64_t r;
    return app_attr_formula_try_to_uint64(formula, &r) == 0 ? r : dft;
}

float app_attr_formula_to_float(app_attr_formula_t formula, float dft) {
    float r;
    return app_attr_formula_try_to_float(formula, &r) == 0 ? r : dft;
}

double app_attr_formula_to_double(app_attr_formula_t formula, double dft) {
    double r;
    return app_attr_formula_try_to_double(formula, &r) == 0 ? r : dft;
}

const char * app_attr_formula_to_string(app_attr_formula_t formula, mem_buffer_t buffer, const char * dft) {
    const char * r = app_attr_formula_try_to_string(formula, buffer);
    return r ? r : dft;
}

static app_attr_formula_t app_attr_formula_in_request_next(struct app_attr_formula_it * it) {
    app_attr_formula_t * data = (app_attr_formula_t *)(it->m_data);
    app_attr_formula_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_request);
    return r;
}

void app_attr_formulas_in_request(app_attr_request_t request, app_attr_formula_it_t it) {
    *(app_attr_formula_t *)(it->m_data) = TAILQ_FIRST(&request->m_formulas);
    it->next = app_attr_formula_in_request_next;
}

static void app_attr_formula_on_arg(void * ctx, xtoken_t arg) {
    app_attr_formula_t formula = ctx;
    char name_buf[128];
    const char * attr_name;
    app_attr_attr_t attr;
    app_attr_attr_binding_t attr_binding;
    
    attr_name = xtoken_to_str(arg, name_buf, sizeof(name_buf));

    attr = app_attr_attr_find(formula->m_request->m_module, attr_name);
    if (attr == NULL) return;

    TAILQ_FOREACH(attr_binding, &formula->m_request->m_attrs, m_next_for_product) {
        if (strcmp(attr_binding->m_attr->m_name, attr_name) == 0) return;
    }

    attr_binding = app_attr_attr_binding_create(app_attr_attr_binding_request, attr, formula->m_request);
    if (attr_binding == NULL) {
        CPE_ERROR(formula->m_request->m_module->m_em, "app_attr_formula: create attr %s binding fail", attr_name);
    }
}

static xtoken_t app_attr_formula_find_value(void * input_ctx, xcomputer_t computer, const char * attr_name, error_monitor_t em) {
    struct app_attr_formula_calc_ctx * ctx = input_ctx;
    app_attr_module_t module = ctx->m_formula->m_request->m_module;
    app_attr_attr_t attr;
    struct dr_data_entry ds_attr_buf;
    dr_data_entry_t ds_attr;

    attr = app_attr_attr_find(module, attr_name);
    if (attr) {
        struct dr_data_entry e;
        e.m_entry = attr->m_entry;
        e.m_data = ((char*)(attr->m_provider->m_data)) + attr->m_start_pos;
        e.m_size = dr_entry_size(attr->m_entry);
        return dr_create_token_from_entry(module->m_computer, &e);
    }

    if (ctx->m_addition_attrs) {
        ds_attr = dr_data_entry_search_in_source(&ds_attr_buf, ctx->m_addition_attrs, attr_name);
        if (ds_attr) {
            return dr_create_token_from_entry(module->m_computer, ds_attr);
        }
    }
    
    return NULL;
}
