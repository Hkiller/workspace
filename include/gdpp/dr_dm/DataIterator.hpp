#ifndef GDPP_DR_DM_DATAITERATOR_H
#define GDPP_DR_DM_DATAITERATOR_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gd/dr_dm/dr_dm_data.h"
#include "System.hpp"

namespace Gd { namespace Dr {

class DataConstIterator {
public:
    Data const * next(void) const { return (Data const *)dr_dm_data_it_next(&m_it); }

    template<typename T>
    T const * next(void) const { 
        Data const * d = this->next();
        return d ? (T const *)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }

private:
    mutable dr_dm_data_it m_it;

friend class DataManager;
friend class DataIterator;
};

class DataIterator : public DataConstIterator {
public:
    using DataConstIterator::next;

    Data * next(void) { return (Data *)dr_dm_data_it_next(&m_it); }

    template<typename T>
    T * next(void) { 
        Data * d = this->next();
        return d ? (T *)dr_dm_data_data((dr_dm_data_t)d) : NULL;
    }
};

}}

#endif
