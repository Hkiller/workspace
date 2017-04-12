#ifndef CPE_UTILS_RWPIPE_H
#define CPE_UTILS_RWPIPE_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RWPIPE_ERR_NO_DATA 1
#define RWPIPE_ERR_SEND_DATA_OVERFLOW 2
#define RWPIPE_ERR_RECV_BUF_TO_SMALL 3
#define RWPIPE_ERR_BAD_DATA 4

typedef struct rwpipe * rwpipe_t;

int rwpipe_send(rwpipe_t q, void const * buf, uint32_t len);
int rwpipe_recv(rwpipe_t q, void * buf, uint32_t * len);

uint32_t rwpipe_total_capacity(rwpipe_t q);
uint32_t rwpipe_data_capacity(rwpipe_t q);
uint32_t rwpipe_data_size(rwpipe_t q);
uint32_t rwpipe_data_left_size(rwpipe_t q);

rwpipe_t rwpipe_init(void * buf, uint32_t capacity);
rwpipe_t rwpipe_attach(void * buf);

#ifdef __cplusplus
}
#endif

#endif
