#ifndef GDPP_NM_OBJECT_H
#define GDPP_NM_OBJECT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/nm/nm_read.h"
#include "System.hpp"
#include "ObjectIterator.hpp"

namespace Cpe { namespace Nm {

class Object : public Cpe::Utils::Noncopyable {
public:
    Object();
    virtual ~Object() = 0;

    const char * name(void) const { return nm_node_name(*this); }
    cpe_hash_string_t name_hs(void) const { return nm_node_name_hs(*this); }
    nm_node_category_t category(void) { return nm_node_category(*this); }

    operator nm_node_t () const { return nm_node_from_data((void*)this); }

    Manager & manager(void) { return *((Manager *)nm_node_mgr(*this)); }
    Manager const & manager(void) const { return *((Manager *)nm_node_mgr(*this)); }

    ObjectIterator groups(void);
    ConstObjectIterator groups(void) const;

    void * operator new (size_t size, nm_mgr_t nmm, const char * name);

    void operator delete(void *p);
    void operator delete(void * p, nm_mgr_t nmm, const char * name) { operator delete(p); }

    static Object * _cast(nm_node_t node);
    static Object * _cast_throw(nm_node_t node);

private:
    void * operator new (size_t size);
};

}}

#endif
