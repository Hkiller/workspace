#include <assert.h>
#include "cpe/pal/pal_math.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/prand.h"

double cpe_prand(cpe_prand_ctx_t ctx) {
    return ctx->m_f(ctx->m_data);
}

struct cpe_prand_ctx_data_basic_1 {
    double r[98];
    int32_t ix1;
    int32_t ix2;
    int32_t ix3;
};

static double cpe_prand_do_basic_1(void * ctx) {
    struct cpe_prand_ctx_data_basic_1 * data = (struct cpe_prand_ctx_data_basic_1 *)ctx;
    double rm1, rm2, t;
    int32_t m1,m2,m3,ia1,ia2,ia3,ic1,ic2,ic3,j;

    m1 = 259200, ia1 = 7141, ic1 = 54773, rm1 = 0.0000038580247;
    m2 = 134456, ia2 = 8121, ic2 = 28411, rm2 = 0.0000074373773;
    m3 = 243000, ia3 = 4561, ic3 = 51349;

    data->ix1 = (ia1 * data->ix1 + ic1) % m1;
    data->ix2 = (ia2 * data->ix2 + ic2) % m2;
    data->ix3 = (ia3 * data->ix3 + ic3) % m3;

    j = 1 + (int)((97 * data->ix3) / m3);

    if (j > 97 || j < 1) {
        assert(0);
        return 1.0f;
    }

    t = data->r[j];
    data->r[j] = (((float)data->ix1 + (float)data->ix2) * rm2) * rm1;
    return t;
}

void cpe_prand_init_basic_1_i(struct cpe_prand_ctx_data_basic_1 * data, int32_t idum) {
    int check_data_size[CPE_TYPE_ARRAY_SIZE(struct cpe_prand_ctx, m_data) >= sizeof(struct cpe_prand_ctx_data_basic_1) ? 1 : -1];
    double rm1, rm2;
    int32_t m1,m2,m3,ia1,ia2,ia3,ic1,ic2,ic3,j;

    (void)check_data_size;

    m1 = 259200, ia1 = 7141, ic1 = 54773, rm1 = 0.0000038580247;
    m2 = 134456, ia2 = 8121, ic2 = 28411, rm2 = 0.0000074373773;
    m3 = 243000, ia3 = 4561, ic3 = 51349;

    data->ix1 = (ic1 + abs(idum)) % m1;
    data->ix1 = (ia1 * data->ix1 + ic1) % m1;
    data->ix2 = data->ix1 % m2;
    data->ix1 = (ia1 * data->ix1 + ic1) % m1;
    data->ix3 = data->ix1 % m3;

    for(j = 1; j <= 97; j++) {
        data->ix1 = (ia1 * data->ix1 + ic1) % m1;
        data->ix2 = (ia2 * data->ix2 + ic2) % m2;
        data->r[j] = (((double)data->ix1) + ((double)data->ix2) * rm2) * rm1;
    }
}

void cpe_prand_init_basic_1(cpe_prand_ctx_t ctx, int32_t idum) {
    struct cpe_prand_ctx_data_basic_1 * data = (struct cpe_prand_ctx_data_basic_1 *)(ctx->m_data);
    cpe_prand_init_basic_1_i(data, idum);
    ctx->m_f = cpe_prand_do_basic_1;
}

struct cpe_prand_ctx_data_gassdev {
    struct cpe_prand_ctx_data_basic_1 m_basic;
    int32_t phase;
    double V1;
    double V2;
    double S;
};

static double cpe_prand_do_gasdev(void * ctx) {
    struct cpe_prand_ctx_data_gassdev * data = (struct cpe_prand_ctx_data_gassdev *)ctx;
    double X;

    if (data->phase == 0) {
        do {
            double U1 = cpe_prand_do_basic_1(&data->m_basic);
            double U2 = cpe_prand_do_basic_1(&data->m_basic);

            assert(U1 >= 0.0f && U1 <= 1.0f);
            assert(U2 >= 0.0f && U2 <= 1.0f);
            
            data->V1 = 2.0 * U1 - 1.0;
            data->V2 = 2.0 * U2 - 1.0;
            data->S = data->V1 * data->V1 + data->V2 * data->V2;
        } while(data->S >= 1.0 || data->S == 0);

        X = data->V1 * sqrt(-2.0 * log(data->S) / data->S);
    }
    else {
        X = data->V2 * sqrt(-2.0 * log(data->S) / data->S);
    }

    data->phase = 1 - data->phase;

    return X;
}

void cpe_prand_init_gasdev(cpe_prand_ctx_t ctx, int32_t idum) {
    struct cpe_prand_ctx_data_gassdev * data = (struct cpe_prand_ctx_data_gassdev *)(ctx->m_data);
    cpe_prand_init_basic_1_i(&data->m_basic, idum);
    data->phase = 0;
    ctx->m_f = cpe_prand_do_gasdev;
}
