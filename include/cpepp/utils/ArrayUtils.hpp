#ifndef CPEPP_UTILS_ARRAYUTILS_H
#define CPEPP_UTILS_ARRAYUTILS_H
#include <cassert>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/math_ex.h"
#include "System.hpp"

namespace Cpe { namespace Utils {

template<typename T, typename CountT>
void array_remove(T * elements, CountT & count, T const * e) {
    assert(e >= elements);

    uint8_t idx = e - elements;
    assert(idx < count);

    memmove( elements + idx, elements + idx + 1, sizeof(T) * (count - idx - 1));
    --count;
}

template<typename T, typename CountT>
void array_remove(T * elements, CountT & count, size_t pos) {
    assert(pos < count);

    memmove( elements + pos, elements + pos + 1, sizeof(T) * (count - pos - 1));
    --count;
}

template<typename T, typename CountT>
void array_copy(T * to, T const * from, CountT count) {
    memcpy((void*)to, (void*)from, sizeof(T) * count);
}

template<typename T, typename CountT>
void array_qsort(T * elements, CountT count, int (*cmp)(T const * l, T const * r)) {
    typedef int (*_pt)(void const * l, void const * r);
    qsort(elements, count, sizeof(T), (_pt)cmp);
}

template<typename T, typename CountT>
T * array_bsearch(T const & key, T * elements, CountT count, int (*cmp)(T const * l, T const * r)) {
    typedef int (*_pt)(void const * l, void const * r);
    return (T *)bsearch(&key, elements, count, sizeof(T), (_pt)cmp);
}

template<typename T, typename CountT>
T const * array_bsearch(T const & key, T const * elements, CountT count, int (*cmp)(T const * l, T const * r)) {
    typedef int (*_pt)(void const * l, void const * r);
    return (T const *)bsearch(&key, elements, count, sizeof(T), (_pt)cmp);
}

}}

#endif
