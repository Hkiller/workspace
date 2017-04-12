#include <assert.h>
#include <ctype.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"

char * cpe_str_dup(char * buf, size_t capacity, const char * str) {
    char * r;
    size_t len;
    
    assert(capacity > 0);

    len = strlen(str);
    if (len + 1 > capacity) len = capacity - 1;
    
    r = strncpy(buf, str, len);
    buf[len] = 0;

    return r;
}

char * cpe_str_dup_trim(char * buf, size_t capacity, const char * str) {
    const char * str_end;
    size_t len;
    char * r;
    
    if (str == NULL) return NULL;
    
    str = cpe_str_trim_head((char *)str);
    str_end = cpe_str_trim_tail((char *)str + strlen(str), str);

    len = str_end - str;
    
    assert(capacity > 0);
    if (len + 1 > capacity) len = capacity - 1;
    
    r = strncpy(buf, str, len);
    buf[len] = 0;

    return r;
}

char *
cpe_str_dup_range(char * buf, size_t capacity, const char * begin, const char * end) {
    size_t len;

    assert(end >= begin);
    
    len = (size_t)(end - begin);
    if (len + 1 > capacity) len = capacity - 1;

    memcpy(buf, begin, len);
    buf[len] = 0;

    return buf;
}

char *
cpe_str_dup_range_trim(char * buf, size_t capacity, const char * begin, const char * end) {
    size_t len;

    assert(end >= begin);

    begin = cpe_str_trim_head((char*)begin);
    end = end > begin ? cpe_str_trim_tail((char *)end, begin) : begin;

    len = (size_t)(end - begin);
    if (len + 1 > capacity) len = capacity - 1;

    memcpy(buf, begin, len);
    buf[len] = 0;

    return buf;
}

char *
cpe_str_dup_len(char * buf, size_t capacity, const char * begin, size_t size) {
    if (buf == NULL) return NULL;

    if (size + 1 > capacity) return NULL;

    memcpy(buf, begin, size);
    buf[size] = 0;

    return buf;
}

char * cpe_str_mem_dup(mem_allocrator_t alloc, const char * str) {
    size_t capacity;
    char * buf;

    if (str == NULL) return NULL;
    
    capacity = strlen(str) + 1;
    buf = mem_alloc(alloc, capacity);
    if (buf == NULL) return NULL;
    memcpy(buf, str, capacity);
    return buf;
}

char * cpe_str_mem_dup_range(mem_allocrator_t alloc, const char * str, const char * end) {
    size_t capacity = end - str;
    char * buf;

    buf = mem_alloc(alloc, capacity + 1);
    if (buf == NULL) return NULL;
    memcpy(buf, str, capacity);
    buf[capacity] = 0;
    
    return buf;
}

char * cpe_str_mem_dup_len(mem_allocrator_t alloc, const char * str, size_t len) {
    char * buf;

    buf = mem_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;
    memcpy(buf, str, len);
    buf[len] = 0;
    
    return buf;
}

char * cpe_str_mem_dup_trim(mem_allocrator_t alloc, const char * str) {
    const char * str_end;
    size_t len;
    char * buf;

    if (str == NULL) return NULL;
    
    str = cpe_str_trim_head((char *)str);
    str_end = cpe_str_trim_tail((char *)str + strlen(str), str);

    len = str_end - str;

    buf = mem_alloc(alloc, len + 1);
    if (buf == NULL) return NULL;

    memcpy(buf, str, len);
    buf[len] = 0;

    return buf;
}

int cpe_str_buf_is_overflow(cpe_str_buf_t buf) {
    return buf->m_overflow;
}

int cpe_str_buf_append(cpe_str_buf_t buf, const char * data, size_t data_len) {
    assert(buf);
    assert(data);
    assert(buf->m_buf);

    if (data_len + buf->m_size + 1 > buf->m_capacity) {
        data_len = buf->m_capacity - buf->m_size - 1;
        buf->m_overflow = 1;
    }

    if (data_len) {
        memcpy(buf->m_buf + buf->m_size, data, data_len + 1);
        buf->m_size += data_len;
    }

    buf->m_buf[buf->m_size] = 0;

    return buf->m_overflow;
}

int cpe_str_buf_cat(cpe_str_buf_t buf, const char * data) {
    return cpe_str_buf_append(buf, data, strlen(data));
}

