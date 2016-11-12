#include "cpe/cfg/cfg_manage.h"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Random.hpp"
#include "usf/logic/logic_require.h"
#include "usfpp/logic/LogicOpStack.hpp"
#include "usfpp/logic/LogicOpContext.hpp"

namespace Usf { namespace Logic {

LogicOpData & LogicOpStackNode::data(const char * name) {
    LogicOpData * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in stack!",
            name);
    }
    return *r;
}

LogicOpData const & LogicOpStackNode::data(const char * name) const {
    LogicOpData const * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in stack!",
            name);
    }
    return *r;
}

LogicOpData &
LogicOpStackNode::checkCreateData(LPDRMETA meta, size_t capacity) {
    logic_data_t data = logic_stack_data_get_or_create(*this, meta, capacity);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s create in stack fail!",
            dr_meta_name(meta));
    }
    return *(LogicOpData*)data;
}

LogicOpData &
LogicOpStackNode::copy(logic_data_t input) {
    logic_data_t data = logic_stack_data_copy(*this, input);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s copy to stack fail!",
            logic_data_name(input));
    }
    return *(LogicOpData*)data;
}

LogicOpRequire &
LogicOpStackNode::createRequire(const char * name) {
    logic_require_t require = logic_require_create(*this, name);

    if (require == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "require %s create in stack fail!",
            name);
    }
    return *(LogicOpRequire*)require;
}

}}
