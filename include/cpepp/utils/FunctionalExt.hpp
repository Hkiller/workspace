#ifndef CPEPP_UTILS_FUNCTIONALEXT_H
#define CPEPP_UTILS_FUNCTIONALEXT_H
#include <functional>
#include <memory>
#include "System.hpp"

namespace Cpe { namespace Utils {

template <class T>
struct Deleter : public ::std::unary_function<T* const, void> {
    /*lint -save -e1404*/
    void operator() (T* const ptr) const { delete ptr; }
    /*lint -restore*/
};

template <class T>
struct DefaultConstructCreater {
    ::std::auto_ptr<T> operator() (void) const { return ::std::auto_ptr<T>(new T); }
};

template <class T>
struct CopyConstructCopyer : public ::std::unary_function<const T* const, ::std::auto_ptr<T> > {
    ::std::auto_ptr<T> operator() (const T* const o) const { return ::std::auto_ptr<T>(new T(*o)); }
};

template <class T>
struct CloneCopyer : public ::std::unary_function<const T* const, ::std::auto_ptr<T> > {
    ::std::auto_ptr<T> operator() (const T* const o) const { return o->clone(); }
};

template <class T>
struct CloneExtCopyer : public ::std::unary_function<const T* const, ::std::auto_ptr<T> > {
    ::std::auto_ptr<T> operator() (const T* const o) const { return o->cloneExt(); }
};

}}

#endif
