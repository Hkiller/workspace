#ifndef CPEPP_OTM_MEMO_H
#define CPEPP_OTM_MEMO_H
#include "cpepp/utils/ClassCategory.hpp"
#include "System.hpp"

namespace Cpe { namespace Otm {

class Memo : public Cpe::Utils::SimulateObject {
public:
    operator otm_memo_t (void) const { return (otm_memo_t)(this); }

    uint32_t lastActionTimeS(void) const { return ((otm_memo_t)this)->m_last_action_time_s; }
    uint32_t nextActionTimeS(void) const { return ((otm_memo_t)this)->m_next_action_time_s; }
    void setNextActionTimeS(uint32_t t) { ((otm_memo_t)this)->m_next_action_time_s = t; }
};

}}

#endif
