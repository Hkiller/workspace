#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/string_utils.h"
#include "xcalc_computer_i.h"
#include "xcalc_token_i.h"
#include "xcalc_context_i.h"

xcomputer_t xcomputer_create(mem_allocrator_t alloc, error_monitor_t em) {
    xcomputer_t computer = mem_alloc(alloc, sizeof(struct xcomputer));
    if (computer == NULL) {
        CPE_ERROR(em, "xcomputer_create: alloc fail!");
        return NULL;
    }

    computer->m_alloc = alloc;
    computer->m_em = em;
    computer->m_allocked_token_count = 0;

    if (cpe_hash_table_init(
            &computer->m_funcs,
            alloc,
            (cpe_hash_fun_t) xcomputer_fun_def_hash,
            (cpe_hash_eq_t) xcomputer_fun_def_eq,
            CPE_HASH_OBJ2ENTRY(xcomputer_fun_def, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "xcomputer_create: init func hash fail!");
        mem_free(alloc, computer);
        return NULL;
    }
    
    if (xcomputer_load_default_funcs(computer) != 0) {
        xcomputer_free(computer);
        return NULL;
    }

    return computer;
}

void xcomputer_free(xcomputer_t computer) {
	xcomputer_func_def_free_all(computer);

	cpe_hash_table_fini(&computer->m_funcs);

	assert(computer->m_allocked_token_count == 0);
	mem_free(computer->m_alloc, computer);
}

void xcomputer_token_it_init(xtoken_it_t token_it) {
    TAILQ_INIT(&token_it->m_not_visited);
    TAILQ_INIT(&token_it->m_visited);
}

void xcomputer_token_it_fini(xcomputer_t computer, xtoken_it_t token_it) {
    while(!TAILQ_EMPTY(&token_it->m_visited)) {
        xtoken_t token = TAILQ_FIRST(&token_it->m_visited);
        TAILQ_REMOVE(&token_it->m_visited, token, m_next);
        xcomputer_free_token(computer, token);
    }

    while(!TAILQ_EMPTY(&token_it->m_not_visited)) {
        xtoken_t token = TAILQ_FIRST(&token_it->m_not_visited);
        TAILQ_REMOVE(&token_it->m_not_visited, token, m_next);
        xcomputer_free_token(computer, token);
    }
}

xtoken_t xtoken_it_next(xtoken_it_t token_it) {
    xtoken_t r = TAILQ_FIRST(&token_it->m_not_visited);

    if (r) {
        TAILQ_REMOVE(&token_it->m_not_visited, r, m_next);
        TAILQ_INSERT_TAIL(&token_it->m_visited, r, m_next);
    }

    return r;
}

int xcomputer_dup_token_str(xcomputer_t computer , xtoken_t token) {
    size_t len;
    char * p;

    assert(token->m_type == XTOKEN_STRING || token->m_type == XTOKEN_VAL);
    assert(token->m_data.str._string);
    assert(token->m_data.str._string <= token->m_data.str._end);

    len = token->m_data.str._end - token->m_data.str._string;

    p = mem_alloc(computer->m_alloc, len + 1);
    if (p == NULL) return -1;

    memcpy(p, token->m_data.str._string, len);
    p[len] = 0;

    token->m_data.str._string = p;
    token->m_data.str._end = NULL;

    return 0;
}

int xcomputer_compute_bool(xcomputer_t computer, const char * str, xcomputer_args_t args, uint8_t * r) {
    xtoken_t r_token = xcomputer_compute(computer, str, args);
    int ret;

    if (r_token == NULL) return -1;

    assert(xtoken_is_data(r_token));

    ret = xtoken_try_to_bool(r_token, r);

    xcomputer_free_token(computer, r_token);

    return ret;
}


xtoken_t xcomputer_alloc_token(xcomputer_t computer) {
    xtoken_t r = mem_alloc(computer->m_alloc, sizeof(struct xtoken));

    if (r == NULL) {
        CPE_ERROR(computer->m_em, "xcomputer_alloc_token: alloc fail!");
        return NULL;
    }

    r->m_type = (uint32_t)-1;
    r->m_sub = NULL;

    computer->m_allocked_token_count++;

    mem_buffer_init(&computer->m_tmp_buffer, computer->m_alloc);

    return r;
}

void xcomputer_free_token(xcomputer_t computer , xtoken_t token) {
    assert(token);
    assert(computer->m_allocked_token_count > 0);

    mem_buffer_clear(&computer->m_tmp_buffer);

    if (token->m_sub) {
        xcomputer_free_token(computer, token->m_sub);
        token->m_sub = NULL;
    }

    if (token->m_type == XTOKEN_STRING && token->m_data.str._end == NULL) {
        mem_free(computer->m_alloc, token->m_data.str._string);
    }

    --computer->m_allocked_token_count;

    mem_free(computer->m_alloc, token);
}


int xcomputer_add_func(xcomputer_t computer, const char * func_name, xcalc_func_t fun, void * fun_ctx) {
    size_t name_len = strlen(func_name) + 1;
    struct xcomputer_fun_def * def;

    def = mem_alloc(computer->m_alloc, sizeof(struct xcomputer_fun_def) + name_len);
    if (def == NULL) {
        CPE_ERROR(computer->m_em, "xcomputer: add fun %s: alloc fail", func_name);
        return -1;
    }

    memcpy(def + 1, func_name, name_len);

    def->m_func_name = (char*)(def + 1);
    def->m_fun = fun;
    def->m_fun_ctx = fun_ctx;

    cpe_hash_entry_init(&def->m_hh);

    if (cpe_hash_table_insert_unique(&computer->m_funcs, def) != 0) {
        CPE_ERROR(computer->m_em, "xcomputer: add fun %s: name duplicate", func_name);
        mem_free(computer->m_alloc, def);
        return -1;
    }

    return 0;
}

void xcomputer_remove_func_by_name(xcomputer_t computer, const char * func_name) {
    struct xcomputer_fun_def * def;
    struct xcomputer_fun_def key;

    key.m_func_name = func_name;

    def = cpe_hash_table_find(&computer->m_funcs, &key);
    if (def == NULL) return;

    cpe_hash_table_remove_by_ins(&computer->m_funcs, def);
    mem_free(computer->m_alloc, def);
}

void xcomputer_func_def_free_all(xcomputer_t computer) {
    struct cpe_hash_it func_def_it;
    struct xcomputer_func_def * func_def;

    cpe_hash_it_init(&func_def_it, &computer->m_funcs);

    func_def = cpe_hash_it_next(&func_def_it);
    while (func_def) {
        struct xcomputer_func_def * next = cpe_hash_it_next(&func_def_it);

        cpe_hash_table_remove_by_ins(&computer->m_funcs, func_def);
        mem_free(computer->m_alloc, func_def);

        func_def = next;
    }
}

struct xcomputer_fun_def * xcomputer_find_fun_def(xcomputer_t computer, const char * func_name) {
    struct xcomputer_fun_def key;
    key.m_func_name = func_name;
    return cpe_hash_table_find(&computer->m_funcs, &key);
}

uint32_t xcomputer_fun_def_hash(struct xcomputer_fun_def * func_def) {
    return cpe_hash_str(func_def->m_func_name, strlen(func_def->m_func_name));
}

int xcomputer_fun_def_eq(struct xcomputer_fun_def * l, struct xcomputer_fun_def * r) {
    return strcmp(l->m_func_name, r->m_func_name) == 0;
}

static xtoken_t xcomputer_find_value_from_str_process_one(
    xcomputer_t computer,
    const char * input, size_t len,
    const char * arg_name, size_t name_len, char pair)
{
    const char * s;
    xtoken_t r;
    const char * r_begin;
    const char * r_end;

    if (len < name_len+1) return NULL;

    if (memcmp(input, arg_name, name_len) != 0) return NULL;

    s = cpe_str_trim_head((char *)(input + name_len));
    if (*s != pair) return NULL;

    r_begin = cpe_str_trim_head((char *)(s + 1));
    r_end = cpe_str_trim_tail((char *)(input + len), r_begin);

    r = xcomputer_alloc_token(computer);
    if (r == NULL) return NULL;

    if (xcomputer_set_token_str_range(computer, r, r_begin, r_end) != 0) {
        xcomputer_free_token(computer, r);
        return NULL;
    }
    
    return r;
}

xtoken_t xcomputer_find_value_from_str(void * input_ctx, xcomputer_t computer, const char * arg_name, error_monitor_t em) {
    struct xcomputer_find_value_from_str_ctx * ctx = (struct xcomputer_find_value_from_str_ctx *)input_ctx;
    const char * p = cpe_str_trim_head((char *)ctx->m_str);
    size_t name_len = strlen(arg_name);
    const char * s;
    
    while((s = strchr(p, ctx->m_sep))) {
        xtoken_t r = xcomputer_find_value_from_str_process_one(computer, p, s - p, arg_name, name_len, ctx->m_pair);
        if (r) return r;
        p = cpe_str_trim_head((char *)(s + 1));
    }

    return xcomputer_find_value_from_str_process_one(computer, p, strlen(p), arg_name, name_len, ctx->m_pair);
}

