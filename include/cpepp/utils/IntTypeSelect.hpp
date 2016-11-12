#ifndef CPEPP_UTILS_INTTYPESELECT_H
#define CPEPP_UTILS_INTTYPESELECT_H
#include "System.hpp"

namespace Cpe { namespace Utils {

template<size_t size>
struct IntTypeSelect;

template<>
struct IntTypeSelect<1> {
    typedef uint8_t uint_type;
    typedef int8_t int_type;
};

template<>
struct IntTypeSelect<2> {
    typedef uint16_t uint_type;
    typedef int16_t int_type;
};

template<>
struct IntTypeSelect<4> {
    typedef uint32_t uint_type;
    typedef int32_t int_type;
};

template<>
struct IntTypeSelect<8> {
    typedef uint64_t uint_type;
    typedef int64_t int_type;
};

}}

#endif
