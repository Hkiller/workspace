#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/weight_selector.h"

void cpe_weight_selector_init(cpe_weight_selector_t selector, mem_allocrator_t alloc) {
    selector->m_alloc = alloc;
    selector->m_weight_count = 0;
    selector->m_weight_capacity = 0;
    selector->m_weights = NULL;
}

void cpe_weight_selector_clear(cpe_weight_selector_t selector) {
    if (selector->m_weights) {
        mem_free(selector->m_alloc, selector->m_weights);
    }
    
    selector->m_weights = NULL;
    selector->m_alloc = NULL;
    selector->m_weight_count = 0;
    selector->m_weight_capacity = 0;
}

int cpe_weight_selector_add_item(cpe_weight_selector_t selector, uint32_t weight) {
    if (selector->m_weight_count >= selector->m_weight_capacity) {
        uint32_t new_capacity = selector->m_weight_capacity < 16 ? 16 : selector->m_weight_capacity * 2;
        uint32_t * new_weights = mem_alloc(selector->m_alloc, sizeof(uint32_t) * new_capacity);
        if (new_weights == NULL) return -1;

        if (selector->m_weights) {
            memcpy(new_weights, selector->m_weights, sizeof(uint32_t) * selector->m_weight_count);
            mem_free(selector->m_alloc, selector->m_weights);
        }

        selector->m_weights = new_weights;
        selector->m_weight_capacity = new_capacity;
    }

    assert(selector->m_weight_count < selector->m_weight_capacity);
    assert(selector->m_weights);

    if (selector->m_weight_count == 0) {
        selector->m_weights[0] = weight;
    }
    else {
        selector->m_weights[selector->m_weight_count] = selector->m_weights[selector->m_weight_count - 1] + weight;
    }

    selector->m_weight_count++;
    
    return 0;
}

void cpe_weight_selector_clear_data(cpe_weight_selector_t selector) {
    selector->m_weight_count = 0;
}

int32_t cpe_weight_selector_select(cpe_weight_selector_t selector, cpe_rand_ctx_t r) {
    uint32_t rv;

    if (selector->m_weight_count == 0) return -1;

    rv = cpe_rand_ctx_generate(r, selector->m_weight_count);
    assert(rv < selector->m_weight_count);
    
    if (selector->m_weights[selector->m_weight_count - 1] > 0) {
        uint32_t * check_begin = selector->m_weights;
        uint32_t * check_end = check_begin + selector->m_weight_count;
        uint32_t * end = check_end;

        while(check_begin < check_end) {
            uint32_t * mid = check_begin + (size_t)((check_end - check_begin) / 2);

            if (*mid < rv) {
                check_begin = mid;
            }
            else {
                check_end = mid;
            }
        }
        
        return check_begin >= end
            ? -1
            : (int32_t)(check_begin - selector->m_weights);
    }
    else {
        return (int32_t)rv;
    }
}

