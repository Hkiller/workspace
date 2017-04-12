#include "cpe/pal/pal_stdlib.h"
#include "plugin_barrage_utils_i.h"

float plugin_barrage_calc_float(BARRAGE_RAND_FLOAT const * v) {
    if (v->adj == 0.0f) return v->base;

	return - v->adj + 2.0 * v->adj * ((float)rand() / RAND_MAX);
}

uint16_t plugin_barrage_calc_uint16(BARRAGE_RAND_UINT16 const * v) {
    uint32_t adj_value;

    if (v->adj == 0) return v->base;

    adj_value = rand() % (2 * v->adj);

    if (adj_value > v->adj) {
        return v->base + (adj_value - v->adj);
    }
    else {
        adj_value = v->adj - adj_value;
        return v->base < adj_value ? 0 : v->base - adj_value;
    }
}

uint8_t plugin_barrage_calc_uint8(BARRAGE_RAND_UINT8 const * v) {
    uint16_t adj_value;

    if (v->adj == 0) return v->base;

    adj_value = rand() % (2 * v->adj);

    if (adj_value > v->adj) {
        return v->base + (adj_value - v->adj);
    }
    else {
        adj_value = v->adj - adj_value;
        return v->base < adj_value ? 0 : v->base - adj_value;
    }
}

void plugin_barrage_calc_pair(BARRAGE_PAIR * r, BARRAGE_RAND_PAIR const * v) {
    r->x = plugin_barrage_calc_float(&v->x);
    r->y = plugin_barrage_calc_float(&v->y);
}
