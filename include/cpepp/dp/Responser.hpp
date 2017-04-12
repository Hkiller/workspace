#ifndef GDPP_DP_RESPONSER_H
#define GDPP_DP_RESPONSER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpe/cfg/cfg_types.h"
#include "cpe/dp/dp_responser.h"
#include "System.hpp"

namespace Cpe { namespace Dp {

class Responser : public Cpe::Utils::SimulateObject {
public:
    operator dp_rsp_t () const { return (dp_rsp_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(dp_rsp_name(*this)); }

    void setProcessor(dp_rsp_process_fun_t processor, void * ctx) {
        dp_rsp_set_processor(*this, processor, ctx);
    }

    void setType(struct dp_rsp_type * rspType) {
        dp_rsp_set_type(*this, rspType);
    }

    void bind(const char * targt);
    void bind(int32_t target);
    void bind(cfg_t cfg);
};

}}

#endif