int cpe_str_buf_cpy(cpe_str_buf_t buf, const char * data) {
    size_t data_len;

    assert(buf);
    assert(data);
    assert(buf->m_buf);

    data_len = strlen(data);

    if (data_len + 1 > buf->m_capacity) {
        data_len = buf->m_capacity - 1;
        buf->m_overflow = 1;
    }
    else {
        buf->m_overflow = 0;
    }

    if (data_len) {
        memcpy(buf->m_buf, data, data_len + 1);
        buf->m_size = data_len;
    }

    return buf->m_overflow;
}

int cpe_str_buf_cat_printf(cpe_str_buf_t buf, const char * fmt, ...) {
    va_list args;
    size_t capacity;
    int r;

    assert(buf);
    assert(fmt);
    assert(buf->m_buf);

    capacity = buf->m_capacity - buf->m_size;

    va_start(args, fmt);
    r = vsnprintf(buf->m_buf + buf->m_size, capacity, fmt, args);

    if (r < 0) {
        buf->m_overflow = 1;
    }
    else {
        buf->m_size += r;
    }

    va_end(args);

    return buf->m_overflow;
}

int cpe_str_buf_printf(cpe_str_buf_t buf, const char * fmt, ...) {
    va_list args;
    int r;

    assert(buf);
    assert(fmt);
    assert(buf->m_buf);

    va_start(args, fmt);
    r = vsnprintf(buf->m_buf, buf->m_capacity, fmt, args);

    if (r < 0) {
        buf->m_overflow = 1;
    }
    else {
        buf->m_size += r;
        buf->m_overflow = 0;
    }

    va_end(args);

    return buf->m_overflow;
}

void cpe_str_toupper(char * data) {
    while(*data) {
        *data = toupper(*data);
        ++data;
    }
}

void cpe_str_tolower(char * data) {
    while(*data) {
        *data = tolower(*data);
        ++data;
    }
}

const char * cpe_str_char_range(const char * begin, const char * end, char c) {
    for(; begin != end; begin++) {
        if (*begin == c) return begin;
    }
    return NULL;
}

const char * cpe_str_char_not_in_pair(const char * p, char f, const char * start_set, const char * end_set) {
    char v;
    int state = 0;

    for(; (v = *p); p++) {
        if (v == f && state == 0) return p;

        if (strchr(start_set, v)) {
            ++state;
        }
        else if (strchr(end_set, v)) {
            --state;
        }
    }

    return NULL;
}

int cpe_str_cmp_part(const char * part_str, size_t part_str_len, const char * full_str) {
    int r = strncmp(part_str, full_str, part_str_len);
    if (r == 0) return full_str[part_str_len] == 0 ? 0 : - (int)part_str_len;
    return r;
}

uint8_t cpe_str_start_with(const char * full_str, const char * prefix) {
    const char * f_p = full_str;
    const char * p_p = prefix;
    
    while(*f_p && *p_p) {
        if (*f_p != *p_p) return 0;
        f_p++;
        p_p++;
    }
    
    return *p_p == 0 ? 1 : 0;
}

uint64_t cpe_str_parse_byte_size_with_dft(const char * astring, uint64_t dft) {
    uint64_t r;
    if (cpe_str_parse_byte_size(&r, astring) != 0) return dft;
    return r;
}

int cpe_str_parse_byte_size(uint64_t * result, const char * astring) {
    size_t sz;
    char * last = NULL;
    long res;
    size_t numsize;

    if (astring == NULL) return -1;

    sz = strlen (astring);
    res = strtol(astring, &last, 10);
    if (res <= 0) return -1;

    assert(last);
    numsize  = last - astring;

    if (numsize == sz) {
        if (result) *result = res;
        return 0;
    }

    if (numsize + 2 != sz) return -1;

    if (astring[sz - 1] == 'B' || astring[sz - 1] == 'b') {
        switch (astring[ sz - 2 ]) {
	    case 'K':
	    case 'k':
            res *= 1024;
            break;
	    case 'M':
	    case 'm':
            res *= 1024 * 1024;
            break;
	    case 'G':
	    case 'g':
            res *= 1024 * 1024 * 1024;
            break;
	    default:
            return -1;
        }
    }

    if (result) *result = res;
    return 0;
}

