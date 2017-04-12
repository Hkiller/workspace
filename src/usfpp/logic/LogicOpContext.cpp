#include <limits>
#include <vector>
#include "cpe/cfg/cfg_manage.h"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Random.hpp"
#include "usfpp/logic/LogicOpContext.hpp"

namespace Usf { namespace Logic {

LogicOpData & LogicOpContext::data(const char * name) {
    LogicOpData * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "data %s not exist in context!",
            name);
    }
    return *r;
}

LogicOpData const & LogicOpContext::data(const char * name) const {
    LogicOpData const * r = findData(name);
    if (r == 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "data %s not exist in context!",
            name);
    }
    return *r;
}

LogicOpData &
LogicOpContext::checkCreateData(LPDRMETA meta, size_t capacity) {
    logic_data_t data = logic_context_data_get_or_create(*this, meta, capacity);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "data %s create in context fail!",
            dr_meta_name(meta));
    }
    return *(LogicOpData*)data;
}

LogicOpData &
LogicOpContext::copy(logic_data_t input) {
    logic_data_t data = logic_context_data_copy(*this, input);
    if (data == 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "data %s copy to context fail!",
            logic_data_name(input));
    }
    return *(LogicOpData*)data;
}

Cpe::Utils::Random &
LogicOpContext::random(cpe_hash_string_t name) {
    return Gd::App::Random::instance(
        app(),
        name == 0 ? Gd::App::Random::DEFAULT_NAME : name);
}

void LogicOpContext::bind(logic_executor_t executor) {
    if (logic_context_bind(*this, executor) != 0) {
        if (executor == NULL) {
            APP_CTX_THROW_EXCEPTION(
                app(),
                ::std::runtime_error,
                "context bind executor, input executor is null!");
        }
        else if (state() != logic_context_state_init) {
            APP_CTX_THROW_EXCEPTION(
                app(),
                ::std::runtime_error,
                "context bind executor, state(%d) error, only support state init!",
                state());
        }
        else {
            APP_CTX_THROW_EXCEPTION(
                app(),
                ::std::runtime_error,
                "context bind executor, unknown error!");
        }
    }
}

void LogicOpContext::timeoutStart(tl_time_span_t timeout_ms) {
    if (logic_context_timeout_start(*this, timeout_ms) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(), 
            ::std::runtime_error,
            "context start timeout error");
    }
}

}}
