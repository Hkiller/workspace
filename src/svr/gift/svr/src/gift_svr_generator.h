#ifndef SVR_GIFT_SVR_GENERATOR_H
#define SVR_GIFT_SVR_GENERATOR_H
#include "gift_svr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t (*gift_svr_generator_prefix_t)[CPE_TYPE_ARRAY_SIZE(SVR_GIFT_GENERATE_RECORD, prefix)];

#define GIFT_SVR_GENERATOR_MAX_DATA_LEN 64

typedef enum gift_svr_generator_char_scope {
    gift_svr_generator_char_scope_num = 1,
    gift_svr_generator_char_scope_letter = 2,
} gift_svr_generator_char_scope_t;

struct gift_svr_generator_block {
    uint8_t m_len;
    gift_svr_generator_char_scope_t m_scope;
};

struct gift_svr_generator {
    gift_svr_t m_svr;
    TAILQ_ENTRY(gift_svr_generator) m_next;
    char m_name[32];
    struct gift_svr_generator_block m_prefix;
    uint8_t m_block_count;
    struct gift_svr_generator_block m_blocks[4];
    uint8_t m_cdkey_len;
    uint16_t m_max_generate_count;
};

gift_svr_generator_t gift_svr_generator_create(gift_svr_t svr, const char * name, cfg_t cfg);
void gift_svr_generator_free(gift_svr_generator_t generator);
gift_svr_generator_t gift_svr_generator_find(gift_svr_t svr, const char * name);

int gift_svr_generator_select_prefix(gift_svr_generator_t generator, char * output);
int gift_svr_generator_gen(gift_svr_generator_t generator, const char * prefix, SVR_GIFT_USE_BASIC * records, uint16_t * record_count);

int gift_svr_generators_load(gift_svr_t svr, cfg_t cfg);

uint8_t gift_svr_generator_prefix_match(gift_svr_generator_t generator, const char * prefix);

const char * gift_svr_generator_scope_name(gift_svr_generator_char_scope_t scope);

#ifdef __cplusplus
}
#endif
    
#endif
