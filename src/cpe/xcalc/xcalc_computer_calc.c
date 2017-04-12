#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "xcalc_computer_i.h"
#include "xcalc_token_i.h"
#include "xcalc_context_i.h"

static const char * s_pri_table_error_msg[] = {
    "pri ok",
    "there is '(' but not ')'",
    "there is fun but not ')'",
    "there is a ':' follow (",
    "there is a '!' follow )",
    "there is a ')', but ')'"
};

/*>: 1, =: 0, <: -1  no: -2 */
#define bg 0
#define lt 1
#define er 2
#define _er(msg_id) ((msg_id) << 2 | er)

#define _is_pri_error(pri) ((pri & 0x3) == er)
#define _is_pri_le(pri) ((pri & 0x03) == lt)
#define _is_pri_bg(pri) ((pri & 0x03) == bg)

#define _x_pri_errmsg(err) s_pri_table_error_msg[(err) >> 2]

typedef int (*xcomputer_op_fun_t)(xcontext_t context);

static const char s_sign_pri_tab[XTOKEN_END + 1][XTOKEN_END + 1];
static xcomputer_op_fun_t s_op_funs[XTOKEN_END];
static xtoken_t xcomputer_find_arg(xcomputer_t computer, xcomputer_args_t args, xtoken_t input);

int xcomputer_visit_args(xcomputer_t computer, const char * str, void * ctx, void (*on_found)(void * ctx, xtoken_t arg)) {
    xcontext_t context;

    context = xcontext_create(computer, str);
    if (context == NULL) return -1;

    while(1) {
        if (context->m_cur_token) {
            xcomputer_free_token(computer, context->m_cur_token);
            context->m_cur_token = NULL;
        }

        if (xcontext_get_token(context) != 0) goto PROCESS_ERROR;
        assert(context->m_cur_token);

        if (xtoken_get_type(context->m_cur_token) == XTOKEN_VAL) {
            if (xcomputer_dup_token_str(computer, context->m_cur_token) != 0) goto PROCESS_ERROR;

            on_found(ctx, context->m_cur_token);

            mem_free(computer->m_alloc, context->m_cur_token->m_data.str._string);
            context->m_cur_token->m_data.str._string = NULL;
        }
        else if (xtoken_get_type(context->m_cur_token) == XTOKEN_END) {
            xcontext_free(context);
            return 0;
        }
    };

PROCESS_ERROR:
    xcontext_free(context);
    return -1;
}

xtoken_t xcomputer_compute(xcomputer_t computer, const char * str, xcomputer_args_t args) {
    xcontext_t context;

    //assert(computer->m_allocked_token_count == 0);

    context = xcontext_create(computer, str);
    if (context == NULL) return NULL;

    while(1) {
        assert(context);
        if (xcontext_get_token(context) != 0) goto PROCESS_ERROR;

        assert(context->m_cur_token);

        if (xtoken_is_data(context->m_cur_token)) {
            if (xtoken_get_type(context->m_cur_token) == XTOKEN_VAL) {
                xtoken_t v = xcomputer_find_arg(computer, args, context->m_cur_token);
                if (v == NULL) goto PROCESS_ERROR;

                xcomputer_free_token(computer, context->m_cur_token);
                context->m_cur_token = v;
            }

            xcontext_push_token(context);
        }
        else {
            uint32_t cur_token_type = xtoken_get_type(context->m_cur_token);

            while(context->m_cur_token != NULL) {
                uint32_t pop_token_type = context->m_sign_token ? xtoken_get_type(context->m_sign_token) : XTOKEN_END; 
                char sgi_pri = s_sign_pri_tab[xtoken_type_index(pop_token_type)][xtoken_type_index(cur_token_type)];

                if (_is_pri_le(sgi_pri)) {
                    xcontext_push_token(context);
                }
                else if (_is_pri_bg(sgi_pri)) {
                    if (pop_token_type < XTOKEN_END) {
                        int r = s_op_funs[pop_token_type](context);
                        if (r < 0) goto PROCESS_ERROR;

                        while(r > 0) {
                            xcomputer_free_token(computer, xcontext_pop_token(context));
                            --r;
                        }

                        if (context->m_sign_token && !xtoken_is_sign(context->m_sign_token)) {
                            xcontext_update_sign_token(context);
                        }
                    }
                    else {
                        xtoken_t result;

                        if (TAILQ_EMPTY(&context->m_tokens)) {
                            CPE_ERROR(computer->m_em, "no token in stack!");
                            goto PROCESS_ERROR;
                        }

                        result = xcontext_pop_token(context);

                        if (!TAILQ_EMPTY(&context->m_tokens)) {
                            CPE_ERROR(computer->m_em, "still have token in stack!");
                            xcomputer_free_token(computer, result);
                            goto PROCESS_ERROR;
                        }

                        if (!xtoken_is_data(result)) {
                            CPE_ERROR(computer->m_em, "result is not data!");
                            xcomputer_free_token(computer, result);
                            goto PROCESS_ERROR;
                        }

                        if (xtoken_get_type(result) == XTOKEN_STRING && result->m_data.str._end != NULL) {
                            if (xcomputer_dup_token_str(computer, result) != 0) {
                                CPE_ERROR(computer->m_em, "result dump str fail!");
                                xcomputer_free_token(computer, result);
                                goto PROCESS_ERROR;
                            }
                        }

                        xcontext_free(context);
                        //assert(computer->m_allocked_token_count == 1);
                        return result;
                    }
                }
                else {
                    CPE_ERROR(computer->m_em, "pri error: %s", _x_pri_errmsg(sgi_pri));
                    goto PROCESS_ERROR;
                }
            }
        }
    }

PROCESS_ERROR:
    if (computer->m_em) {
        xcontext_dump_stack(context);
    }

    xcontext_free(context);
    //assert(computer->m_allocked_token_count == 0);
    return NULL;
}

