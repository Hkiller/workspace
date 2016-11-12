#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_pkg.h"
#include "protocol/svr/set/set_share_pkg.h"

extern char g_metalib_svr_set_share[];
static LPDRMETA g_meta_set_pkg_head = NULL;
static LPDRMETA g_meta_set_pkg_carry = NULL;
static void set_pkg_head_dump(dp_req_t head, write_stream_t s);

dp_req_t set_pkg_head_find(dp_req_t body) {
    return dp_req_child_find(body, req_type_set_pkg_head);
}

dp_req_t set_pkg_head_check_create(dp_req_t body) {
    dp_req_t r = dp_req_child_find(body, req_type_set_pkg_head);
    if (r) return r;

    r = set_pkg_head_create(dp_req_mgr(body));
    if (r == NULL) return r;

    dp_req_add_to_parent(r, body);
    return r;
}

dp_req_t set_pkg_head_create(dp_mgr_t dp_mgr) {
    dp_req_t r;

    if (g_meta_set_pkg_head == NULL) {
        g_meta_set_pkg_head = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_set_share, "set_pkg_head");
        if (g_meta_set_pkg_head == NULL) {
            return NULL;
        }
    }

    r = dp_req_create(dp_mgr, sizeof(SET_PKG_HEAD));
    if (r == NULL) return NULL;

    dp_req_set_dumper(r, set_pkg_head_dump);
    dp_req_set_meta(r, g_meta_set_pkg_head);
    dp_req_set_size(r, sizeof(SET_PKG_HEAD));

    return r;
}

dp_req_t set_pkg_carry_find(dp_req_t body) {
    return dp_req_child_find(body, req_type_set_pkg_carry);
}

dp_req_t set_pkg_carry_check_create(dp_req_t body, size_t capacity) {
    dp_req_t r = dp_req_child_find(body, req_type_set_pkg_carry);
    if (r) {
        if (set_pkg_carry_capacity(r) >= capacity) {
            set_pkg_carry_set_size(r, 0);
            return r;
        }

        if (dp_req_manage_by_parent(r)) {
            dp_req_free(r);
        }
        else {
            dp_req_set_parent(r, NULL);
        }
        r = NULL;
    }

    r = set_pkg_carry_create(dp_req_mgr(body), capacity);
    if (r == NULL) return NULL;

    set_pkg_carry_set_size(r, 0);
    dp_req_add_to_parent(r, body);

    return r;
}

dp_req_t set_pkg_carry_create(dp_mgr_t dp_mgr, size_t capacity) {
    dp_req_t r;

    if (g_meta_set_pkg_carry == NULL) {
        g_meta_set_pkg_carry = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_set_share, "set_pkg_carry");
        if (g_meta_set_pkg_carry == NULL) {
            return NULL;
        }
    }

    r = dp_req_create(dp_mgr, capacity + 1);
    if (r == NULL) return NULL;

    dp_req_set_meta(r, g_meta_set_pkg_carry);

    return r;
}

void set_pkg_init(dp_req_t head) {
    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);
    bzero(dp_req_data(head), dp_req_size(head));
}

int set_pkg_init_response(dp_req_t body, dp_req_t request) {
    dp_req_t response_head;
    SET_PKG_HEAD * response_head_buf;
    dp_req_t request_head;
    SET_PKG_HEAD * request_head_buf;
    dp_req_t response_carry;
    dp_req_t request_carry;

    request_head = set_pkg_head_find(request);
    if (request_head == NULL) return -1;

    response_head = set_pkg_head_check_create(body);
    if (response_head == NULL) return -1;

    response_head_buf = dp_req_data(response_head);
    request_head_buf = dp_req_data(request_head);
    
    response_head_buf->to_svr_type = request_head_buf->from_svr_type;
    response_head_buf->to_svr_id = request_head_buf->from_svr_id;
    response_head_buf->sn = request_head_buf->sn;
    response_head_buf->from_svr_type = request_head_buf->to_svr_type;
    response_head_buf->from_svr_id = request_head_buf->to_svr_id;
    response_head_buf->flags = 0;
    set_pkg_set_category(response_head, set_pkg_response);

    if ((request_carry = set_pkg_carry_find(request))) {
        response_carry = set_pkg_carry_check_create(body, dp_req_size(request_carry));
        if (response_carry == NULL) return -1;

        memcpy(dp_req_data(response_carry), dp_req_data(request_carry), dp_req_size(request_carry));
        dp_req_set_size(response_carry, dp_req_size(request_carry));
    }
    else {
        response_carry = set_pkg_carry_check_create(body, 0);
        if (response_carry == NULL) return -1;
    }

    return 0;
}

