#include "xcalc_function_i.h"

xtoken_t xcomputer_func_strlen(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t arg;

    if (xcomputer_func_get_arg_1(&arg, func_name, args, em) != 0) return NULL;

    if (arg->m_type != XTOKEN_STRING) {
        CPE_ERROR(em, "%s: arg is not string!", func_name);
        return NULL;
    }

    return xcomputer_func_create_token_int32(computer, func_name, (int32_t)strlen(arg->m_data.str._string), em);
}