static xtoken_t xcomputer_find_arg(xcomputer_t computer, xcomputer_args_t args, xtoken_t arg) {
    xtoken_t r;

    assert(xtoken_get_type(arg) == XTOKEN_VAL);
    assert(arg->m_data.str._string);

    if (args == NULL) return NULL;

    if (arg->m_data.str._end) {
        char s = *arg->m_data.str._end;

        *arg->m_data.str._end = 0;
        r = args->m_find_arg(args->m_ctx, computer, arg->m_data.str._string, computer->m_em);

        if (r == NULL) {
            CPE_ERROR(computer->m_em, "arg %s not exist!", arg->m_data.str._string);
        }

        *arg->m_data.str._end = s;

        return r;
    }
    else {
        r = args->m_find_arg(args->m_ctx, computer, arg->m_data.str._string, computer->m_em);

        if (r == NULL) {
            CPE_ERROR(computer->m_em, "arg %s not exist!", arg->m_data.str._string);
        }

        return r;
    }
}

static void xcomputer_tmp_buffer_append_token(xcomputer_t computer, xtoken_t token) {
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&computer->m_tmp_buffer);

    assert(xtoken_is_data(token));

    if (xtoken_get_type(token) == XTOKEN_NUM_INT) {
        stream_printf((write_stream_t)&s, "%d", token->m_data.num._int);
    }
    else if (xtoken_get_type(token) == XTOKEN_NUM_FLOAT) {
        stream_printf((write_stream_t)&s, "%f", (float)token->m_data.num._double);
    }
    else {
        if (token->m_data.str._end) {
            stream_write((write_stream_t)&s, token->m_data.str._string, token->m_data.str._end - token->m_data.str._string);
        }
        else {
            stream_printf((write_stream_t)&s, "%s", token->m_data.str._string);
        }
    }
}

static int xcomputer_set_token_str_from_buffer(xcomputer_t computer , xtoken_t token) {
    char * p;
    size_t len;
    const char * src;

    len = mem_buffer_size(&computer->m_tmp_buffer);
    src = mem_buffer_make_continuous(&computer->m_tmp_buffer, 0);
    if (src == NULL) return -1;

    if (len == 0 || src[len - 1] != 0) {
        p = mem_alloc(computer->m_alloc, len + 1);
        if (p == NULL) return -1;
        memcpy(p, src, len);
        p[len] = 0;
    }
    else {
        p = mem_alloc(computer->m_alloc, len);
        if (p == NULL) return -1;
        memcpy(p, src, len);
    }

    if (token->m_type == XTOKEN_STRING && token->m_data.str._end == NULL) {
        mem_free(computer->m_alloc, token->m_data.str._string);
    }

    token->m_type = XTOKEN_STRING;
    token->m_data.str._string = p;
    token->m_data.str._end = NULL;

    return 0;
}

