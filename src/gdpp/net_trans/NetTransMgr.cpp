#include "cpepp/utils/ErrorCollector.hpp"
#include "gdpp/app/Log.hpp"
#include "gd/net_trans/net_trans_group.h"
#include "gdpp/net_trans/NetTransMgr.hpp"

namespace Gd { namespace NetTrans {

NetTransMgr & NetTransMgr::_cast(net_trans_manage_t mgr) {
    if (mgr == NULL) {
        throw ::std::runtime_error("Gd::Evt::NetTransMgr::_cast: input mgr is NULL!");
    }

    return *(NetTransMgr*)mgr;
}

NetTransMgr & NetTransMgr::instance(gd_app_context_t app, const char * name) {
    net_trans_manage_t mgr = net_trans_manage_find_nc(app, name);
    if (mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "net_trans_manage %s not exist!", name ? name : "default");
    }

    return *(NetTransMgr*)mgr;
}

NetTransTaskBuilder
NetTransMgr::createTask(const char * group_name, size_t carry_data_capacity, void const * carry_data) {
    net_trans_group_t group = net_trans_group_find(*this, group_name);
    if (group == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "net_trans_manage %s group %s not exist!", name(), group_name);
    }

    net_trans_task_t task = net_trans_task_create(group, carry_data_capacity);
    if (task == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "net_trans_manage %s create task in group %s fail!", name(), group_name);
    }

    if (carry_data_capacity && carry_data) {
        memcpy(net_trans_task_data(task), carry_data, carry_data_capacity);
    }
    
    return NetTransTaskBuilder(task);
}

NetTransTaskBuilder
NetTransMgr::createTask(net_trans_group_t group, size_t carry_data_capacity, void const * carry_data) {
    net_trans_task_t task = net_trans_task_create(group, carry_data_capacity);
    if (task == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "net_trans_manage %s create task in group %s fail!", name(), net_trans_group_name(group));
    }

    if (carry_data_capacity && carry_data) {
        memcpy(net_trans_task_data(task), carry_data, carry_data_capacity);
    }
    
    return NetTransTaskBuilder(task);
}

}}


