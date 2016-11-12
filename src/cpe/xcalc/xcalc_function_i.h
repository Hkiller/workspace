#ifndef CPE_XCALC_FUNCTION_I_H
#define CPE_XCALC_FUNCTION_I_H
#include "cpe/pal/pal_string.h"
#include "xcalc_computer_i.h"

int xcomputer_func_get_arg_1(xtoken_t * arg_1, const char * func_name, xtoken_it_t args, error_monitor_t em);
int xcomputer_func_get_arg_2(xtoken_t * arg_1, xtoken_t * arg_2, const char * func_name, xtoken_it_t args, error_monitor_t em);

xtoken_t xcomputer_func_create_token_str_dup(xcomputer_t computer, const char * func_name, const char * v, error_monitor_t em);
xtoken_t xcomputer_func_create_token_str_dup_range(xcomputer_t computer, const char * func_name, const char * v, const char * e, error_monitor_t em);
xtoken_t xcomputer_func_create_token_float(xcomputer_t computer, const char * func_name, float v, error_monitor_t em);
xtoken_t xcomputer_func_create_token_int32(xcomputer_t computer, const char * func_name, int32_t v, error_monitor_t em);
    
    
#endif
