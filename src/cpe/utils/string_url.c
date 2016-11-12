#include "cpe/pal/pal_ctype.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/error.h"
#include "cpe/utils/string_url.h"

static unsigned char hexchars[] = "0123456789ABCDEF";

ssize_t cpe_url_encode(char * result, size_t result_capacity, const char * input, size_t input_size, error_monitor_t em) {
    size_t i;
    ssize_t j = 0;

    for(i = 0; i < input_size; ++i) {
        char ch = input[i];
        
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')
            || ch == '.' || ch == '-' || ch == '_' || ch == '*' )
        {
            if (j + 1 > result_capacity) {
                CPE_ERROR(em, "cpe_url_encode: result overflow!");
                return -1;
            }
            
            result[j++] = ch;
        }
        else if (ch == ' ') {
            if (j + 1 > result_capacity) {
                CPE_ERROR(em, "cpe_url_encode: result overflow!");
                return -1;
            }
            
            result[j++] = '+';
        }
        else {
            if (j + 3 > result_capacity) {
                CPE_ERROR(em, "cpe_url_encode: result overflow!");
                return -1;
            }

            result[j++] = '%';
            result[j++] = hexchars[(((uint8_t)ch) & 0xF0) >> 4];
            result[j++] = hexchars[((uint8_t)ch) & 0x0F];
        }
    }

    if (j + 1 > result_capacity) {
        CPE_ERROR(em, "cpe_url_encode: result overflow!");
        return -1;
    }
            
    result[j] = '\0';
    
    return j;
}

ssize_t cpe_url_decode(char * result, size_t result_capacity, const char * input, size_t input_size, error_monitor_t em) {
    ssize_t j = 0;

    while(input_size > 0) {
        char ch = *input;
        int used;
        
        if (j + 1 > result_capacity) {
            CPE_ERROR(em, "cpe_url_decode: result overflow!");
            return -1;
        }
            
        if (ch == '+') {
            result[j++] = ' ';
            used = 1;
        }
        else if (ch == '%' && input_size >= 2 && isxdigit((int)input[1]) && isxdigit((int)input[2])) {
            int value;
            
            ch = input[j + 1];
            if (isupper(ch)) ch = tolower(ch);
            value = (ch >= '0' && ch <= '9' ? ch - '0' : ch - 'a' + 10) * 16;

            ch = input[j + 2];
            if (isupper(ch)) ch = tolower(ch);
            value += ch >= '0' && ch <= '9' ? ch - '0' : ch - 'a' + 10;

            result[j++] = (char)value;
            used = 3;
        }
        else {
            result[j++] = ch;
            used = 1;
        }

        input_size -= used;
        input += used;
    }
    
    if (j + 1 > result_capacity) {
        CPE_ERROR(em, "cpe_url_encode: result overflow!");
        return -1;
    }
            
    result[j] = '\0';
    
    return j;
}

size_t cpe_url_decode_inline(char * str, size_t len) {
    char *dest = str;
    char *data = str;

    int value;
    int c;

    while(len--) {
        if (*data == '+') {
            *dest = ' ';
        }
        else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
                 && isxdigit((int) *(data + 2)))
        {

            c = ((unsigned char *)(data+1))[0];
            if (isupper(c))
                c = tolower(c);
            value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;
            c = ((unsigned char *)(data+1))[1];
            if (isupper(c))
                c = tolower(c);
            value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

            *dest = (char)value ;
            data += 2;
            len -= 2;
        } else {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';
    return dest - str;
}
