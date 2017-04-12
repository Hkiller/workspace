#include "cpe/pal/pal_string.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/random.h"
#include "xcalc_function_i.h"

xtoken_t xcomputer_func_angle_to_radians(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t arg;
    float angle;

    if (xcomputer_func_get_arg_1(&arg, func_name, args, em) != 0) return NULL;

    if (xtoken_try_to_float(arg, &angle) != 0) {
        CPE_ERROR(em, "%s: arg convert to float fail!", func_name);
        return NULL;
    }

    return xcomputer_func_create_token_float(computer, func_name, cpe_math_angle_to_radians(angle), em);
}

xtoken_t xcomputer_func_mod(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t arg_1;
    xtoken_t arg_2;
    int32_t arg1_int;
    int32_t arg2_int;
    float arg1_float;
    float arg2_float;
    
    if (xcomputer_func_get_arg_2(&arg_1, &arg_2, func_name, args, em) != 0) return NULL;

    if (xtoken_try_to_int32(arg_1, &arg1_int) == 0 && xtoken_try_to_int32(arg_2, &arg2_int) == 0) {
        return xcomputer_func_create_token_int32(computer, func_name, arg2_int % arg1_int, em);
    }
    else if (xtoken_try_to_float(arg_1, &arg1_float) == 0 && xtoken_try_to_float(arg_2, &arg2_float) == 0) {
        return xcomputer_func_create_token_float(computer, func_name, fmodf(arg2_float, arg2_float), em);
    }
    else {
        CPE_ERROR(em, "%s: art type error!", func_name);
        return NULL;
    }
}

xtoken_t xcomputer_func_random(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t arg_1;
    float arg_1_float;
    uint8_t arg_1_float_p = 0;
    int32_t arg_1_int;
    uint8_t arg_1_int_p = 0;
    xtoken_t arg_2;
    
    if (xcomputer_func_get_arg_1(&arg_1, func_name, args, em) != 0) {
        return xcomputer_func_create_token_float(computer, func_name, cpe_rand_ctx_generate_f(cpe_rand_ctx_dft()), em);
    }

    if (xtoken_try_to_float(arg_1, &arg_1_float) == 0) {
        arg_1_float_p = 1;
    }
    else if (xtoken_try_to_int32(arg_1, &arg_1_int) == 0) {
        arg_1_int_p = 1;
    }
    else {
        CPE_ERROR(em, "%s: not support arg type %d!", func_name, arg_1->m_type);
        return NULL;
    }
    
    if (xcomputer_func_get_arg_1(&arg_2, func_name, args, em) == 0) { /*单个参数随机 */
        if (arg_1_int_p) {
            return xcomputer_func_create_token_int32(computer, func_name, cpe_rand_dft(arg_1_int), em);
        }
        else {
            assert(arg_1_float_p);

            return xcomputer_func_create_token_float(
                computer, func_name,
                cpe_rand_ctx_generate_f_range(cpe_rand_ctx_dft(), 0.0f, arg_1_float),
                em);
        }
    }
    else {
        float arg_2_float;
        uint8_t arg_2_float_p = 0;
        int32_t arg_2_int;
        uint8_t arg_2_int_p = 0;

        if (xtoken_try_to_float(arg_2, &arg_2_float) == 0) {
            arg_2_float_p = 1;
        }
        else if (xtoken_try_to_int32(arg_2, &arg_2_int) == 0) {
            arg_2_int_p = 1;
        }
        else {
            CPE_ERROR(em, "%s: not support arg type %d!", func_name, arg_2->m_type);
            return NULL;
        }

        if (arg_2_float_p || arg_1_float_p) {
            if (!arg_1_float_p) arg_1_float = (float)arg_1_int;
            if (!arg_2_float_p) arg_2_float = (float)arg_2_int;

            return xcomputer_func_create_token_float(
                computer, func_name,
                cpe_rand_ctx_generate_f_range(cpe_rand_ctx_dft(), arg_1_float, arg_2_float),
                em);
        }
        else {
            assert(arg_1_int_p);
            assert(arg_2_int_p);

            if (arg_1_int < arg_2_int) {
                return xcomputer_func_create_token_int32(
                    computer, func_name,
                    arg_1_int + cpe_rand_dft(arg_2_int - arg_1_int),
                    em);
            }
            else if (arg_1_int > arg_2_int) {
                return xcomputer_func_create_token_int32(
                    computer, func_name,
                    arg_2_int + cpe_rand_dft(arg_1_int - arg_2_int),
                    em);
            }
            else {
                return xcomputer_func_create_token_int32(computer, func_name, arg_1_int, em);
            }
        }
    }
}

xtoken_t xcomputer_func_random_select(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    uint32_t count = 0;
    xtoken_t token;
    xtoken_t r = NULL;
    
    TAILQ_FOREACH(token, &args->m_not_visited, m_next) {
        count++;
    }

    if (count > 0) {
        uint32_t pos = cpe_rand_ctx_generate(cpe_rand_ctx_dft(), count);
        while(pos > 0) {
            xtoken_it_next(args);
            pos--;
        }
        r = xtoken_it_next(args);
        TAILQ_REMOVE(&args->m_not_visited, r, m_next);
    }

    return r;
}

