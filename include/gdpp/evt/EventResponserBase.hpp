#ifndef GDPP_EVT_RESPONSER_BASE_H
#define GDPP_EVT_RESPONSER_BASE_H
#include "EventResponser.hpp"
#include "EventCenter.hpp"

namespace Gd { namespace Evt {

class EventResponserBase : public EventResponser {
public:
	EventResponserBase(EventCenter & eventCenter);
	~EventResponserBase();

    template<typename T>
    void registerResponser(const char * oid, T & r, void (T::*fun)(const char * oid, Event const & e)) {
        _eventCenter.registerResponser(oid, r, fun);
    }

protected:
    EventCenter & eventCenter(void) { return _eventCenter; }
    EventCenter const & eventCenter(void) const { return _eventCenter; }

private:	
	EventCenter & _eventCenter;
};

}}

#endif
