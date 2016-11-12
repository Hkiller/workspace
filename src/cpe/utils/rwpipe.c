#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/rwpipe.h"

struct rwpipe {
    uint32_t magic;
    uint32_t capacity;
    uint32_t r_p;
    uint32_t w_p;
};

rwpipe_t rwpipe_init(void * buf, uint32_t capacity) {
    rwpipe_t head = (rwpipe_t)buf;

    if (capacity < sizeof(struct rwpipe)) return NULL;

    head->magic = 0x45678532;
    head->capacity = (uint32_t)capacity;
    head->r_p = 0;
    head->w_p = 0;

    return head;
}

rwpipe_t rwpipe_attach(void * buf) {
    rwpipe_t head = (rwpipe_t)buf;
    uint32_t data_capacity;

    if (head->magic != 0x45678532) return NULL;
    if (head->capacity < sizeof(struct rwpipe)) return NULL;

    data_capacity = head->capacity - sizeof(struct rwpipe);

    if (head->w_p > data_capacity || head->r_p > data_capacity) return NULL;

    return head;
}

uint32_t rwpipe_total_capacity(rwpipe_t q) {
    return q->capacity;
}

uint32_t rwpipe_data_capacity(rwpipe_t q) {
    return q->capacity - sizeof(struct rwpipe);
}

uint32_t rwpipe_data_size(rwpipe_t q) {
    if (q->r_p <= q->w_p) {
        return q->w_p - q->r_p;
    }
    else {
        uint32_t data_capacity = q->capacity - sizeof(struct rwpipe);
        return data_capacity - q->r_p + q->w_p;
    }
}

uint32_t rwpipe_data_left_size(rwpipe_t q) {
    if (q->w_p < q->r_p) {
        return q->r_p - q->w_p - 1;
    }
    else {
        uint32_t data_capacity = q->capacity - sizeof(struct rwpipe);
        return data_capacity - q->w_p + q->r_p - 1;
    }
}

int rwpipe_send(rwpipe_t q, void const * buf, uint32_t len) {
    char * buf_begin;
    uint32_t capacity;

    assert(q);

    buf_begin = (char *)(q + 1);

    capacity = q->capacity - sizeof(struct rwpipe);

    if (q->w_p >= q->r_p) {
        uint32_t len_1 = capacity - q->w_p;
        uint32_t len_2 = q->r_p;

        if ((len_1 + len_2) <= (len + 4)) return RWPIPE_ERR_SEND_DATA_OVERFLOW;

        if (len_1 >= 4 + len) {
            memcpy(buf_begin + q->w_p, &len, 4);
            memcpy(buf_begin + q->w_p + 4, buf, len);
            q->w_p += 4 + len;
        }
        else if (len_1 >= 4) {
            uint32_t copy_len_1 = len_1 - 4;
            memcpy(buf_begin + q->w_p, &len, 4);
            memcpy(buf_begin + q->w_p + 4, buf, copy_len_1);
            memcpy(buf_begin, ((const char *)buf) + copy_len_1, len - copy_len_1);
            q->w_p = len - copy_len_1;
        }
        else {
            uint32_t copy_len_2 = 4 - len_1;
            memcpy(buf_begin + q->w_p, &len, len_1);
            memcpy(buf_begin, ((const char *)&len) + len_1, copy_len_2);
            memcpy(buf_begin + copy_len_2, buf, len);
            q->w_p = copy_len_2 + len;
        }
    }
    else {
        if ((q->r_p - q->w_p) <= (len + 4)) return RWPIPE_ERR_SEND_DATA_OVERFLOW;

        memcpy(buf_begin + q->w_p, &len, 4);
        memcpy(buf_begin + q->w_p + 4, buf, len);
        q->w_p += 4 + len;
    }

    assert(q->w_p != q->r_p);
    return 0;
}

int rwpipe_recv(rwpipe_t q, void * buf, uint32_t * len) {
    char * buf_begin;
    uint32_t capacity;

    assert(q);
    assert(q);

    buf_begin = (char *)(q + 1);

    capacity = q->capacity - sizeof(struct rwpipe);

    if (q->r_p == q->w_p) {
        return RWPIPE_ERR_NO_DATA;
    }
    else if (q->r_p < q->w_p) {
        uint32_t pkg_len;
        uint32_t buf_len = q->w_p - q->r_p;
        if (buf_len < 4) return RWPIPE_ERR_BAD_DATA;
        memcpy(&pkg_len, buf_begin + q->r_p, 4);

        if ((pkg_len + 4) > buf_len) return RWPIPE_ERR_BAD_DATA;
        if (pkg_len > *len) return RWPIPE_ERR_RECV_BUF_TO_SMALL;

        memcpy(buf, buf_begin + q->r_p + 4, pkg_len);
        *len = pkg_len;
        q->r_p += (4 + pkg_len);

        assert(q->r_p <= q->w_p);
    }
    else {
        uint32_t len_1 = capacity - q->r_p;
        uint32_t len_2 = q->w_p;
        uint32_t pkg_len;

        if ((len_1 + len_2) < 4) return RWPIPE_ERR_BAD_DATA;

        if (len_1 < 4) {
            memcpy(&pkg_len, buf_begin + q->r_p, len_1);
            memcpy(((char *)&pkg_len) + len_1, buf_begin, 4 - len_1);

            if ((len_1 + len_2) < (pkg_len + 4)) return RWPIPE_ERR_BAD_DATA;
            if (pkg_len > *len) return RWPIPE_ERR_RECV_BUF_TO_SMALL;

            memcpy(buf, buf_begin + (4 - len_1), pkg_len);
            *len = pkg_len;
            q->r_p = (4 - len_1) + pkg_len;

            assert(q->r_p <= q->w_p);
        }
        else {
            memcpy(&pkg_len, buf_begin + q->r_p, 4);
            
            if ((len_1 + len_2) < (pkg_len + 4)) return RWPIPE_ERR_BAD_DATA;
            if (pkg_len > *len) return RWPIPE_ERR_RECV_BUF_TO_SMALL;

            if (len_1 < (pkg_len + 4)) {
                uint32_t copy_len_1 = len_1 - 4;
                memcpy(buf, buf_begin + q->r_p + 4, copy_len_1);
                memcpy(((char *)buf) + copy_len_1, buf_begin, pkg_len - copy_len_1);
                *len = pkg_len;
                q->r_p = pkg_len - copy_len_1;

                assert(q->r_p <= q->w_p);
            }
            else {
                memcpy(buf, buf_begin + q->r_p + 4, pkg_len);
                *len = pkg_len;
                q->r_p += pkg_len;
            }
        }
    }

    return 0;
}