uint64_t cpe_str_parse_timespan_ms_with_dft(const char * astring, uint64_t dft) {
    uint64_t r;
    if (cpe_str_parse_timespan_ms(&r, astring) != 0) return dft;
    return r;
}

int cpe_str_parse_timespan_ms(uint64_t * result, const char * astring) {
    size_t sz;
    char * last = NULL;
    long res;
    size_t numsize;
    size_t post_size;
    const char * post_str;

    if (astring == NULL) return -1;

    sz = strlen (astring);
    res = strtol(astring, &last, 10);
    if (res <= 0) return -1;

    assert(last);
    numsize  = last - astring;
    post_size = sz - numsize;

    post_str = astring + numsize;
    if (post_size == 1 && post_str[0] == 's') {
        res *= 1000;
    }
    else if (post_size == 1 && post_str[0] == 'm') {
        res *= 60 * 1000;
    }
    else if (post_size == 2 && post_str[0] == 'm' && post_str[1] == 's') {
    }
    else if (post_size == 1 && post_str[0] == 'h') {
        res *= 60 * 60 * 1000;
    }
    else {
        return -1;
    }

    if (result) *result = res;
    return 0;
}

char * cpe_str_trim_head(char * p) {
    unsigned char v;
    while((v = *(unsigned char *)p) != 0) {
        if (isspace(v)) {
            ++p;
        }
        else {
            return p;
        }
    }

    return p;
}

char * cpe_str_trim_tail(char * p, const char * head) {
    while((p - 1) >= head) {
        unsigned char v = *(unsigned char *)(p - 1);

        if (isspace(v)) {
            --p;
        }
        else {
            return p;
        }
        
    }

    return p;
}

char * cpe_str_mask_uint16(uint16_t v, char * buf, size_t buf_size) {
    uint8_t i;
    uint8_t bit_count = sizeof(v) * 8;
    assert(buf);
    assert(buf_size >= bit_count + 1);

    for(i = 0; i < bit_count; ++i) {
        buf[i] = (v & 1 << (bit_count - i - 1)) ? '1' : '0';
    }

    buf[bit_count] = 0;

    return buf;
}

char * cpe_str_read_and_remove_arg_pocess_arg(char * input, char const * arg_name, size_t name_len, char pair) {
    if (memcmp(input, arg_name, name_len) != 0) return NULL;

    input = cpe_str_trim_head(input += name_len);
    if (*input != pair) return NULL;

    return cpe_str_trim_head(input + 1);
}

char * cpe_str_read_and_remove_arg(char * input, const char * arg_name, char sep, char pair) {
    size_t name_len = strlen(arg_name);
    char * s;
    char * p = cpe_str_trim_head(input);
    char * end_p = p + strlen(p);
    char * r;

    while((s = (char*)cpe_str_char_not_in_pair(p, sep, "{[('\"", "\"')]}"))) {
        char * changed_pos = cpe_str_trim_tail(s, p);
        *changed_pos = 0;

        r = cpe_str_read_and_remove_arg_pocess_arg(p, arg_name, name_len, pair);

        if (r) {
            char inline_buf[128];
            size_t copy_len = strlen(r);
            size_t left_len = strlen(s + 1) + 1;

            if (r[0] == '\'' && r[copy_len - 1] == '\'') {
                r++; copy_len-= 2;
            }
            else if (r[0] == '"' && r[copy_len - 1] == '"') {
                r++; copy_len-= 2;
            }
                
            end_p = p + left_len;

            if (copy_len + 1 > CPE_ARRAY_SIZE(inline_buf)) {
                void * buf = cpe_str_mem_dup(NULL, r);
                memmove(p, s + 1, left_len);
                strcpy(p + left_len, buf);
                mem_free(NULL, buf);
            }
            else {
                cpe_str_dup(inline_buf, sizeof(inline_buf), r);
                memmove(p, s + 1, left_len);
                strcpy(p + left_len, inline_buf);
            }
                
            return p + left_len;
        }
        else {
            *changed_pos = sep;
            p = cpe_str_trim_head(s + 1);
        }
    }

    if (end_p - p > name_len) {
        r = cpe_str_read_and_remove_arg_pocess_arg(p, arg_name, name_len, pair);
        if (r) {
            *p = 0;

            end_p = cpe_str_trim_tail(r + strlen(r), r);
            * end_p = 0;

            if (r[0] == '\'' && *(end_p - 1) == '\'') {
                r++;
                *(end_p - 1) = 0;
            }
            else if (r[0] == '"' && *(end_p - 1) == '"') {
                r++;
                *(end_p - 1) = 0;
            }
            
            return r;
        }
    }
    
    return NULL;
}

