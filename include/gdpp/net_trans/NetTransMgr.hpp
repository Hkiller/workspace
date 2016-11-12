#ifndef GDPP_NETTRANS_EVENTCENTER_H
#define GDPP_NETTRANS_EVENTCENTER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "gdpp/app/Application.hpp"
#include "gd/net_trans/net_trans_manage.h"
#include "System.hpp"
#include "NetTransProcessor.hpp"
#include "NetTransTaskBuilder.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
# pragma warning(disable:4407)
#endif

namespace Gd { namespace NetTrans {

class NetTransMgr : public Cpe::Utils::SimulateObject {
public:
    operator net_trans_manage_t (void) const { return (net_trans_manage_t)this; }

    const char * name(void) const { return net_trans_manage_name(*this); }
    cpe_hash_string_t name_hs(void) const { return net_trans_manage_name_hs(*this); }

    App::Application & app(void) { return App::Application::_cast(net_trans_manage_app(*this)); }
    App::Application const & app(void) const { return App::Application::_cast(net_trans_manage_app(*this)); }

    NetTransTaskBuilder createTask(const char * group_name, size_t carry_data_capacity = 0, void const * carry_data = NULL);
    NetTransTaskBuilder createTask(net_trans_group_t group, size_t carry_data_capacity = 0, void const * carry_data = NULL);

    template<typename T>
    NetTransTaskBuilder createTask(const char * group_name, T const & data) {
        return createTask(group_name, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }
    
    template<typename T>
    NetTransTaskBuilder createTask(net_trans_group_t group, T const & data) {
        return createTask(group, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }
    
    void removeTasksByCtx(void * ctx) { net_trans_mgr_remove_tasks_by_commit_ctx(*this, ctx); }
    void removeTasksByOp(net_trans_task_commit_op_t op, void * ctx) { net_trans_mgr_remove_tasks_by_commit_op(*this, op, ctx); }
    
    static NetTransMgr & instance(gd_app_context_t app, const char * name = NULL);
    static NetTransMgr & _cast(net_trans_manage_t mgr);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
