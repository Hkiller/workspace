#include <assert.h>
#include "zlib.h"
#include "cpe/pal/pal_platform.h"
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

dr_cvt_result_t bpg_pkg_encode(dp_req_t body, void * output, size_t * output_capacity, error_monitor_t em, int debug) {
    dr_cvt_result_t r;
    bpg_pkg_t pkg = bpg_pkg_find(body);
    void * op_buff = bpg_pkg_op_buff(pkg->m_mgr);
    size_t op_buff_size = pkg->m_mgr->m_op_buff_capacity;
    BASEPKG * output_pkg = (BASEPKG *)op_buff;
    size_t input_size;

    r = bpg_pkg_encode_data(body, op_buff, &op_buff_size, em, debug);
    if (r != dr_cvt_result_success) return r;

    input_size = op_buff_size;
    r = dr_cvt_encode(
        bpg_pkg_base_cvt(pkg),
        bpg_pkg_base_meta(pkg),
        output, output_capacity,
        op_buff, &input_size,
        em, debug);

    if (r != dr_cvt_result_success) {
        if (pkg->m_mgr->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&pkg->m_mgr->m_dump_buff);
            LPDRMETALIB head_metalib = dr_ref_lib(pkg->m_mgr->m_metalib_basepkg_ref);
            LPDRMETA head_meta = head_metalib ? dr_lib_find_meta_by_name(head_metalib, "basepkg_head") : NULL;
            mem_buffer_clear_data(&pkg->m_mgr->m_dump_buff);
            dr_json_print((write_stream_t)&stream, &output_pkg->head, sizeof(BASEPKG_HEAD), head_meta, DR_JSON_PRINT_MINIMIZE, 0);
            stream_putc((write_stream_t)&stream, 0);

            CPE_ERROR(
                em, "%s: encode: base encode fail!, output-capacity=%d, input-size=%d\nhead: %s",
                bpg_pkg_manage_name(pkg->m_mgr), (int)*output_capacity, (int)op_buff_size,
                (const char *)mem_buffer_make_continuous(&pkg->m_mgr->m_dump_buff, 0));
        }
        else {
            CPE_ERROR(
                em, "%s: encode: base encode fail!, output-capacity=%d, input-size=%d",
                bpg_pkg_manage_name(pkg->m_mgr), (int)*output_capacity, (int)op_buff_size);
        }
    }
    else {
        if (pkg->m_mgr->m_debug) {
            struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&pkg->m_mgr->m_dump_buff);
            LPDRMETALIB head_metalib = dr_ref_lib(pkg->m_mgr->m_metalib_basepkg_ref);
            LPDRMETA head_meta = head_metalib ? dr_lib_find_meta_by_name(head_metalib, "basepkg_head") : NULL;
            mem_buffer_clear_data(&pkg->m_mgr->m_dump_buff);
            dr_json_print((write_stream_t)&stream, &output_pkg->head, sizeof(BASEPKG_HEAD), head_meta, DR_JSON_PRINT_MINIMIZE, 0);
            stream_putc((write_stream_t)&stream, 0);
            
            CPE_INFO(
                em, "%s: encode: encode success!, output-capacity=%d, input-size=%d\nhead: %s",
                bpg_pkg_manage_name(pkg->m_mgr), (int)*output_capacity, (int)op_buff_size,
                (const char *)mem_buffer_make_continuous(&pkg->m_mgr->m_dump_buff, 0));
        }
    }

    return r;
}

