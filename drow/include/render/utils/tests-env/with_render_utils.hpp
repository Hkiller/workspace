#ifndef DROW_RENDER_UTILS_WITH_RENDER_UTILS_H
#define DROW_RENDER_UTILS_WITH_RENDER_UTILS_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_matrix_3x3.h"
#include "../ui_matrix_4x4.h"
#include "../ui_quaternion.h"
#include "../ui_rect.h"
#include "../ui_transform.h"
#include "../ui_utils_types.h"
#include "../ui_vector_2.h"
#include "../ui_vector_3.h"
#include "../ui_vector_4.h"

namespace render { namespace utils { namespace testenv {

class with_render_utils : public ::testenv::env<> {
public:
    const char * dump(ui_matrix_4x4_t m);
    const char * dump(ui_vector_3_t v);
    const char * dump(ui_transform_t t);
    const char * dump(ui_quaternion_t t);

    bool adj_eq(ui_quaternion_t l, ui_quaternion_t r, ui_vector_3_t tester = NULL);
    bool adj_eq(ui_matrix_4x4_t l, ui_matrix_4x4_t r, ui_vector_3_t tester = NULL);
};

}}}

#endif

