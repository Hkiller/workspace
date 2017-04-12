#include "cpe/pal/pal_string.h"
#include "xcalc_computer_i.h"

/*regex */
xtoken_t xcomputer_func_regex(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);

/*string*/
xtoken_t xcomputer_func_strlen(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);

/*math*/
xtoken_t xcomputer_func_angle_to_radians(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);
xtoken_t xcomputer_func_mod(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);
xtoken_t xcomputer_func_random(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);
xtoken_t xcomputer_func_random_select(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em);

static struct {
    const char * m_func_name;
    xcalc_func_t m_fun;
} g_default_funs[] = {
    { "strlen", xcomputer_func_strlen }
    , { "regex", xcomputer_func_regex }
    , { "angle-to-radians", xcomputer_func_angle_to_radians }
    , { "mod", xcomputer_func_mod }
    , { "random", xcomputer_func_random }
    , { "random-select", xcomputer_func_random_select }
};

int xcomputer_load_default_funcs(xcomputer_t computer) {
    int i;

    for(i = 0; i < CPE_ARRAY_SIZE(g_default_funs); ++i) {
        if (xcomputer_add_func(computer, g_default_funs[i].m_func_name, g_default_funs[i].m_fun, NULL) != 0) {
            CPE_ERROR(computer->m_em, "xcomputer: load default funcs: add func %s fail!", g_default_funs[i].m_func_name);
            return -1;
        }
    }

    return 0;
}

xtoken_t xcomputer_func_create_token_float(xcomputer_t computer, const char * func_name, float v, error_monitor_t em) {
    xtoken_t r = xcomputer_alloc_token(computer);
    if (r == NULL) {
        CPE_ERROR(em, "%s: alloc token fail!", func_name);
        return NULL;
    }

    xcomputer_set_token_float(computer, r, v);

    return r;
}

xtoken_t xcomputer_func_create_token_int32(xcomputer_t computer, const char * func_name, int32_t v, error_monitor_t em) {
    xtoken_t r = xcomputer_alloc_token(computer);
    if (r == NULL) {
        CPE_ERROR(em, "%s: alloc token fail!", func_name);
        return NULL;
    }

    xcomputer_set_token_int(computer, r, v);

    return r;
}

xtoken_t xcomputer_func_create_token_str_dup(xcomputer_t computer, const char * func_name, const char * v, error_monitor_t em) {
    xtoken_t r = xcomputer_alloc_token(computer);
    if (r == NULL) {
        CPE_ERROR(em, "%s: alloc token fail!", func_name);
        return NULL;
    }

    if (xcomputer_set_token_str(computer, r, v) != 0) {
        CPE_ERROR(em, "%s: set token str fail!", func_name);
        xcomputer_free_token(computer, r);
        return NULL;
    }

    return r;
}

xtoken_t xcomputer_func_create_token_str_dup_range(xcomputer_t computer, const char * func_name, const char * v, const char * e, error_monitor_t em) {
    xtoken_t r = xcomputer_alloc_token(computer);
    if (r == NULL) {
        CPE_ERROR(em, "%s: alloc token fail!", func_name);
        return NULL;
    }

    if (xcomputer_set_token_str_range(computer, r, v, e) != 0) {
        CPE_ERROR(em, "%s: set token str range fail!", func_name);
        xcomputer_free_token(computer, r);
        return NULL;
    }

    return r;
}

int xcomputer_func_get_arg_1(xtoken_t * arg_1, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    *arg_1 = xtoken_it_next(args);

    if (*arg_1 == NULL) {
        CPE_ERROR(em, "%s: no arg!", func_name);
        return -1;
    }

    if (xtoken_it_next(args) != NULL) {
        CPE_ERROR(em, "%s: too many arg!", func_name);
        return -1;
    }

    return 0;
}


int xcomputer_func_get_arg_2(xtoken_t * arg_1, xtoken_t * arg_2, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    *arg_1 = xtoken_it_next(args);
    if (*arg_1 == NULL) {
        CPE_ERROR(em, "%s: not enough arg (pos 0)!", func_name);
        return -1;
    }

    *arg_2 = xtoken_it_next(args);
    if (*arg_2 == NULL) {
        CPE_ERROR(em, "%s: not enough arg (pos 1)!", func_name);
        return -1;
    }

    if (xtoken_it_next(args) != NULL) {
        CPE_ERROR(em, "%s: too many arg!", func_name);
        return -1;
    }

    return 0;
}
