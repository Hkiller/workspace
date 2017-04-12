#ifndef GDPP_DP_REQUEST_H
#define GDPP_DP_REQUEST_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/dp/dp_request.h"
#include "cpepp/dr/System.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Dp {

class Request : public Cpe::Utils::SimulateObject {
public:
    operator dp_req_t () const { return (dp_req_t)this; }

    void * data(void) { return dp_req_data(*this); }
    void const * data(void) const { return dp_req_data(*this); }

    size_t size(void) const { return dp_req_size(*this); }
    void setSize(size_t size) { dp_req_set_size(*this, size); }

    size_t capacity(void) const { return dp_req_capacity(*this); }

    void setType(const char * type) { dp_req_set_type(*this, type); }
    const char * type(void) const { return dp_req_type(*this); }

    dp_mgr_t mgr(void) {
        return dp_req_mgr( *this );
    }

    void setParent(dp_req_t parent) {
        dp_req_set_parent(*this, parent);
    }

    Request * parent(void) {
        return (Request*)(dp_req_parent(*this));
    }
    Request const * parent(void) const {
        return (Request*)(dp_req_parent(*this));
    }

    Request & parent(const char * type);
    Request const & parent(const char * type) const;

    Request * findParent(const char * type) { 
        return (Request*)dp_req_parent_find(*this, type);
    }
    Request const * findParent(const char * type) const { 
        return (Request*)dp_req_parent_find(*this, type);
    }

    Request & brother(const char * type);
    Request const & brother(const char * type) const;

    Request * findBrother(const char * type) { 
        return (Request*)dp_req_brother_find(*this, type);
    }
    Request const * findBrother(const char * type) const { 
        return (Request*)dp_req_brother_find(*this, type);
    }

    Request & child(const char * type);
    Request const & child(const char * type) const;

    Request * findChild(const char * type) { 
        return (Request*)dp_req_child_find(*this, type);
    }
    Request const * findChild(const char * type) const { 
        return (Request*)dp_req_child_find(*this, type);
    }

    static Request * _create(dp_mgr_t mgr, size_t capacity) {
        return (Request*)dp_req_create(mgr, capacity);
    }

    static Request & _cast(dp_req_t req);

    static void _free(Request * req) { if (req) { dp_req_free(*req); } }

    template<typename T>
    T & as(void) { return *((T*)dp_req_data(*this)); }

    template<typename T>
    T const & as(void) const { return *((T const*)dp_req_data(*this)); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif

