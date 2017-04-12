#include <assert.h>
#include "zlib.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/dr_store/dr_ref.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "protocol/base/base_package.h"
#include "bpg_pkg_internal_ops.h"

dr_cvt_result_t
bpg_pkg_decode(
    dp_req_t body,
    const void * input, size_t * input_capacity,
    error_monitor_t em, int debug)
{
    dr_cvt_result_t r;
    void * decode_buff;
    size_t base_pkg_size;
    bpg_pkg_t pkg = bpg_pkg_find(body);
    assert(pkg);

    decode_buff = bpg_pkg_op_buff(pkg->m_mgr);
    if (decode_buff == NULL) {
        CPE_ERROR(em, "%s: decode: no op buff!", bpg_pkg_manage_name(pkg->m_mgr));
        return dr_cvt_result_error;
    }

    base_pkg_size = pkg->m_mgr->m_op_buff_capacity;
    r =  dr_cvt_decode(
        bpg_pkg_base_cvt(pkg), bpg_pkg_base_meta(pkg),
        decode_buff, &base_pkg_size,
        input, input_capacity, 
        em, debug);
    if (r != dr_cvt_result_success) return r;

    return bpg_pkg_decode_data(body, decode_buff, &base_pkg_size, em, debug);
}

dr_cvt_result_t bpg_pkg_decode_data(dp_req_t body, const void * input, size_t * input_capacity, error_monitor_t em, int debug) {
    dr_cvt_result_t r;
    BASEPKG * input_pkg;
    BASEPKG_HEAD input_pkg_head_buf;
    char * input_data;
    BASEPKG * output_pkg;
    char * output_buf;
    size_t output_buf_capacity;
    LPDRMETALIB metalib;
    LPDRMETA meta;
    uint16_t i;
    dr_cvt_t data_cvt;
    bpg_pkg_t pkg = bpg_pkg_find(body);
    assert(pkg);

    metalib = bpg_pkg_manage_data_metalib(pkg->m_mgr);
    if (metalib == NULL) {
        CPE_ERROR(
            em, "%s: decode:  data meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return dr_cvt_result_error;
    }

    data_cvt = bpg_pkg_data_cvt(pkg);
    if (data_cvt == NULL) {
        CPE_ERROR(
            em, "%s: decode:  data meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return dr_cvt_result_error;
    }

    input_pkg = (BASEPKG *)input;
    if (input_pkg->head.flags & BASEPKG_HEAD_FLAG_ZIP) {
        Bytef * unzip_buf;
        uLongf unzip_size;
        size_t unzip_capacity;
        int rv;

        unzip_capacity = mem_buffer_size(&pkg->m_mgr->m_zip_buff);
        if (unzip_capacity < 4096) unzip_capacity = 4096;

    UNZIP_RETRY:
        if (mem_buffer_size(&pkg->m_mgr->m_zip_buff) != unzip_capacity) {
            mem_buffer_set_size(&pkg->m_mgr->m_zip_buff, unzip_capacity);
        }

        unzip_buf = (Bytef *)mem_buffer_make_continuous(&pkg->m_mgr->m_zip_buff, 0);
        if (unzip_buf == NULL) goto UNZIP_RETRY;

        memcpy(unzip_buf, input, sizeof(BASEPKG_HEAD));

        unzip_size = mem_buffer_size(&pkg->m_mgr->m_zip_buff) - sizeof(BASEPKG_HEAD);
        rv = uncompress(
                unzip_buf + sizeof(BASEPKG_HEAD), &unzip_size,
                ((const Bytef *)input) + sizeof(BASEPKG_HEAD), *input_capacity - sizeof(BASEPKG_HEAD));

        if (rv != Z_OK) {
            if (rv == Z_BUF_ERROR) {
                unzip_capacity *= 2;
                goto UNZIP_RETRY;
            }
            else {
                CPE_ERROR(
                    em, "%s: decode: unzip fail, output_size=%d, input_size=%d!",
                    bpg_pkg_manage_name(pkg->m_mgr),
                    (int)(mem_buffer_size(&pkg->m_mgr->m_zip_buff) - sizeof(BASEPKG_HEAD)),
                    (int)(*input_capacity - sizeof(BASEPKG_HEAD)));
                return dr_cvt_result_error;
            }
        }
        
        input_data = (char *)unzip_buf;
        input_pkg->head.flags &= ~BASEPKG_HEAD_FLAG_ZIP;
    }
    else {
        if (*input_capacity >= sizeof(BASEPKG_HEAD)) {
            if ((input_pkg->head.bodytotallen + sizeof(BASEPKG_HEAD)) > *input_capacity) {
                CPE_ERROR(
                    em, "%s: decode: totalbodylen error, totalbodylen=%d, headlen=%d, input-total-len=%d!",
                    bpg_pkg_manage_name(pkg->m_mgr),
                    (int)input_pkg->head.bodytotallen, (int)sizeof(BASEPKG_HEAD), (int)*input_capacity);
                return dr_cvt_result_error;
            }

            input_data = (char *)input;
        }
        else {
            if (input_pkg->head.bodytotallen != 0) {
                CPE_ERROR(
                    em, "%s: decode: totalbodylen error, totalbodylen=%d, headlen=%d, input-total-len=%d!",
                    bpg_pkg_manage_name(pkg->m_mgr),
                    (int)input_pkg->head.bodytotallen, (int)sizeof(BASEPKG_HEAD), (int)*input_capacity);
                return dr_cvt_result_error;
            }

            bzero(&input_pkg_head_buf, sizeof(input_pkg_head_buf));
            memcpy(&input_pkg_head_buf, input, *input_capacity);
            input_data = (char *)&input_pkg_head_buf;
        }
    }
        
    input_pkg = (BASEPKG *)input_data;

    output_buf = (char *)dp_req_data(body);
    output_pkg = (BASEPKG *)output_buf;
    output_buf_capacity = dp_req_capacity(body);

    output_pkg->head.sn = input_pkg->head.sn;
    output_pkg->head.cmd = input_pkg->head.cmd ;
    output_pkg->head.errorNo = input_pkg->head.errorNo;
    output_pkg->head.clientId = input_pkg->head.clientId;
    output_pkg->head.flags = 0;
    output_pkg->head.headlen = sizeof(BASEPKG_HEAD);
    output_pkg->head.bodylen = 0;
    output_pkg->head.bodytotallen = 0;
    output_pkg->head.appendInfoCount = 0;

    if (output_buf_capacity < sizeof(input_pkg->head)) {
        CPE_ERROR(
            em, "%s: decode: buf capacity %d too small to contain head!",
            bpg_pkg_manage_name(pkg->m_mgr), (int)output_buf_capacity);
        return dr_cvt_result_error;
    }

    output_buf_capacity -= sizeof(BASEPKG_HEAD);
    output_buf += sizeof(BASEPKG_HEAD);
    input_data += sizeof(BASEPKG_HEAD);

    if (input_pkg->head.bodylen) {
        size_t use_size = output_buf_capacity;
        if ((meta = bpg_pkg_manage_find_meta_by_cmd(pkg->m_mgr, input_pkg->head.cmd))) {
            size_t tmp_len = input_pkg->head.bodylen;
            r =  dr_cvt_decode(data_cvt, meta, output_buf, &use_size, input_data, &tmp_len, em, debug);
            if (r != dr_cvt_result_success) {
                CPE_ERROR(
                    em, "%s: decode: decode main meta (%s) error, output-size=%d, input-size=%d!",
                    bpg_pkg_manage_name(pkg->m_mgr), dr_meta_name(meta), (int)use_size, (int)tmp_len);
                return r;
            }

            CPE_PAL_ALIGN_DFT(use_size);

            output_pkg->head.bodylen = use_size;
            output_pkg->head.bodytotallen = use_size;

            output_buf += use_size;
            output_buf_capacity -= use_size;
        }
        else {
            CPE_ERROR(
                em, "%s: decode: no main meta of cmd %d!",
                bpg_pkg_manage_name(pkg->m_mgr), (int)input_pkg->head.cmd);
            return dr_cvt_result_error;
        }

        input_data += input_pkg->head.bodylen;
    }

    for(i = 0; i < input_pkg->head.appendInfoCount; ++i) {
        APPENDINFO * o_append_info;
        APPENDINFO const * append_info = &input_pkg->head.appendInfos[i];
        char * append_data = input_data;
        size_t use_size = output_buf_capacity;
        size_t tmp_size = append_info->size;

        input_data += append_info->size;

        meta = dr_lib_find_meta_by_id(metalib, append_info->id);
        if (meta == NULL) {
            CPE_ERROR(
                em, "%s: decode: append %d: meta of id %d not exist in lib %s!",
                bpg_pkg_manage_name(pkg->m_mgr), i, append_info->id, dr_lib_name(metalib));
            continue;
        }

        r =  dr_cvt_decode(bpg_pkg_data_cvt(pkg), meta, output_buf, &use_size, append_data, &tmp_size, em, debug);
        if (r != dr_cvt_result_success) {
            CPE_ERROR(
                em, "%s: decode: append %d: decode append meta (%s) error!",
                bpg_pkg_manage_name(pkg->m_mgr), i, dr_meta_name(meta));
            continue;
        }

        CPE_PAL_ALIGN_DFT(use_size);

        o_append_info = &output_pkg->head.appendInfos[output_pkg->head.appendInfoCount++];
        o_append_info->id = append_info->id;
        o_append_info->size = use_size;
        output_pkg->head.bodytotallen += use_size;

        output_buf += use_size;
        output_buf_capacity -= use_size;
    }

    dp_req_set_size(body, output_pkg->head.bodytotallen);

    return dr_cvt_result_success;
}
