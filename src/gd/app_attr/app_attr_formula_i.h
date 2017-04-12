#ifndef APP_ATTR_FORMULA_I_H
#define APP_ATTR_FORMULA_I_H
#include "gd/app_attr/app_attr_formula.h"
#include "app_attr_request_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_attr_formula {
    app_attr_request_t m_request;
    TAILQ_ENTRY(app_attr_formula) m_next_for_request;
    char m_name[64];
    char * m_def;
    xtoken_t m_result;
};

void app_attr_formula_real_free(app_attr_formula_t formulary);
    
#ifdef __cplusplus
}
#endif

#endif
