#ifndef UI_UTILS_VECTOR_4_H
#define UI_UTILS_VECTOR_4_H
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_vector_4 {
	union {
		float		value[4];

		struct {
			float	x;
			float	y;
			float	z;
			float	w;
		};
	};
};

int ui_vector_4_cmp(ui_vector_4_t l, ui_vector_4_t r);

void ui_vector_4_cross_product(ui_vector_4_t to, ui_vector_4_t l, ui_vector_4_t r);
void ui_vector_4_inline_cross_product(ui_vector_4_t to, ui_vector_4_t o);
    
void ui_vector_4_print(write_stream_t s, ui_vector_4_t t, const char * prefix);
    
#define UI_VECTOR_4_INITLIZER(__x, __y, __z, __w) { { { (__x), (__y), (__z), (__w) } } }

extern ui_vector_4 UI_VECTOR_4_ZERO;
extern ui_vector_4 UI_VECTOR_4_IDENTITY;
extern ui_vector_4 UI_VECTOR_4_NEGATIVE_ONE;
extern ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_X;
extern ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_Y;
extern ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_Z;
extern ui_vector_4 UI_VECTOR_4_POSITIVE_UNIT_W;
extern ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_X;
extern ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_Y;
extern ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_Z;
extern ui_vector_4 UI_VECTOR_4_NEGATIVE_UNIT_w;
    
#ifdef __cplusplus
}
#endif
    
#endif
