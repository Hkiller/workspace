#ifndef SVRPP_SET_LOGIC_RSP_RSPMANAGER_H
#define SVRPP_SET_LOGIC_RSP_RSPMANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "gdpp/app/Application.hpp"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class RspManager : public Cpe::Utils::SimulateObject {
public:
    operator set_logic_rsp_manage_t() const { return (set_logic_rsp_manage_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(set_logic_rsp_manage_name(*this)); }

    Usf::Logic::LogicOpManager & logicManager(void) { return *(Usf::Logic::LogicOpManager*)set_logic_rsp_manage_logic(*this); }
    Usf::Logic::LogicOpManager const & logicManager(void) const { return *(Usf::Logic::LogicOpManager*)set_logic_rsp_manage_logic(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(set_logic_rsp_manage_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(set_logic_rsp_manage_app(*this)); }

    bool hasOp(const char * rspName) const;
    RspOpContext & createOp(const char * rspName, logic_context_t from = NULL);
    RspOpContext & createFollowOp(logic_context_t context, const char * rspName);
    RspOpContext * tryCreateOp(const char * rspName, logic_context_t from = NULL);
    RspOpContext * tryCreateFollowOp(logic_context_t context, const char * rspName);

    void loadRsps(cfg_t cfg, LPDRMETALIB metalib = NULL);

    static RspManager & _cast(set_logic_rsp_manage_t rsp_mgr);
    static RspManager & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