set_pkg_pack_state_t set_pkg_pack_state(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return (set_pkg_pack_state_t)((head_buf->flags >> 2) & 1);
}

void set_pkg_set_pack_state(dp_req_t head, set_pkg_pack_state_t c) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    head_buf->flags &= ~(((uint16_t)1) << 2);
    head_buf->flags |= (((uint16_t)c) & 1) << 2;
}

set_pkg_category_t set_pkg_category(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return (set_pkg_category_t)(head_buf->flags & 0x3);
}

void set_pkg_set_category(dp_req_t head, set_pkg_category_t c) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    head_buf->flags &= ~((uint16_t)0x3);
    head_buf->flags |= ((uint16_t)c) & 0x3;
}

uint32_t set_pkg_sn(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return head_buf->sn;
}

void set_pkg_set_sn(dp_req_t head, uint32_t sn) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    head_buf->sn = sn;
}

uint16_t set_pkg_to_svr_type(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return head_buf->to_svr_type;
}

uint16_t set_pkg_to_svr_id(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return head_buf->to_svr_id;
}

void set_pkg_set_to_svr(dp_req_t head, uint16_t to_svr_type, uint16_t to_svr_id) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    head_buf->to_svr_type = to_svr_type;
    head_buf->to_svr_id = to_svr_id;
}

uint16_t set_pkg_from_svr_type(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return head_buf->from_svr_type;
}

uint16_t set_pkg_from_svr_id(dp_req_t head) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    return head_buf->from_svr_id;
}

void set_pkg_set_from_svr(dp_req_t head, uint16_t from_svr_type, uint16_t from_svr_id) {
    SET_PKG_HEAD * head_buf;

    assert(head);
    assert(dp_req_meta(head) == g_meta_set_pkg_head);

    head_buf = dp_req_data(head);

    head_buf->from_svr_type = from_svr_type;
    head_buf->from_svr_id = from_svr_id;
}

int set_pkg_carry_copy(dp_req_t carry, uint8_t size, void * data) {
    if (set_pkg_carry_set_size(carry, size) != 0) return -1;

    memcpy(set_pkg_carry_data(carry), data, size);

    return 0;
}

int set_pkg_carry_set_buf(dp_req_t carry, void * buf, size_t capacity) {
    uint8_t size = *(uint8_t*)buf;

    if ((size + 1) > capacity) return -1;

    dp_req_set_buf(carry, buf, size + 1);
    set_pkg_carry_set_size(carry, size);

    return 0;
}

int set_pkg_carry_set_size(dp_req_t carry, uint8_t size) {
    if ((((size_t)size) + 1) > dp_req_capacity(carry)) return - 1;

    dp_req_set_size(carry, size + 1);

    *(uint8_t *)dp_req_data(carry) = size;

    return 0;
}

uint8_t set_pkg_carry_size(dp_req_t carry) {
    return *(uint8_t *)dp_req_data(carry);
}

void * set_pkg_carry_data(dp_req_t carry) {
    return ((char*)dp_req_data(carry)) + 1;
}

uint8_t set_pkg_carry_capacity(dp_req_t carry) {
    size_t c = dp_req_capacity(carry);
    assert(c >= 1 && c <= (256 + 1));
    return (uint8_t)(c - 1);
}

static void set_pkg_head_dump(dp_req_t head, write_stream_t s) {
    SET_PKG_HEAD * head_buf;
    head_buf = dp_req_data(head);

    stream_printf(
        s, "set_pkg_head(%d bytes): {\"to_svr_type\":%d,\"to_svr_id\":%d,\"from_svr_type\":%d,\"from_svr_id\":%d,\"sn\":%d,\"flags\":0x%0xd",
        (int)dp_req_size(head), head_buf->to_svr_type, head_buf->to_svr_id, head_buf->from_svr_type, head_buf->from_svr_id, head_buf->sn, head_buf->flags);

    switch(set_pkg_category(head)) {
    case set_pkg_request:
        stream_printf(s, "(request");
        break;
    case set_pkg_response:
        stream_printf(s, "(response");
        break;
    case set_pkg_notify:
        stream_printf(s, "(notify");
        break;
    default:
        stream_printf(s, "(???");
        break;
    }

    if (set_pkg_pack_state(head) == set_pkg_packed) {
        stream_printf(s, ", packed");
    }

    stream_printf(s, ")}");
}

const char * req_type_set_pkg_head = "set_pkg_head";
const char * req_type_set_pkg_carry = "set_pkg_carry";
