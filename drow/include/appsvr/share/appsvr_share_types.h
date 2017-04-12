#ifndef APPSVR_SHARE_TYPES_H
#define APPSVR_SHARE_TYPES_H
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum appsvr_share_request_block_category {
    appsvr_share_request_block_title,
    appsvr_share_request_block_content,
    appsvr_share_request_block_remote_picture,
    appsvr_share_request_block_navigation,
} appsvr_share_request_block_category_t;
    
typedef struct appsvr_share_module * appsvr_share_module_t;
typedef struct appsvr_share_adapter * appsvr_share_adapter_t;
typedef struct appsvr_share_adapter_it * appsvr_share_adapter_it_t;
typedef struct appsvr_share_request * appsvr_share_request_t;
typedef struct appsvr_share_request_block * appsvr_share_request_block_t;
typedef struct appsvr_share_request_block_it * appsvr_share_request_block_it_t;

#ifdef __cplusplus
}
#endif

#endif
