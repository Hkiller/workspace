#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "xcalc_token_i.h"

int xtoken_try_to_int32(xtoken_t token, int32_t * r) {
    char * endptr = NULL;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r = (int32_t)token->m_data.num._int;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = (int32_t)token->m_data.num._double;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            char s = *token->m_data.str._end;
            *r = (int32_t)strtol(token->m_data.str._string, &endptr, 10);
            *token->m_data.str._end = s;
            return endptr == token->m_data.str._end ? 0 : -1;
        }
        else {
            *r = (int32_t)strtol(token->m_data.str._string, &endptr, 10);
            return (endptr && *endptr == 0) ? 0 : -1;
        }
    default:
        return -1;
    }
}

int xtoken_try_to_uint32(xtoken_t token, uint32_t * r) {
    char * endptr = NULL;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r = (int32_t)token->m_data.num._int;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = (int32_t)token->m_data.num._double;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            char s = *token->m_data.str._end;
            *r = (uint32_t)strtol(token->m_data.str._string, &endptr, 10);
            *token->m_data.str._end = s;
            return endptr == token->m_data.str._end ? 0 : -1;
        }
        else {
            *r = (uint32_t)strtol(token->m_data.str._string, &endptr, 10);
            return (endptr && *endptr == 0) ? 0 : -1;
        }
    default:
        return -1;
    }
}

int xtoken_try_to_int64(xtoken_t token, int64_t * r) {
    char * endptr = NULL;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r = token->m_data.num._int;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = (int64_t)token->m_data.num._double;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            char s = *token->m_data.str._end;
            *r = strtol(token->m_data.str._string, &endptr, 10);
            *token->m_data.str._end = s;
            return endptr == token->m_data.str._end ? 0 : -1;
        }
        else {
            *r = strtol(token->m_data.str._string, &endptr, 10);
            return (endptr && *endptr == 0) ? 0 : -1;
        }
    default:
        return -1;
    }
}

int xtoken_try_to_uint64(xtoken_t token, uint64_t * r) {
    char * endptr = NULL;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r = token->m_data.num._int;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = (uint64_t)token->m_data.num._double;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            char s = *token->m_data.str._end;
            *r = (uint64_t)strtol(token->m_data.str._string, &endptr, 10);
            *token->m_data.str._end = s;
            return endptr == token->m_data.str._end ? 0 : -1;
        }
        else {
            *r = (uint64_t)strtol(token->m_data.str._string, &endptr, 10);
            return (endptr && *endptr == 0) ? 0 : -1;
        }
    default:
        return -1;
    }
}

int xtoken_try_to_float(xtoken_t token, float * r) {
    char * endptr = NULL;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r =(float)token->m_data.num._int;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = (float)token->m_data.num._double;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            char s = *token->m_data.str._end;
            *r = strtod(token->m_data.str._string, &endptr);
            *token->m_data.str._end = s;
            return endptr == token->m_data.str._end ? 0 : -1;
        }
        else {
            *r = strtof(token->m_data.str._string, &endptr);
            return (endptr && *endptr == 0) ? 0 : -1;
        }
    default:
        return -1;
    }
}

int xtoken_try_to_double(xtoken_t token, double * r) {
    char * endptr = NULL;

    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r = token->m_data.num._int;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = token->m_data.num._double;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            char s = *token->m_data.str._end;
            *r = strtod(token->m_data.str._string, &endptr);
            *token->m_data.str._end = s;
            return endptr == token->m_data.str._end ? 0 : -1;
        }
        else {
            *r = strtod(token->m_data.str._string, &endptr);
            return (endptr && *endptr == 0) ? 0 : -1;
        }
    default:
        return -1;
    }
}

int xtoken_try_to_bool(xtoken_t token, uint8_t * r) {
    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        *r = token->m_data.num._int ? 1 : 0;
        return 0;
    case XTOKEN_NUM_FLOAT:
        *r = token->m_data.num._double ? 1 : 0;
        return 0;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        assert(token->m_data.str._string);
        if (token->m_data.str._end) {
            *r = (token->m_data.str._end > token->m_data.str._string) ? 1 : 0;
        }
        else {
            *r = token->m_data.str._string[0] ? 1 : 0;
        }
        return 0;
    default:
        return -1;
    }
}