int cpe_str_read_process_one(
    char * r, size_t r_capacity,
    const char * input, size_t len,
    const char * arg_name, size_t name_len, char pair)
{
    const char * s;
    const char * r_begin;
    const char * r_end;
    size_t r_len;

    if (len < name_len+1) return -1;

    if (memcmp(input, arg_name, name_len) != 0) return -1;

    s = cpe_str_trim_head((char *)(input + name_len));
    if (*s != pair) return -1;

    r_begin = cpe_str_trim_head((char *)(s + 1));
    r_end = cpe_str_trim_tail((char *)(input + len), r_begin);
    r_len = r_end - r_begin;

    if ((r_len + 1) > r_capacity) {
        r_len = r_capacity - 1;
    }

    memcpy(r, r_begin, r_len);
    r[r_len] = 0;
    
    return 0;
}

int cpe_str_read_arg(char * r, size_t r_capacity, const char * i, const char * arg_name, char sep, char pair) {
    const char * p = cpe_str_trim_head((char *)i);
    size_t name_len = strlen(arg_name);
    const char * s;
    
    while((s = cpe_str_char_not_in_pair(p, sep, "{[('\"", "\"')]}"))) {
        if (cpe_str_read_process_one(r, r_capacity, p, s - p, arg_name, name_len, pair) == 0) return 0;
        p = cpe_str_trim_head((char *)(s + 1));
    }

    return cpe_str_read_process_one(r, r_capacity, p, strlen(p), arg_name, name_len, pair);
}

int cpe_str_read_arg_range(char * r, size_t r_capacity, const char * i, const char * i_end, const char * arg_name, char sep, char pair) {
    const char * p = cpe_str_trim_head((char *)i);
    size_t name_len = strlen(arg_name);
    const char * s;

    i_end = cpe_str_trim_tail((char*)i_end, p);

    while((s = strchr(p, sep)) && s < i_end) {
        if (cpe_str_read_process_one(r, r_capacity, p, s - p, arg_name, name_len, pair) == 0) return 0;
        p = cpe_str_trim_head((char *)(s + 1));
    }

    return p < i_end
        ? cpe_str_read_process_one(r, r_capacity, p, i_end - p, arg_name, name_len, pair)
        : -1;
}

char * cpe_str_read_process_one_mem_dup(
    mem_allocrator_t alloc,
    const char * input, size_t len,
    const char * arg_name, size_t name_len, char pair)
{
    const char * s;
    const char * r_begin;
    const char * r_end;

    if (len < name_len+1) return NULL;

    if (memcmp(input, arg_name, name_len) != 0) return NULL;

    s = cpe_str_trim_head((char *)(input + name_len));
    if (*s != pair) return NULL;

    r_begin = cpe_str_trim_head((char *)(s + 1));
    r_end = cpe_str_trim_tail((char *)(input + len), r_begin);

    return cpe_str_mem_dup_range(alloc, r_begin, r_end);
}

char * cpe_str_read_arg_mem_dup(mem_allocrator_t alloc, const char * i, const char * arg_name, char sep, char pair) {
    const char * p = cpe_str_trim_head((char *)i);
    size_t name_len = strlen(arg_name);
    const char * s;
    
    while((s = strchr(p, sep))) {
        char * r = cpe_str_read_process_one_mem_dup(alloc, p, s - p, arg_name, name_len, pair);
        if (r) return r;
        p = cpe_str_trim_head((char *)(s + 1));
    }

    return cpe_str_read_process_one_mem_dup(alloc, p, strlen(p), arg_name, name_len, pair);
}

char * cpe_str_read_arg_range_mem_dup(mem_allocrator_t alloc, const char * i, const char * i_end, const char * arg_name, char sep, char pair) {
    const char * p = cpe_str_trim_head((char *)i);
    size_t name_len = strlen(arg_name);
    const char * s;

    i_end = cpe_str_trim_tail((char*)i_end, p);

    while((s = strchr(p, sep)) && s < i_end) {
        char * r = cpe_str_read_process_one_mem_dup(alloc, p, s - p, arg_name, name_len, pair);
        if (r) return r;
        
        p = cpe_str_trim_head((char *)(s + 1));
    }

    return p < i_end
        ? cpe_str_read_process_one_mem_dup(alloc, p, i_end - p, arg_name, name_len, pair)
        : NULL;
}

