#ifndef SVRPP_SET_LOGIC_RSP_OPCONTEXT_H
#define SVRPP_SET_LOGIC_RSP_OPCONTEXT_H
#include "usfpp/logic/LogicOpContext.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Svr { namespace Set {

class RspOpContext : public Usf::Logic::LogicOpContext  {
public:
    void setCmd(uint32_t cmd);
    uint32_t cmd(void) const;

    void setResponse(uint32_t cmd);
    uint32_t response(void) const;

    void setSn(uint32_t sn);
    uint32_t sn(void) const;

    uint16_t fromSvrType(void) const;
    uint16_t fromSvrId(void) const;
    void setFromSvr(uint16_t from_svr_type, uint16_t from_svr_id);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
