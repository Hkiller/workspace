#include <stdexcept>
#include <sstream>
#include "cpepp/utils/RangeMgr.hpp"

namespace Cpe { namespace Utils {

RangeMgr::RangeMgr(mem_allocrator_t alloc) {
    if (cpe_range_mgr_init(&m_rm, alloc) != 0) {
        throw ::std::runtime_error("init RangeMgr fail!");
    }
}

RangeMgr::~RangeMgr() {
    cpe_range_mgr_fini(&m_rm);
}

ptr_int_t RangeMgr::getOne(void) {
    ptr_int_t r = cpe_range_get_one(&m_rm);
    if (r < 0) {
        throw ::std::runtime_error("get one from RangeMgr fail!");
    }
    return r;
}

void RangeMgr::putOne(ptr_int_t value) {
    if (cpe_range_put_one(&m_rm, value) != 0) {
        throw ::std::runtime_error("put one to RangeMgr fail!");
    }
}

Range RangeMgr::getRange(size_t require_count) {
    Range r = cpe_range_get_range(&m_rm, require_count);
    if (!cpe_range_is_valid(r)) {
        ::std::ostringstream os;
        os << "put range(size=" << require_count << ") from RangeMgr fail!";
        throw ::std::runtime_error(os.str());
    }
    return r;
}

void RangeMgr::putRange(ptr_int_t start, ptr_int_t end) {
    if (cpe_range_put_range(&m_rm, start, end) != 0) {
        ::std::ostringstream os;
        os << "put range[" << start << "," << end << ") to RangeMgr fail!";
        throw ::std::runtime_error(os.str());
    }
}

}}
