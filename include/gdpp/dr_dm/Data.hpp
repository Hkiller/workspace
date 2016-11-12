#ifndef GDPP_DR_DM_DATA_H
#define GDPP_DR_DM_DATA_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gd/dr_dm/dr_dm_data.h"
#include "System.hpp"

namespace Gd { namespace Dr {

class Data : public Cpe::Utils::SimulateObject {
public:
    operator dr_dm_data_t() const { return (dr_dm_data_t)this; }

    template<typename T>
    T & as(void) { return * (T*)dr_dm_data_data(*this); }

    template<typename T>
    T const & as(void) const { return * (T*)dr_dm_data_data(*this); }

    static Data & _cast(dr_dm_data_t data);
    static Data & create(dr_dm_manage_t mgr, const void * data, size_t size);
};

}}

#endif
