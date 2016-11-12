#include "cpe/utils/buffer.h"
#include "XComputerTest.hpp"

void XComputerTest::SetUp() {
    Base::SetUp();
    m_computer = xcomputer_create(t_allocrator(), t_em());
    m_result = NULL;
}

void XComputerTest::TearDown() {
    if (m_result) {
        xcomputer_free_token(m_computer, m_result);
        m_result = NULL;
    }

    if (m_computer) {
        xcomputer_free(m_computer);
        m_computer = NULL;
    }

    Base::TearDown();
}

xtoken_t find_arg_from_str(const char * begin, const char * end, xcomputer_t computer, const char * arg_name) {
    int n = strlen(arg_name);
    char * sep = strchr(begin, '=');
    char * endptr;
    long value_l;
    double value_d;

    if (sep == NULL || sep >= end) return NULL;

    while(begin < end && (*begin == ' ' || *begin == '\n')) begin++;

    if (memcmp(begin, arg_name, n) != 0) return NULL;

    begin += n;
    while(begin < sep && (*begin == ' ' || *begin == '\n')) begin++;


    if (*begin != '=') return NULL;

    begin += 1;
    while(begin < end && (*begin == ' ' || *begin == '\n')) begin++;

    value_l = strtol(begin, &endptr, 10);
    while(endptr < end && (*begin == ' ' || *begin == '\n')) endptr++;
    if(endptr == end) {
        xtoken_t r = xcomputer_alloc_token(computer);
        if (r == NULL) return NULL;
        xcomputer_set_token_int(computer, r, value_l);
        return r;
    }

    value_d = strtod(begin, &endptr);
    while(endptr < end && (*begin == ' ' || *begin == '\n')) endptr++;
    if(endptr == end) {
        xtoken_t r = xcomputer_alloc_token(computer);
        if (r == NULL) return NULL;
        xcomputer_set_token_float(computer, r, value_d);
        return r;
    }

    xtoken_t r = xcomputer_alloc_token(computer);
    if (r == NULL) return NULL;
    xcomputer_set_token_str_range(computer, r, begin, end);
    return r;
}

xtoken_t find_arg_from_str(void * ctx, xcomputer_t computer, const char * arg_name, error_monitor_t em) {
    const char * str_args = (const char *)ctx;
    const char * sep;

    while((sep = strchr(str_args, ','))) {
        xtoken_t r = find_arg_from_str(str_args, sep, computer, arg_name);
        if (r) return r;

        str_args = sep + 1;
    }
    
    return find_arg_from_str(str_args, str_args + strlen(str_args), computer, arg_name);
}

bool XComputerTest::calc(const char * def, const char * str_args) {
    if (m_result) {
        xcomputer_free_token(m_computer, m_result);
        m_result = NULL;
    }

    if (str_args) {
        struct xcomputer_args args;
        args.m_ctx = (void*)str_args;
        args.m_find_arg = find_arg_from_str;
        m_result = xcomputer_compute(m_computer, def, &args);
    }
    else {
        m_result = xcomputer_compute(m_computer, def, NULL);
    }

    return m_result != NULL;
}
