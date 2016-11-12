#ifndef GDPP_EVT_SYSTEM_H
#define GDPP_EVT_SYSTEM_H
#include "gdpp/app/System.hpp"
#include "gd/evt/evt_types.h"

namespace Gd { namespace Evt {

typedef evt_processor_id_t ProcessorID;

class Event;
class EventCenter;
class EventResponser;
class EventResponserBase;
typedef void (EventResponser::*EventProcessFun)(const char * oid, Event const & e);

}}

#endif
