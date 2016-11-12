#ifndef USFPP_LOGIC_SYSTEM_H
#define USFPP_LOGIC_SYSTEM_H
#include <memory>
#include "cpe/pal/pal_types.h"
#include "gdpp/app/System.hpp"
#include "cpepp/cfg/System.hpp"
#include "cpepp/utils/System.hpp"
#include "usf/logic/logic_types.h"

namespace Usf { namespace Logic {

class LogicOp;
class LogicOpData;
class LogicOpContext;
class LogicOpManager;
class LogicOpExecutor;
class LogicOpType;
class LogicOpTypeGroup;
class LogicOpStackNode;
class LogicOpRequire;

typedef logic_context_id_t LogicOpContextID;

}}

#endif
