#ifndef USFPP_BPG_RSP_RSPMANAGER_H
#define USFPP_BPG_RSP_RSPMANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usfpp/logic/System.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class RspManager : public Cpe::Utils::SimulateObject {
public:
    operator bpg_rsp_manage_t() const { return (bpg_rsp_manage_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(bpg_rsp_manage_name(*this)); }

    Usf::Logic::LogicOpManager & logicManager(void) { return *(Usf::Logic::LogicOpManager*)bpg_rsp_manage_logic(*this); }
    Usf::Logic::LogicOpManager const & logicManager(void) const { return *(Usf::Logic::LogicOpManager*)bpg_rsp_manage_logic(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_rsp_manage_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(bpg_rsp_manage_app(*this)); }

    bool hasOp(const char * rspName) const;
    RspOpContext & createOp(const char * rspName, logic_context_t from = NULL);
    RspOpContext & createFollowOp(logic_context_t context, const char * rspName);
    RspOpContext * tryCreateOp(const char * rspName, logic_context_t from = NULL);
    RspOpContext * tryCreateFollowOp(logic_context_t context, const char * rspName);

    void loadRsps(cfg_t cfg, LPDRMETALIB metalib = NULL);

    static RspManager & _cast(bpg_rsp_manage_t rsp_mgr);
    static RspManager & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
