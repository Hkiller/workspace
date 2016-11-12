#ifndef CPEPP_OTM_SYSTEM_H
#define CPEPP_OTM_SYSTEM_H
#include "cpe/tl/tl_types.h"
#include "cpe/otm/otm_types.h"

namespace Cpe { namespace Otm {

class Memo;
class Timer;
class ManagerBase;

template<size_t capacity>
class MemoBuf;

template<typename ContextT>
class Manager;

template<typename ContextT>
class TimerProcessor;

}}

#endif
