#ifndef UI_UTILS_VECTOR_2_H
#define UI_UTILS_VECTOR_2_H
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_vector_2 {
	union {
		float value[2];

		struct {
			float x;
			float y;
		};

		struct {
			float u;
			float v;
		};

		struct {
			float w;
			float h;
		};
	};
};

float ui_vector_2_length(ui_vector_2_t v);

void ui_vector_2_add(ui_vector_2_t to, ui_vector_2_t v1, ui_vector_2_t v2);
void ui_vector_2_inline_add(ui_vector_2_t to_v, ui_vector_2_t o);

void ui_vector_2_sub(ui_vector_2_t to, ui_vector_2_t v1, ui_vector_2_t v2);
void ui_vector_2_inline_sub(ui_vector_2_t to_v, ui_vector_2_t o);
    
void ui_vector_2_normalize(ui_vector_2_t to, ui_vector_2_t v);
void ui_vector_2_inline_normalize(ui_vector_2_t v);

float ui_vector_2_dot_product(ui_vector_2_t v1, ui_vector_2_t v2);

void ui_vector_2_cross_product(ui_vector_2_t to, ui_vector_2_t v1, ui_vector_2_t v2);
void ui_vector_2_inline_cross_product(ui_vector_2_t to, ui_vector_2_t o);

int ui_vector_2_cmp(ui_vector_2_t l, ui_vector_2_t r);

#define UI_VECTOR_2_INITLIZER(__x, __y) { { { (float)(__x), (float)(__y) } } }
#define UI_INIT_IDENTITY_VECTOR_2 UI_VECTOR_2_INITLIZER(1.0f, 1.0f)
#define UI_INIT_ZERO_VECTOR_2 UI_VECTOR_2_INITLIZER(0.0f, 0.0f)

extern ui_vector_2 UI_VECTOR_2_ZERO;
extern ui_vector_2 UI_VECTOR_2_IDENTITY;
extern ui_vector_2 UI_VECTOR_2_NEGATIVE_ONE;
extern ui_vector_2 UI_VECTOR_2_POSITIVE_UNIT_X;
extern ui_vector_2 UI_VECTOR_2_POSITIVE_UNIT_Y;
extern ui_vector_2 UI_VECTOR_2_NEGATIVE_UNIT_X;
extern ui_vector_2 UI_VECTOR_2_NEGATIVE_UNIT_Y;

#define ui_assert_vector_2_sane( __v ) cpe_assert_float_sane( (__v)->x ); cpe_assert_float_sane( (__v)->y )
    
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
inline bool operator==(struct ui_vector_2 const & l, struct ui_vector_2 const & r) { return ui_vector_2_cmp((ui_vector_2_t)&l, (ui_vector_2_t)&r) == 0; }
inline bool operator!=(struct ui_vector_2 const & l, struct ui_vector_2 const & r) { return ui_vector_2_cmp((ui_vector_2_t)&l, (ui_vector_2_t)&r) != 0; }
inline bool operator<(struct ui_vector_2 const & l, struct ui_vector_2 const & r) { return ui_vector_2_cmp((ui_vector_2_t)&l, (ui_vector_2_t)&r) < 0; }
inline bool operator<=(struct ui_vector_2 const & l, struct ui_vector_2 const & r) { return ui_vector_2_cmp((ui_vector_2_t)&l, (ui_vector_2_t)&r) <= 0; }
inline bool operator>(struct ui_vector_2 const & l, struct ui_vector_2 const & r) { return ui_vector_2_cmp((ui_vector_2_t)&l, (ui_vector_2_t)&r) > 0; }
inline bool operator>=(struct ui_vector_2 const & l, struct ui_vector_2 const & r) { return ui_vector_2_cmp((ui_vector_2_t)&l, (ui_vector_2_t)&r) >= 0; }
inline struct ui_vector_2 & operator+=(struct ui_vector_2 & l, struct ui_vector_2 const & r) { ui_vector_2_inline_add(&l, (ui_vector_2_t)&r); return l; }
inline struct ui_vector_2 mk_ui_vector_2(float x, float y) { ui_vector_2 r = UI_VECTOR_2_INITLIZER(x, y); return r; }
#endif

#endif
