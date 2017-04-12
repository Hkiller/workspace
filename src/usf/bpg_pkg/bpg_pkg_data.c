#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "protocol/base/base_package.h"
#include "bpg_pkg_internal_types.h"

LPDRMETALIB bpg_pkg_data_meta_lib(dp_req_t body) {
    bpg_pkg_t pkg = bpg_pkg_find(body);
    assert(pkg);
    return dr_ref_lib(pkg->m_mgr->m_metalib_ref);
}


void * bpg_pkg_body(dp_req_t body) {
    return ((char *)dp_req_data(body)) + sizeof(BASEPKG_HEAD);
}

size_t bpg_pkg_body_capacity(dp_req_t body) {
    return dp_req_capacity(body) - sizeof(BASEPKG_HEAD);
}

size_t bpg_pkg_body_size(dp_req_t body) {
    return dp_req_size(body) - sizeof(BASEPKG_HEAD);
}

LPDRMETA bpg_pkg_main_data_meta(dp_req_t body, error_monitor_t em) {
    bpg_pkg_t pkg = bpg_pkg_find(body);
    assert(pkg);
    return bpg_pkg_manage_find_meta_by_cmd(pkg->m_mgr, bpg_pkg_cmd(body));
}

LPDRMETA bpg_pkg_append_data_meta(dp_req_t body, bpg_pkg_append_info_t append_info, error_monitor_t em) {
    bpg_pkg_t pkg = bpg_pkg_find(body);
    LPDRMETALIB metalib;
    LPDRMETA data_meta;

    assert(pkg);

    metalib = bpg_pkg_manage_data_metalib(pkg->m_mgr);
    if (metalib == NULL) {
        CPE_ERROR(
            em, "%s: bpg_pkg_append_data_meta:  data meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return NULL;
    }

    data_meta = dr_lib_find_meta_by_id(metalib, bpg_pkg_append_info_id(append_info));
    if (data_meta == NULL) {
        CPE_ERROR(
            em, "%s: bpg_pkg_append_data_meta:  meta of id %d not exist in lib %s!",
            bpg_pkg_manage_name(pkg->m_mgr), bpg_pkg_append_info_id(append_info), dr_lib_name(metalib));
        return NULL;
    }

    return data_meta;
}

int32_t bpg_pkg_append_info_count(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);
    return head->appendInfoCount;
}

bpg_pkg_append_info_t bpg_pkg_append_info_at(dp_req_t body, int32_t pos) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return (pos >= 0 && pos < head->appendInfoCount)
        ? (bpg_pkg_append_info_t)(&head->appendInfos[pos])
        : NULL;
}

uint32_t bpg_pkg_append_info_id(bpg_pkg_append_info_t append_info) {
    return ((APPENDINFO *)append_info)->id;
}

uint32_t bpg_pkg_append_info_size(bpg_pkg_append_info_t append_info) {
    return ((APPENDINFO *)append_info)->size;
}

void * bpg_pkg_body_data(dp_req_t body) {
    return ((char *)dp_req_data(body)) + sizeof(BASEPKG_HEAD);
}
    
uint32_t bpg_pkg_body_len(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->bodylen;
}

int bpg_pkg_set_main_data(dp_req_t body, void const * buf, size_t size, error_monitor_t em) {
    bpg_pkg_t pkg = bpg_pkg_find(body);
    BASEPKG_HEAD * head;
    size_t remain_size;
    void * pkg_data;

    CPE_PAL_ALIGN_DFT(size);

    assert(pkg);
    assert(buf);

    head = (BASEPKG_HEAD *)dp_req_data(body);

    if (head->appendInfoCount > 0) {
        CPE_ERROR(em, "bpg_pkg_set_data: already have append info!");
        return -1;
    }

    remain_size = dp_req_capacity(body) - sizeof(BASEPKG_HEAD);
    if (remain_size < size) {
        CPE_ERROR(
            em, "bpg_pkg_set_data: not enough buf! buf-size=%d, input-size=%d",
            (int)remain_size, (int)size);
        return -1;
    }

    pkg_data = head + 1;
    if (pkg_data != buf) memcpy(pkg_data, buf, size);

    dp_req_set_size(body, sizeof(BASEPKG_HEAD) + size);

    head->bodylen = size;
    head->bodytotallen = size;

    return 0;
}

