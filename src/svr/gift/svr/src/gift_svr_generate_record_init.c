#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "cpe/dr/dr_data.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/aom/aom_obj_hash.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_stub_buff.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "gift_svr.h"

extern char g_metalib_svr_gift_pro[];

static int gift_svr_generate_record_meta_build(gift_svr_t svr) {
    struct DRInBuildMetaLib * in_build_lib;
    struct DRInBuildMeta * in_build_meta;
    struct DRInBuildMetaEntry *  in_build_entry;
    LPDRMETA src_generate_record_meta;
    LPDRMETALIB metalib;
    LPDRMETAENTRY data_first_entry;

    src_generate_record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_gift_pro, "svr_gift_generate_record");
    if (src_generate_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: meta svr_gift_generate_record not exist", gift_svr_name(svr));
        return -1;
    }

    in_build_lib = dr_inbuild_create_lib();
    if (in_build_lib == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: create builder fail", gift_svr_name(svr));
        return -1;
    }

    /*数据meta */
    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: create meta generate_record in metalib fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "generate_record");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_data_meta));

    if (dr_inbuild_meta_copy_entrys(in_build_meta, src_generate_record_meta) != 0
        || dr_inbuild_meta_copy_entrys(in_build_meta, svr->m_data_meta) != 0
        || dr_inbuild_meta_add_key_entries(in_build_meta, "_id") != 0)
    {
        CPE_ERROR(svr->m_em, "%s: install meta: copy entries fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    /*记录集合的meta */
    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: create meta generate_record_list in metalib fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "generate_record_list");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_data_meta));

    in_build_entry = dr_inbuild_meta_add_entry(in_build_meta);
    if (in_build_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: add entry in generate_record_list fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_entry_set_type(in_build_entry, "uint32");
    dr_inbuild_entry_set_name(in_build_entry, "record_count");
    dr_inbuild_entry_set_id(in_build_entry, 1);

    in_build_entry = dr_inbuild_meta_add_entry(in_build_meta);
    if (in_build_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: add entry in generate_record_list fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_entry_set_type(in_build_entry, "generate_record");
    dr_inbuild_entry_set_name(in_build_entry, "records");
    dr_inbuild_entry_set_array_count(in_build_entry, 0);
    dr_inbuild_entry_set_array_refer(in_build_entry, "record_count");
    dr_inbuild_entry_set_id(in_build_entry, 2);

    /*创建lib */
    if (dr_inbuild_build_lib(&svr->m_record_metalib, in_build_lib, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: build record metalib fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    metalib = (LPDRMETALIB)mem_buffer_make_continuous(&svr->m_record_metalib, 0);
    svr->m_generate_record_meta = dr_lib_find_meta_by_name(metalib, "generate_record");
    if (svr->m_generate_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: find record in metalib fail", gift_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    /*初始化generate_record相关的数据 */
    /*    id*/
    svr->m_generate_record_id_entry = dr_meta_find_entry_by_name(svr->m_generate_record_meta, "_id");
    assert(svr->m_generate_record_id_entry);
    svr->m_generate_record_id_start_pos = dr_entry_data_start_pos(svr->m_generate_record_id_entry, 0);
    svr->m_generate_record_id_capacity = dr_entry_element_size(svr->m_generate_record_id_entry);

    /*    first entry*/
    data_first_entry = dr_meta_find_entry_by_name(svr->m_generate_record_meta, dr_entry_name(dr_meta_entry_at(svr->m_data_meta, 0)));
    assert(data_first_entry);
    svr->m_generate_record_data_start_pos = dr_entry_data_start_pos(data_first_entry, 0);

    /*初始化generate_record_list相关的数据 */
    svr->m_generate_record_list_meta = dr_lib_find_meta_by_name(metalib, "generate_record_list");
    assert(svr->m_generate_record_list_meta);

    svr->m_generate_record_size = dr_meta_size(svr->m_generate_record_meta);
    svr->m_generate_record_list_count_entry = dr_meta_find_entry_by_name(svr->m_generate_record_list_meta, "record_count");
    assert(svr->m_generate_record_list_count_entry);
    svr->m_generate_record_list_data_entry = dr_meta_find_entry_by_name(svr->m_generate_record_list_meta, "records");
    assert(svr->m_generate_record_list_data_entry);

    dr_inbuild_free_lib(in_build_lib);

    return 0;
}

static int gift_svr_init_generate_record_mgr(gift_svr_t svr, uint32_t generate_record_count, float bucket_ratio) {
    size_t generate_record_buff_capacity;
    set_svr_stub_buff_t generate_record_buff;
    size_t hash_table_buff_capacity;
    set_svr_stub_buff_t hash_table_buff;

    assert(svr->m_generate_record_mgr == NULL);
    assert(svr->m_generate_record_hash == NULL);

    /*初始化记录数组 */
    if (aom_obj_mgr_buf_calc_capacity(&generate_record_buff_capacity, svr->m_generate_record_meta, generate_record_count, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: generate_record init: calc buf capacity fail. generate_record_count=%d!",
            gift_svr_name(svr), generate_record_count);
        return -1;
    }

    generate_record_buff = set_svr_stub_buff_check_create(svr->m_stub, "generate_record_buff", generate_record_buff_capacity);
    if (generate_record_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: generate_record init: create generate_record_buff fail, capacity=%d!",
            gift_svr_name(svr), (int)generate_record_buff_capacity);
        return -1;
    }

    if (!set_svr_stub_buff_is_init(generate_record_buff)) {
        if (aom_obj_mgr_buf_init(
                svr->m_generate_record_meta,
                set_svr_stub_buff_data(generate_record_buff), set_svr_stub_buff_capacity(generate_record_buff), svr->m_em)
            != 0)
        {
            CPE_ERROR(svr->m_em,  "%s: generate_record init: init generate_record buf fail!", gift_svr_name(svr));
            return -1;
        }
        set_svr_stub_buff_set_init(generate_record_buff, 1);
    }

    svr->m_generate_record_mgr = aom_obj_mgr_create(svr->m_alloc, set_svr_stub_buff_data(generate_record_buff), set_svr_stub_buff_capacity(generate_record_buff), svr->m_em);
    if (svr->m_generate_record_mgr == NULL) {
        CPE_ERROR(svr->m_em,  "%s: generate_record init: create aom obj mgr fail!", gift_svr_name(svr));
        return -1;
    }

    /*初始化记录hash表 */
    hash_table_buff_capacity = aom_obj_hash_table_buf_calc_capacity(svr->m_generate_record_mgr, bucket_ratio);

    hash_table_buff = set_svr_stub_buff_check_create(svr->m_stub, "hash_table_buff", hash_table_buff_capacity);
    if (hash_table_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: generate_record init: create hash_table_buff fail, capacity=%d!",
            gift_svr_name(svr), (int)hash_table_buff_capacity);
        return -1;
    }

    if (!set_svr_stub_buff_is_init(hash_table_buff) || aom_obj_mgr_allocked_obj_count(svr->m_generate_record_mgr) == 0) {
        if (aom_obj_hash_table_buf_init(
                svr->m_generate_record_mgr, bucket_ratio,
                dr_meta_key_hash,
                set_svr_stub_buff_data(hash_table_buff), set_svr_stub_buff_capacity(hash_table_buff),
                svr->m_em)
            != 0)
        {
            CPE_ERROR(svr->m_em,  "%s: generate_record init: init hash table buff fail!", gift_svr_name(svr));
            return -1;
        }

        set_svr_stub_buff_set_init(hash_table_buff, 1);
    }

    svr->m_generate_record_hash =
        aom_obj_hash_table_create(
            svr->m_alloc, svr->m_em,
            svr->m_generate_record_mgr, dr_meta_key_hash, dr_meta_key_cmp,
            set_svr_stub_buff_data(hash_table_buff), set_svr_stub_buff_capacity(hash_table_buff));
    if (svr->m_generate_record_hash == NULL) {
        CPE_ERROR(svr->m_em,  "%s: generate_record init: create aom hash table fail!", gift_svr_name(svr));
        return -1;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em,  "%s: generate_record init: success, generate_record-size=%d, generate_record-count=%d, generate_record-buf=%.2fm, hash-buf=%.2fm!",
            gift_svr_name(svr), (int)dr_meta_size(aom_obj_mgr_meta(svr->m_generate_record_mgr)),
            aom_obj_mgr_allocked_obj_count(svr->m_generate_record_mgr),
            generate_record_buff_capacity / 1024.0 / 1024.0, hash_table_buff_capacity / 1024.0 / 1024.0);
    }
    
    return 0;
}

int gift_svr_generate_record_init(gift_svr_t svr, LPDRMETA data_meta, uint32_t generate_record_count, float bucket_ratio) {
    assert(svr->m_data_meta == NULL);

    svr->m_data_meta = data_meta;
    svr->m_data_size = dr_meta_size(svr->m_data_meta);
    
    if (gift_svr_generate_record_meta_build(svr) != 0) return -1;
    if (gift_svr_init_generate_record_mgr(svr, generate_record_count, bucket_ratio) != 0) return -1;
    return 0;
}
