#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/base64.h"

static const uint8_t table64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t cpe_base64_encode(write_stream_t output, read_stream_t input) {
    uint8_t buf[128];
    int wlen = 0;
    int buf_len;
    int read_len;

    for(buf_len = 0, read_len = stream_read(input, buf, sizeof(buf));
        buf_len + read_len > 0;
        read_len = stream_read(input, buf + buf_len, sizeof(buf) - buf_len))
    {
        uint8_t * rp = buf;
        int have_next = read_len > 0 ? 1 : 0;

        buf_len += read_len;
        read_len = 0;

        while(buf_len > 0) {
            if(buf_len > 2) { //够3个字符  
                stream_putc(output, table64[(rp[0] >> 2 ) & 0x3F]);  //右移两位，与00111111是防止溢出，自加  
                stream_putc(output, table64[((rp[0] & 3) << 4) | (rp[1] >> 4)]);  
                stream_putc(output, table64[((rp[1] & 0xF) << 2) | (rp[2] >> 6)]);
                stream_putc(output, table64[rp[2] & 0x3F]);

                rp += 3;  
                buf_len -= 3;  
                wlen +=4;  
            }
            else if (buf_len == 2) {
                if (have_next) break;

                stream_putc(output, table64[(rp[0] >> 2 ) & 0x3F]);  //右移两位，与00111111是防止溢出，自加  
                stream_putc(output, table64[((rp[0] & 3) << 4) | (rp[1] >> 4)]);
                stream_putc(output, table64[((rp[1] & 0x0F) << 2)]);
                stream_putc(output, '=');

                rp += 2;  
                buf_len -= 2;  
                wlen +=4;  
            }
            else if (buf_len == 1) {
                if (have_next) break;

                stream_putc(output, table64[(rp[0] >> 2 ) & 0x3F]);  //右移两位，与00111111是防止溢出，自加  
                stream_putc(output, table64[(rp[0] & 3) << 4 ]);
                stream_putc(output, '=');
                stream_putc(output, '=');

                rp += 1;  
                buf_len -= 1;  
                wlen +=4;
            }
        }

        if (buf_len) memmove(buf, rp, buf_len);
    }

    return wlen;  
}  

char cpe_base64_char_to_index(char c) {
    if((c >= 'A') && (c <= 'Z')) return c - 'A';
    if((c >= 'a') && (c <= 'z')) return c - 'a' + 26;
    if((c >= '0') && (c <= '9')) return c - '0' + 52;
    if(c == '+') return 62;
    if(c == '/') return 63;  
    if(c == '=') return 0;  
    return 0;  
}

size_t cpe_base64_decode(write_stream_t output, read_stream_t input) {
    char buf[128];
    int wlen = 0;
    int buf_len;
    int read_len;
    char lpCode[4];

    for(buf_len = 0, read_len = stream_read(input, buf, sizeof(buf));
        buf_len + read_len > 0;
        read_len = stream_read(input, buf + buf_len, sizeof(buf) - buf_len))
    {
        char * rp = buf;
        int have_next = read_len > 0 ? 1 : 0;

        buf_len += read_len;
        read_len = 0;

        while(buf_len > 4) {      /*不足三个字符，忽略 */
            lpCode[0] = cpe_base64_char_to_index(rp[0]);
            lpCode[1] = cpe_base64_char_to_index(rp[1]);
            lpCode[2] = cpe_base64_char_to_index(rp[2]);
            lpCode[3] = cpe_base64_char_to_index(rp[3]);

            stream_putc(output, (lpCode[0] << 2) | (lpCode[1] >> 4));
            stream_putc(output, (lpCode[1] << 4) | (lpCode[2] >> 2));
            stream_putc(output, (lpCode[2] << 6) | (lpCode[3]));
  
            rp += 4;  
            buf_len -= 4;  
            wlen += 3;  
        }

        if (have_next) {
            if (buf_len > 0) memmove(buf, rp, buf_len);
        }
        else {
            if (buf_len == 4) {
                lpCode[0] = cpe_base64_char_to_index(rp[0]);
                lpCode[1] = cpe_base64_char_to_index(rp[1]);
                stream_putc(output, (lpCode[0] << 2) | (lpCode[1] >> 4));
                wlen++;

                if (rp[2] != '=') {
                    lpCode[2] = cpe_base64_char_to_index(rp[2]);
                    stream_putc(output, (lpCode[1] << 4) | (lpCode[2] >> 2));
                    wlen++;
                }

                if (rp[3] != '=') {
                    lpCode[3] = cpe_base64_char_to_index(rp[3]);
                    stream_putc(output, (lpCode[2] << 6) | (lpCode[3]));
                    wlen++;
                }
            }

            break;
        }
    }

    return wlen;  
}
