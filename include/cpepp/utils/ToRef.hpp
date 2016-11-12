#ifndef CPEPP_UTILS_TOREF_H
#define CPEPP_UTILS_TOREF_H
#include <typeinfo>
#include <memory>
#include "System.hpp"

namespace Cpe { namespace Utils {

void __throw_ptr_is_null_exception(const char * typeName);

template<typename T>
inline T & toRef(T * p) { 
    if (p == 0) __throw_ptr_is_null_exception(typeid(T).name());
    return *p;
}

template<typename T>
inline T const & toRef(T const * p) { 
    if (p == 0) __throw_ptr_is_null_exception(typeid(T).name());
    return *p;
}

template<typename T>
inline T & toRef(::std::auto_ptr<T> const & p) { 
    if (p.get() == 0) __throw_ptr_is_null_exception(typeid(T).name());
    return *p;
}

}}

#endif
