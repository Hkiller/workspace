#include "cpe/pal/pal_stdlib.h"
#include "svr/set/share/set_utils.h"

int set_shm_key_get(uint16_t svr_type, uint16_t svr_id, char tag) {
    return (((uint32_t)svr_type) << 24)
        + (((uint32_t)svr_id) << 8)
        + tag;
}
