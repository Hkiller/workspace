#include "cpe/utils/math_ex.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "cpe/xcalc/xcalc_token.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui_sprite_2d_module_i.h"

static xtoken_t ui_sprite_2d_angle_flip_x(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t r;
    xtoken_t arg = xtoken_it_next(args);
    double angle;

    if (arg == NULL) {
        CPE_ERROR(em, "angle-flip-x: no arg!");
        return NULL;
    }

    if (xtoken_it_next(args) != NULL) {
        CPE_ERROR(em, "angle-flip-x: too many arg!");
        return NULL;
    }

    if (xtoken_try_to_double(arg, &angle) != 0) {
        CPE_ERROR(em, "angle-flip-x: to double fail!");
        return NULL;
    }

    angle = cpe_math_angle_regular(180.0f - angle);
    
    r = xcomputer_alloc_token(computer);
    if (r == NULL) {
        CPE_ERROR(em, "angle-flip-x: alloc result fail!");
        return NULL;
    }
    
    xcomputer_set_token_float(computer, r, angle);
    
    return r;
}

int ui_sprite_2d_add_angle_flip_x(ui_sprite_2d_module_t module) {
    if (xcomputer_add_func(
            ui_sprite_repository_computer(module->m_repo), 
            "angle-flip-x", ui_sprite_2d_angle_flip_x, NULL)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: add xcomputer function angle-flip-x fail!",
            ui_sprite_2d_module_name(module));
        return -1;
    }

    return 0;
}

void ui_sprite_2d_remove_angle_flip_x(ui_sprite_2d_module_t module) {
    xcomputer_remove_func_by_name(
        ui_sprite_repository_computer(module->m_repo), "angle-flip-x");
}
