#ifndef GDPP_NM_OBJECTITERATOR_H
#define GDPP_NM_OBJECTITERATOR_H
#include "cpe/nm/nm_read.h"
#include "System.hpp"

namespace Cpe { namespace Nm {

class ObjectIterator {
public:
    ObjectIterator() : _next(NULL) {}
    ObjectIterator(nm_node_it it) : _next(NULL), _it(it) { _next = next_i(); }

    ObjectIterator & operator=(nm_node_it it) {
        _it = it;
        _next = next_i();
        return *this;
    }

    Object * next(void) { 
        if (_next == NULL) return NULL;
        Object * r = _next;
        _next = next_i();
        return r;
    }

private:
    Object * next_i(void);

    Object * _next;
    struct nm_node_it _it;

friend class Manager;
friend class Object;
friend class Group;
friend class ConstObjectIterator;
};

class ConstObjectIterator {
public:
    ConstObjectIterator() {}
    ConstObjectIterator(ObjectIterator const & o) : _next(o._next), _it(o._it) {}
    ConstObjectIterator(nm_node_it it) : _next(NULL), _it(it) { _next = next_i(); }

    ConstObjectIterator & operator=(ObjectIterator const & o) {
        _next = o._next;
        _it = o._it;
        return *this;
    }

    ConstObjectIterator & operator=(nm_node_it it) {
        _it = it;
        _next = next_i();
        return *this;
    }


    Object const * next(void) {
        if (_next == NULL) return NULL;
        Object const * r = _next;
        _next = next_i();
        return r;
    }

private:
    Object const * next_i(void);

    Object const * _next;
    struct nm_node_it _it;

friend class Manager;
friend class Object;
friend class Group;
};

}}

#endif
