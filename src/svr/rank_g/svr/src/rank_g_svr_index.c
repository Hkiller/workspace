#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "rank_g_svr_index.h"
#include "rank_g_svr_season_info.h"
#include "rank_g_svr_rank_tree.h"

rank_g_svr_index_t rank_g_svr_index_find(rank_g_svr_t svr, uint16_t id) {
    rank_g_svr_index_t index;

    TAILQ_FOREACH(index, &svr->m_indexs, m_next) {
        if (index->m_id == id) return index;
    }

    return NULL;
}

rank_g_svr_index_t
rank_g_svr_index_create(rank_g_svr_t svr, uint16_t id, const char * entry_path) {
    rank_g_svr_index_t index;
    int start_pos;

    index = mem_alloc(svr->m_alloc, sizeof(struct rank_g_svr_index));
    if (index == NULL) {
        CPE_ERROR(svr->m_em, "%s: create index %d: alloc fail!", rank_g_svr_name(svr), id);
        return NULL;
    }

    index->m_rank_entry = dr_meta_find_entry_by_path_ex(svr->m_record_meta, entry_path, &start_pos);
    if (index->m_rank_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: create index %d: entry %s not exist!", rank_g_svr_name(svr), id, entry_path);
        mem_free(svr->m_alloc, index);
        return NULL;
    }
    index->m_rank_start_pos = start_pos;

    index->m_svr = svr;
    index->m_id = id;
    index->m_season_use = 0;
    index->m_season_keep_count = 0;
    index->m_season_entry = NULL;
    index->m_season_entry_start_pos = 0;
    index->m_season_cur = NULL;
    index->m_record_season = 0;
    index->m_record_to_rank_pos = NULL;
    index->m_rank_tree = NULL;
    index->m_rank_tree_buff = NULL;

    TAILQ_INIT(&index->m_season_infos);

    svr->m_index_count++;
    TAILQ_INSERT_TAIL(&svr->m_indexs, index, m_next);
    
    return index;
}

void rank_g_svr_index_free(rank_g_svr_index_t index) {
    rank_g_svr_t svr = index->m_svr;
    
    while(!TAILQ_EMPTY(&index->m_season_infos)) {
        rank_g_svr_season_info_free(TAILQ_FIRST(&index->m_season_infos));
    }

    if (index->m_record_to_rank_pos) {
        mem_free(svr->m_alloc, index->m_record_to_rank_pos);
    }

    if (index->m_rank_tree) {
        rt_free(index->m_rank_tree);
    }

    if (index->m_rank_tree_buff) {
        mem_free(svr->m_alloc, index->m_rank_tree_buff);
    }

    assert(svr->m_index_count > 0);
    svr->m_index_count--;
    TAILQ_REMOVE(&svr->m_indexs, index, m_next);
    mem_free(svr->m_alloc, index);
}

int rank_g_svr_index_set_season_info(rank_g_svr_index_t index, uint16_t keep_count, const char * season_attr) {
    rank_g_svr_t svr = index->m_svr;
    int start_pos;
    LPDRMETAENTRY  season_entry;
    
    season_entry = dr_meta_find_entry_by_path_ex(svr->m_record_meta, season_attr, &start_pos);
    if (season_entry == NULL) {
        CPE_ERROR(svr->m_em, "%s: index %d: season entry %s not exist!", rank_g_svr_name(svr), index->m_id, season_attr);
        return -1;
    }

    index->m_season_use = 1;
    index->m_season_keep_count = keep_count;
    index->m_season_entry = season_entry;
    index->m_season_entry_start_pos = (uint16_t)start_pos;
    
    return 0;
}

