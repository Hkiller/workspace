#include "cpe/utils/buffer.h"
#include "MD5Test.hpp"

const char * MD5Test::str_md5(const char * input) {
    cpe_md5_ctx mdContext;
    size_t len = strlen(input);

    cpe_md5_ctx_init(&mdContext);
    cpe_md5_ctx_update(&mdContext, input, len);
    cpe_md5_ctx_final(&mdContext);

    return to_str(&mdContext);
}

const char * MD5Test::to_str(cpe_md5_value_t value) {
    struct mem_buffer buffer;
    
    mem_buffer_init(&buffer, t_allocrator());
    const char * r = t_tmp_strdup(cpe_md5_dump(value, &buffer));
    mem_buffer_clear(&buffer);

    EXPECT_TRUE(r != NULL);

    return r;
}

const char * MD5Test::to_str(cpe_md5_ctx_t ctx) {
    return to_str(&ctx->value);
}