void xcomputer_set_token_float(xcomputer_t computer, xtoken_t token, double v) {
    if (token->m_type == XTOKEN_STRING && token->m_data.str._end == NULL) {
        mem_free(computer->m_alloc, token->m_data.str._string);
    }
    token->m_type = XTOKEN_NUM_FLOAT;
    token->m_data.num._double = (double)(v);
}

void xcomputer_set_token_int(xcomputer_t computer, xtoken_t token, int64_t v) {
    if (token->m_type == XTOKEN_STRING && token->m_data.str._end == NULL) {
        mem_free(computer->m_alloc, token->m_data.str._string);
    }
    token->m_type = XTOKEN_NUM_INT;
    token->m_data.num._int = (int)(v);
}

int xcomputer_set_token_str(xcomputer_t computer, xtoken_t token, const char * v) {
    size_t len = strlen(v);
    char * p;

    p = mem_alloc(computer->m_alloc, len + 1);
    if (p == NULL) return -1;
    memcpy(p, v, len);
    p[len] = 0;

    if (token->m_type == XTOKEN_STRING && token->m_data.str._end == NULL) {
        mem_free(computer->m_alloc, token->m_data.str._string);
    }
    token->m_type = XTOKEN_STRING;
    token->m_data.str._string = p;
    token->m_data.str._end = NULL;

    return 0;
}

int xcomputer_set_token_str_range(xcomputer_t computer, xtoken_t token, const char * begin, const char * end) {
    size_t len = end - begin;
    char * p;

    p = mem_alloc(computer->m_alloc, len + 1);
    if (p == NULL) return -1;
    memcpy(p, begin, len);
    p[len] = 0;

    if (token->m_type == XTOKEN_STRING && token->m_data.str._end == NULL) {
        mem_free(computer->m_alloc, token->m_data.str._string);
    }
    token->m_type = XTOKEN_STRING;
    token->m_data.str._string = p;
    token->m_data.str._end = NULL;

    return 0;
}

static int xcomputer_do_add(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "add data error"); 
        return -1;
    }

    if (xtoken_get_type(left) == XTOKEN_STRING || xtoken_get_type(right) == XTOKEN_STRING) {
        mem_buffer_clear_data(&context->m_computer->m_tmp_buffer);
        xcomputer_tmp_buffer_append_token(context->m_computer, left);
        xcomputer_tmp_buffer_append_token(context->m_computer, right);
        if (xcomputer_set_token_str_from_buffer(context->m_computer, left) != 0) return -1;
    }
    else {
        if (xtoken_get_type(left) == XTOKEN_NUM_INT) {
            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_int(context->m_computer, left, left->m_data.num._int + right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, (left->m_data.num._int + right->m_data.num._double));
            }
        }
        else {
            assert(xtoken_get_type(left) == XTOKEN_NUM_FLOAT);

            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double + right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, (left->m_data.num._double + right->m_data.num._double));
            }
        }
    }

    return 2;
}

static int xcomputer_do_sub(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "sub data error"); 
        return -1;
    }

    if (xtoken_get_type(left) == XTOKEN_STRING || xtoken_get_type(right) == XTOKEN_STRING) {
        CPE_ERROR(context->m_computer->m_em, "sub can`t operate add string"); 
        return -1;
    }
    else {
        if (xtoken_get_type(left) == XTOKEN_NUM_INT) {
            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_int(context->m_computer, left, left->m_data.num._int - right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, (double)left->m_data.num._int - right->m_data.num._double);
            }
        }
        else {
            assert(xtoken_get_type(left) == XTOKEN_NUM_FLOAT);

            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double - (double)right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double - right->m_data.num._double);
            }
        }
    }

    return 2;
}

static int xcomputer_do_mul(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "mul data error"); 
        return -1;
    }

    if (xtoken_get_type(left) == XTOKEN_STRING || xtoken_get_type(right) == XTOKEN_STRING) {
        CPE_ERROR(context->m_computer->m_em, "mul can`t operate add string"); 
        return -1;
    }
    else {
        if (xtoken_get_type(left) == XTOKEN_NUM_INT) {
            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_int(context->m_computer, left, left->m_data.num._int * right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, (double)left->m_data.num._int * right->m_data.num._double);
            }
        }
        else {
            assert(xtoken_get_type(left) == XTOKEN_NUM_FLOAT);

            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double * (double)right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double * right->m_data.num._double);
            }
        }
    }

    return 2;
}

