#include "appsvr_share_request_block_i.h"

appsvr_share_request_block_t
appsvr_share_request_block_create(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category, void const * data, size_t data_size)
{
    appsvr_share_module_t module = request->m_module;
    appsvr_share_request_block_t block;

    block = TAILQ_FIRST(&module->m_free_request_blocks);
    if (block) {
        TAILQ_REMOVE(&module->m_free_request_blocks, block, m_next);
    }
    else {
        block = mem_calloc(module->m_alloc, sizeof(struct appsvr_share_request_block));
        if (block == NULL) {
            CPE_ERROR(module->m_em, "appsvr_share_request_block_create_from_data: alloc fail!");
            return NULL;
        }
    }

    block->m_request = request;
    block->m_category = category;
    block->m_data = mem_alloc(module->m_alloc, data_size);
    if (block->m_data == NULL) {
        CPE_ERROR(module->m_em, "appsvr_share_request_block_create_from_data: alloc data fail, size=%d!", (int)data_size);
        block->m_request = (appsvr_share_request_t)module;
        TAILQ_INSERT_TAIL(&module->m_free_request_blocks, block, m_next);
        return NULL;
    }
    memcpy(block->m_data, data, data_size);
    block->m_data_size = data_size;
    
    TAILQ_INSERT_TAIL(&request->m_blocks, block, m_next);
    return block;
}

void appsvr_share_request_block_free(appsvr_share_request_block_t request_block) {
    appsvr_share_module_t module = request_block->m_request->m_module;

    mem_free(module->m_alloc, request_block->m_data);
    
    TAILQ_REMOVE(&request_block->m_request->m_blocks, request_block, m_next);
    
    request_block->m_request = (appsvr_share_request_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_request_blocks, request_block, m_next);
}

void appsvr_share_request_block_real_free(appsvr_share_request_block_t request_block) {
    appsvr_share_module_t module = (appsvr_share_module_t)request_block->m_request;

    TAILQ_REMOVE(&module->m_free_request_blocks, request_block, m_next);
    mem_free(module->m_alloc, request_block);
}

appsvr_share_request_block_category_t
appsvr_share_request_block_category(appsvr_share_request_block_t request_block) {
    return request_block->m_category;
}

void * appsvr_share_request_block_data(appsvr_share_request_block_t request_block) {
    return request_block->m_data;
}

size_t appsvr_share_request_block_data_size(appsvr_share_request_block_t request_block) {
    return request_block->m_data_size;
}
    

static appsvr_share_request_block_t appsvr_share_request_block_next(struct appsvr_share_request_block_it * it) {
    appsvr_share_request_block_t * data = (appsvr_share_request_block_t *)(it->m_data);
    appsvr_share_request_block_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next);
    return r;
}

void appsvr_share_request_blocks(appsvr_share_request_t request, appsvr_share_request_block_it_t it) {
    *(appsvr_share_request_block_t *)(it->m_data) = TAILQ_FIRST(&request->m_blocks);
    it->next = appsvr_share_request_block_next;
}

struct appsvr_share_request_block_in_category_ctx {
    appsvr_share_request_block_category_t m_category;
    appsvr_share_request_block_t m_block;
};

static appsvr_share_request_block_t appsvr_share_request_block_in_category_next(struct appsvr_share_request_block_it * it) {
    struct appsvr_share_request_block_in_category_ctx * data = (struct appsvr_share_request_block_in_category_ctx *)(it->m_data);
    appsvr_share_request_block_t r;

    if (data->m_block == NULL) return NULL;
    
    r = data->m_block;

    for(data->m_block = TAILQ_NEXT(data->m_block, m_next);
        data->m_block;
        data->m_block = TAILQ_NEXT(data->m_block, m_next))
    {
        if (data->m_block->m_category == data->m_category) break;
    }
    
    return r;
}

void appsvr_share_request_blocks_in_category(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category,
    appsvr_share_request_block_it_t it)
{
    struct appsvr_share_request_block_in_category_ctx * data = (struct appsvr_share_request_block_in_category_ctx *)(it->m_data);
    data->m_category = category;
    data->m_block = TAILQ_FIRST(&request->m_blocks);
    while(data->m_block && data->m_block->m_category != category) {
        data->m_block = TAILQ_NEXT(data->m_block, m_next);
    }
    it->next = appsvr_share_request_block_in_category_next;
}

appsvr_share_request_block_t
appsvr_share_request_block_find_first(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category)
{
    appsvr_share_request_block_t block;

    TAILQ_FOREACH(block, &request->m_blocks, m_next) {
        if (block->m_category == category) return block;
    }

    return NULL;
}

const char * appsvr_share_request_block_get_str(
    appsvr_share_request_t request, appsvr_share_request_block_category_t category,
    uint8_t index, const char * dft)
{
    appsvr_share_request_block_t block;

    TAILQ_FOREACH(block, &request->m_blocks, m_next) {
        if (block->m_category == category) {
            if (index == 0) {
                return (const char *)block->m_data;
            }
            else {
                index--;
            }
        }
    }

    return dft;
}
