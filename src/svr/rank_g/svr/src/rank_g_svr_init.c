#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_build.h"
#include "rank_g_svr.h"

extern char g_metalib_svr_rank_g_pro[];

static int rank_g_svr_season_record_meta_build(rank_g_svr_t svr) {
    struct DRInBuildMetaLib * in_build_lib;
    struct DRInBuildMeta * in_build_meta;
    struct DRInBuildMetaEntry *  in_build_entry;
    LPDRMETA src_season_record_meta;
    LPDRMETALIB metalib;
    LPDRMETAENTRY data_first_entry;

    src_season_record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_rank_g_pro, "svr_rank_g_season_record_common");
    if (src_season_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: meta svr_rank_g_season_record_common not exist", rank_g_svr_name(svr));
        return -1;
    }

    in_build_lib = dr_inbuild_create_lib();
    if (in_build_lib == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: create builder fail", rank_g_svr_name(svr));
        return -1;
    }

    /*数据meta */
    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: create meta season_record in metalib fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "season_record");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_record_meta));

    if (dr_inbuild_metalib_copy_meta_ref_metas(in_build_lib, svr->m_record_meta) != 0
        || dr_inbuild_meta_copy_entrys(in_build_meta, src_season_record_meta) != 0
        || dr_inbuild_meta_copy_entrys(in_build_meta, svr->m_record_meta) != 0
        || dr_inbuild_meta_add_key_entries(in_build_meta, "_id") != 0)
    {
        CPE_ERROR(svr->m_em, "%s: install meta: copy entries fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    /*记录集合的meta */
    in_build_meta = dr_inbuild_metalib_add_meta(in_build_lib);
    if (in_build_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: create meta season_record_list in metalib fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_meta_set_type(in_build_meta, CPE_DR_TYPE_STRUCT);
    dr_inbuild_meta_set_name(in_build_meta, "season_record_list");
    dr_inbuild_meta_set_align(in_build_meta, dr_meta_align(svr->m_record_meta));

    in_build_entry = dr_inbuild_meta_add_entry(in_build_meta);
    if (in_build_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: add entry in season_record_list fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_entry_set_type(in_build_entry, "uint32");
    dr_inbuild_entry_set_name(in_build_entry, "record_count");
    dr_inbuild_entry_set_id(in_build_entry, 1);

    in_build_entry = dr_inbuild_meta_add_entry(in_build_meta);
    if (in_build_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: add entry in season_record_list fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    dr_inbuild_entry_set_type(in_build_entry, "season_record");
    dr_inbuild_entry_set_name(in_build_entry, "records");
    dr_inbuild_entry_set_array_count(in_build_entry, 0);
    dr_inbuild_entry_set_array_refer(in_build_entry, "record_count");
    dr_inbuild_entry_set_id(in_build_entry, 2);

    /*创建lib */
    if (dr_inbuild_tsort(in_build_lib, svr->m_em) != 0
        || dr_inbuild_build_lib(&svr->m_gen_metalib, in_build_lib, svr->m_em) != 0)
    {
        CPE_ERROR(svr->m_em, "%s: create: build record meta: build record metalib fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    metalib = (LPDRMETALIB)mem_buffer_make_continuous(&svr->m_gen_metalib, 0);
    svr->m_season_record_meta = dr_lib_find_meta_by_name(metalib, "season_record");
    if (svr->m_season_record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: install meta: find record in metalib fail", rank_g_svr_name(svr));
        dr_inbuild_free_lib(in_build_lib);
        return -1;
    }

    /*初始化season_record相关的数据 */
    /*    first entry*/
    data_first_entry = dr_meta_find_entry_by_name(svr->m_season_record_meta, dr_entry_name(dr_meta_entry_at(svr->m_record_meta, 0)));
    assert(data_first_entry);
    svr->m_season_record_data_start_pos = dr_entry_data_start_pos(data_first_entry, 0);

    /*初始化season_record_list相关的数据 */
    svr->m_season_record_list_meta = dr_lib_find_meta_by_name(metalib, "season_record_list");
    assert(svr->m_season_record_list_meta);

    svr->m_season_record_size = dr_meta_size(svr->m_season_record_meta);
    svr->m_season_record_list_count_entry = dr_meta_find_entry_by_name(svr->m_season_record_list_meta, "record_count");
    assert(svr->m_season_record_list_count_entry);
    svr->m_season_record_list_data_entry = dr_meta_find_entry_by_name(svr->m_season_record_list_meta, "records");
    assert(svr->m_season_record_list_data_entry);

    dr_inbuild_free_lib(in_build_lib);

    return 0;
}

int rank_g_svr_set_record_meta(rank_g_svr_t svr, LPDRMETA meta) {
    if (dr_meta_key_entry_num(meta) != 1) {
        CPE_ERROR(
            svr->m_em, "%s: create: meta %s key attr count error, require 1, but is %d.",
            rank_g_svr_name(svr), dr_meta_name(meta), dr_meta_key_entry_num(meta));
        return -1;
    }

    svr->m_record_meta = meta;
    svr->m_record_size = dr_meta_size(meta);
    svr->m_uin_entry = dr_meta_key_entry_at(meta, 0);
    svr->m_uin_start_pos = (uint32_t)dr_entry_data_start_pos(svr->m_uin_entry, 0);

    if (rank_g_svr_season_record_meta_build(svr) != 0) return -1;
    
    return 0;
}