int rank_g_svr_index_init_record(rank_g_svr_index_t index, uint32_t record_count) {
    rank_g_svr_t svr = index->m_svr;
    size_t rt_capacity;
    uint32_t i;
    struct aom_obj_it record_it;
    void * record;
    
    if (index->m_record_to_rank_pos) {
        mem_free(svr->m_alloc, index->m_record_to_rank_pos);
        index->m_record_to_rank_pos = NULL;
    }

    if (index->m_rank_tree) {
        rt_free(index->m_rank_tree);
        index->m_rank_tree = NULL;
    }

    if (index->m_rank_tree_buff) {
        mem_free(svr->m_alloc, index->m_rank_tree_buff);
        index->m_rank_tree_buff = NULL;
    }
    
    index->m_record_to_rank_pos = mem_alloc(svr->m_alloc, sizeof(uint32_t) * record_count);
    if (index->m_record_to_rank_pos == NULL) {
        CPE_ERROR(svr->m_em, "%s: index %d: alloc fail, record count = %d", rank_g_svr_name(svr), index->m_id, record_count);
        return -1;
    }
    for(i = 0; i < record_count; ++i) {
        index->m_record_to_rank_pos[i] = INVALID_RANK_TREE_NODE_POS;
    }

    rt_capacity = rt_buff_calc_capacity(record_count);
    index->m_rank_tree_buff = mem_alloc(svr->m_alloc, rt_capacity);
    if (index->m_rank_tree_buff == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: alloc fail, record count = %d, capacity=%d",
            rank_g_svr_name(svr), index->m_id, record_count, (int)rt_capacity);

        mem_free(svr->m_alloc, index->m_record_to_rank_pos);
        index->m_record_to_rank_pos = NULL;

        return -1;
    }

    if (rt_buff_init(svr->m_em, record_count, index->m_rank_tree_buff, rt_capacity) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: alloc fail, record count = %d, capacity=%d",
            rank_g_svr_name(svr), index->m_id, record_count, (int)rt_capacity);

        mem_free(svr->m_alloc, index->m_record_to_rank_pos);
        index->m_record_to_rank_pos = NULL;

        mem_free(svr->m_alloc, index->m_rank_tree_buff);
        index->m_rank_tree_buff = NULL;

        return -1;
    }
    
    index->m_rank_tree = rt_create(svr, index->m_rank_tree_buff, rt_capacity);
    if (index->m_rank_tree == NULL) {
        CPE_ERROR(svr->m_em, "%s: index %d: create rank tree fail", rank_g_svr_name(svr), index->m_id);
        
        mem_free(svr->m_alloc, index->m_record_to_rank_pos);
        index->m_record_to_rank_pos = NULL;
        
        mem_free(svr->m_alloc, index->m_rank_tree_buff);
        index->m_rank_tree_buff = NULL;

        return -1;
    }

    aom_objs(svr->m_record_mgr, &record_it);
    while((record = aom_obj_it_next(&record_it))) {
        uint32_t record_idx = (uint32_t)aom_obj_index(svr->m_record_mgr, record);

        if (index->m_season_use) {
            uint16_t season;
            
            if (dr_entry_try_read_uint16(
                    &season,
                    ((const char *)record) + index->m_season_entry_start_pos, index->m_season_entry, svr->m_em)
                != 0)
            {
                CPE_INFO(
                    svr->m_em, "%s: index %d: init: record %s: read season attr fail!",
                    rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record));
                continue;
            }

            if (season == 0) continue;

            if (index->m_record_season == season) {
            }
            else if (index->m_record_season == 0) {
                index->m_record_season = season; /*启动存储流程 */
            }
            else {
                CPE_INFO(
                    svr->m_em, "%s: index %d: init: record %s: season %d is not curent season %d or record season %d!",
                    rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record),
                    season, index->m_season_cur->m_season_id, index->m_record_season);
                continue;
            }
        }
        
        if (rank_g_svr_index_update(index, record, record_idx) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: index %d: init: record[%d]: %s",
                rank_g_svr_name(svr), index->m_id, record_idx, rank_g_svr_record_dump(svr, record));
            return -1;
        }
    }

    if (rt_size(index->m_rank_tree) == 0 && index->m_season_cur) {
        assert(index->m_record_season == 0);
        index->m_record_season = index->m_season_cur->m_season_id;
    }
    
    CPE_INFO(
        svr->m_em, "%s: index %d(%s): init success, ",
        rank_g_svr_name(svr), index->m_id, dr_entry_name(index->m_rank_entry));
    
    return 0;
}

int rank_g_svr_index_season_check_update(rank_g_svr_index_t index, void const * record) {
    rank_g_svr_t svr = index->m_svr;
    uint16_t season;
    uint32_t cur_time;
    
    assert(index->m_season_use);

    if (index->m_season_cur == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: season check update: record %s: no season exist!",
            rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record));
        return SVR_RANK_G_ERROR_SEASON_NOT_EXIST;
    }

    if (index->m_record_season != index->m_season_cur->m_season_id) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: season check update: seasion %d is saving, next season = %d",
            rank_g_svr_name(svr), index->m_id, index->m_record_season, index->m_season_cur->m_season_id);
        return SVR_RANK_G_ERROR_SEASON_SAVING;
    }
    
    if (dr_entry_try_read_uint16(
            &season,
            ((const char *)record) + index->m_season_entry_start_pos, index->m_season_entry, svr->m_em)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: index %d: season check update: record %s: read season attr fail!",
            rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record));
        return -1;
    }

    if (season == 0) return 0;

    if (index->m_season_cur->m_season_id != season) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: season check update: record %s: curent season is %d, mismatch!",
            rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record), index->m_season_cur->m_season_id);
        return SVR_RANK_G_ERROR_SEASON_MISMATCH;
    }

    cur_time = rank_g_svr_cur_time(svr);
    if (cur_time < index->m_season_cur->m_begin_time) {
        CPE_ERROR(
            svr->m_em, "%s: index %d: season check update: record %s: curent season %d not start, cur-time=%d, start-time=%d!",
            rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record), index->m_season_cur->m_season_id,
            cur_time, index->m_season_cur->m_begin_time);
        return SVR_RANK_G_ERROR_SEASON_NOT_BEGIN;
    }

    if (cur_time >= index->m_season_cur->m_end_time) {
        rank_g_svr_season_info_t next_season = TAILQ_NEXT(index->m_season_cur, m_next);
        if (next_season == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: index %d: season check update: record %s: no next season!",
                rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record));
            return SVR_RANK_G_ERROR_SEASON_NOT_EXIST;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: index %d: season check update: record %s: curent season %d not start, cur-time=%d, next season start-time=%d!",
                rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record), index->m_season_cur->m_season_id,
                cur_time, next_season->m_begin_time);
            return SVR_RANK_G_ERROR_SEASON_NOT_BEGIN;
        }
    }
    
    return 0;
}

