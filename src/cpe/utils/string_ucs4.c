#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_ucs4.h"
#include "cpe/utils/memory.h"

struct cpe_str_ucs4 {
    mem_allocrator_t m_alloc;
    size_t m_len;
};

cpe_str_ucs4_t cpe_str_ucs4_alloc(mem_allocrator_t alloc, size_t len) {
    cpe_str_ucs4_t str = mem_alloc(alloc, sizeof(struct cpe_str_ucs4) + sizeof(uint32_t) * (len + 1));
    if (str == NULL) return NULL;

    str->m_alloc = alloc;
    str->m_len = len;

    cpe_str_ucs4_data(str)[len] = 0;

    return str;
}

cpe_str_ucs4_t cpe_str_ucs4_dup_str(mem_allocrator_t alloc, uint32_t const * src) {
    size_t len = 0;

    while(src[len++] != 0) {}

    return cpe_str_ucs4_dup_len(alloc, src, len);
}

cpe_str_ucs4_t cpe_str_ucs4_dup_len(mem_allocrator_t alloc, uint32_t const * src, size_t len) {
    cpe_str_ucs4_t r_str;
    size_t i;
    uint32_t * wp;
    
    r_str = cpe_str_ucs4_alloc(alloc, len);
    if (r_str == NULL) return NULL;

    wp = cpe_str_ucs4_data(r_str);
    
    for(i = 0; i < len; ++i) {
        *wp++ = src[i];
    }
    
    return r_str;
}

cpe_str_ucs4_t cpe_str_ucs4_dup_range(mem_allocrator_t alloc, uint32_t const * begin, uint32_t const * end) {
    return cpe_str_ucs4_dup_len(alloc, begin, end - begin);
}

cpe_str_ucs4_t cpe_str_ucs4_dup(mem_allocrator_t alloc, cpe_str_ucs4_t src) {
    return cpe_str_ucs4_dup_len(alloc, cpe_str_ucs4_data(src), src->m_len);
}

void cpe_str_ucs4_free(cpe_str_ucs4_t ucs4_str) {
    mem_free(ucs4_str->m_alloc, ucs4_str);
}

size_t cpe_str_ucs4_len(cpe_str_ucs4_t ucs4_str) {
    return ucs4_str->m_len;
}

uint32_t * cpe_str_ucs4_data(cpe_str_ucs4_t ucs4_str) {
    return (uint32_t *)(ucs4_str + 1);
}

cpe_str_ucs4_t cpe_str_ucs4_from_utf8_len(mem_allocrator_t alloc, const char * src, size_t src_len) {
    cpe_str_ucs4_t r_str;
    size_t i;
    uint32_t * wp;

    r_str = cpe_str_ucs4_alloc(alloc, cpe_str_utf8_wlen(src, src + src_len));
    if (r_str == NULL) return NULL;

    wp = cpe_str_ucs4_data(r_str);

	for (i = 0; i < src_len; ) {
		uint32_t w;

		if ((src[i] & 0x80) == 0) {							// U-00000000 - U-0000007F : 0xxxxxxx 
			w = (uint32_t)(src[i]);
			i += 1;
		}
		else if ((src[i] & 0xe0) == 0xc0 && i+1 < src_len) {	// U-00000080 - U-000007FF : 110xxxxx 10xxxxxx 
			w  = (uint32_t)(src[i+0] & 0x3f) << 6;
			w |= (uint32_t)(src[i+1] & 0x3f);
			i += 2;
		}
		else if ((src[i] & 0xf0) == 0xe0 && i+2 < src_len) {	// U-00000800 - U-0000FFFF : 1110xxxx 10xxxxxx 10xxxxxx 
			w  = (uint32_t)(src[i+0] & 0x1f) << 12;
			w |= (uint32_t)(src[i+1] & 0x3f) << 6;
			w |= (uint32_t)(src[i+2] & 0x3f);
			i += 3;
		}
		else if ((src[i] & 0xf8) == 0xf0) { // U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx , this is not Chinese
			w  = 0x20u;
			i += 4;
		}
		else if ((src[i] & 0xfc) == 0xf8) { // U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx , this is not Chinese
			w  = 0x20u;
			i += 5;
		}
		else {						// U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx , this is not Chinese
			w  = 0x20u;
			i += 6;
		}

		*wp++ = w;
	}

	return r_str;
}

cpe_str_ucs4_t cpe_str_ucs4_from_utf8(mem_allocrator_t alloc, const char * src) {
    return cpe_str_ucs4_from_utf8_len(alloc, src, strlen(src));
}

