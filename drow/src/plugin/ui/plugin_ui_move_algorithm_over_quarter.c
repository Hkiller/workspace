#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_move_algorithm.h"
#include "plugin/ui/plugin_ui_move_algorithm_meta.h"
#include "plugin_ui_move_algorithm_oval_quarter_i.h"

static int plugin_ui_move_alogrithm_oval_quarter_init(plugin_ui_move_algorithm_t algorithm, void * ctx) {
    struct plugin_ui_move_algorithm_oval_quarter * oval_quarter = plugin_ui_move_algorithm_data(algorithm);
    oval_quarter->m_speed = 0.0f;
    return 0;
}

static int plugin_ui_move_alogrithm_oval_quarter_calc_duration(plugin_ui_move_algorithm_t algorithm, float * result, void * ctx, ui_vector_2_t from, ui_vector_2_t to) {
    plugin_ui_module_t module = ctx;
    struct plugin_ui_move_algorithm_oval_quarter * oval_quarter = plugin_ui_move_algorithm_data(algorithm);

    if (oval_quarter->m_speed <= 0.0f) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm_oval_quarter_calc_duration: move speed not set, can`t calc duration");
        return -1;
    }

    *result = cpe_math_distance(from->x, from->y, to->x, to->y) / oval_quarter->m_speed;
    return 0;
}

static int plugin_ui_move_alogrithm_oval_quarter_calc_pos(
    plugin_ui_move_algorithm_t algorithm, void * ctx, ui_vector_2_t result, ui_vector_2_t from, ui_vector_2_t to, float percent)
{
    float radius_a = to->x - from->x;
    float radius_b = to->y - from->y;
    
    result->y = radius_b * (2 * percent  - percent * percent);
    result->x = sqrt((2* radius_b - result->y) * result->y) * radius_a / radius_b;

    result->x += from->x;
    result->y += from->y;

    return 0;
}

static int plugin_ui_move_algorithm_oval_quarter_setup(plugin_ui_move_algorithm_t algorithm, void * ctx, char * arg_buf_will_change) {
    const char * str_value;
    struct plugin_ui_move_algorithm_oval_quarter * oval_quarter = plugin_ui_move_algorithm_data(algorithm);

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "ui-move.speed", ',', '='))) {
        oval_quarter->m_speed = atof(str_value);
    }
    
    return 0;
}

int plugin_ui_move_algorithm_oval_quarter_regist(plugin_ui_module_t module) {
    plugin_ui_move_algorithm_meta_t meta =
        plugin_ui_move_algorithm_meta_create(
            module,
            "oval-quarter",
            module,
            sizeof(struct plugin_ui_move_algorithm_oval_quarter),
            plugin_ui_move_alogrithm_oval_quarter_init,
            NULL,
            plugin_ui_move_alogrithm_oval_quarter_calc_duration,
            plugin_ui_move_alogrithm_oval_quarter_calc_pos,
            plugin_ui_move_algorithm_oval_quarter_setup);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm_oval_quarter_regist: create meta fail");
        return -1;
    }

    return 0;
}

void plugin_ui_move_algorithm_oval_quarter_unregist(plugin_ui_module_t module) {
    plugin_ui_move_algorithm_meta_t meta = plugin_ui_move_algorithm_meta_find(module, "oval-quarter");
    if (meta) {
        plugin_ui_move_algorithm_meta_free(meta);
    }
}
