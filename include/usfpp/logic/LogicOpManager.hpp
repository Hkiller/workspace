#ifndef USFPP_LOGIC_OPMANAGER_H
#define USFPP_LOGIC_OPMANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_require.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Logic {

class LogicOpManager : public Cpe::Utils::SimulateObject {
public:
    operator logic_manage_t() const { return (logic_manage_t)this; }

    const char * name(void) const { return logic_manage_name(*this); }
    cpe_hash_string_t name_hs(void) const { return logic_manage_name_hs(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(logic_manage_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(logic_manage_app(*this)); }

    LogicOpContext & createContext(
        size_t capacity = 0,
        const void * data = 0,
        logic_context_id_t id = INVALID_LOGIC_CONTEXT_ID);

    template<typename T>
    LogicOpContext &
    createContext(
        T const & data,
        logic_context_id_t id = INVALID_LOGIC_CONTEXT_ID)
    {
        return createContext(sizeof(data), &data, id);
    }

    LogicOpContext & context(logic_context_id_t id);
    LogicOpContext const & context(logic_context_id_t id) const;
    LogicOpContext * findContext(logic_context_id_t id) { return (LogicOpContext *)logic_context_find(*this, id); }
    LogicOpContext const * findContext(logic_context_id_t id) const { return (LogicOpContext *)logic_context_find(*this, id); }
    void clearContexts(void) { logic_context_free_all(*this); }

    LogicOpRequire & require(logic_require_id_t id);
    LogicOpRequire const & require(logic_require_id_t id) const;
    LogicOpRequire * findRequire(logic_require_id_t id) { return (LogicOpRequire *)logic_require_find(*this, id); }
    LogicOpRequire const * findRequire(logic_require_id_t id) const { return (LogicOpRequire *)logic_require_find(*this, id); }

    void destory(void) { logic_manage_free(*this); }

    static LogicOpManager & instance(gd_app_context_t app = 0, cpe_hash_string_t name = 0);
    static LogicOpManager & instance(gd_app_context_t app, const char * name);

    static LogicOpManager * find(gd_app_context_t app = 0, cpe_hash_string_t name = 0);
    static LogicOpManager * find(gd_app_context_t app, const char * name);

    static LogicOpManager & install(gd_app_context_t app, mem_allocrator_t alloc = 0, const char * name = 0);
    static LogicOpManager & install(gd_app_context_t app, gd_timer_mgr_t timer_mgr, mem_allocrator_t alloc = 0, const char * name = 0);
    static void uninstall(gd_app_context_t app, cpe_hash_string_t name = 0);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