xtoken_data_type_t xtoken_data_type(xtoken_t token) {
    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        return xtoken_data_int;
    case XTOKEN_NUM_FLOAT:
        return xtoken_data_double;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        return xtoken_data_str;
    default:
        return xtoken_data_none;
    }
}

const char * xtoken_try_to_str(xtoken_t token) {
    if (token->m_type != XTOKEN_STRING && token->m_type != XTOKEN_VAL) return NULL;

    if (token->m_data.str._end) return NULL;

    return token->m_data.str._string;
}

const char * xtoken_to_str(xtoken_t token, char * buf, size_t buf_size) {
    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        snprintf(buf, buf_size, FMT_INT64_T, token->m_data.num._int);
        return buf;
    case XTOKEN_NUM_FLOAT:
        snprintf(buf, buf_size, "%f", (float)token->m_data.num._double);
        return buf;
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        if (token->m_data.str._end == NULL) {
            return token->m_data.str._string;
        }
        else {
            return cpe_str_dup_range(buf, buf_size, token->m_data.str._string, token->m_data.str._end);
        }
    default:
        cpe_str_dup(buf, buf_size, "N/A");
        return buf;
    }
}

const char * xtoken_to_str_with_buffer(xtoken_t token, mem_buffer_t buffer) {
    char buf[64];
    
    mem_buffer_clear_data(buffer);
    
    switch(token->m_type) {
    case XTOKEN_NUM_INT:
        snprintf(buf, sizeof(buf), FMT_INT64_T, token->m_data.num._int);
        return mem_buffer_strdup(buffer, buf);
    case XTOKEN_NUM_FLOAT:
        snprintf(buf, sizeof(buf), "%f", (float)token->m_data.num._double);
        return mem_buffer_strdup(buffer, buf);
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        if (token->m_data.str._end == NULL) {
            return token->m_data.str._string;
        }
        else {
            return mem_buffer_strdup_range(buffer, token->m_data.str._string, token->m_data.str._end);
        }
    default:
        return NULL;
    }
}

static int xtoken_do_str_cmp(const char * p1, int len1, const char * p2, int len2) {
    if (len1 < len2) {
        int r = memcmp(p1, p2, len1);
        return r == 0 ? -1 : r;
    }
    else if (len1 > len2) {
        int r = memcmp(p1, p2, len2);
        return r == 0 ? 1 : r;
    }
    else {
        return memcmp(p1, p2, len2);
    }
}

