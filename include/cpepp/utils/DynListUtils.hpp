#ifndef CPEPP_UTILS_DYNLISTUTILS_H
#define CPEPP_UTILS_DYNLISTUTILS_H
#include <vector>
#include "cpe/pal/pal_types.h"

#define CALC_DYNLIST_SIZE(__listType, __itemType, __size)               \
    ((sizeof(__listType) - sizeof(__itemType)) /*count*/                \
     + sizeof(__itemType) * (__size))

#define DEFINE_DYNLISTBUF(__bufName, __listType, __itemType, __size, __fixsize)   \
    char __bufName ## _fixBuf[__fixsize];                               \
    ::std::vector<char> __bufName ## _dynamicBuf;                       \
    size_t __bufName ## _size =                                         \
        CALC_DYNLIST_SIZE(__listType, __itemType, __size);              \
    __listType * __bufName = 0;                                         \
    if (__bufName ## _size <= sizeof(__bufName ## _fixBuf)) {           \
        __bufName  = (__listType*) __bufName ## _fixBuf;                \
    }                                                                   \
    else {                                                              \
        __bufName ## _dynamicBuf.resize(__bufName ## _size, 0);         \
        __bufName = (__listType*)& __bufName ## _dynamicBuf[0];         \
    }


#endif

