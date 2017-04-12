#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/binpack.h"
#include "gd/net_trans/net_trans_task.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_remoteanim_block_i.h"
#include "plugin_remoteanim_obj_i.h"

plugin_remoteanim_block_t
plugin_remoteanim_block_create(plugin_remoteanim_group_t group, const char * block_name) {
    plugin_remoteanim_module_t module = group->m_module;
    plugin_remoteanim_block_t block;

    block = TAILQ_FIRST(&module->m_free_blocks);
    if (block) {
        TAILQ_REMOVE(&module->m_free_blocks, block, m_next);
    }
    else {
        block = mem_alloc(module->m_alloc, sizeof(struct plugin_remoteanim_block));
        if (block == NULL) {
            CPE_ERROR(module->m_em, "plugin_remoteanim_block_create: alloc fail!");
            return NULL;
        }
    }

    block->m_group = group;
    block->m_state = plugin_remoteanim_block_state_init;
    block->m_trans_task = NULL;
    block->m_name = block->m_name_buf;
    block->m_size = UI_VECTOR_2_ZERO;
    block->m_place = UI_RECT_ZERO;
    cpe_str_dup(block->m_name_buf, sizeof(block->m_name_buf), block_name);
    TAILQ_INIT(&block->m_objs);

    cpe_hash_entry_init(&block->m_hh);
    if (cpe_hash_table_insert(&module->m_blocks, block) != 0) {
        CPE_ERROR(module->m_em, "plugin_remoteanim_block_create: group %s block %s insert fail!", group->m_name, block->m_name);
        block->m_group = (void*)module;
        TAILQ_INSERT_TAIL(&module->m_free_blocks, block, m_next);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&group->m_blocks, block, m_next);
    return block;
}

void plugin_remoteanim_block_free(plugin_remoteanim_block_t block) {
    plugin_remoteanim_module_t module = block->m_group->m_module;

    while(!TAILQ_EMPTY(&block->m_objs)) {
        plugin_remoteanim_obj_t obj = TAILQ_FIRST(&block->m_objs);
        
        assert(obj->m_block == block);
        TAILQ_REMOVE(&block->m_objs, obj, m_next_for_block);
        obj->m_block = NULL;
    }

    if (block->m_trans_task) {
        net_trans_task_free(block->m_trans_task);
        block->m_trans_task = NULL;
    }

    TAILQ_REMOVE(&block->m_group->m_blocks, block, m_next);
    cpe_hash_table_remove_by_ins(&module->m_blocks, block);

    TAILQ_REMOVE(&block->m_group->m_blocks, block, m_next);
    
    block->m_group = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_blocks, block, m_next);
}

void plugin_remoteanim_block_real_free(plugin_remoteanim_block_t block) {
    plugin_remoteanim_module_t module = (plugin_remoteanim_module_t)block->m_group;
    TAILQ_REMOVE(&module->m_free_blocks, block, m_next);
    mem_free(module->m_alloc, block);
}

plugin_remoteanim_block_t
plugin_remoteanim_block_find(plugin_remoteanim_group_t group, const char * name) {
    struct plugin_remoteanim_block key;
    key.m_group = group;
    key.m_name = name;
    return cpe_hash_table_find(&group->m_module->m_blocks, &key);
}

