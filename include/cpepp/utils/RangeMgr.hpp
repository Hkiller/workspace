#ifndef CPEPP_UTILS_RANGEMGR_H
#define CPEPP_UTILS_RANGEMGR_H
#include "cpe/utils/range.h"
#include "ClassCategory.hpp"

namespace Cpe { namespace Utils {

typedef struct cpe_range Range;

class RangeMgr : public Noncopyable {
public:
    RangeMgr(mem_allocrator_t alloc);
    ~RangeMgr();

    bool empty(void) const { 
        return cpe_range_mgr_is_empty(const_cast<struct cpe_range_mgr *>(&m_rm)) ? true : false;
    }
    void clear(void) { cpe_range_mgr_clear(&m_rm); }

    ptr_int_t getOne(void);
    void putOne(ptr_int_t value); 

    Range getRange(size_t require_count);
    void putRange(ptr_int_t start, ptr_int_t end);
    void putRange(Range const &  r) { putRange(r.m_start, r.m_end); }

private:
    struct cpe_range_mgr m_rm;
};

}}

#endif