int rank_g_svr_index_update(rank_g_svr_index_t index, void const * record, uint32_t record_id) {
    rank_g_svr_t svr = index->m_svr;
    uint32_t rank_value;

    if (dr_entry_try_read_uint32(
            &rank_value,
            ((const char *)record) + index->m_rank_start_pos, index->m_rank_entry, svr->m_em)
        != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: index %d: update record %s: read sort attr fail!",
            rank_g_svr_name(svr), index->m_id, rank_g_svr_record_dump(svr, record));

        return -1;
    }
    
    if (index->m_season_use) {
        uint16_t season;
        
        if (index->m_season_cur == NULL) {
            if (svr->m_debug) {
                CPE_INFO(svr->m_em, "%s: index %d: no cur season, ignore!", rank_g_svr_name(svr), index->m_id);
            }
            return 0;
        }
        
        dr_entry_try_read_uint16(
            &season,
            ((const char *)record) + index->m_season_entry_start_pos, index->m_season_entry, svr->m_em);

        if (season != index->m_record_season) {
            if (svr->m_debug) {
                CPE_INFO(
                    svr->m_em, "%s: index %d: record season %d mismatch with record %s!",
                    rank_g_svr_name(svr), index->m_id, index->m_record_season, rank_g_svr_record_dump(svr, record));
            }
            return 0;
        }
    }

    if (index->m_record_to_rank_pos[record_id] != INVALID_RANK_TREE_NODE_POS) { /*已经记录在排行榜中 */
        rt_node_t rank_node;

        rank_node = rt_node_get(index->m_rank_tree, index->m_record_to_rank_pos[record_id]);
        assert(rank_node);
        assert(rt_node_record_id(rank_node) == record_id);

        if (rt_node_value(rank_node) == rank_value) {
            if (svr->m_debug) {
                CPE_INFO(svr->m_em, "%s: index %d: rank value no change, skip!", rank_g_svr_name(svr), index->m_id);
            }
            return 0;
        }

        rt_erase(index->m_rank_tree, rank_node);
        index->m_record_to_rank_pos[record_id] = INVALID_RANK_TREE_NODE_POS;
    }
    
    if (rt_size(index->m_rank_tree) < rt_capacity(index->m_rank_tree)) {
        /*排行榜没有满，新插入一个 */
        rt_node_t rank_node = rt_insert(index->m_rank_tree, rank_value, record_id);
        assert(rank_node);

        index->m_record_to_rank_pos[record_id] = rt_node_idx(index->m_rank_tree, rank_node);
        return 0;
    }
    else {
        /*和最后一个比较 */
        rt_node_t last_node = rt_last(index->m_rank_tree);
        assert(last_node);

        if (rt_node_value(last_node) > rank_value) {
            /*超出范围，则返回排行榜满了 */
            return SVR_RANK_G_ERROR_FULL;
        }
        else {
            /*否则，替换最后一个 */
            index->m_record_to_rank_pos[rt_node_record_id(last_node)] = INVALID_RANK_TREE_NODE_POS;
            rt_erase(index->m_rank_tree, last_node);


            /*插入新节点 */
            last_node = rt_insert(index->m_rank_tree, rank_value, record_id);
            assert(last_node);
            index->m_record_to_rank_pos[record_id] = rt_node_idx(index->m_rank_tree, last_node);

            return 0;
        }
    }
}

void rank_g_svr_index_remove(rank_g_svr_index_t index, uint32_t record_id) {
    if (index->m_record_to_rank_pos[record_id] != INVALID_RANK_TREE_NODE_POS) {
        /*已经记录在排行榜中 */
        rt_node_t rank_node;

        rank_node = rt_node_get(index->m_rank_tree, index->m_record_to_rank_pos[record_id]);
        assert(rank_node);
        assert(rt_node_record_id(rank_node) == record_id);

        rt_erase(index->m_rank_tree, rank_node);
        index->m_record_to_rank_pos[record_id] = INVALID_RANK_TREE_NODE_POS;
    }

    index->m_record_to_rank_pos[record_id] = INVALID_RANK_TREE_NODE_POS;
}
