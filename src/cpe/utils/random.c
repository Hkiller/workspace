#include <assert.h>
#include "cpe/utils/random.h"
#include "cpe/pal/pal_limits.h"

uint32_t cpe_rand(int32_t seed, uint32_t max) {
    seed = seed ^ (seed >> 11);
    seed = seed ^ ((seed << 7) & (2636928640u));
    seed = seed ^ ((seed << 15) & (4022730752u));
    seed = seed ^ ((seed >> 18));

    return max ? seed % max : seed;
}

void cpe_rand_ctx_init(struct cpe_rand_ctx * ctx, int32_t seed) {
    uint32_t i;

    ctx->m_state_arr[0] = seed;
    ctx->m_index = 0;

    for (i = 1; i < (sizeof(ctx->m_state_arr) / sizeof(int32_t)); ++i) {
        ctx->m_state_arr[i] = 1812433253 * (ctx->m_state_arr[i - 1] ^ (ctx->m_state_arr[i - 1] >> 30)) + i;
    }
}

uint32_t cpe_rand_ctx_generate(struct cpe_rand_ctx * ctx, uint32_t max) {
    uint32_t r;

    if (ctx->m_index == 0) {
        uint32_t i;
		for (i = 0; i < (sizeof(ctx->m_state_arr) / sizeof(int32_t)) ; ++i) {
			int32_t y = (ctx->m_state_arr[i] & 0x8000) + ((ctx->m_state_arr[(i + 1) % (sizeof(ctx->m_state_arr) / sizeof(int32_t))]) & 0x7fff);
			ctx->m_state_arr[i] = ctx->m_state_arr[(i + 397) % (sizeof(ctx->m_state_arr) / sizeof(int32_t))] ^ (y >> 1);

			if ( (y % 2 == 1) ) {  // y is odd
				ctx->m_state_arr[i] = ctx->m_state_arr[i] ^ 2567483615u;
			}
		}
    }

    r = cpe_rand(ctx->m_state_arr[ctx->m_index], max);
    ctx->m_index = (ctx->m_index + 1) % (sizeof(ctx->m_state_arr) / sizeof(int32_t));
    return r;
}

float cpe_rand_ctx_generate_f(struct cpe_rand_ctx * ctx) {
    uint32_t value = cpe_rand_ctx_generate(ctx, INT32_MAX);
    return (((float)value) / ((float)(INT32_MAX - 1)));
}

float cpe_rand_ctx_generate_f_range(struct cpe_rand_ctx * ctx, float min, float max) {
    return min + (max - min) * cpe_rand_ctx_generate_f(ctx);
}

int g_cpe_rand_ctx_dft_init = 0;
struct cpe_rand_ctx g_cpe_rand_ctx_dft;

struct cpe_rand_ctx * cpe_rand_ctx_dft(void) {
    if (!g_cpe_rand_ctx_dft_init) {
        cpe_rand_ctx_init(&g_cpe_rand_ctx_dft, (int32_t)time(0));
        g_cpe_rand_ctx_dft_init = 1;
    }

    return &g_cpe_rand_ctx_dft;
}
