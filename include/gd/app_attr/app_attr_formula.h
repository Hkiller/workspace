#ifndef GD_APP_ATTR_FORMULA_H
#define GD_APP_ATTR_FORMULA_H
#include "app_attr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_attr_formula_it {
    app_attr_formula_t (*next)(struct app_attr_formula_it * it);
    char m_data[64];
};
    
app_attr_formula_t app_attr_formula_create(app_attr_request_t request, const char * name, const char * formula);
void app_attr_formula_free(app_attr_formula_t formula);

app_attr_formula_t app_attr_formula_find(app_attr_request_t request, const char * name);
void app_attr_formulas_in_request(app_attr_request_t request, app_attr_formula_it_t it);

app_attr_request_t app_attr_formula_request(app_attr_formula_t formula);
const char * app_attr_formula_name(app_attr_formula_t formula);
const char * app_attr_formula_def(app_attr_formula_t formula);

uint8_t app_attr_formula_is_all_attr_readable(app_attr_formula_t formula);

int app_attr_formula_calc(app_attr_formula_t formula, dr_data_source_t addition_attrs);

int app_attr_formula_try_to_int8(app_attr_formula_t formula, int8_t * r);
int app_attr_formula_try_to_uint8(app_attr_formula_t formula, uint8_t * r);
int app_attr_formula_try_to_int16(app_attr_formula_t formula, int16_t * r);
int app_attr_formula_try_to_uint16(app_attr_formula_t formula, uint16_t * r);
int app_attr_formula_try_to_int32(app_attr_formula_t formula, int32_t * r);
int app_attr_formula_try_to_uint32(app_attr_formula_t formula, uint32_t * r);
int app_attr_formula_try_to_int64(app_attr_formula_t formula, int64_t * r);
int app_attr_formula_try_to_uint64(app_attr_formula_t formula, uint64_t * r);
int app_attr_formula_try_to_float(app_attr_formula_t formula, float * r);
int app_attr_formula_try_to_double(app_attr_formula_t formula, double * r);    
const char * app_attr_formula_try_to_string(app_attr_formula_t formula, mem_buffer_t buffer);
    
int8_t app_attr_formula_to_int8(app_attr_formula_t formula, int8_t dft);
uint8_t app_attr_formula_to_uint8(app_attr_formula_t formula, uint8_t dft);
int16_t app_attr_formula_to_int16(app_attr_formula_t formula, int16_t dft);
uint16_t app_attr_formula_to_uint16(app_attr_formula_t formula, uint16_t dft);
int32_t app_attr_formula_to_int32(app_attr_formula_t formula, int32_t dft);
uint32_t app_attr_formula_to_uint32(app_attr_formula_t formula, uint32_t dft);
int64_t app_attr_formula_to_int64(app_attr_formula_t formula, int64_t dft);
uint64_t app_attr_formula_to_uint64(app_attr_formula_t formula, uint64_t dft);
float app_attr_formula_to_float(app_attr_formula_t formula, float dft);
double app_attr_formula_to_double(app_attr_formula_t formula, double dft);    
const char * app_attr_formula_to_string(app_attr_formula_t formula, mem_buffer_t buffer, const char * dft);

#define app_attr_formula_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
