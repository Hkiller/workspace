#ifndef GD_NET_TRANS_GROUP_I_H
#define GD_NET_TRANS_GROUP_I_H
#include "gd/net_trans/net_trans_group.h"
#include "net_trans_manage_i.h"

struct net_trans_group {
    net_trans_manage_t m_mgr;
    const char * m_name;
    uint64_t m_connect_timeout_ms;
    uint64_t m_transfer_timeout_ms;
    int m_forbid_reuse;
    net_trans_task_list_t m_tasks;

    struct cpe_hash_entry m_hh_for_mgr;
};

uint32_t net_trans_group_hash(net_trans_group_t group);
int net_trans_group_eq(net_trans_group_t l, net_trans_group_t r);
void net_trans_group_free_all(net_trans_manage_t mgr);

#endif


