#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/net/net_endpoint.h"
#include "net_internal_ops.h"

static void net_ep_pages_inc(net_mgr_t nmgr) {
    int next_ep_page_capacity;
    struct net_ep_page ** next_ep_pages;
    size_t next_buf_size;

    next_ep_page_capacity =
        (int)(nmgr->m_ep_page_capacity < 16 ? 16 : nmgr->m_ep_page_capacity << 1);

    next_buf_size = sizeof(struct net_ep_page *) * next_ep_page_capacity;

    next_ep_pages = mem_alloc(nmgr->m_alloc, next_buf_size);
    if (next_ep_pages == NULL) return;
    bzero(next_ep_pages, next_buf_size);

    if (cpe_range_put_range(
            &nmgr->m_ep_ids,
            nmgr->m_ep_page_capacity * GD_NET_EP_COUNT_PER_PAGE,
            next_ep_page_capacity * GD_NET_EP_COUNT_PER_PAGE)
        != 0)
    {
        mem_free(nmgr->m_alloc, next_ep_pages);
        return;
    }

    if (nmgr->m_ep_pages) {
        memcpy(next_ep_pages, nmgr->m_ep_pages, sizeof(struct net_ep_page *) * nmgr->m_ep_page_capacity);
        mem_free(nmgr->m_alloc, nmgr->m_ep_pages);
    }

    nmgr->m_ep_pages = next_ep_pages;
    nmgr->m_ep_page_capacity = next_ep_page_capacity;
    return;
}

net_ep_t
net_ep_pages_alloc_ep(net_mgr_t nmgr) {
    ptr_int_t epPos;
    size_t pagePos;
    size_t posInPage;
    struct net_ep_page * page;
    net_ep_t ep;

    assert(nmgr);

    epPos = cpe_range_get_one(&nmgr->m_ep_ids);
    if (epPos < 0) {
        net_ep_pages_inc(nmgr);
        epPos = cpe_range_get_one(&nmgr->m_ep_ids);
    }

    if (epPos < 0) return NULL;

    pagePos = ((size_t)epPos) / GD_NET_EP_COUNT_PER_PAGE;
    posInPage = ((size_t)epPos) % GD_NET_EP_COUNT_PER_PAGE;

    assert(pagePos < nmgr->m_ep_page_capacity);

    page = nmgr->m_ep_pages[pagePos];
    if (page == NULL) {
        int i;
        page = mem_alloc(nmgr->m_alloc, sizeof(struct net_ep_page));
        if (page == NULL) {
            cpe_range_put_one(&nmgr->m_ep_ids, epPos);
            return NULL;
        }

        for(i = 0; i < GD_NET_EP_COUNT_PER_PAGE; ++i) {
            page->m_eps[i].m_id = GD_NET_EP_INVALID_ID;
        }

        nmgr->m_ep_pages[pagePos] = page;
    }

    ep = &page->m_eps[posInPage];
    ep->m_id = (net_ep_id_t)epPos;
    ep->m_mgr = nmgr;
    return ep;
}

void net_ep_pages_free_ep(net_ep_t ep) {
    assert(ep);
    assert(ep->m_id != GD_NET_EP_INVALID_ID);

    cpe_range_put_one(&ep->m_mgr->m_ep_ids, (ptr_int_t)ep->m_id);
    ep->m_id = GD_NET_EP_INVALID_ID;
}

void net_ep_pages_free(net_mgr_t nmgr) {
    size_t pagePos;
    size_t epPos;
    assert(nmgr);

    if (nmgr->m_ep_pages == NULL) return;

    for(pagePos = 0; pagePos < nmgr->m_ep_page_capacity; ++pagePos) {
        if (nmgr->m_ep_pages[pagePos] == NULL) continue;

        for(epPos = 0; epPos < GD_NET_EP_COUNT_PER_PAGE; ++epPos) {
            net_ep_t ep = &nmgr->m_ep_pages[pagePos]->m_eps[epPos];

            if (ep->m_id != GD_NET_EP_INVALID_ID) {
                net_ep_free(ep);
            }
        }

        mem_free(nmgr->m_alloc, nmgr->m_ep_pages[pagePos]);
        nmgr->m_ep_pages[pagePos] = NULL;
    }

    mem_free(nmgr->m_alloc, nmgr->m_ep_pages);
    nmgr->m_ep_pages = NULL;
    nmgr->m_ep_page_capacity = 0;
}

net_ep_t net_ep_find(net_mgr_t nmgr, net_ep_id_t id) {
    size_t pagePos;
    size_t posInPage;
    struct net_ep_page * page;
    net_ep_t ep;

    pagePos = ((int)id) / GD_NET_EP_COUNT_PER_PAGE;
    posInPage = ((int)id) % GD_NET_EP_COUNT_PER_PAGE;

    if (pagePos >= nmgr->m_ep_page_capacity) return NULL;

    page = nmgr->m_ep_pages[pagePos];
    if (page == NULL) return NULL;

    ep = &page->m_eps[posInPage];
    if (ep->m_id == GD_NET_EP_INVALID_ID) return NULL;

    assert(ep->m_id == id);
    return ep;
}
