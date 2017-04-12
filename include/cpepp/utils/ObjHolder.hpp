#ifndef CPEPP_UTILS_OBJHOLDER_H
#define CPEPP_UTILS_OBJHOLDER_H
#include <typeinfo>
#include <ostream>
#include <memory>
#include <cassert>
#include "System.hpp"
#include "FunctionalExt.hpp"

namespace Cpe { namespace Utils {

template<
    typename T
    , typename _Copyer = CopyConstructCopyer<T>
    , typename _Deletor = Deleter<T> >
class ObjHolder: private _Copyer, private _Deletor {
public:
    ObjHolder(_Copyer copyer = _Copyer(), _Deletor deletor = _Deletor())
        : _Copyer(copyer)
        , _Deletor(deletor)
        , m_obj(0)
    {
    }

    explicit ObjHolder(
        ::std::auto_ptr<T> obj
        , _Copyer copyer = _Copyer()
        , _Deletor deletor = _Deletor())
        : _Copyer(copyer)
        , _Deletor(deletor)
        , m_obj(obj.release())
    {
    }

    ObjHolder(
        ObjHolder const & o
        , _Copyer copyer = _Copyer()
        , _Deletor deletor = _Deletor())
        : _Copyer(copyer)
        , _Deletor(deletor)
        , m_obj(0)
    {
        if (o.m_obj) {
            m_obj = copyer()(o.m_obj).release();
        }
    }

    ObjHolder(
        T const & o
        , _Copyer copyer = _Copyer()
        , _Deletor deletor = _Deletor())
        : _Copyer(copyer)
        , _Deletor(deletor)
        , m_obj(0)
    {
        m_obj = copyer()(&o).release();
    }

    ~ObjHolder() throw() {
        try { deletor()(m_obj); } catch(...) {}
    }

    ObjHolder & operator=(T const & o) {
        ::std::auto_ptr<T> newObj = copyer()(&o);
        deletor()(m_obj);
        m_obj = newObj.release();
        return *this;
    } 

    ObjHolder & operator=(ObjHolder const & o) {
        if (&o == this) return *this;

        if (o.m_obj) {
            ::std::auto_ptr<T> newObj = copyer()(o.m_obj);
            deletor()(m_obj);
            m_obj = newObj.release();
        }
        else {
            deletor()(m_obj);
            m_obj = 0;
        }

        return *this;
    } 

    ::std::auto_ptr<T> reset(::std::auto_ptr<T> o) {
        T * ret = m_obj;
        m_obj = o.release();
        return ::std::auto_ptr<T>(ret);
    }

    ::std::auto_ptr<T> retrieve(void) {
        T * ret = m_obj;
        m_obj = 0;
        return ::std::auto_ptr<T>(ret);
    }

    T & get(void) { assert(m_obj); return *m_obj; }
    T const & get(void) const { assert(m_obj); return *m_obj; }

    operator T& () { assert(m_obj); return *m_obj; }
    operator T const & () const { assert(m_obj); return *m_obj; }

    bool valid(void) const { return m_obj != 0; }

    void swap(ObjHolder & o) throw() {
        ::std::swap(m_obj, o.m_obj);
    }

    T * data(void) const { return m_obj; }

    void clear(void) {
        deletor()(m_obj);
        m_obj = 0;
    }

    bool operator==(ObjHolder const & o) const {
        if (m_obj == 0 && o.m_obj == 0) return true;
        if (m_obj && o.m_obj) return *m_obj == *o.m_obj;
        return false;
    }

private:
    _Copyer const & copyer(void) const { return *this; }
    _Deletor const & deletor(void) const { return *this; }

    T * m_obj;
};

template<typename T, typename _Copyer, typename _Deletor>
inline ::std::ostream & operator<< (::std::ostream& os, ObjHolder<T, _Copyer, _Deletor> const & o) {
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