int cpe_str_to_bool(uint8_t * r, const char * str) {
    if (str[0] == 0) {
        *r = 0;
    }
    else if (strcasecmp(str, "true") == 0) {
        *r = 1;
    }
    else if (strcasecmp(str, "false") == 0) {
        *r = 0;
    }
    else {
        char * endp;
        int v;
        v = (int)strtol(str, &endp, 10);
        if (endp && *endp == 0) {
            *r = v ? 1 : 0;
        }
        else {
            return -1;
        }
    }

    return 0;
}

int cpe_str_to_int8(int8_t * r, const char * str) {
    char * endp = NULL;
    long v;

    v = strtol(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (int8_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_uint8(uint8_t * r, const char * str) {
    char * endp = NULL;
    long v;

    v = strtol(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (uint8_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_int16(int16_t * r, const char * str) {
    char * endp = NULL;
    long v;

    v = strtol(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (int16_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_uint16(uint16_t * r, const char * str) {
    char * endp = NULL;
    long v;

    v = strtol(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (uint16_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_int32(int32_t * r, const char * str) {
    char * endp = NULL;
    long v;

    v = strtol(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (int32_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_uint32(uint32_t * r, const char * str) {
    char * endp = NULL;
    long v;

    v = strtol(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (uint32_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_int64(int64_t * r, const char * str) {
    char * endp = NULL;
    long long v;

    v = strtoll(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (int64_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_uint64(uint64_t * r, const char * str) {
    char * endp = NULL;
    unsigned long long v;

    v = strtoull(str, &endp, 10);
    if (endp && *endp == 0) {
        *r = (uint64_t)v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_float(float * r, const char * str) {
    char * endp = NULL;
    float v;

    v = strtof(str, &endp);
    if (endp && *endp == 0) {
        *r = v;
        return 0; 
    }
    else {
        return -1;
    }
}

int cpe_str_to_double(double * r, const char * str) {
    char * endp = NULL;
    double v;

    v = strtod(str, &endp);
    if (endp && *endp == 0) {
        *r = v;
        return 0; 
    }
    else {
        return -1;
    }
}

uint8_t cpe_str_is_in_list(const char * check, const char * list, char sep) {
    const char * sep_pos;
    size_t check_len = strlen(check);

    while((sep_pos = strchr(list, sep))) {
        size_t len = sep_pos - list;
        const char * cur = list;
        list = sep_pos + 1;

        if (len != check_len) continue;
        if (memcmp(cur, check, len) != 0) continue;

        return 1;
    }

    return strcmp(check, list) == 0 ? 1 : 0;
}

void cpe_str_list_for_each(const char * list, char sep, void (*visit_fun)(void * ctx, const char * value), void * ctx) {
    const char * sep_pos;

    while((sep_pos = strchr(list, sep))) {
        size_t len = sep_pos - list;
        char buf[128];

        if (len + 1 > CPE_ARRAY_SIZE(buf)) continue;

        memcpy(buf, list, len);
        buf[len] = 0;
        visit_fun(ctx, buf);

        list = sep_pos + 1;
    }

    if (list[0]) {
        visit_fun(ctx, list);
    }
}

const char * cpe_str_format_ipv4(char * buf, size_t capacity, uint32_t ip) {
    snprintf(
        buf, capacity, "%d.%d.%d.%d",
        (int)((ip & 0x000000FF)),
        (int)((ip & 0x0000FF00) >> 8),
        (int)((ip & 0x00FF0000) >> 16),
        (int)((ip & 0xFF000000) >> 24)
        );
    return buf;
}

uint8_t cpe_str_is_ipv4(const char * ip) {
    uint8_t part = 0;
    uint8_t num_count = 0;
    uint8_t i;

    for(i = 0; ip[i]; ++i) {
        if (ip[i] >= '0' && ip[i] <= '9') {
            num_count++;
        }
        else if (ip[i] == '.') {
            if (num_count == 0) return 0;
            part++;
            num_count = 0;
        }
        else {
            return 0;
        }
    }
    if (num_count) part++;
    
    return part == 4 ? 1 : 0;
}