dr_cvt_result_t bpg_pkg_encode_data(dp_req_t body, void * output, size_t * output_capacity, error_monitor_t em, int debug) {
    dr_cvt_result_t r;
    LPDRMETALIB metalib;
    LPDRMETA meta;
    uint16_t i;
    dr_cvt_t data_cvt;
    bpg_pkg_t pkg = bpg_pkg_find(body);

    char * input_data = (char *)dp_req_data(body);
    BASEPKG * input_pkg = (BASEPKG*)input_data;

    char * output_buf = output;
    size_t output_buf_capacity = *output_capacity;
    BASEPKG * output_pkg = (BASEPKG *)output_buf;

    assert(pkg);

    metalib = bpg_pkg_manage_data_metalib(pkg->m_mgr);
    if (metalib == NULL) {
        CPE_ERROR(
            em, "%s: encode:  data meta not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return dr_cvt_result_error;
    }

    data_cvt = bpg_pkg_data_cvt(pkg);
    if (data_cvt == NULL) {
        CPE_ERROR(
            em, "%s: encode:  data cvt not exist!",
            bpg_pkg_manage_name(pkg->m_mgr));
        return dr_cvt_result_error;
    }

    output_pkg->head.sn = input_pkg->head.sn;
    output_pkg->head.cmd = input_pkg->head.cmd ;
    output_pkg->head.errorNo = input_pkg->head.errorNo;
    output_pkg->head.clientId = input_pkg->head.clientId;
    output_pkg->head.flags = 0;
    output_pkg->head.headlen = sizeof(BASEPKG_HEAD);
    output_pkg->head.bodylen = 0;
    output_pkg->head.bodytotallen = 0;
    output_pkg->head.appendInfoCount = 0;

    output_buf += sizeof(input_pkg->head);
    output_buf_capacity -= sizeof(input_pkg->head);
    input_data += sizeof(input_pkg->head);

    if (input_pkg->head.bodylen) {
        if ((meta = bpg_pkg_manage_find_meta_by_cmd(pkg->m_mgr, input_pkg->head.cmd))) {
            size_t use_size = output_buf_capacity;
            size_t tmp_len = input_pkg->head.bodylen;
            r =  dr_cvt_encode(data_cvt, meta, output_buf, &use_size, input_data, &tmp_len, em, debug);
            if (r != dr_cvt_result_success) {
                CPE_ERROR(
                    em, "%s: encode: encode main meta (%s) error!",
                    bpg_pkg_manage_name(pkg->m_mgr), dr_meta_name(meta));
                return r;
            }

            output_pkg->head.bodylen = use_size;
            output_pkg->head.bodytotallen = use_size;
            output_buf += use_size;
            output_buf_capacity -= use_size;
        }
        else {
            CPE_ERROR(
                em, "%s: encode: no main meta of cmd %d!",
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
                em, "%s: encode: append %d: meta of id %d not exist in lib %s!",
                bpg_pkg_manage_name(pkg->m_mgr), i, append_info->id, dr_lib_name(metalib));
            continue;
        }

        r =  dr_cvt_encode(bpg_pkg_data_cvt(pkg), meta, output_buf, &use_size, append_data, &tmp_size, em, debug);
        if (r != dr_cvt_result_success) {
            CPE_ERROR(
                em, "%s: encode: append %d: encode append meta (%s) error, output-size=%d, input-size=%d!",
                bpg_pkg_manage_name(pkg->m_mgr), i, dr_meta_name(meta), (int)use_size, (int)tmp_size);
            continue;
        }


        o_append_info = &output_pkg->head.appendInfos[output_pkg->head.appendInfoCount++];
        o_append_info->id = append_info->id;
        o_append_info->size = use_size;
        output_pkg->head.bodytotallen += use_size;

        output_buf += use_size;
        output_buf_capacity -= use_size;
    }

    if (output_pkg->head.bodytotallen > pkg->m_mgr->m_zip_size_threshold) {
        void * zip_buff;
        uLongf ziped_size;

        mem_buffer_set_size(&pkg->m_mgr->m_zip_buff, output_pkg->head.bodytotallen);
        zip_buff = mem_buffer_make_continuous(&pkg->m_mgr->m_zip_buff, 0);

        memcpy(zip_buff, output_pkg->body, output_pkg->head.bodytotallen);

        ziped_size = *output_capacity - sizeof(BASEPKG_HEAD);
        if (compress((Bytef*)output_pkg->body, &ziped_size, zip_buff, output_pkg->head.bodytotallen) != 0) {
            CPE_ERROR(
                em, "%s: encode: zip fail, output_size=%d, input_size=%d!",
                bpg_pkg_manage_name(pkg->m_mgr), (int)ziped_size, (int)output_pkg->head.bodytotallen);
            return dr_cvt_result_error;
        }
        
        if (pkg->m_mgr->m_debug) {
            CPE_INFO(
                em, "%s: encode: zip success!, %d to %d",
                bpg_pkg_manage_name(pkg->m_mgr), (int)output_pkg->head.bodytotallen, (int)ziped_size);
        }

        output_pkg->head.flags |= BASEPKG_HEAD_FLAG_ZIP;
        output_pkg->head.bodytotallen = (uint16_t)ziped_size;
    }

    *output_capacity = sizeof(BASEPKG_HEAD) + output_pkg->head.bodytotallen;

    return 0;
}
