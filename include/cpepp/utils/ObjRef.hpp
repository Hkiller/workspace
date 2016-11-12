#ifndef CPEPP_UTILS_OBJREF_H
#define CPEPP_UTILS_OBJREF_H
#include <typeinfo>
#include <ostream>
#include <cassert>
#include "ToRef.hpp"

namespace Cpe { namespace Utils {

template<typename T>
class ObjRef {
public:
    ObjRef() : m_obj(0) {}
    ObjRef(T & obj) : m_obj(&obj) {}
    ObjRef(ObjRef const & o) : m_obj(o.m_obj) {}

    ObjRef & operator=(T & obj) { m_obj = &obj; return *this; }
    ObjRef & operator=(T const & obj) { m_obj = &obj; return *this; }

    T & get(void) const { assert(m_obj); return *m_obj; }
    operator T& () const { return toRef(m_obj); }

    bool valid(void) const { return m_obj != 0; }
    T * data(void) const { return m_obj; }
    void clear(void) { m_obj = 0; }

    void swap(ObjRef & o) {
        T * buf = o.m_obj;
        o.m_obj = m_obj;
        m_obj = buf;
    }

    void reset(T * p) { m_obj = p; }

    bool operator==(ObjRef const & o) const {
        if (m_obj == 0 && o.m_obj == 0) return true;
        else if(m_obj != 0 && o.m_obj != 0) return *m_obj == *o.m_obj;
        else { return false; }
    }

private:
    T * m_obj;
};

template<typename T>
inline ::std::ostream & operator<< (::std::ostream& os, ObjRef<T> const & o) {
    T * p = o.data();
    if (p == 0) {
        return os << "NULL[" << typeid(o).name() << "]";
    }
    else {
        return os << *p;
    }
}

}}

#endif
