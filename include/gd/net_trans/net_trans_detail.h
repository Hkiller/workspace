#ifndef GD_NET_TRANS_DETAIL_H
#define GD_NET_TRANS_DETAIL_H
#include "curl/curl.h"
#include "net_trans_types.h"

#ifdef __cplusplus
extern "C" {
#endif

CURLM * net_trans_manage_handler(net_trans_manage_t mgr);
CURL * net_trans_task_handler(net_trans_task_t task);

#ifdef __cplusplus
}
#endif

#endif
