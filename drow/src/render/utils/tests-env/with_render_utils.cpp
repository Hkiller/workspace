#include "cpe/utils/buffer.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "render/utils/tests-env/with_render_utils.hpp"

namespace render { namespace utils { namespace testenv {

const char * with_render_utils::dump(ui_matrix_4x4_t m) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    ui_matrix_4x4_print((write_stream_t)&ws, m, "    ");

    mem_buffer_append_char(&dump_buffer, 0);

    return cpe_str_mem_dup(t_tmp_allocrator(), (char *)mem_buffer_make_continuous(&dump_buffer, 0));
}

const char * with_render_utils::dump(ui_vector_3_t v) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    ui_vector_3_print((write_stream_t)&ws, v, "    ");

    mem_buffer_append_char(&dump_buffer, 0);

    return cpe_str_mem_dup(t_tmp_allocrator(), (char *)mem_buffer_make_continuous(&dump_buffer, 0));
}

const char * with_render_utils::dump(ui_transform_t t) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    ui_transform_print((write_stream_t)&ws, t, "    ");

    mem_buffer_append_char(&dump_buffer, 0);

    return cpe_str_mem_dup(t_tmp_allocrator(), (char *)mem_buffer_make_continuous(&dump_buffer, 0));
}

const char * with_render_utils::dump(ui_quaternion_t t) {
    struct mem_buffer dump_buffer;

    mem_buffer_init(&dump_buffer, t_tmp_allocrator());

    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&dump_buffer);

    ui_quaternion_print((write_stream_t)&ws, t, "    ");

    mem_buffer_append_char(&dump_buffer, 0);

    return cpe_str_mem_dup(t_tmp_allocrator(), (char *)mem_buffer_make_continuous(&dump_buffer, 0));
}

bool with_render_utils::adj_eq(ui_quaternion_t l, ui_quaternion_t r, ui_vector_3_t tester) {
    ui_vector_3 default_tester = UI_VECTOR_3_INITLIZER(3.34, 5.67, 7.89);
    
    if (tester == NULL) {
        tester = &default_tester;
    }

    ui_vector_3 r_l;
    ui_quaternion_adj_vector_3(l, &r_l, tester);

    ui_vector_3 r_r;
    ui_quaternion_adj_vector_3(l, &r_r, tester);

    return ui_vector_3_cmp(&r_l, &r_r) == 0;
}

bool with_render_utils::adj_eq(ui_matrix_4x4_t l, ui_matrix_4x4_t r, ui_vector_3_t tester) {
    ui_vector_3 default_tester = UI_VECTOR_3_INITLIZER(3.34, 5.67, 7.89);
    
    if (tester == NULL) {
        tester = &default_tester;
    }

    ui_vector_3 r_l;
    ui_matrix_4x4_adj_vector_3(l, &r_l, tester);

    ui_vector_3 r_r;
    ui_matrix_4x4_adj_vector_3(l, &r_r, tester);

    return ui_vector_3_cmp(&r_l, &r_r) == 0;
}


}}}
