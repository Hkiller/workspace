#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_chanel.h"
#include "net_internal_types.h"

net_chanel_type_id_t
net_chanel_type_id(net_chanel_t chanel) {
    return chanel->m_type->id;
}

const char * net_chanel_type_name(net_chanel_t chanel) {
    return chanel->m_type->name;
}

void net_chanel_free(net_chanel_t chanel) {
    assert(chanel);
    chanel->m_type->destory(chanel);
}

size_t net_chanel_data_size(net_chanel_t chanel) {
    assert(chanel);
    return chanel->m_type->data_size(chanel);
}

net_chanel_state_t net_chanel_state(net_chanel_t chanel);
