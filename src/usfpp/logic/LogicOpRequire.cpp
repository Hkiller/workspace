#include <limits>
#include "cpe/cfg/cfg_manage.h"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Random.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/logic/LogicOpContext.hpp"

namespace Usf { namespace Logic {

LogicOpData & LogicOpRequire::data(const char * name) {
    LogicOpData * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in require!",
            name);
    }
    return *r;
}

LogicOpData const & LogicOpRequire::data(const char * name) const {
    LogicOpData const * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s not exist in require!",
            name);
    }
    return *r;
}

LogicOpData &
LogicOpRequire::checkCreateData(LPDRMETA meta, size_t capacity) {
    logic_data_t data = logic_require_data_get_or_create(*this, meta, capacity);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s create in require fail!",
            dr_meta_name(meta));
    }
    return *(LogicOpData*)data;
}

LogicOpData &
LogicOpRequire::copy(logic_data_t input) {
    logic_data_t data = logic_require_data_copy(*this, input);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(),
            ::std::runtime_error,
            "data %s copy to require fail!",
            logic_data_name(input));
    }
    return *(LogicOpData*)data;
}

void LogicOpRequire::timeoutStart(tl_time_span_t timeout_ms) {
    if (logic_require_timeout_start(*this, timeout_ms) != 0) {
        APP_CTX_THROW_EXCEPTION(
            context().app(), 
            ::std::runtime_error,
            "require %s start timeout error", name().c_str());
    }
}

}}
