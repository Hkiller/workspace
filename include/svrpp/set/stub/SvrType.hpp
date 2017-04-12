#ifndef SVRPP_SET_STUB_SVRINFO_H
#define SVRPP_SET_STUB_SVRINFO_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "svr/set/stub/set_svr_svr_info.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class SvrType : public Cpe::Utils::SimulateObject {
public:
    operator set_svr_svr_info_t() const { return (set_svr_svr_info_t)this; }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
