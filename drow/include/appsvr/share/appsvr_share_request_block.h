#ifndef APPSVR_SHARE_REQUEST_BLOCK_H
#define APPSVR_SHARE_REQUEST_BLOCK_H
#include "appsvr_share_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_share_request_block_it {
    appsvr_share_request_block_t (*next)(struct appsvr_share_request_block_it * it);
    char m_data[64];
};
    
appsvr_share_request_block_t
appsvr_share_request_block_create(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category,
    void const * data, size_t data_size);

void appsvr_share_request_block_free(appsvr_share_request_block_t request_block);

appsvr_share_request_block_category_t appsvr_share_request_block_category(appsvr_share_request_block_t request_block);
void * appsvr_share_request_block_data(appsvr_share_request_block_t request_block);
size_t appsvr_share_request_block_data_size(appsvr_share_request_block_t request_block);

void appsvr_share_request_blocks(
    appsvr_share_request_t request,
    appsvr_share_request_block_it_t block_it);
    
void appsvr_share_request_blocks_in_category(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category,
    appsvr_share_request_block_it_t block_it);    

appsvr_share_request_block_t
appsvr_share_request_block_find_first(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category);

const char * appsvr_share_request_block_get_str(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category, uint8_t index, const char * dft);
    
#ifdef __cplusplus
}
#endif

#endif
