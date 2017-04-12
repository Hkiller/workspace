#ifndef CPEPP_TL_TIMELINE_H
#define CPEPP_TL_TIMELINE_H
#include "cpe/tl/tl_manage.h"
#include "System.hpp"

namespace Cpe { namespace Tl {

class TimeLine : public Cpe::Utils::SimulateObject {
public:
    operator tl_t (void) const { return (tl_t)(this); }
};

}}

#endif
