#include "gdpp/timer/TimerProcessorBase.hpp"

namespace Gd { namespace Timer {

TimerProcessor::~TimerProcessor() {
}

TimerProcessorBase::TimerProcessorBase(TimerCenter & eventCenter)
    : _eventCenter(eventCenter)
{
}

TimerProcessorBase::~TimerProcessorBase() {
    _eventCenter.unregisterTimer(*this);
}

}}

