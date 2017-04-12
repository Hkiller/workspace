#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "net_internal_ops.h"

static struct net_chanel_type s_net_chanel_type_queue;

net_chanel_t
net_chanel_queue_create(net_mgr_t nmgr, void * data, size_t capacity) {
    struct net_chanel_queue * chanel;

    assert(nmgr);

    if (data == NULL || capacity <= 0) return NULL;

    chanel = mem_alloc(nmgr->m_alloc, sizeof(struct net_chanel_queue));
    if (chanel == NULL) return NULL;

    chanel->m_mgr = nmgr;
    chanel->m_type = &s_net_chanel_type_queue;
    chanel->m_state = net_chanel_state_empty;
    chanel->m_buf = data;
    chanel->m_size = 0;
    chanel->m_capacity = capacity;
    chanel->m_destory_fun = NULL;
    chanel->m_destory_ctx = NULL;

    TAILQ_INSERT_TAIL(&nmgr->m_chanels, (net_chanel_t)chanel, m_next);

    return (net_chanel_t)chanel;
}

size_t net_chanel_queue_capacity(net_chanel_t chanel) {
    assert(chanel);

    if (chanel->m_type != &s_net_chanel_type_queue) return 0;

    return ((struct net_chanel_queue *)chanel)->m_capacity;
}

void * net_chanel_queue_buf(net_chanel_t chanel) {
    assert(chanel);

    if (chanel->m_type != &s_net_chanel_type_queue) return NULL;

    return ((struct net_chanel_queue *)chanel)->m_buf;
}

void net_chanel_queue_set_close(
    net_chanel_t bc,
    void (*destory_fun)(net_chanel_t chanel, void * ctx),
    void * destory_ctx)
{
    struct net_chanel_queue * chanel;

    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    chanel->m_destory_fun = destory_fun;
    chanel->m_destory_ctx = destory_ctx;
}

static size_t net_chanel_queue_data_size(net_chanel_t bc) {
    struct net_chanel_queue * chanel;

    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    return chanel->m_size;
}

static void net_chanel_queue_destory(net_chanel_t bc) {
    struct net_chanel_queue * chanel;

    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    if (chanel->m_destory_fun) {
        chanel->m_destory_fun(bc, chanel->m_destory_ctx);
    }

    TAILQ_REMOVE(&chanel->m_mgr->m_chanels, chanel, m_next);
    mem_free(chanel->m_mgr->m_alloc, chanel); 
}

#define net_chanel_queue_calac_state(chanel) \
    ( chanel->m_size == 0                    \
        ? net_chanel_state_empty             \
        : ( chanel->m_size == chanel->m_capacity    \
            ? net_chanel_state_full                 \
            : net_chanel_state_have_data) )

static int net_chanel_queue_read_from_buf(net_chanel_t bc, const void * buf, size_t size) {
    struct net_chanel_queue * chanel;

    assert(bc);
    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    if (size > chanel->m_capacity - chanel->m_size) return -1;

    memcpy(chanel->m_buf + chanel->m_size, buf, size);
    chanel->m_size += size;
    chanel->m_state = net_chanel_queue_calac_state(chanel);

    return 0;
}

static ssize_t net_chanel_queue_write_to_buf(net_chanel_t bc, void * buf, size_t capacity) {
    struct net_chanel_queue * chanel;
    size_t size;

    assert(bc);
    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    size = capacity;
    if (size > chanel->m_size) size = chanel->m_size;

    memcpy(buf, chanel->m_buf, size);
    chanel->m_size -= size;
    if (chanel->m_size) {
        memmove(buf, (char *)buf + size, chanel->m_size);
    }

    chanel->m_state = net_chanel_queue_calac_state(chanel);

    return size;
}

static ssize_t net_chanel_queue_read_from_net(net_chanel_t bc, int fd) {
    struct net_chanel_queue * chanel;
    ssize_t recv_size;

    assert(bc);
    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    recv_size = cpe_recv(fd, chanel->m_buf + chanel->m_size, chanel->m_capacity - chanel->m_size, 0);
    if (recv_size < 0) return -1;

    chanel->m_size += recv_size;
    chanel->m_state = net_chanel_queue_calac_state(chanel);

    return recv_size;
}

static ssize_t net_chanel_queue_write_to_net(net_chanel_t bc, int fd) {
    struct net_chanel_queue * chanel;
    ssize_t send_size;

    assert(bc);
    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;
    if (chanel->m_size == 0) return 0;

    send_size = cpe_send(fd, chanel->m_buf, chanel->m_size, 0);
    if (send_size < 0) return send_size;

    if (send_size != chanel->m_size) {
        memmove(chanel->m_buf, chanel->m_buf + send_size, chanel->m_size - send_size);
        chanel->m_size -= send_size;
    }
    else {
        chanel->m_size = 0;
    }

    chanel->m_state = net_chanel_queue_calac_state(chanel);

    return send_size;
}

static void * net_chanel_queue_peek(net_chanel_t bc, void * buf, size_t size) {
    struct net_chanel_queue * chanel;

    assert(bc);
    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    if (chanel->m_size < size) return NULL;

    return chanel->m_buf;
}

static void net_chanel_queue_erase(net_chanel_t bc, size_t size) {
    struct net_chanel_queue * chanel;

    assert(bc);
    assert(bc->m_type == &s_net_chanel_type_queue);

    chanel = (struct net_chanel_queue *)bc;

    if (size > chanel->m_size) size = chanel->m_size;

    if (size != chanel->m_size) {
        memmove(chanel->m_buf, chanel->m_buf + size, chanel->m_size - size);
        chanel->m_size -= size;
    }
    else {
        chanel->m_size = 0;
    }

    chanel->m_state = net_chanel_queue_calac_state(chanel);
}

static struct net_chanel_type s_net_chanel_type_queue = {
    "queue"
    , net_chanel_type_id_queue
    , net_chanel_queue_read_from_net
    , net_chanel_queue_write_to_net
    , net_chanel_queue_read_from_buf
    , net_chanel_queue_write_to_buf
    , net_chanel_queue_peek
    , net_chanel_queue_erase
    , net_chanel_queue_data_size
    , net_chanel_queue_destory
};
