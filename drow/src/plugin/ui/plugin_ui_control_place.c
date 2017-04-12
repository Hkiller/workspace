#include "render/utils/ui_vector_2.h"
#include "plugin_ui_control_i.h"

int plugin_ui_control_place_on_screen(
    plugin_ui_control_t control, ui_vector_2_t pivot_screen_pt, ui_rect_t screen_rect)
{
    plugin_ui_control_t parent = control->m_parent;
    ui_vector_2 target_size;
    ui_vector_2 pt_to_p;
    ui_vector_2_t cur_pt_to_p;
    ui_vector_2_t cur_size;
    ui_vector_2_t scale;

    if (control->m_cache_flag) plugin_ui_control_update_cache(control, 0);
    
    cur_pt_to_p = plugin_ui_control_real_pt_to_p(control);
    cur_size = plugin_ui_control_real_sz_no_scale(control);
    scale = plugin_ui_control_render_scale(control);

    /* printf("xxxxxx place_on_screen, pt_to_p=(%f,%f), size=(%f,%f), scale=(%f,%f), screen-rect=(%f,%f)-(%f,%f)\n", */
    /*        cur_pt_to_p->x, cur_pt_to_p->y, cur_size->x, cur_size->y, scale->x, scale->y, */
    /*        screen_rect->lt.x, screen_rect->lt.y, screen_rect->rb.x, screen_rect->rb.y); */
    
    if (scale->x == 0 || scale->y == 0) {
        target_size.x = target_size.y = 0;
    }
    else {
        ui_vector_2_sub(&target_size, &screen_rect->rb, &screen_rect->lt);
        target_size.x /= scale->x;
        target_size.y /= scale->y;
    }

    ui_vector_2_sub(&pt_to_p, &screen_rect->lt, plugin_ui_control_real_pt_abs(parent));

    if ((int)cur_pt_to_p->x != (int)(pt_to_p.x + 0.5f) || (int)cur_pt_to_p->y != (int)(pt_to_p.y + 0.5f)) {
        plugin_ui_control_set_render_pt_by_real(control, &pt_to_p);
    }

    if ((int)cur_size->x != (int)(target_size.x + 0.5f) || (int)cur_size->y != (int)(target_size.y + 0.5f)) {
        plugin_ui_control_set_render_sz_by_real(control, &target_size);
    }

    return 0;
}