cpe_str_ucs4_t cpe_str_ucs4_from_utf8_range(mem_allocrator_t alloc, const char * begin, const char * end) {
    return cpe_str_ucs4_from_utf8_len(alloc, begin, end - begin);
}

char * cpe_str_ucs4_to_utf8(cpe_str_ucs4_t ucs4_str, mem_allocrator_t alloc) {
    uint32_t * src = cpe_str_ucs4_data(ucs4_str);

    return cpe_str_utf8_from_ucs4_range(alloc, src, src + ucs4_str->m_len);
}

char * cpe_str_utf8_from_ucs4_range(mem_allocrator_t alloc, uint32_t const * begin, uint32_t const * end) {
    char * r_str;
    size_t i;
    char * wp;
    size_t len = end - begin;

    r_str = mem_alloc(alloc, cpe_str_ucs4_clen(begin, end) + 1);
    if (r_str == NULL) return NULL;

    wp = r_str;

	for (i = 0; i < len; ++i) {
		if (begin[i] < 0x80) {
			*wp++ = (char)begin[i];
		}
		else if (begin[i] < 0x800) {
			*wp++ = (char)(0xC0 |  (begin[i] >> 6));
			*wp++ = (char)(0x80 |  (begin[i]        & 0x3F));
		}
		else if (begin[i] < 0x10000) {
			*wp++ = (char)(0xE0 |  (begin[i] >> 12));
			*wp++ = (char)(0x80 | ((begin[i] >> 6 ) & 0x3F));
			*wp++ = (char)(0x80 |  (begin[i]        & 0x3F));
		}
	}

    *wp++ = 0;
    
    return r_str;
}

char * cpe_str_utf8_from_ucs4_len(mem_allocrator_t alloc, uint32_t const * src, size_t len) {
    return cpe_str_utf8_from_ucs4_range(alloc, src, src + len);
}

size_t cpe_str_ucs4_clen(const uint32_t * begin, const uint32_t * end) {
	size_t r_len = 0;
    size_t len = end - begin;
    size_t i;
    
	for (i = 0; i < len; ++i) {
		if (begin[i] < 0x80) {
            r_len += 1;
		}
		else if (begin[i] < 0x800) {
            r_len += 2;
		}
		else if (begin[i] < 0x10000) {
            r_len += 3;
		}
	}

	return r_len;
}

size_t cpe_str_utf8_wlen(const char * begin, const char * end) {
	size_t r_len = 0;
    size_t src_len = end - begin;
    uint32_t i;
    
	for (i = 0; i < src_len;) {
        if ((begin[i] & 0x80) == 0) {							// U-00000000 - U-0000007F : 0xxxxxxx 
            i += 1;
        }
        else if ((begin[i] & 0xe0) == 0xc0 && i+1 < src_len) {	// U-00000080 - U-000007FF : 110xxxxx 10xxxxxx 
            i += 2;
        }
        else if ((begin[i] & 0xf0) == 0xe0 && i+2 < src_len) {	// U-00000800 - U-0000FFFF : 1110xxxx 10xxxxxx 10xxxxxx 
            i += 3;
        }
        else if ((begin[i] & 0xf8) == 0xf0) { // U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx , this is not Chinese
            i += 4;
        }
        else if ((begin[i] & 0xfc) == 0xf8) { // U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx , this is not Chinese
            i += 5;
        }
        else {						// U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx , this is not Chinese
            i += 6;
        }

        r_len++;
	}

	return r_len;
}

size_t cpe_char_ucs4_clen(uint32_t c) {
    if (c < 0x80) {
        return 1;
    }
    else if (c < 0x800) {
        return 2;
    }
    else if (c < 0x10000) {
        return 3;
    }
    else {
        return 0;
    }
}

size_t cpe_char_ucs4_to_utf8(char * r, size_t capacity, uint32_t c) {
    if (c < 0x80) {
        if (capacity >= 1) {
            r[0] = (char)c;
            return 1;
        }
        else {
            return 0;
        }
    }
    else if (c < 0x800) {
        if (capacity >= 2) {
            r[0] = (char)(0xC0 |  (c >> 6));
            r[1] = (char)(0x80 |  (c & 0x3F));
            return 2;
        }
        else {
            return 0;
        }
    }
    else if (c < 0x10000) {
        if (capacity >= 3) {
            r[0] = (char)(0xE0 |  (c >> 12));
            r[1] = (char)(0x80 | ((c >> 6 ) & 0x3F));
            r[2] = (char)(0x80 |  (c & 0x3F));
            return 3;
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}
