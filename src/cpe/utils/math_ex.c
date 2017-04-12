#include <assert.h>
#include "cpe/utils/math_ex.h"

uint32_t cpe_math_32_is_pow2(uint32_t value) {
	return value != 0 &&
        ((value & (value - 1)) == 0);
}

uint32_t cpe_math_32_round_to_pow2(uint32_t value) {
	if (value != 0) {
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		value++;
	}
    
	return value;
}

float cpe_math_distance(float x1, float y1, float x2, float y2) {
	return (float)sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

float cpe_math_distance_square(float x1, float y1, float x2, float y2) {
    float x = x2 - x1;
    float y = y2 - y1;

    return x * x + y * y;
}

float cpe_math_angle(float x1, float y1, float x2, float y2) {
    float diff_x = x2 - x1;
    float diff_y = y2 - y1;

    return (float)cpe_math_radians_to_angle(atan2f(diff_y, diff_x));
}

float cpe_math_angle_regular(float angle) {
    while(angle > 180.0f) angle -= 360.0;
    while(angle < -180.0f) angle += 360.0;
    return angle;
}

float cpe_math_angle_flip_x(float angle) {
    assert(angle <= 180.f && angle >= -180.f);
    return angle > 0 ? (180.0f - angle) : (-180.0f - angle);
}

float cpe_math_angle_add(float angle_1, float angle_2) {
	float r = 0.0f;
    assert(angle_1 <= 180.f && angle_1 >= -180.f);
    assert(angle_2 <= 180.f && angle_2 >= -180.f);

    r = angle_1 + angle_2;

    return r > 180.0f ? (r - 360.f)
        : r < -180.0f ? (r + 360.f)
        : r;
}

float cpe_math_angle_diff(float angle_1, float angle_2) {
	float r = 0.0f;
    assert(angle_1 <= 180.f && angle_1 >= -180.f);
    assert(angle_2 <= 180.f && angle_2 >= -180.f);

    r = angle_2 - angle_1;

    return r > 180.0f ? (r - 360.f)
        : r < -180.0f ? (r + 360.f)
        : r;
}

float cpe_math_radians(float x1, float y1, float x2, float y2) {
    float diff_x = x2 - x1;
    float diff_y = y2 - y1;

    cpe_assert_float_sane(x1);
    cpe_assert_float_sane(y1);
    cpe_assert_float_sane(x2);
    cpe_assert_float_sane(y2);

    return atan2f(diff_y, diff_x);
}

float cpe_math_radians_regular(float radians) {
    while(radians > M_PI) radians -= (float)(2 * M_PI);
    while(radians < - M_PI) radians += (float)(2 * M_PI);
    return radians;
}

float cpe_math_radians_add(float radians_1, float radians_2) {
	float r = 0.0f;
    assert(radians_1 <= (float)M_PI  && radians_1 >= (float)-M_PI);
    assert(radians_2 <= (float)M_PI && radians_2 >= (float)-M_PI);

    r = radians_1 + radians_2;

    return r > M_PI ? (r - 2 * M_PI)
        : r < - M_PI ? (r + 2 * M_PI)
        : r;
}

float cpe_math_radians_diff(float radians_1, float radians_2) {
	float r = 0.0f;
    //assert(radians_1 <= M_PI && radians_1 >= -M_PI);
    //assert(radians_2 <= M_PI && radians_2 >= -M_PI);

    r = radians_1 - radians_2;

    return r > M_PI ? (float)(r - 2 * M_PI)
        : r < - M_PI ? (float)(r + 2 * M_PI)
        : r;
}

float cpe_math_acos( float value ) {
	if (value >  1.0f)
		value =  1.0f;
	if (value < -1.0f)
		value = -1.0f;

	return acos(value);
}

float cpe_math_asin( float value ) {
	if (value >  1.0f)
		value =  1.0f;
	if (value < -1.0f)
		value = -1.0f;

	return asin(value);
}

float cpe_math_angle_flip_deg(float angle_deg, uint8_t flip_x, uint8_t flip_y) {
    cpe_assert_float_sane(angle_deg);

    if (flip_x) {
		angle_deg = cpe_math_angle_regular(180.0f - angle_deg);
    }

    if (flip_y) {
		angle_deg = cpe_math_angle_regular(- angle_deg);
    }

    cpe_assert_float_sane(angle_deg);

    return angle_deg;
}

float cpe_math_angle_flip_rad(float angle_rad, uint8_t flip_x, uint8_t flip_y) {
    cpe_assert_float_sane(angle_rad);

    if (flip_x) {
		angle_rad = cpe_math_radians_regular(M_PI - angle_rad);
    }

    if (flip_y) {
		angle_rad = cpe_math_radians_regular(- angle_rad);
    }

    cpe_assert_float_sane(angle_rad);
    return angle_rad;
}
