#ifndef GDPP_TIMER_PROCESSOR_BASE_H
#define GDPP_TIMER_PROCESSOR_BASE_H
#include "TimerProcessor.hpp"
#include "TimerCenter.hpp"

namespace Gd { namespace Timer {

class TimerProcessorBase : public TimerProcessor {
public:
	TimerProcessorBase(TimerCenter & eventCenter);
	~TimerProcessorBase();

    template<typename T>
    TimerID registerTimer(
        T & r,
        void (T::*fun)(TimerID timerId),
        tl_time_span_t delay,
        tl_time_span_t span,
        int repeatCount = -1)
    {
        return _eventCenter.registerTimer(r, fun, delay, span, repeatCount);
    }

    void unregisterTimer(TimerID timerId) { _eventCenter.unregisterTimer(timerId); }

private:	
	TimerCenter & _eventCenter;
};

}}

#endif
