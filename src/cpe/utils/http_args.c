#include "cpe/pal/pal_string.h"
#include "cpe/utils/http_args.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/string_url.h"

int cpe_http_args_parse_inline(cpe_http_arg_t args, uint16_t * arg_count, char * input, error_monitor_t em) {
    uint16_t count = 0;
    char * sep;
    char * eq;

    input = cpe_str_trim_head(input);
    
    while((sep = strchr(input, '&'))) {
        cpe_http_arg_t one;
        
        if (count + 1 > *arg_count) {
            CPE_ERROR(em, "cpe_http_args_parse_inline: arg count overflow, capacity=%d", *arg_count);
            return -1;
        }

        one = &args[count++];
        
        * cpe_str_trim_tail(sep, input) = 0;
        if ((eq = strchr(input, '='))) {
            * cpe_str_trim_tail(eq, input) = 0;
            one->name = input;
            cpe_url_decode_inline(one->name, strlen(one->name));
            
            one->value = cpe_str_trim_head(eq + 1);
            cpe_url_decode_inline(one->value, strlen(one->value));
        }
        else {
            CPE_ERROR(em, "cpe_http_args_parse_inline: arg %s error", input);
            return -1;
        }
        
        input = cpe_str_trim_head(sep + 1);
    }

    if (*input != 0) {
        cpe_http_arg_t one;

        if (count + 1 > *arg_count) {
            CPE_ERROR(em, "cpe_http_args_parse_inline: arg count overflow, capacity=%d", *arg_count);
            return -1;
        }

        one = &args[count++];
        
        * cpe_str_trim_tail(input + strlen(input), input) = 0;
        if ((eq = strchr(input, '='))) {
            * cpe_str_trim_tail(eq, input) = 0;
            one->name = input;
            one->value = cpe_str_trim_head(eq + 1);
        }
        else {
            CPE_ERROR(em, "cpe_http_args_parse_inline: arg %s error", input);
            return -1;
        }
    }

    *arg_count = count;
    return 0;
}

cpe_http_arg_t cpe_http_args_find(cpe_http_arg_t args, uint16_t arg_count, const char * name) {
    uint16_t i;
    
    for(i = 0; i < arg_count; ++i) {
        cpe_http_arg_t one = args + i;
        if (strcmp(one->name, name) == 0) return one;
    }

    return NULL;
}

char * cpe_http_args_find_value(cpe_http_arg_t args, uint16_t arg_count, const char * name) {
    uint16_t i;
    
    for(i = 0; i < arg_count; ++i) {
        cpe_http_arg_t one = args + i;
        if (strcmp(one->name, name) == 0) return one->value;
    }

    return NULL;
}

int cpe_http_args_remove(cpe_http_arg_t args, uint16_t * arg_count, const char * name) {
    uint16_t i;
    int removed = 0;
    
    for(i = 0; i < *arg_count; ++i) {
        cpe_http_arg_t one = args + i;
        if (strcmp(one->name, name) == 0) {
            memmove(one, one + 1, sizeof(*one) * (*arg_count - i - 1));
            (*arg_count)--;
            removed = 1;
            break;
        }
    }

    return removed;
}

void cpe_http_args_print(write_stream_t s, cpe_http_arg_t args, uint16_t arg_count) {
    uint16_t i;
    for(i = 0; i < arg_count; ++i) {
        cpe_http_arg_t one = args + i;
        
        if (i > 0) { stream_putc(s, '&'); }
        stream_printf(s, "%s=%s", one->name, one->value);
    }
}

char * cpe_http_args_to_string(cpe_http_arg_t args, uint16_t arg_count, mem_buffer_t buffer) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    cpe_http_args_print((write_stream_t)&stream, args, arg_count);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

uint8_t cpe_http_args_all_exists(cpe_http_arg_t args, uint16_t arg_count, const char * * names, uint16_t name_count) {
    uint16_t i;
    for (i = 0; i < name_count; ++i) {
        if (cpe_http_args_find(args, arg_count, names[i]) == NULL) return 0;
    }

    return 1;
}
