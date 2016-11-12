#ifndef CPE_UTILS_MATH_EX_H
#define CPE_UTILS_MATH_EX_H
#include "cpe/pal/pal_math.h"
#include "cpe/pal/pal_types.h"
#include "assert_ex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define cpe_assert_float_nan(__v) cpe_assert_soft(__v == __v, "float is nan")
#define cpe_assert_float_infinite(__v) cpe_assert_soft(__v != INFINITY, "float is infinity")
#define cpe_assert_float_sane(__v) do { cpe_assert_float_nan(__v); cpe_assert_float_infinite(__v); } while(0)
    
uint32_t cpe_math_32_is_pow2(uint32_t value);
uint32_t cpe_math_32_round_to_pow2(uint32_t value);

float cpe_math_distance(float x1, float y1, float x2, float y2);
float cpe_math_distance_square(float x1, float y1, float x2, float y2);
    
/*角度 */
float cpe_math_angle(float x1, float y1, float x2, float y2);
float cpe_math_angle_add(float angle_1, float angle_2);
float cpe_math_angle_diff(float angle_1, float angle_2);
float cpe_math_angle_regular(float angle);
float cpe_math_angle_flip_x(float angle);
float cpe_math_angle_flip_deg(float angle_deg, uint8_t flip_x, uint8_t flip_y);
    
/*弧度 */
float cpe_math_radians(float x1, float y1, float x2, float y2);
float cpe_math_radians_add(float angle_1, float angle_2);
float cpe_math_radians_diff(float angle_1, float angle_2);
float cpe_math_radians_regular(float radians);
float cpe_math_angle_flip_rad(float angle_rad, uint8_t flip_x, uint8_t flip_y);
    
/*角度弧度转换 */
#define cpe_math_angle_to_radians(__angle) ( (__angle) / 180.0f * M_PI )
#define cpe_math_radians_to_angle(__radians) ( (__radians) * 180.0f * M_1_PI )

#define cpe_cos_angle(__angle) cos(cpe_math_angle_to_radians(__angle))
#define cpe_cos_radians(__radians) cos(__radians)

#define cpe_sin_angle(__angle) sin(cpe_math_angle_to_radians(__angle))
#define cpe_sin_radians(__radians) sin(__radians)

#define cpe_min(__a, __b) ((__a) < (__b) ? (__a) : (__b))
#define cpe_max(__a, __b) ((__a) > (__b) ? (__a) : (__b))
#define cpe_limit_in_range(__v, __min, __max) ((__v) < (__min) ? (__min) : ((__v) > (__max) ? (__max) : (__v)))

/*查表快速计算 */
float cpe_fast_sin( float radians );
float cpe_fast_cos( float radians );

void cpe_math_fast_calc_init(void);

/*查反三角函数 */
float cpe_math_acos( float value );
float cpe_math_asin( float value );

/*float 比较 */
#define cpe_float_cmp(__l, __r, __d) ( (((__l) + (__d)) < (__r)) ? -1 : (((__l) - (__d)) > (__r)) ? 1 : 0)

#define cpe_cmp(__l, __r) ( (__l) == (__r) ? 0 : ((__l) < (__r) ? -1 : 1) )
    
#ifdef __cplusplus
}
#endif

#endif
