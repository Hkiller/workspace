#ifndef USFPP_LOGIC_LOGICOPTYPEGROUP_H
#define USFPP_LOGIC_LOGICOPTYPEGROUP_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/logic/logic_executor_type.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpTypeGroup : public Cpe::Utils::SimulateObject {
public:
    operator logic_executor_type_group_t() const { return (logic_executor_type_group_t)this; }

    const char * name(void) const { return logic_executor_type_group_name(*this); }
    cpe_hash_string_t name_hs(void) const { return logic_executor_type_group_name_hs(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(logic_executor_type_group_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(logic_executor_type_group_app(*this)); }

    LogicOpType const * findType(const char * name) const { return (LogicOpType const *)logic_executor_type_find(*this, name); }
    LogicOpType * findType(const char * name) { return (LogicOpType *)logic_executor_type_find(*this, name); }

    LogicOpType const & type(const char * name) const;
    LogicOpType & type(const char * name);

    void destory(void) { logic_executor_type_group_free(*this); }

    static LogicOpTypeGroup & instance(gd_app_context_t app = 0, cpe_hash_string_t name = 0);
    static LogicOpTypeGroup & instance(gd_app_context_t app, const char * name);

    static LogicOpTypeGroup * find(gd_app_context_t app = 0, cpe_hash_string_t name = 0);
    static LogicOpTypeGroup * find(gd_app_context_t app, const char * name);

    static LogicOpTypeGroup & install(gd_app_context_t app, mem_allocrator_t alloc = 0, const char * name = 0);
    static void uninstall(gd_app_context_t app, cpe_hash_string_t name = 0);
};

}}

#endif
