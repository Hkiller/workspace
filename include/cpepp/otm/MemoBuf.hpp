#ifndef CPEPP_OTM_MEMOBUF_H
#define CPEPP_OTM_MEMOBUF_H
#include "cpe/otm/otm_memo.h"
#include "System.hpp"

namespace Cpe { namespace Otm {

template<size_t capacity>
class MemoBuf {
public:
    operator otm_memo_t (void) { return &m_memos[0]; }

	otm_memo * findMemo(otm_timer_id_t timer_id) {
        return otm_memo_find(timer_id, m_memos, capacity);
	}

private:
    struct otm_memo m_memos[capacity];
};

}}

#endif
