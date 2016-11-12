#ifndef USFPP_LOGIC_LOGICOPTYPE_H
#define USFPP_LOGIC_LOGICOPTYPE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic/logic_executor_type.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpType : public Cpe::Utils::SimulateObject {
public:
    operator logic_executor_type_t() const { return (logic_executor_type_t)this; }

    const char * name(void) const { return logic_executor_type_name(*this); }
    logic_executor_category_t category(void) const { return logic_executor_type_category(*this); }
};

}}

#endif
