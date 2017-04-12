#ifndef CPEPP_TL_MANAGER_H
#define CPEPP_TL_MANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/tl/tl_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Tl {

class Manager : public Cpe::Utils::SimulateObject {
public:
    operator tl_manage_t (void) const { return (tl_manage_t)(this); }

    tl_manage_state_t state(void) const { return tl_manage_state(*this); }
    void pause(void) { tl_manage_pause(*this); }
    void resume(void) { tl_manage_resume(*this); }
    void rate(float rate) { tl_manage_rate(*this, rate); }

    tl_time_t curTime(void) const { return tl_manage_time(*this); }
    uint32_t curTimeSec(void) const { return (uint32_t)(curTime() / 1000); }

    ptr_int_t tick(ptr_int_t count = -1) { return tl_manage_tick(*this, count); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
