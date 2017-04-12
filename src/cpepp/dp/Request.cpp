#include <sstream>
#include <stdexcept>
#include "cpepp/dp/Request.hpp"

namespace Cpe { namespace Dp {

Request & Request::parent(const char * type) {
    Request * r = findParent(type);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "can`t find parent request of type " << type << "!";
        throw ::std::runtime_error(os.str().c_str());
    }
    return *r;
}

Request const & Request::parent(const char * type) const {
    Request const * r = findParent(type);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "can`t find parent request of type " << type << "!";
        throw ::std::runtime_error(os.str().c_str());
    }
    return *r;
}

Request & Request::brother(const char * type) {
    Request * r = findBrother(type);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "can`t find brother request of type " << type << "!";
        throw ::std::runtime_error(os.str().c_str());
    }
    return *r;
}

Request const & Request::brother(const char * type) const {
    Request const * r = findBrother(type);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "can`t find brother request of type " << type << "!";
        throw ::std::runtime_error(os.str().c_str());
    }
    return *r;
}

Request & Request::child(const char * type) {
    Request * r = findChild(type);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "can`t find child request of type " << type << "!";
        throw ::std::runtime_error(os.str().c_str());
    }
    return *r;
}

Request const & Request::child(const char * type) const {
    Request const * r = findChild(type);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "can`t find child request of type " << type << "!";
        throw ::std::runtime_error(os.str().c_str());
    }
    return *r;
}

Request & Request::_cast(dp_req_t req) {
    if (req == NULL) {
        throw ::std::runtime_error("input req is null");
    }

    return *(Request*)req;
}

}}
