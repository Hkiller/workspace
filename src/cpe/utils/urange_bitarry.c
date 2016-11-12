#include <assert.h>
#include "cpe/utils/range_bitarry.h"

int cpe_urange_put_from_ba(
    cpe_urange_mgr_t ra,
    cpe_ba_t ba,
    int32_t baStartPos,
    size_t ba_capacity,
    cpe_ba_value_t require)
{
    int32_t start;
    int32_t end;
    size_t i;

    start = end = -1;

    for(i = 0; i < ba_capacity; ++i, ++baStartPos) {
        if (cpe_ba_get(ba, i) != require) continue;

        if (end < 0) {
            start = baStartPos;
            end = baStartPos + 1;
        }
        else {
            if (end == baStartPos) {
                ++end;
            }
            else {
                if (cpe_urange_put_urange(ra, start, end) != 0) return -1;

                start = baStartPos;
                end = baStartPos + 1;
            }
        }
    }

    if (!(end < 0)) {
        if (cpe_urange_put_urange(ra, start, end) != 0) return -1;
    }

    return 0;
}

