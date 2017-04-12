#include "gd/app/app_context.h"
#include "gdpp/app/Log.hpp"
#include "usf/logic/logic_context.h"
#include "usfpp/logic/LogicOpTypeGroup.hpp"

namespace Usf { namespace Logic {

LogicOpType const & LogicOpTypeGroup::type(const char * name) const {
    if (LogicOpType const * r = findType(name)) {
        return *r;
    }

    APP_CTX_THROW_EXCEPTION(
        app(),
        ::std::runtime_error,
        "Usf::Logic::LogicOpTypeGroup %s have not type %s!",
        this->name(), name);
}

LogicOpType & LogicOpTypeGroup::type(const char * name) {
    if (LogicOpType * r = findType(name)) {
        return *r;
    }

    APP_CTX_THROW_EXCEPTION(
        app(),
        ::std::runtime_error,
        "Usf::Logic::LogicOpTypeGroup %s have not type %s!",
        this->name(), name);
}


LogicOpTypeGroup &
LogicOpTypeGroup::instance(gd_app_context_t app, cpe_hash_string_t name) {
    if (LogicOpTypeGroup * r = find(app, name)) {
        return *r;
    }
    else {
        if (app == 0) app = gd_app_ins();

        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "get Usf::Logic::LogicOpTypeGroup '%s' fail!",
            name ? cpe_hs_data(name) : "default");
    }
}

LogicOpTypeGroup &
LogicOpTypeGroup::instance(gd_app_context_t app, const char * name) {
    if (LogicOpTypeGroup * r = find(app, name)) {
        return *r;
    }
    else {
        if (app == 0) app = gd_app_ins();

        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "get Usf::Logic::LogicOpTypeGroup '%s' fail!",
            name ? name : "default");
    }
}

LogicOpTypeGroup *
LogicOpTypeGroup::find(gd_app_context_t app, cpe_hash_string_t name) {
    if (app == 0) app = gd_app_ins();

    logic_executor_type_group_t type_group =
        name == 0
        ? logic_executor_type_group_default(app)
        : logic_executor_type_group_find(app, name);

    return (LogicOpTypeGroup*)type_group;
}

LogicOpTypeGroup *
LogicOpTypeGroup::find(gd_app_context_t app, const char * name) {
    if (app == 0) app = gd_app_ins();

    logic_executor_type_group_t type_group =
        name == 0
        ? logic_executor_type_group_default(app)
        : logic_executor_type_group_find_nc(app, name);

    return (LogicOpTypeGroup*)type_group;
}

LogicOpTypeGroup &
LogicOpTypeGroup::install(gd_app_context_t app, mem_allocrator_t alloc, const char * name) {
    logic_executor_type_group_t type_group = logic_executor_type_group_create(app, name, alloc);
    if (type_group == 0) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "create Usf::Logic::LogicOpTypeGroup %s fail!", name);
    }

    return *(LogicOpTypeGroup*)type_group;
}

void LogicOpTypeGroup::uninstall(gd_app_context_t app, cpe_hash_string_t name) {
    if (LogicOpTypeGroup * opTypeGroup = find(app, name)) {
        opTypeGroup->destory();
    }
}

}}
