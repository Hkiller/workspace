#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_move_algorithm.h"
#include "plugin/ui/plugin_ui_move_algorithm_meta.h"
#include "plugin_ui_move_algorithm_linear_i.h"

static int plugin_ui_move_alogrithm_linear_init(plugin_ui_move_algorithm_t algorithm, void * ctx) {
    struct plugin_ui_move_algorithm_linear * linear = plugin_ui_move_algorithm_data(algorithm);
    linear->m_speed = 0.0f;
    return 0;
}

static int plugin_ui_move_alogrithm_linear_calc_duration(plugin_ui_move_algorithm_t algorithm, float * result, void * ctx, ui_vector_2_t from, ui_vector_2_t to) {
    plugin_ui_module_t module = ctx;
    struct plugin_ui_move_algorithm_linear * linear = plugin_ui_move_algorithm_data(algorithm);

    if (linear->m_speed <= 0.0f) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm_linear_calc_duration: move speed not set, can`t calc duration");
        return -1;
    }

    *result = cpe_math_distance(from->x, from->y, to->x, to->y) / linear->m_speed;
    return 0;
}

static int plugin_ui_move_alogrithm_linear_calc_pos(
    plugin_ui_move_algorithm_t algorithm, void * ctx, ui_vector_2_t result, ui_vector_2_t from, ui_vector_2_t to, float percent)
{
    result->x = from->x + (to->x - from->x) * percent;
    result->y = from->y + (to->y - from->y) * percent;
    return 0;
}

static int plugin_ui_move_algorithm_linear_setup(plugin_ui_move_algorithm_t algorithm, void * ctx, char * arg_buf_will_change) {
    const char * str_value;
    struct plugin_ui_move_algorithm_linear * linear = plugin_ui_move_algorithm_data(algorithm);

    if ((str_value = cpe_str_read_and_remove_arg(arg_buf_will_change, "ui-move.speed", ',', '='))) {
        linear->m_speed = atof(str_value);
    }
    
    return 0;
}

int plugin_ui_move_algorithm_linear_regist(plugin_ui_module_t module) {
    plugin_ui_move_algorithm_meta_t meta =
        plugin_ui_move_algorithm_meta_create(
            module,
            "linear",
            module,
            sizeof(struct plugin_ui_move_algorithm_linear),
            plugin_ui_move_alogrithm_linear_init,
            NULL,
            plugin_ui_move_alogrithm_linear_calc_duration,
            plugin_ui_move_alogrithm_linear_calc_pos,
            plugin_ui_move_algorithm_linear_setup);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_move_algorithm_linear_regist: create meta fail");
        return -1;
    }

    return 0;
}

void plugin_ui_move_algorithm_linear_unregist(plugin_ui_module_t module) {
    plugin_ui_move_algorithm_meta_t meta = plugin_ui_move_algorithm_meta_find(module, "linear");
    if (meta) {
        plugin_ui_move_algorithm_meta_free(meta);
    }
}
