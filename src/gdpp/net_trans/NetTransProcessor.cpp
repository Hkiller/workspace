#include "cpepp/utils/ErrorCollector.hpp"
#include "gdpp/app/Log.hpp"
#include "gd/net_trans/net_trans_group.h"
#include "gdpp/net_trans/NetTransProcessorBase.hpp"
#include "gdpp/net_trans/NetTransMgr.hpp"

namespace Gd { namespace NetTrans {

NetTransProcessor::~NetTransProcessor() {
}

NetTransProcessorBase::NetTransProcessorBase(NetTransMgr & eventMgr)
    : m_netTransMgr(eventMgr)
    , m_group(NULL)
{
    m_group = net_trans_group_create(m_netTransMgr, NULL);
    if (m_group == NULL) {
        APP_CTX_THROW_EXCEPTION(m_netTransMgr.app(), ::std::runtime_error, "net_trans_processor create group fail!");
    }
}

NetTransProcessorBase::~NetTransProcessorBase() {
    net_trans_group_free(m_group);
    assert(m_group);
}

}}

