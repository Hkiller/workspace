#ifndef CPE_NET_CHANEL_H
#define CPE_NET_CHANEL_H
#include "net_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*chanel basic operations*/
net_chanel_type_id_t net_chanel_type_id(net_chanel_t chanel);
const char * net_chanel_type_name(net_chanel_t chanel);
size_t net_chanel_data_size(net_chanel_t chanel);
net_chanel_state_t net_chanel_state(net_chanel_t chanel);

void net_chanel_free(net_chanel_t chanel);

/*queue chanel operations*/
net_chanel_t
net_chanel_queue_create(net_mgr_t nmgr, void * data, size_t capacity);

size_t net_chanel_queue_capacity(net_chanel_t chanel);
void * net_chanel_queue_buf(net_chanel_t chanel);

void net_chanel_queue_set_close(
    net_chanel_t chanel,
    void (*destory_fun)(net_chanel_t chanel, void * ctx),
    void * destory_ctx);

#ifdef __cplusplus
}
#endif

#endif
