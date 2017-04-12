#ifndef GDPP_TIMER_SYSTEM_H
#define GDPP_TIMER_SYSTEM_H
#include "gdpp/app/System.hpp"
#include "gd/timer/timer_types.h"

namespace Gd { namespace Timer {

typedef gd_timer_id_t TimerID;

class TimerCenter;
class TimerProcessor;
class TimerProcessorBase;
typedef void (TimerProcessor::*TimerProcessFun)(TimerID id);

}}

#endif
