#ifndef CPEPP_UTILS_TYPEUTILS_H
#define CPEPP_UTILS_TYPEUTILS_H
#include "cpe/pal/pal_types.h"

namespace Cpe { namespace Utils {

template<typename ToT, typename FromT>
inline ToT & calc_cast(FromT & o) {
    return *(ToT*)(
        ((char*)&o) + ( ((char*)((ToT *)10000)) - ((char*)((FromT*)((ToT *)10000))) )
        );
}

template<typename ToT, typename FromT>
inline ToT const & calc_cast(FromT const & o) {
    return *(ToT const *)(
        ((char const *)&o) + ( ((char*)((ToT *)10000)) - ((char*)((FromT*)((ToT *)10000))) )
        );
}

template<typename ToT, typename FromT>
inline ToT * calc_cast(FromT * o) {
    return (ToT*)(
        ((char*)o) + ( ((char*)((ToT *)10000)) - ((char*)((FromT*)((ToT *)10000))) )
        );
}

template<typename ToT, typename FromT>
inline ToT const * calc_cast(FromT const * o) {
    return (ToT const *)(
        ((char const *)o) + ( ((char*)((ToT *)10000)) - ((char*)((FromT*)((ToT *)10000))) )
        );
}

}}

#endif