int xtoken_cmp(xtoken_t l, xtoken_t r) {
    char buf[32];
    double buf_double;
    assert(xtoken_is_data(l));
    assert(xtoken_is_data(r));

    switch(l->m_type) {
    case XTOKEN_NUM_INT:
        switch(r->m_type) {
        case XTOKEN_NUM_INT:
            return (int)(l->m_data.num._int - r->m_data.num._int);
        case XTOKEN_NUM_FLOAT:
            return l->m_data.num._int < r->m_data.num._double ? -1 : (l->m_data.num._int == r->m_data.num._double ? 0 : 1);
        case XTOKEN_STRING:
        case XTOKEN_VAL:
            if (xtoken_try_to_double(r, &buf_double) == 0) {
                return l->m_data.num._int < buf_double ? -1 : (l->m_data.num._int == buf_double ? 0 : 1);
            }
            else {
                return xtoken_do_str_cmp(
                    buf,
                    snprintf(buf, sizeof(buf), FMT_INT64_T, l->m_data.num._int),
                    r->m_data.str._string,
                    r->m_data.str._end ? (int)(r->m_data.str._end - r->m_data.str._string) : (int)strlen(r->m_data.str._string));
            }
        default:
            assert(0);
            return 0;
        }
    case XTOKEN_NUM_FLOAT:
        switch(r->m_type) {
        case XTOKEN_NUM_INT:
            return l->m_data.num._double < r->m_data.num._int ? -1 : (l->m_data.num._double == r->m_data.num._int ? 0 : 1);
        case XTOKEN_NUM_FLOAT:
            return l->m_data.num._double < r->m_data.num._double ? -1 : (l->m_data.num._double == r->m_data.num._double ? 0 : 1);
        case XTOKEN_STRING:
        case XTOKEN_VAL:
            if (xtoken_try_to_double(r, &buf_double) == 0) {
                return l->m_data.num._double < buf_double ? -1 : (l->m_data.num._double == buf_double ? 0 : 1);
            }
            else {
                return xtoken_do_str_cmp(
                    buf,
                    snprintf(buf, sizeof(buf), "%f", (float)l->m_data.num._double),
                    r->m_data.str._string,
                    (int)(r->m_data.str._end ? (r->m_data.str._end - r->m_data.str._string) : strlen(r->m_data.str._string)));
            }
        default:
            assert(0);
            return 0;
        }
    case XTOKEN_STRING:
    case XTOKEN_VAL:
        switch(r->m_type) {
        case XTOKEN_NUM_INT:
            if (xtoken_try_to_double(l, &buf_double) == 0) {
                return buf_double < r->m_data.num._int ? -1 : (buf_double == r->m_data.num._int ? 0 : 1);
            }
            else {
                return xtoken_do_str_cmp(
                    l->m_data.str._string,
                    (int)(l->m_data.str._end ? (l->m_data.str._end - l->m_data.str._string) : strlen(l->m_data.str._string)),
                    buf,
                    snprintf(buf, sizeof(buf), FMT_INT64_T, r->m_data.num._int));
            }
        case XTOKEN_NUM_FLOAT:
            if (xtoken_try_to_double(l, &buf_double) == 0) {
                return buf_double < r->m_data.num._double ? -1 : (buf_double == r->m_data.num._double ? 0 : 1);
            }
            else {
                return xtoken_do_str_cmp(
                l->m_data.str._string,
                (int)(l->m_data.str._end ? (l->m_data.str._end - l->m_data.str._string) : strlen(l->m_data.str._string)),
                buf,
                snprintf(buf, sizeof(buf), "%f", (float)r->m_data.num._double));
            }
        case XTOKEN_STRING:
        case XTOKEN_VAL:
            return xtoken_do_str_cmp(
                l->m_data.str._string,
                (int)(l->m_data.str._end ? (l->m_data.str._end - l->m_data.str._string) : strlen(l->m_data.str._string)),
                r->m_data.str._string,
                (int)(r->m_data.str._end ? (r->m_data.str._end - r->m_data.str._string) : strlen(r->m_data.str._string)));
        default:
            assert(0);
            return 0;
        }
    default:
        assert(0);
        return 0;
    }
}

xtoken_t xtoken_set_sub(xtoken_t token, xtoken_t sub) {
    xtoken_t old_sub = token->m_sub;
    token->m_sub = sub;
    return old_sub;
}

double xtoken_get_double_2(xtoken_t token) {
    if (token->m_type == XTOKEN_NUM_INT) {
        return (double)token->m_data.num._int;
    }
    else if (token->m_type == XTOKEN_STRING || token->m_type == XTOKEN_VAL) {
        return 0;
    }
    else {
        return token->m_data.num._double;
    }
}

void xtoken_dump(write_stream_t s, xtoken_t token) {
    stream_printf(s, "type: %s(%u)", xtoken_type_name(token->m_type), token->m_type);

    if (xtoken_is_data(token)) {
        switch(token->m_type) {
        case XTOKEN_NUM_INT:
            stream_printf(s, ", value="FMT_INT64_T, token->m_data.num._int);
            break;
        case XTOKEN_NUM_FLOAT:
            stream_printf(s, ", value=%f", (float)token->m_data.num._double);
            break;
        case XTOKEN_STRING:
        case XTOKEN_VAL:
            if (token->m_data.str._end) {
                char save = *token->m_data.str._end;
                *token->m_data.str._end = 0;
                stream_printf(s, ", value=%s", token->m_data.str._string);
                *token->m_data.str._end = save;
            }
            else {
                stream_printf(s, ", value=%s", token->m_data.str._string);
            }
            break;
        default:
            break;
        }
    }
}

static const char * g_token_names[XTOKEN_NUMBER] = {
    "+",
    "-",
    "*",
    "/",
    "==",
    "!-",
    ">",
    ">=",
    "<",
    "<=",
    "&&",
    "||",
    "!",
    "(",
    ")",
    ":",
    "?",
    ",",
    "fun",
    "end",
    "val",
    "int",
    "dou",
    "str",
    "err"
};

const char * xtoken_type_name(uint32_t token_type) {
    assert(token_type >= 0 && xtoken_type_index(token_type) < CPE_ARRAY_SIZE(g_token_names));
    return g_token_names[xtoken_type_index(token_type)];
}

void xtoken_set_str(xtoken_t token, char * begin, char * end) {
    token->m_data.str._string = begin;
    token->m_data.str._end = end;
}
