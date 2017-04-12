#include "gdpp/evt/EventResponserBase.hpp"

namespace Gd { namespace Evt {

EventResponser::~EventResponser() {
}

EventResponserBase::EventResponserBase(EventCenter & eventCenter)
    : _eventCenter(eventCenter)
{
}

EventResponserBase::~EventResponserBase() {
    _eventCenter.unregisterResponser(*this);
}

}}

