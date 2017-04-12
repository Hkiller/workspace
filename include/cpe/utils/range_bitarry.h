#ifndef CPE_UTILS_RANGEBITARRAY_H
#define CPE_UTILS_RANGEBITARRAY_H
#include "range.h"
#include "bitarry.h"

#ifdef __cplusplus
extern "C" {
#endif

int cpe_range_put_from_ba(
    cpe_range_mgr_t ra,
    cpe_ba_t ba,
    int32_t baStartPos,
    size_t ba_capacity,
    cpe_ba_value_t require);

int cpe_urange_put_from_ba(
    cpe_urange_mgr_t ra,
    cpe_ba_t ba,
    int32_t baStartPos,
    size_t ba_capacity,
    cpe_ba_value_t require);

#ifdef __cplusplus
}
#endif

#endif