static int xcomputer_do_div(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "div data error"); 
        return -1;
    }

    if (xtoken_get_type(left) == XTOKEN_STRING || xtoken_get_type(right) == XTOKEN_STRING) {
        CPE_ERROR(context->m_computer->m_em, "div can`t operate add string"); 
        return -1;
    }
    else {
        if (right->m_data.num._int == 0) {
            CPE_ERROR(context->m_computer->m_em, "div 0 !!!"); 
            return -1;
        }

        if (xtoken_get_type(left) == XTOKEN_NUM_INT) {
            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_int(context->m_computer, left, left->m_data.num._int / right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, (double)left->m_data.num._int / right->m_data.num._double);
            }
        }
        else {
            assert(xtoken_get_type(left) == XTOKEN_NUM_FLOAT);

            if (xtoken_get_type(right) == XTOKEN_NUM_INT) {
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double / (double)right->m_data.num._int);
            }
            else {
                assert(xtoken_get_type(right) == XTOKEN_NUM_FLOAT);
                xcomputer_set_token_float(context->m_computer, left, left->m_data.num._double / right->m_data.num._double);
            }
        }
    }

    return 2;
}

static int xcomputer_do_eq(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "eq data error"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, xtoken_cmp(left, right) == 0 ? 1 : 0);

    return 2;
}

static int xcomputer_do_ne(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "ne data error"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, xtoken_cmp(left, right) != 0 ? 1 : 0);

    return 2;
}

static int xcomputer_do_bg(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "bg data error"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, xtoken_cmp(left, right) > 0 ? 1 : 0);

    return 2;
}

static int xcomputer_do_be(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "be data error"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, xtoken_cmp(left, right) >= 0 ? 1 : 0);

    return 2;
}

static int xcomputer_do_lt(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "lt data error"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, xtoken_cmp(left, right) < 0 ? 1 : 0);

    return 2;
}

static int xcomputer_do_le(xcontext_t context) {
    xtoken_t left, right;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "le data error"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, xtoken_cmp(left, right) <= 0 ? 1 : 0);

    return 2;
}

static int xcomputer_do_and(xcontext_t context) {
    xtoken_t left, right;
    uint8_t left_v, right_v;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "and data error"); 
        return -1;
    }

    if (xtoken_try_to_bool(left, &left_v) != 0) {
        CPE_ERROR(context->m_computer->m_em, "and left as bool fail"); 
        return -1;
    }

    if (xtoken_try_to_bool(right, &right_v) != 0) {
        CPE_ERROR(context->m_computer->m_em, "and left as bool fail"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, (left_v && right_v) ? 1 : 0);

    return 2;
}

static int xcomputer_do_or(xcontext_t context) {
    xtoken_t left, right;
    uint8_t left_v, right_v;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((left == NULL || !xtoken_is_data(left)) || (right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "or data error"); 
        return -1;
    }

    if (xtoken_try_to_bool(left, &left_v) != 0) {
        CPE_ERROR(context->m_computer->m_em, "or left as bool fail"); 
        return -1;
    }

    if (xtoken_try_to_bool(right, &right_v) != 0) {
        CPE_ERROR(context->m_computer->m_em, "or left as bool fail"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, left, (left_v || right_v) ? 1 : 0);

    return 2;
}

static int xcomputer_do_not(xcontext_t context) {
    xtoken_t right;
    uint8_t right_v;

    assert(context->m_sign_token);

    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((right == NULL || !xtoken_is_data(right))) {
        CPE_ERROR(context->m_computer->m_em, "right data error"); 
        return -1;
    }

    if (xtoken_try_to_bool(right, &right_v) != 0) {
        CPE_ERROR(context->m_computer->m_em, "not right as bool fail"); 
        return -1;
    }

    xcomputer_set_token_int(context->m_computer, context->m_sign_token, (!right_v) ? 1 : 0);

    return 1;
}

static int xcomputer_do_left_bracket(xcontext_t context) {
    xtoken_t right;

    assert(context->m_sign_token);

    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if (right != TAILQ_FIRST(&context->m_tokens)) {
        CPE_ERROR(context->m_computer->m_em, "operator() right arg num error"); 
        return -1;
    }

    TAILQ_REMOVE(&context->m_tokens, context->m_sign_token, m_next);
    xcomputer_free_token(context->m_computer, context->m_sign_token);
    xcontext_update_sign_token(context);
    xcomputer_free_token(context->m_computer, context->m_cur_token);
    context->m_cur_token = NULL;

    return 0;
}

static int xcomputer_do_right_bracket(xcontext_t context) {
    xtoken_t sign;

    sign = context->m_sign_token;
    assert(sign);

    TAILQ_REMOVE(&context->m_tokens, sign, m_next);
    xcomputer_free_token(context->m_computer, sign);
    xcontext_update_sign_token(context);

    return 0;
}

static int xcomputer_do_colon(xcontext_t context) {
    xtoken_t left, right;
    xtoken_t old_sub;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);
    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);

    if ((right == NULL || !xtoken_is_data(right)) || (left == NULL || !xtoken_is_data(left))) {
        CPE_ERROR(context->m_computer->m_em, "colon data error"); 
        return -1;
    }

    right = xcontext_pop_token(context);
    assert(left);

    old_sub = xtoken_set_sub(left, right);
    if (old_sub) {
        xcomputer_free_token(context->m_computer, old_sub);
    }

    return 1;
}

