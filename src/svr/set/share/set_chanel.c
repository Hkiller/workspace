#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_pbuf.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/share/set_chanel.h"
#include "protocol/svr/set/set_share_chanel.h"
#include "protocol/svr/set/set_share_pkg.h"

//#define SET_CHANEL_DEBUG
#define SET_SHARE_SAVE_HEAD_SIZE (sizeof(uint32_t) + sizeof(SET_PKG_HEAD) + 1) /*长度计数以及一个协议头*/
#define SET_CHANTE_FLAGS_PEAKED ((uint32_t)0x00000001)

static void set_chanel_pipe_save_ignore_pkg(void * buf, size_t capacity);
static int set_chanel_pipe_save_pkg(dp_req_t body, dp_req_t head, dp_req_t carry, void * buf, size_t capacity, error_monitor_t em);
static int set_chanel_pipe_load_pkg(dp_req_t body, dp_req_t head, dp_req_t carry, void * buf, size_t capacity, error_monitor_t em);
static int set_chanel_pipe_write(SVR_SET_CHANEL * chanel, SVR_SET_PIPE * pipe, dp_req_t body, size_t * size, error_monitor_t em);
static int set_chanel_pipe_peak(SVR_SET_CHANEL * chanel, SVR_SET_PIPE * pipe, dp_req_t body, error_monitor_t em);
static int set_chanel_pipe_erase(SVR_SET_CHANEL * chanel, SVR_SET_PIPE * pipe, error_monitor_t em);

int set_chanel_r_write(set_chanel_t input_chanel, dp_req_t body, size_t * size, error_monitor_t em) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    return set_chanel_pipe_write(chanel, &chanel->r, body, size, em);
}

int set_chanel_r_peak(set_chanel_t input_chanel, dp_req_t body, error_monitor_t em) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    int r = set_chanel_pipe_peak(chanel, &chanel->r, body, em);
    if (r == 0) chanel->r.flags |= SET_CHANTE_FLAGS_PEAKED;
    return r;
}

int set_chanel_r_is_peaked(set_chanel_t input_chanel) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    return chanel->r.flags & SET_CHANTE_FLAGS_PEAKED;
}

int set_chanel_r_erase(set_chanel_t input_chanel, error_monitor_t em) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    int r = set_chanel_pipe_erase(chanel, &chanel->r, em);
    if (r == 0) chanel->r.flags &= ~SET_CHANTE_FLAGS_PEAKED;
    return r;
}

int set_chanel_w_write(set_chanel_t input_chanel, dp_req_t body, size_t * size, error_monitor_t em) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    return set_chanel_pipe_write(chanel, &chanel->w, body, size, em);
}

int set_chanel_w_peak(set_chanel_t input_chanel, dp_req_t body, error_monitor_t em) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    int r = set_chanel_pipe_peak(chanel, &chanel->w, body, em);
    if (r == 0) chanel->w.flags |= SET_CHANTE_FLAGS_PEAKED; 
    return r;
}

int set_chanel_w_is_peaked(set_chanel_t input_chanel) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    return chanel->w.flags & SET_CHANTE_FLAGS_PEAKED;
}

int set_chanel_w_erase(set_chanel_t input_chanel, error_monitor_t em) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    int r = set_chanel_pipe_erase(chanel, &chanel->w, em);
    if (r == 0) chanel->w.flags &= ~SET_CHANTE_FLAGS_PEAKED;
    return r;
}

