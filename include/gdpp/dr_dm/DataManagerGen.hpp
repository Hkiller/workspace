#ifndef GDPP_DR_DM_DATAMANAGERGEN_H
#define GDPP_DR_DM_DATAMANAGERGEN_H
#include "DataManager.hpp"

namespace Gd { namespace Dr {

template<typename OuterT>
class DataManagerGen : public DataManager {
public:
    static OuterT & _cast(dr_dm_manage_t data_manage) {
        return static_cast<OuterT&>(DataManager::_cast(data_manage));
    }

    static OuterT & instance(gd_app_context_t app, const char * name = OuterT::NAME) { 
        return static_cast<OuterT&>(DataManager::instance(app, name));
    }
};

}}

#endif
