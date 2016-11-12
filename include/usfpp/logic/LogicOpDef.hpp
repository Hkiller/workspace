#ifndef USFPP_LOGIC_LOGICOPDEF_H
#define USFPP_LOGIC_LOGICOPDEF_H
#include "gdpp/app/ModuleDef.hpp"
#include "LogicOp.hpp"

#define USFPP_LOGICOP_DEF(__module_name, __module_impl)               \
    GDPP_APP_MODULE_DEF_EX(                                           \
        __module_name, __module_impl, ::Usf::Logic::LogicOp::init)

#define USFPP_LOGICOP_DEF_BASE(__module_name) USFPP_LOGICOP_DEF(__module_name, __module_name)

#endif