void * bpg_pkg_main_data(dp_req_t body) {
    return (char *)dp_req_data(body) + sizeof(BASEPKG_HEAD);
}

size_t bpg_pkg_main_data_len(dp_req_t body) {
    BASEPKG_HEAD * head;

    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->bodylen;
}

int bpg_pkg_add_append_data(dp_req_t body, LPDRMETA meta, const void * buf, size_t size, error_monitor_t em) {
    BASEPKG * basepkg;
    APPENDINFO * appendInfo;
    size_t cur_size;
    size_t remain_size;

    CPE_PAL_ALIGN_DFT(size);

    /* assert(pkg); */

    basepkg = (BASEPKG *)dp_req_data(body);

    if (basepkg->head.appendInfoCount >= APPEND_INFO_MAX_COUNT) {
        CPE_ERROR(em, "bpg_pkg_add_append_data: max append info reached!");
        return -1;
    }

    cur_size = dp_req_size(body);
    remain_size = dp_req_capacity(body) - cur_size;
    if (remain_size < size) {
        CPE_ERROR(
            em, "bpg_pkg_add_append_data: not enough buf! buf-size=%d, input-size=%d",
            (int)remain_size, (int)size);
        return -1;
    }

    memcpy(((char *)dp_req_data(body)) + cur_size, buf, size);

    appendInfo = &basepkg->head.appendInfos[basepkg->head.appendInfoCount++];
    appendInfo->id = dr_meta_id(meta);
    appendInfo->size = size;

    dp_req_set_size(body, cur_size + size);

    basepkg->head.bodytotallen += size;

    return 0;
}

void * bpg_pkg_append_data(dp_req_t body, bpg_pkg_append_info_t append_info) {
    int pos;
    BASEPKG * basepkg;
    char * buf;
    int i;

    basepkg = (BASEPKG *)dp_req_data(body);

    pos = (((APPENDINFO *)append_info) - basepkg->head.appendInfos);

    if (pos < 0 || pos > basepkg->head.appendInfoCount) return NULL;

    buf = (char*)basepkg->body;
    buf += basepkg->head.bodylen;

    for(i = 0; i < pos; ++i) {
        buf += basepkg->head.appendInfos[i].size;
    }

    return buf;
}

uint32_t bpg_pkg_cmd(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->cmd;
}

void bpg_pkg_set_cmd(dp_req_t body, uint32_t cmd) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    head->cmd = cmd;
}

uint32_t bpg_pkg_sn(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->sn;
}

void bpg_pkg_set_sn(dp_req_t body, uint32_t sn) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    head->sn = sn;
}

uint32_t bpg_pkg_flags(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->flags;
}

int bpg_pkg_flag_enable(dp_req_t body, bpg_pkg_flag_t flag) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return ((head->flags & (uint32_t)flag) == (uint32_t)flag) ? 1 : 0;
}

void bpg_pkg_flag_set_enable(dp_req_t body, bpg_pkg_flag_t flag, int is_enable) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    if (is_enable) {
        head->flags |= (uint32_t)flag;
    }
    else {
        head->flags &= ~(uint32_t)flag;
    }
}

void bpg_pkg_set_flags(dp_req_t body, uint32_t flags) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    head->flags = flags;
}

uint32_t bpg_pkg_errno(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->errorNo;
}

void bpg_pkg_set_errno(dp_req_t body, uint32_t errorNo) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    head->errorNo = errorNo;
}

uint64_t bpg_pkg_client_id(dp_req_t body) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    return head->clientId;
}

void bpg_pkg_set_client_id(dp_req_t body, uint64_t client_id) {
    BASEPKG_HEAD * head;
    head = (BASEPKG_HEAD *)dp_req_data(body);

    head->clientId = client_id;
}