static int xcomputer_do_ask(xcontext_t context) {
    xtoken_t left;
    xtoken_t check;
    xtoken_t result = NULL;

    assert(context->m_sign_token);

    left = TAILQ_NEXT(context->m_sign_token, m_next);

    if (left == NULL || !xtoken_is_data(left)) {
        CPE_ERROR(context->m_computer->m_em, "ask data error"); 
        return -1;
    }

    for(check = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);
        result == NULL && check;
        check = TAILQ_PREV(check, xtoken_list, m_next))
    {
        xtoken_t sub_token = xtoken_set_sub(check, NULL);
        if (sub_token == NULL) {
            if (check == TAILQ_FIRST(&context->m_tokens)) {
                result = check;
            }
            else {
                CPE_ERROR(context->m_computer->m_em, "default data must at the end"); 
                return -1;
            }
        }
        else {
            if (xtoken_cmp(left, check) == 0) {
                result = sub_token;
            }
            else {
                xcomputer_free_token(context->m_computer, sub_token);
            }
        }
    }

    if (result == NULL) {
        CPE_ERROR(context->m_computer->m_em, "ask format error!"); 
        return -1;
    }

    while(TAILQ_FIRST(&context->m_tokens) != context->m_sign_token) {
        check = xcontext_pop_token(context);
        if (check != result) {
            xcomputer_free_token(context->m_computer, check);
        }
    }

    assert(TAILQ_FIRST(&context->m_tokens) == context->m_sign_token);
    xcomputer_free_token(context->m_computer, xcontext_pop_token(context));
    xcomputer_free_token(context->m_computer, xcontext_pop_token(context));
    TAILQ_INSERT_HEAD(&context->m_tokens, result, m_next);

    return 0;
}

static int xcomputer_do_comma(xcontext_t context) {
    xtoken_t right;

    assert(context->m_sign_token);

    right = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);
    if (right == NULL || !xtoken_is_data(right)) {
        CPE_ERROR(context->m_computer->m_em, "comma data error"); 
        return -1;
    }

    if (right != TAILQ_FIRST(&context->m_tokens)) {
        CPE_ERROR(context->m_computer->m_em, "comma format error"); 
        return -1;
    }

    right = xcontext_pop_token(context);

    xcomputer_free_token(context->m_computer, xcontext_pop_token(context));
    
    TAILQ_INSERT_HEAD(&context->m_tokens, right, m_next);

    return 0;
}

