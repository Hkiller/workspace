#ifndef GDPP_NETTRANS_PROCESSOR_BASE_H
#define GDPP_NETTRANS_PROCESSOR_BASE_H
#include "NetTransProcessor.hpp"
#include "NetTransTaskBuilder.hpp"
#include "NetTransMgr.hpp"

namespace Gd { namespace NetTrans {

class NetTransProcessorBase : public NetTransProcessor {
public:
	NetTransProcessorBase(NetTransMgr & eventMgr);
	~NetTransProcessorBase();

    NetTransTaskBuilder
    createNetTransTask(size_t carry_data_capacity = 0, void const * carry_data = NULL) {
        return m_netTransMgr.createTask(m_group, carry_data_capacity, carry_data);
    }

    template<typename T>
    NetTransTaskBuilder createNetTransTask(T const & carry_data) {
        return m_netTransMgr.createTask(m_group, carry_data);
    }
    
private:
	NetTransMgr & m_netTransMgr;
    net_trans_group_t m_group;
};

}}

#endif
