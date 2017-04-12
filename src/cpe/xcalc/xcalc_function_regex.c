#include "pcre2.h"
#include "xcalc_function_i.h"
#include "xcalc_token_i.h"

xtoken_t xcomputer_func_regex(void * ctx, xcomputer_t computer, const char * func_name, xtoken_it_t args, error_monitor_t em) {
    xtoken_t arg1;
    xtoken_t arg2;
    xtoken_t result;
    pcre2_code * re;
    pcre2_match_data * match_data;
    PCRE2_SIZE erroroffset;
    int errornumber;
    int rc;
    PCRE2_SIZE *ovector;

    if (xcomputer_func_get_arg_2(&arg1, &arg2, func_name, args, em) != 0) return NULL;

    if (arg1->m_type != XTOKEN_STRING) {
        CPE_ERROR(em, "%s: regex arg1 is not string!", func_name);
        return NULL;
    }

    if (arg2->m_type != XTOKEN_STRING) {
        CPE_ERROR(em, "%s: regex arg1 is not string!", func_name);
        return NULL;
    }

    re = pcre2_compile(
        (PCRE2_SPTR)arg1->m_data.str._string,
        arg1->m_data.str._end ? (arg1->m_data.str._end - arg1->m_data.str._string) : PCRE2_ZERO_TERMINATED,
        0, &errornumber, &erroroffset, NULL);
    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        CPE_ERROR(em, "xcomputer: %s: compilation failed at offset %d: %s\n", func_name, (int)erroroffset, buffer);
        return NULL;
    }

    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    rc = pcre2_match(
        re,
        (PCRE2_SPTR)arg2->m_data.str._string,
        arg2->m_data.str._end ? (arg2->m_data.str._end - arg2->m_data.str._string) : PCRE2_ZERO_TERMINATED,
        0, 0, match_data, NULL);
    
    if (rc < 0) {
        if(rc == PCRE2_ERROR_NOMATCH) {
            result = xcomputer_func_create_token_str_dup(computer, func_name, "", em);
        }
        else {
            CPE_ERROR(em, "xcomputer: %s: match error, rv=%d", func_name, rc);
            result = NULL;
        }
    }
    else if (rc == 0 || rc < 1) {
        result = xcomputer_func_create_token_str_dup(computer, func_name, "", em);
    }
    else {
        ovector = pcre2_get_ovector_pointer(match_data);
        result = xcomputer_func_create_token_str_dup_range(
            computer, func_name,
            arg2->m_data.str._string + ovector[2],
            arg2->m_data.str._string + ovector[3],
            em);
    }
    
    pcre2_code_free(re);
    pcre2_match_data_free(match_data);
    
    return result;
}