static int xcomputer_do_func(xcontext_t context) {
    xtoken_t fun;
    xtoken_t arg;
    struct xtoken_it args;
    xtoken_t result;
    struct xcomputer_fun_def * def;

    assert(context->m_sign_token);

    fun = context->m_sign_token;

    /*寻找函数定义 */
    assert(fun->m_data.str._string);
    if (fun->m_data.str._end) {
        char s = *fun->m_data.str._end;
        *fun->m_data.str._end = 0;

        def = xcomputer_find_fun_def(context->m_computer, fun->m_data.str._string);
        if (def == NULL) {
            CPE_ERROR(context->m_computer->m_em, "function %s not exist", fun->m_data.str._string); 
        }
        *fun->m_data.str._end = s;
    }
    else {
        def = xcomputer_find_fun_def(context->m_computer, fun->m_data.str._string);
        if (def == NULL) {
            CPE_ERROR(context->m_computer->m_em, "function %s not exist", fun->m_data.str._string); 
        }
    }

    if (def == NULL) return -1;

    /*构造函数参数列表 */
    xcomputer_token_it_init(&args);
    for(arg = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next);
        arg && xtoken_is_data(arg);
        arg = TAILQ_PREV(context->m_sign_token, xtoken_list, m_next))
    {
        if (arg->m_type == XTOKEN_STRING && arg->m_data.str._end) {
            if (xcomputer_dup_token_str(context->m_computer, arg) != 0) {
                CPE_ERROR(context->m_computer->m_em, "do function: dup str fail fail");
                xcomputer_token_it_fini(context->m_computer, &args);
                return -1;
            }
        }

        TAILQ_REMOVE(&context->m_tokens, arg, m_next);
        TAILQ_INSERT_HEAD(&args.m_not_visited, arg, m_next);
    }

    result = def->m_fun(def->m_fun_ctx, context->m_computer, def->m_func_name, &args, context->m_computer->m_em);
    xcomputer_token_it_fini(context->m_computer, &args);

    if (result == NULL) {
        CPE_ERROR(context->m_computer->m_em, "function %s eval fail", def->m_func_name);
        return -1;
    }

    xcomputer_free_token(context->m_computer, xcontext_pop_token(context));
    xcomputer_free_token(context->m_computer, context->m_cur_token);
    context->m_cur_token = NULL;

    TAILQ_INSERT_HEAD(&context->m_tokens, result, m_next);

    return 0;
}

static const char s_sign_pri_tab[XTOKEN_END + 1][XTOKEN_END + 1] = {
          /* +  -   *   /   ==  !=  >   >=  <   <=  &&  ||  !   (   )   :   ?   ,   fun end */
/* +   */ { bg, bg, lt, lt, bg, bg, bg, bg, bg, bg, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* -   */ { bg, bg, lt, lt, bg, bg, bg, bg, bg, bg, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* *   */ { bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* /   */ { bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* ==  */ { lt, lt, lt, lt, bg, bg, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* !=  */ { lt, lt, lt, lt, bg, bg, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* >   */ { lt, lt, lt, lt, bg, bg, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* >=  */ { lt, lt, lt, lt, bg, bg, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* <   */ { lt, lt, lt, lt, bg, bg, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* <=  */ { lt, lt, lt, lt, bg, bg, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* &&  */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* ||  */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* !   */ { bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, lt, lt, bg, bg, bg, bg, lt, bg },
/* (   */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, _er(3), lt, er, lt, _er(1) },
/* )   */ { bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, bg, _er(4), _er(5), bg, bg, bg, bg, er, bg },
/* :   */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, bg, lt, bg, lt, bg },
/* ?   */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, lt, lt, lt, lt, bg },
/* ,   */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, lt, lt, bg, lt, bg },
/* fun */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg, lt, lt, lt, lt, _er(2) },
/* end */ { lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, lt, bg },
};

static xcomputer_op_fun_t s_op_funs[XTOKEN_END] = {
/* +   */ xcomputer_do_add,
/* -   */ xcomputer_do_sub,
/* *   */ xcomputer_do_mul,
/* /   */ xcomputer_do_div,
/* ==  */ xcomputer_do_eq,
/* !=  */ xcomputer_do_ne,
/* >   */ xcomputer_do_bg,
/* >=  */ xcomputer_do_be,
/* <   */ xcomputer_do_lt,
/* <=  */ xcomputer_do_le,
/* &&  */ xcomputer_do_and,
/* ||  */ xcomputer_do_or,
/* !   */ xcomputer_do_not,
/* (   */ xcomputer_do_left_bracket,
/* )   */ xcomputer_do_right_bracket,
/* :   */ xcomputer_do_colon,
/* ?   */ xcomputer_do_ask,
/* ,   */ xcomputer_do_comma,
/* fun */ xcomputer_do_func
};
