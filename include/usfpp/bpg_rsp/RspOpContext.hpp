#ifndef USFPP_BPG_RSP_OPCONTEXT_H
#define USFPP_BPG_RSP_OPCONTEXT_H
#include "usfpp/logic/LogicOpContext.hpp"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class RspOpContext : public Logic::LogicOpContext  {
public:
    void addAdditionData(uint32_t meta_id);
    void removeAdditionData(uint32_t meta_id);

    void setClientId(uint64_t client_id);
    uint64_t clientId(void) const;

    void setCmd(uint32_t cmd);
    uint32_t cmd(void) const;

    void setSn(uint32_t sn);
    uint32_t sn(void) const;

    void setResponse(bool needResponse);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