static void plugin_remoteanim_block_on_result(net_trans_task_t task, void * ctx) {
    plugin_remoteanim_block_t block = ctx;
    plugin_remoteanim_module_t module = block->m_group->m_module;
    ui_vector_2 old_size = block->m_size;
    ui_cache_pixel_buf_t pixel_buf = NULL;
    binpack_rect_t place_rect;
    ui_cache_pixel_level_info_t pixel_level_info;
    
    if (net_trans_task_result(task) != net_trans_result_ok) {
        CPE_ERROR(
            module->m_em, "remoteanim %s.%s: net trans fail, result=%s, errno=%d",
            block->m_group->m_name, block->m_name,
            net_trans_task_result_str(net_trans_task_result(task)),
            net_trans_task_errno(task));
        goto LOAD_FAIL;
    }

    pixel_buf = ui_cache_pixel_buf_create(ui_runtime_module_cache_mgr(module->m_runtime));
    if (pixel_buf == NULL) {
        CPE_ERROR(
            module->m_em, "remoteanim %s.%s: create pixel buf fail!",
            block->m_group->m_name, block->m_name);
        goto LOAD_FAIL;
    }

    if (ui_cache_pixel_buf_load_from_data(
            pixel_buf,
            mem_buffer_make_continuous(net_trans_task_buffer(task), 0),
            mem_buffer_size(net_trans_task_buffer(task)),
            module->m_em, module->m_alloc)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "remoteanim %s.%s: load data to pixel buf fail!",
            block->m_group->m_name, block->m_name);
        goto LOAD_FAIL;
    }

    pixel_level_info = ui_cache_pixel_buf_level_info_at(pixel_buf, 0);
    block->m_size.x = ui_cache_pixel_buf_level_width(pixel_level_info);
    block->m_size.y = ui_cache_pixel_buf_level_height(pixel_level_info);
    
    /*计算图片放置位置 */
    place_rect = binpack_maxrects_ctx_insert(block->m_group->m_texture_alloc, block->m_size.x, block->m_size.y, binpack_maxrects_best_short_side_fit, 0);
    if (place_rect == NULL) {
        CPE_ERROR(
            module->m_em, "remoteanim %s.%s: place fail, size=(%f,%f)",
            block->m_group->m_name, block->m_name,
            block->m_size.x, block->m_size.y);
        goto LOAD_FAIL;
    }

    block->m_place.lt.x = place_rect->x;
    block->m_place.lt.y = place_rect->y;
    block->m_place.rb.x = place_rect->x + block->m_size.x;
    block->m_place.rb.y = place_rect->y + block->m_size.y;

    if (ui_cache_texture_upload(
            block->m_group->m_texture, &block->m_place,
            ui_cache_pixel_buf_format(pixel_buf),
            ui_cache_pixel_buf_pixel_buf(pixel_buf),
            ui_cache_pixel_buf_total_buf_size(pixel_buf)) != 0)
    {
        CPE_ERROR(module->m_em, "remoteanim %s.%s: upload data fail!", block->m_group->m_name, block->m_name);
        goto LOAD_FAIL;
    }
    
    ui_cache_pixel_buf_free(pixel_buf);
    block->m_state = plugin_remoteanim_block_state_installed;
    block->m_trans_task = NULL;
    
    if (module->m_debug) {
        CPE_INFO(module->m_em, "remoteanim %s.%s: install success", block->m_group->m_name, block->m_name);
    }
    return;
    
LOAD_FAIL:
    block->m_place = UI_RECT_ZERO;
    block->m_size = old_size;
    block->m_state = plugin_remoteanim_block_state_fail;
    block->m_trans_task = NULL;
    if (pixel_buf) ui_cache_pixel_buf_free(pixel_buf);
    return;
}

int plugin_remoteanim_block_start(plugin_remoteanim_block_t block, const char * url) {
    plugin_remoteanim_module_t module = block->m_group->m_module;
    
    if (block->m_state != plugin_remoteanim_block_state_init) return 0;

    assert(block->m_trans_task == NULL);
    block->m_trans_task = net_trans_task_create(block->m_group->m_trans_group, 0);

    net_trans_task_set_commit_op(block->m_trans_task, plugin_remoteanim_block_on_result, block, NULL);
    net_trans_task_set_debug(block->m_trans_task, 1);
    /*发送请求 */
    if (net_trans_task_set_get(block->m_trans_task, url) != 0) {
        CPE_ERROR(module->m_em, "remoteanim %s.%s: set url %s fail", block->m_group->m_name, block->m_name, url);
        net_trans_task_free(block->m_trans_task);
        block->m_trans_task = NULL;
        return -1;
    }

    if (net_trans_task_start(block->m_trans_task) != 0) {
        CPE_ERROR(module->m_em, "remoteanim %s.%s: start query url %s fail", block->m_group->m_name, block->m_name, url);
        net_trans_task_free(block->m_trans_task);
        block->m_trans_task = NULL;
        return -1;
    }

    block->m_state = plugin_remoteanim_block_state_downloading;
    return 0;
}

uint32_t plugin_remoteanim_block_hash(plugin_remoteanim_block_t block) {
    return cpe_hash_str(block->m_name, strlen(block->m_name));
}

int plugin_remoteanim_block_eq(plugin_remoteanim_block_t l, plugin_remoteanim_block_t r) {
    return (l->m_group == r->m_group && strcmp(l->m_name, r->m_name) == 0) ? 1 : 0;
}
