#ifndef UI_UTILS_PERCENT_DECORATE_I_H
#define UI_UTILS_PERCENT_DECORATE_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif

union ui_percent_decorator_data {
    struct {
        float m_rate;
    } m_ease;
    struct {
        float m_period;
    } m_elastic;
    struct {
        ui_vector_2 m_control_in;
        ui_vector_2 m_control_out;
        uint16_t m_last_i;
        float m_last;
    } m_bessel;
};

#ifdef __cplusplus
}
#endif

#endif
