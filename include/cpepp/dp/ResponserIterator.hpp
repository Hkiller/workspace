#ifndef GDPP_DP_RESPONSERITERATOR_H
#define GDPP_DP_RESPONSERITERATOR_H
#include "cpe/dp/dp_manage.h"
#include "System.hpp"

namespace Cpe { namespace Dp {

class ResponserIterator {
public:
    ResponserIterator() {
        m_it.m_next_fun = NULL;
        m_it.m_context = NULL;
    }

    ResponserIterator(struct dp_rsp_it it) : m_it(it) {}
    ResponserIterator(ResponserIterator const & o) : m_it(o.m_it) {}

    ResponserIterator & operator=(struct dp_rsp_it const & o) { m_it = o; return *this; }
    ResponserIterator & operator=(ResponserIterator const & o) { m_it = o.m_it; return *this; }

    Responser * next(void) {
        return m_it.m_next_fun ? (Responser*)m_it.m_next_fun(&m_it) : NULL;
    }

private:
    struct dp_rsp_it m_it;

friend class Manager;
friend class ConstResponserIterator;
};

class ConstResponserIterator {
public:
    ConstResponserIterator() {
        m_it.m_next_fun = NULL;
        m_it.m_context = NULL;
    }

    ConstResponserIterator(struct dp_rsp_it it) : m_it(it) {}
    ConstResponserIterator(ResponserIterator const & o) : m_it(o.m_it) {}
    ConstResponserIterator(ConstResponserIterator const & o) : m_it(o.m_it) {}

    ConstResponserIterator & operator=(struct dp_rsp_it const & o) { m_it = o; return *this; }
    ConstResponserIterator & operator=(ConstResponserIterator const & o) { m_it = o.m_it; return *this; }
    ConstResponserIterator & operator=(ResponserIterator const & o) { m_it = o.m_it; return *this; }

    Responser const * next(void) {
        return m_it.m_next_fun ? (Responser const *)m_it.m_next_fun(&m_it) : NULL;
    }

private:
    struct dp_rsp_it m_it;

friend class Manager;
};

}}

#endif

