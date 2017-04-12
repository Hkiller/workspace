#ifndef UI_UTILS_PERCENT_DECORATOR_H
#define UI_UTILS_PERCENT_DECORATOR_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_percent_decorator * ui_percent_decorator_t;
typedef float (*ui_percent_decorator_fun_t)(ui_percent_decorator_t decorator, float input);

struct ui_percent_decorator {
    ui_percent_decorator_fun_t m_fun;
    char m_data[64];
};

int ui_percent_decorator_setup(ui_percent_decorator_t decorator, const char * def, error_monitor_t em);

float ui_percent_decorator_decorate(ui_percent_decorator_t decorator, float input);

#ifdef __cplusplus
}
#endif

#endif
