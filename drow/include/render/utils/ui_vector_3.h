#ifndef UI_UTILS_VECTOR_3_H
#define UI_UTILS_VECTOR_3_H
#include "cpe/utils/utils_types.h"
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_vector_3 {
	union {
		float value[3];

		struct {
			float x;
			float y;
			float z;
		};

		struct {
			float u;
			float v;
			float w;
		};
	};
};

int ui_vector_3_cmp(ui_vector_3_t l, ui_vector_3_t r);

void ui_vector_3_cross_product(ui_vector_3_t to, ui_vector_3_t l, ui_vector_3_t r);
void ui_vector_3_inline_cross_product(ui_vector_3_t to, ui_vector_3_t o);
    
void ui_vector_3_print(write_stream_t s, ui_vector_3_t t, const char * prefix);
    
#define UI_VECTOR_3_INITLIZER(__x, __y, __z) { { { (__x), (__y), (__z) } } }

extern ui_vector_3 UI_VECTOR_3_ZERO;
extern ui_vector_3 UI_VECTOR_3_IDENTITY;
extern ui_vector_3 UI_VECTOR_3_NEGATIVE_ONE;
extern ui_vector_3 UI_VECTOR_3_POSITIVE_UNIT_X;
extern ui_vector_3 UI_VECTOR_3_POSITIVE_UNIT_Y;
extern ui_vector_3 UI_VECTOR_3_POSITIVE_UNIT_Z;
extern ui_vector_3 UI_VECTOR_3_NEGATIVE_UNIT_X;
extern ui_vector_3 UI_VECTOR_3_NEGATIVE_UNIT_Y;
extern ui_vector_3 UI_VECTOR_3_NEGATIVE_UNIT_Z;

#ifdef __cplusplus
}
#endif

#endif
