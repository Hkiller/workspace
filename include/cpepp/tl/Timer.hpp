#ifndef CPEPP_TL_TIMER_H
#define CPEPP_TL_TIMER_H
#include "System.hpp"

namespace Cpe { namespace Tl {

class Timer {
public:
    virtual void onTimer(int timerId) = 0;
    virtual ~Timer() throw();
};

}}

#endif

