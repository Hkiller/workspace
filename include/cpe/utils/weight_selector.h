#ifndef CPE_UTILS_WEIGHTSELECTOR_H
#define CPE_UTILS_WEIGHTSELECTOR_H
#include "memory.h"
#include "random.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cpe_weight_selector {
    mem_allocrator_t m_alloc;
    uint32_t m_weight_count;
    uint32_t m_weight_capacity;
    uint32_t * m_weights;
};

typedef struct cpe_weight_selector * cpe_weight_selector_t;

void cpe_weight_selector_init(cpe_weight_selector_t selector, mem_allocrator_t alloc);
void cpe_weight_selector_clear(cpe_weight_selector_t selector);

int cpe_weight_selector_add_item(cpe_weight_selector_t selector, uint32_t weight);
void cpe_weight_selector_clear_data(cpe_weight_selector_t selector);

int32_t cpe_weight_selector_select(cpe_weight_selector_t selector, cpe_rand_ctx_t r);

#ifdef __cplusplus
}
#endif

#endif

