#ifndef APPSVR_SHARE_REQUEST_BLOCK_I_H
#define APPSVR_SHARE_REQUEST_BLOCK_I_H
#include "appsvr/share/appsvr_share_request_block.h"
#include "appsvr_share_request_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_share_request_block {
    appsvr_share_request_t m_request;
    TAILQ_ENTRY(appsvr_share_request_block) m_next;
    appsvr_share_request_block_category_t m_category;
    void * m_data;
    size_t m_data_size;
};

void appsvr_share_request_block_real_free(appsvr_share_request_block_t block);
    
#ifdef __cplusplus
}
#endif

#endif