const char * bpg_pkg_dump(dp_req_t body, mem_buffer_t buffer) {
    bpg_pkg_t req = bpg_pkg_find(body);
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    LPDRMETALIB metalib;
    LPDRMETA meta;
    BASEPKG_HEAD * head;
    const char * data;
    int i;

    mem_buffer_clear_data(buffer);

    if (req == NULL) {
        stream_printf(((write_stream_t)&stream), "not bpg pkg");
        stream_putc((write_stream_t)&stream, 0);
        return (const char *)mem_buffer_make_continuous(buffer, 0);
    }

    data = dp_req_data(body);

    head = (BASEPKG_HEAD *)data;
    
    stream_printf(((write_stream_t)&stream), "head: ");

    metalib = dr_ref_lib(req->m_mgr->m_metalib_basepkg_ref);
    if ((meta = metalib ? dr_lib_find_meta_by_name(metalib, "basepkg_head") : NULL)) {
        dr_json_print((write_stream_t)&stream, head, sizeof(BASEPKG_HEAD), meta, DR_JSON_PRINT_MINIMIZE, 0);
    }
    else {
        stream_printf((write_stream_t)&stream, "[no meta] cmd=%d", head->cmd);
    }
    data += sizeof(BASEPKG_HEAD);

    stream_printf(((write_stream_t)&stream), "\nbody: ");
    if (head->bodylen > 0) {
        if ((meta = bpg_pkg_main_data_meta(body, NULL))) {
            stream_printf(((write_stream_t)&stream), " %s", dr_meta_name(meta));
            dr_json_print((write_stream_t)&stream, data, head->bodylen, meta, DR_JSON_PRINT_MINIMIZE, 0);
        }
        else {
            stream_printf((write_stream_t)&stream, "[no meta] bodylen=%d", head->bodylen);
        }

        data += head->bodylen;
    }
    else {
        stream_printf((write_stream_t)&stream, "[no data]");
    }

    metalib = dr_ref_lib(req->m_mgr->m_metalib_ref);
    for(i = 0; i < head->appendInfoCount; ++i) {
        APPENDINFO const * append_info = &head->appendInfos[i];

        if ((meta = metalib ? dr_lib_find_meta_by_id(metalib, append_info->id) : NULL)) {
            stream_printf((write_stream_t)&stream, "\nappend %d(%s): ", append_info->id, dr_meta_name(meta));

            dr_json_print((write_stream_t)&stream, data, append_info->size, meta, DR_JSON_PRINT_MINIMIZE, 0);
        }
        else {
            stream_printf(
                (write_stream_t)&stream, "\nappend [no meta]: id=%d, size=%d, origin-size=%d", 
                append_info->id, append_info->size, append_info->size);
        }

        data += append_info->size;
    }

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

bpg_pkg_debug_level_t bpg_pkg_debug_level(dp_req_t body) {
    bpg_pkg_t pkg = bpg_pkg_find(body);
    assert(pkg);
    return bpg_pkg_manage_debug_level(pkg->m_mgr, bpg_pkg_cmd(body));
}

void bpg_pkg_init(dp_req_t body) {
    BASEPKG_HEAD * head;

    assert(dp_req_capacity(body) >= sizeof(BASEPKG_HEAD));

    dp_req_set_size(body, sizeof(BASEPKG_HEAD));
    bzero(dp_req_data(body), sizeof(BASEPKG_HEAD));

    head = (BASEPKG_HEAD *)dp_req_data(body);
    head->headlen = sizeof(BASEPKG_HEAD);
}

void bpg_pkg_clear(dp_req_t body) {
    BASEPKG_HEAD * head;

    bzero(dp_req_data(body), dp_req_capacity(body));
    dp_req_set_size(body, sizeof(BASEPKG_HEAD));

    head = (BASEPKG_HEAD *)dp_req_data(body);
    head->headlen = sizeof(BASEPKG_HEAD);
}
