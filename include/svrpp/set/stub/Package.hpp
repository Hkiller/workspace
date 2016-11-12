#ifndef SVRPP_SET_STUB_PACKAGE_H
#define SVRPP_SET_STUB_PACKAGE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dp/Request.hpp"
#include "svr/set/share/set_pkg.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class PkgBody : public Cpe::Dp::Request {
public:
    PkgHead & head(void);
    PkgHead const & head(void) const;

    static PkgBody & _cast(dp_req_t req);
};

class PkgHead : public Cpe::Dp::Request {
public:
    uint32_t sn(void) const { return set_pkg_sn(*this); }
    void setSn(uint32_t sn) { return set_pkg_set_sn(*this, sn); }

    set_pkg_category_t category(void) const { return set_pkg_category(*this); }
    void setCategory(set_pkg_category_t c) { set_pkg_set_category(*this, c); }

    uint16_t fromSvrType(void) const { return set_pkg_from_svr_type(*this); }
    uint16_t fromSvrId(void) const { return set_pkg_from_svr_id(*this); }
    void setFromSvr(uint16_t from_svr_type, uint16_t from_svr_id) { set_pkg_set_from_svr(*this, from_svr_type, from_svr_id); }

    uint16_t toSvrType(void) const { return set_pkg_to_svr_type(*this); }
    uint16_t toSvrId(void) const { return set_pkg_to_svr_id(*this); }
    void setToSvr(uint16_t to_svr_type, uint16_t to_svr_id) { set_pkg_set_to_svr(*this, to_svr_type, to_svr_id); }

    static PkgHead & _cast(dp_req_t req);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
