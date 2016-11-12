#ifndef USFPP_LOGIC_USE_SYSTEM_H
#define USFPP_LOGIC_USE_SYSTEM_H
#include "cpepp/dr/System.hpp"
#include "usf/logic_use/logic_use_types.h"
#include "usfpp/logic/System.hpp"

namespace Usf { namespace Logic {

class LogicAsyncOp;
class LogicOpDynData;

template<typename ListT, typename EleT = typename Cpe::Dr::MetaTraits<ListT>::dyn_element_type>
class LogicOpDynList;

template<typename DataT>
class LogicOpDynDataT;

}}

#endif