static int set_chanel_pipe_erase(SVR_SET_CHANEL * chanel, SVR_SET_PIPE * pipe, error_monitor_t em) {
    char * buf = ((char *)chanel) + pipe->begin;
    SET_PKG_HEAD * head;
    uint32_t total_size;
    uint32_t capacity;
    uint32_t wp = pipe->wp;

#ifdef SET_CHANEL_DEBUG 
    CPE_INFO(em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: erase begin!", wp, pipe->rp, pipe->capacity);
#endif

tag_TRY_AGAIN:
    if (wp == pipe->rp) return set_chanel_error_chanel_empty;

    if (wp > pipe->rp) {
        capacity = wp - pipe->rp;

        if (capacity < SET_SHARE_SAVE_HEAD_SIZE) {
            CPE_ERROR(
                em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: erase last small pkg(size=%d)",
                wp, pipe->rp, pipe->capacity, capacity);
            pipe->rp = wp;
            return set_chanel_error_chanel_empty;
        }

        total_size = *((uint32_t*)(buf + pipe->rp));

        if (capacity < total_size) {
            CPE_ERROR(
                em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: found bad pkg (pkg-size=%d, buf-size=%d), clear chanel",
                wp, pipe->rp, pipe->capacity, total_size, capacity);
            pipe->rp = wp;
            return set_chanel_error_chanel_empty;
        }

        if (total_size < SET_SHARE_SAVE_HEAD_SIZE) {
            CPE_ERROR(
                em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: found small pkg (pkg-size=%d, min-size=%d, buf-size=%d), erase pkg",
                wp, pipe->rp, pipe->capacity, total_size, (int)SET_SHARE_SAVE_HEAD_SIZE, capacity);
            pipe->rp += total_size;
            if (pipe->rp == pipe->capacity) pipe->rp = 0;
            goto tag_TRY_AGAIN;
        }

        head = (SET_PKG_HEAD *)(buf + pipe->rp + sizeof(uint32_t));
        if (head->to_svr_id == 0 && head->to_svr_type == 0) {
            pipe->rp += total_size;
            if (pipe->rp == pipe->capacity) pipe->rp = 0;
            goto tag_TRY_AGAIN;
        }

        pipe->rp += total_size;
        if (pipe->rp == pipe->capacity) pipe->rp = 0;

#ifdef SET_CHANEL_DEBUG 
        CPE_INFO(em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: erase success, size=%d!", wp, pipe->rp, pipe->capacity, total_size);
#endif

        return 0;
    }
    else {
        assert(wp < pipe->rp);
        assert(pipe->rp < pipe->capacity);

        capacity = pipe->capacity - pipe->rp;
        if (capacity < SET_SHARE_SAVE_HEAD_SIZE) {
#ifdef SET_CHANEL_DEBUG
            CPE_INFO(
                em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: erase last small pkg(size=%d)",
                wp, pipe->rp, pipe->capacity, capacity);
#endif
            pipe->rp = 0;
            goto tag_TRY_AGAIN;
        }

        total_size = *((uint32_t*)(buf + pipe->rp));
        if (capacity < total_size) {
            CPE_ERROR(
                em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: found bad pkg (pkg-size=%d, buf-size=%d), clear chanel",
                wp, pipe->rp, pipe->capacity, total_size, capacity);
            pipe->rp = 0;
            goto tag_TRY_AGAIN;
        }

        if (total_size < SET_SHARE_SAVE_HEAD_SIZE) {
            CPE_ERROR(
                em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: found small pkg (pkg-size=%d, min-size=%d, buf-size=%d), erase pkg",
                wp, pipe->rp, pipe->capacity, total_size, (int)SET_SHARE_SAVE_HEAD_SIZE, capacity);
            pipe->rp += total_size;
            if (pipe->rp == pipe->capacity) pipe->rp = 0;
            goto tag_TRY_AGAIN;
        }

        head = (SET_PKG_HEAD *)(buf + pipe->rp + sizeof(uint32_t));
        if (head->to_svr_id == 0 && head->to_svr_type == 0) {
            pipe->rp += total_size;
            if (pipe->rp == pipe->capacity) pipe->rp = 0;
            goto tag_TRY_AGAIN;
        }

        pipe->rp += total_size;
        if (pipe->rp == pipe->capacity) pipe->rp = 0;

#ifdef SET_CHANEL_DEBUG 
        CPE_INFO(em, "set_chanel_pipe_erase: wp=%d, rp=%d, capacity=%d: erase success, size=%d!", wp, pipe->rp, pipe->capacity, total_size);
#endif

        return 0;
    }
}

static int set_chanel_pipe_peak(SVR_SET_CHANEL * chanel, SVR_SET_PIPE * pipe, dp_req_t body, error_monitor_t em) {
    dp_req_t head = set_pkg_head_check_create(body);
    dp_req_t carry = set_pkg_carry_check_create(body, 0);
    char * buf = ((char *)chanel) + pipe->begin;
    int rv;
    SET_PKG_HEAD * head_buf;
    uint32_t wp = pipe->wp;

    assert(head);
    if (head == NULL) {
        CPE_ERROR(
            em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: error %s!",
            wp, pipe->rp, pipe->capacity, set_chanel_str_error(set_chanel_error_no_memory));
        return set_chanel_error_no_memory;
    }

PIPE_PEAK_TRY_AGAIN:
    if (wp == pipe->rp) return set_chanel_error_chanel_empty;

    if (wp > pipe->rp) {
        rv = set_chanel_pipe_load_pkg(body, head, carry, buf + pipe->rp, wp - pipe->rp, em);
        switch(rv) {
        case 0:
            break;
        case set_chanel_error_bad_data:
            CPE_ERROR(
                em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: error %s!",
                wp, pipe->rp, pipe->capacity, set_chanel_str_error(rv));
            pipe->rp = wp;
            return rv;
        default:
            CPE_ERROR(
                em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: error %s!",
                wp, pipe->rp, pipe->capacity, set_chanel_str_error(rv));
            return rv;
        }
    }
    else {
        uint32_t capacity;

        assert(wp < pipe->rp);

        capacity = pipe->capacity - pipe->rp;
        if (capacity < SET_SHARE_SAVE_HEAD_SIZE) {
#ifdef SET_CHANEL_DEBUG 
            CPE_INFO(em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: ignore last small buf!", wp, pipe->rp, pipe->capacity);
#endif
            pipe->rp = 0;
            goto PIPE_PEAK_TRY_AGAIN;
        }

        rv = set_chanel_pipe_load_pkg(body, head, carry, buf + pipe->rp, capacity, em);
        switch(rv) {
        case 0:
            break;
        case set_chanel_error_bad_data:
            CPE_ERROR(
                em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: error %s!",
                wp, pipe->rp, pipe->capacity, set_chanel_str_error(rv));
            pipe->rp = wp;
            return rv;
        default:
            CPE_ERROR(
                em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: error %s!",
                wp, pipe->rp, pipe->capacity, set_chanel_str_error(rv));
            return rv;
        }
    }

    /*ignore empty pkg*/
    head_buf = dp_req_data(head);
    assert(head_buf);
    if (head_buf->to_svr_id == 0 && head_buf->to_svr_type == 0) {
        uint32_t total_size = *(uint32_t*)(buf + pipe->rp);
#ifdef SET_CHANEL_DEBUG 
        CPE_INFO(em, "set_chanel_pipe_peak: wp=%d, rp=%d, capacity=%d: erase dummy pkg, size=%d!", wp, pipe->rp, pipe->capacity, total_size);
#endif
        pipe->rp += total_size;
        assert(pipe->rp <= pipe->capacity);
        if (pipe->rp == pipe->capacity) pipe->rp = 0;
        goto PIPE_PEAK_TRY_AGAIN;
    }

    return 0;
}

static int set_chanel_pipe_write(SVR_SET_CHANEL * chanel, SVR_SET_PIPE * pipe, dp_req_t body, size_t * size, error_monitor_t em) {
    dp_req_t head = set_pkg_head_find(body);
    dp_req_t carry = set_pkg_carry_find(body);
    char * buf = ((char *)chanel) + pipe->begin;
    int write_size;
    uint32_t rp = pipe->rp;

    assert(head);

#ifdef SET_CHANEL_DEBUG 
    CPE_INFO(em, "set_chanel_pipe_write: wp=%d, rp=%d, capacity=%d: write begin!", pipe->wp, rp, pipe->capacity);
#endif

    assert(pipe->wp < pipe->capacity);

    if (pipe->wp >= rp) {
        assert(pipe->wp != pipe->capacity);

        write_size = set_chanel_pipe_save_pkg(body, head, carry, buf + pipe->wp, pipe->capacity - pipe->wp, em);
        if (write_size > 0) {
            assert(pipe->wp + write_size <= pipe->capacity);
            pipe->wp += write_size;
            if (pipe->wp == pipe->capacity) pipe->wp = 0;
        }
        else if (write_size == set_chanel_error_chanel_full) {
            if (rp <= 0) {
                CPE_ERROR(em, "set_chanel_pipe_write: wp=%d, rp=%d, capacity=%d: chanel_full!", pipe->wp, rp, pipe->capacity);
                return set_chanel_error_chanel_full;
            }

            write_size = set_chanel_pipe_save_pkg(body, head, carry, buf, rp - 1, em);
            if (write_size > 0) {
                set_chanel_pipe_save_ignore_pkg(buf + pipe->wp, pipe->capacity - pipe->wp);
                pipe->wp = write_size;
            }
            else {
                CPE_ERROR(em, "set_chanel_pipe_write: wp=%d, rp=%d, capacity=%d: chanel_full!", pipe->wp, rp, pipe->capacity);
                return set_chanel_error_chanel_full;
            }
        }
        else {
            assert(write_size < 0);
            CPE_ERROR(
                em, "set_chanel_pipe_write: wp=%d, rp=%d, capacity=%d: error %s!",
                pipe->wp, rp, pipe->capacity, set_chanel_str_error(write_size));
            return write_size;
        }
    }
    else {
        write_size = set_chanel_pipe_save_pkg(body, head, carry, buf + pipe->wp, rp - pipe->wp - 1, em);
        if (write_size > 0) {
            pipe->wp += write_size;
            if (pipe->wp == pipe->capacity) pipe->wp = 0;
        }
        else {
            assert(write_size < 0);
            CPE_ERROR(
                em, "set_chanel_pipe_write: wp=%d, rp=%d, capacity=%d: error %s!",
                pipe->wp, rp, pipe->capacity, set_chanel_str_error(write_size));
            return write_size;
        }
    }

    assert(write_size > 0);
    if (size) *size = write_size;

#ifdef SET_CHANEL_DEBUG 
    CPE_INFO(em, "set_chanel_pipe_write: wp=%d, rp=%d, capacity=%d: write success, size=%d!", pipe->wp, rp, pipe->capacity, write_size);
#endif

    return 0;
}

static void set_chanel_pipe_save_ignore_pkg(void * buf, size_t capacity) {
    SET_PKG_HEAD * head;

    if (capacity < SET_SHARE_SAVE_HEAD_SIZE) return;
    
    *((uint32_t*)buf) = (uint32_t)capacity;
    head = (SET_PKG_HEAD *)(((uint32_t*)buf) + 1);
    head->to_svr_id = 0;
    head->to_svr_type = 0;
    *((uint8_t*)(head + 1)) = 0;
}

static int set_chanel_pipe_load_pkg(dp_req_t body, dp_req_t head, dp_req_t carry, void * buf, size_t capacity, error_monitor_t em) {
    uint32_t total_size;
    uint32_t left_size;
    SET_PKG_HEAD * head_buf;
    char * read_buf = buf;
    uint32_t read_pos;

    if (capacity < sizeof(uint32_t)) {
        CPE_ERROR(em, "set_chanel_pipe_load_pkg: capacity too small: capacity=%d!", (int)capacity);
        return set_chanel_error_bad_data;
    }

    total_size = *((uint32_t*)read_buf);
    if (capacity < total_size) {
        CPE_ERROR(
            em, "set_chanel_pipe_load_pkg: found small pkg: total_size=%d, head_size=%d!",
            total_size, (int)SET_SHARE_SAVE_HEAD_SIZE);
        return set_chanel_error_bad_data;
    }

    if (total_size < SET_SHARE_SAVE_HEAD_SIZE) {
        CPE_ERROR(
            em, "set_chanel_pipe_load_pkg: bad data: total_size=%d, head_size=%d!",
            total_size, (int)SET_SHARE_SAVE_HEAD_SIZE);
        return set_chanel_error_bad_data;
    }

    read_pos = sizeof(uint32_t);
    left_size = total_size - sizeof(uint32_t);

    head_buf = (SET_PKG_HEAD *)(read_buf + read_pos);
    read_pos += sizeof(SET_PKG_HEAD);
    left_size -= sizeof(SET_PKG_HEAD);

    dp_req_set_buf(head, head_buf, sizeof(SET_PKG_HEAD));
    dp_req_set_size(head, sizeof(SET_PKG_HEAD));

    if (set_pkg_carry_set_buf(carry, read_buf + read_pos, left_size) != 0) {
        uint8_t size = *(read_buf + read_pos);
        CPE_ERROR(em, "set_chanel_pipe_load_pkg: set_pkg_carry_set_buf fail, size=%d, buf-size=%d!", size, left_size);
        return set_chanel_error_bad_data;
    }

    read_pos += dp_req_size(carry);
    left_size -= dp_req_size(carry);

    dp_req_set_buf(body, read_buf + read_pos, left_size);
    dp_req_set_size(body, left_size);

    return 0;
}

static int set_chanel_pipe_save_pkg(dp_req_t body, dp_req_t head, dp_req_t carry, void * input_buf, size_t capacity, error_monitor_t em) {
    char * buf = input_buf;
    uint32_t total_size;
    size_t head_size;
    uint8_t carry_size = 0;
    SET_PKG_HEAD * pkg_head_buf;

    if (carry) {
        carry_size = set_pkg_carry_size(carry);
        assert(carry_size + 1 == dp_req_size(carry));
    }

    head_size = sizeof(uint32_t) + sizeof(SET_PKG_HEAD) + 1 + carry_size;

    if (capacity < head_size) return set_chanel_error_chanel_full;

    if (set_pkg_pack_state(head) == set_pkg_packed) {
        size_t body_capacity;
        int decode_size;

        body_capacity = capacity - head_size;

        decode_size = 
            dr_pbuf_read(
                buf + head_size, body_capacity,
                dp_req_data(body), dp_req_size(body), dp_req_meta(body), em);
        if (decode_size < 0) {
            if (decode_size == dr_code_error_not_enough_output) {
                return set_chanel_error_chanel_full;
            }
            else {
                return set_chanel_error_decode;
            }
        }

        total_size = head_size + decode_size;
    }
    else {
        total_size = head_size + dp_req_size(body);
        
        if (capacity < total_size) return set_chanel_error_chanel_full;

        /*填写包体 */
        memcpy(buf + head_size, dp_req_data(body), dp_req_size(body));
    }

    /*填写总大小 */
    memcpy(buf, &total_size, sizeof(total_size));

    /*填写包头 */
    pkg_head_buf = (SET_PKG_HEAD*)(buf + sizeof(total_size));
    memcpy(pkg_head_buf, dp_req_data(head), sizeof(SET_PKG_HEAD));
    pkg_head_buf->flags &= ~((uint16_t)(1 << 2));

    /*填写carry_data */
    *(uint8_t*)(buf + (sizeof(total_size) + sizeof(SET_PKG_HEAD))) = carry_size;
    if (carry_size) {
        memcpy(buf + sizeof(total_size) + sizeof(SET_PKG_HEAD) + 1, set_pkg_carry_data(carry), carry_size);
    }

    return total_size;
}

const char * set_chanel_str_error(int err) {
    switch(err) {
    case 0:
        return "success";
    case set_chanel_evt_not_enouth_data:
        return "not_enouth_data";
    case set_chanel_error_bad_data:
        return "bad_data";
    case set_chanel_error_chanel_full:
        return "chanel_full"; 
    case set_chanel_error_chanel_empty:
        return "chanel_empty";
    case set_chanel_error_no_memory:
        return "no_memory";
    case set_chanel_error_decode:
        return "decode_error";
    default:
        return "unknown error";
    }
}

const char * set_chanel_dump(set_chanel_t input_chanel, mem_buffer_t buffer) {
    SVR_SET_CHANEL * chanel = (SVR_SET_CHANEL *)input_chanel;
    struct write_stream_buffer stream;

    write_stream_buffer_init(&stream, buffer);
    stream_printf(
        (write_stream_t)&stream, "chanel: rq(capacity=%d, wp=%d, rp=%d) wq(capacity=%d, wp=%d, rp=%d)",
        chanel->r.capacity, chanel->r.wp, chanel->r.rp,
        chanel->w.capacity, chanel->w.wp, chanel->w.rp);

    return mem_buffer_make_continuous(buffer, 0);
}
