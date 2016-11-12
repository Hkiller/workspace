#include <stdexcept>
#include "cpe/pal/pal_string.h"
#include "svrpp/set/stub/Package.hpp"

namespace Svr { namespace Set {

PkgHead & PkgBody::head(void) {
    dp_req_t head = set_pkg_head_find(*this);
    if (head == NULL) {
        throw ::std::runtime_error("Svr::Set::PkgBody::head: head not exist!");
    }

    return *(PkgHead*)head;
}

PkgHead const & PkgBody::head(void) const {
    dp_req_t head = set_pkg_head_find(*this);
    if (head == NULL) {
        throw ::std::runtime_error("Svr::Set::PkgBody::head: head not exist!");
    }

    return *(PkgHead*)head;
}

PkgBody & PkgBody::_cast(dp_req_t req) {
    if (req == NULL) {
        throw ::std::runtime_error("Svr::Set::PkgBody::_cast: input req is NULL!");
    }

    return *(PkgBody*)req;
}

PkgHead & PkgHead::_cast(dp_req_t req) {
    if (req == NULL) {
        throw ::std::runtime_error("Svr::Set::PkgHead::_cast: input req is NULL!");
    }

    if (strcmp(dp_req_type(req), req_type_set_pkg_head) != 0) {
        throw ::std::runtime_error("Svr::Set::PkgHead::_cast: not head_pkg bind!");
    }

    return *(PkgHead*)req;
}

}}

