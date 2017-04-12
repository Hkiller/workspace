#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/sorted_vector.h"
#include "cpe/utils/algorithm.h"
#include "ui_sprite_group_i.h"
#include "ui_sprite_entity_it_i.h"
#include "ui_sprite_group_binding_i.h"

struct ui_sprite_group_entity_it_data {
    mem_allocrator_t m_alloc;
    ui_sprite_world_t m_world;
    uint32_t m_cur_pos;
    uint32_t m_entity_count;
    uint32_t m_entity_capacity;
};

static ui_sprite_entity_t ui_sprite_group_entity_it_next(ui_sprite_entity_it_t it) {
    struct ui_sprite_group_entity_it_data * data = (void*)(it + 1);
    uint32_t * entities = (void*)(data + 1);

    while(data->m_cur_pos < data->m_entity_count) {
        ui_sprite_entity_t entity = ui_sprite_entity_find_by_id(data->m_world, entities[data->m_cur_pos++]);
        if (entity && !entity->m_is_wait_destory) return entity;
    }

    return NULL;
}

static void ui_sprite_group_entity_it_free(ui_sprite_entity_it_t it) {
    struct ui_sprite_group_entity_it_data * data = (void*)(it + 1);
    mem_free(data->m_alloc, it);
}

struct ui_sprite_group_entity_it_node {
    ui_sprite_group_t m_gruop;
    ui_sprite_group_binding_t m_cur_binding;
};

struct ui_sprite_group_entity_it_ctx {
    mem_allocrator_t m_alloc;
    ui_sprite_world_t m_world;
    ui_sprite_entity_it_t m_it;
    struct ui_sprite_group_entity_it_data * m_data;
    struct cpe_sorted_vector m_entities;

    struct ui_sprite_group_entity_it_node m_stack[64];
    uint8_t m_stack_count;
};

static int ui_sprite_group_entity_it_push_group(struct ui_sprite_group_entity_it_ctx * ctx, ui_sprite_group_t gruop) {
    struct ui_sprite_group_entity_it_node * node;

    if (ctx->m_stack_count + 1 >= CPE_ARRAY_SIZE(ctx->m_stack)) return -1;

    node = &ctx->m_stack[ctx->m_stack_count++];
    node->m_gruop = gruop;
    node->m_cur_binding = TAILQ_FIRST(&gruop->m_elements);

    return 0;
}

static int ui_sprite_group_entity_it_resize(struct ui_sprite_group_entity_it_ctx * ctx) {
    ui_sprite_entity_it_t new_it;
    struct ui_sprite_group_entity_it_data * new_data;
    uint32_t new_capacity = 
        ctx->m_data
        ? (ctx->m_data->m_entity_capacity * 2)
        : 64;

    new_it = mem_alloc(
        ctx->m_alloc,
        sizeof(struct ui_sprite_entity_it)
        + sizeof(struct ui_sprite_group_entity_it_data)
        + sizeof(uint32_t) * new_capacity);
    if (new_it == NULL) return -1;

    new_data = (void*)(new_it + 1);

    if (ctx->m_it == NULL) {
        new_it->m_next = ui_sprite_group_entity_it_next;
        new_it->m_free = ui_sprite_group_entity_it_free;

        new_data->m_alloc = ctx->m_alloc;
        new_data->m_world = ctx->m_world;
        new_data->m_cur_pos = 0;
        new_data->m_entity_count = 0;
        new_data->m_entity_capacity = new_capacity;
    }
    else {
        memcpy(new_it, ctx->m_it, 
               sizeof(struct ui_sprite_entity_it)
               + sizeof(struct ui_sprite_group_entity_it_data)
               + sizeof(uint32_t) * ctx->m_data->m_entity_count);
        new_data->m_entity_capacity = new_capacity;

        assert(new_data->m_entity_count == ctx->m_data->m_entity_count);

        ui_sprite_group_entity_it_free(ctx->m_it);
    }

    ctx->m_it = new_it;
    ctx->m_data = new_data;

    cpe_sorted_vector_init(
        &ctx->m_entities, 
        new_data + 1, new_data->m_entity_capacity, new_data->m_entity_count,
        sizeof(uint32_t), cpe_comap_uint32);

    return 0;
}

static int ui_sprite_group_entity_it_add_entity(struct ui_sprite_group_entity_it_ctx * ctx, uint32_t entity_id) {
    void * insert_pos;

    if (ctx->m_data->m_entity_count >= ctx->m_data->m_entity_capacity) {
        if (ui_sprite_group_entity_it_resize(ctx) != 0) return -1;
    }

    insert_pos = cpe_sorted_vector_lower_bound(&ctx->m_entities, &entity_id);
    if (insert_pos == cpe_sorted_vector_end(&ctx->m_entities)
        || *(uint32_t*)insert_pos != entity_id)
    {
        cpe_sorted_vector_insert_at(&ctx->m_entities, insert_pos, &entity_id);
        ctx->m_data->m_entity_count = (uint32_t)cpe_sorted_vector_size(&ctx->m_entities);
    }
    
    return 0;
}

ui_sprite_entity_it_t ui_sprite_group_entities(mem_allocrator_t alloc, ui_sprite_group_t group) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    struct ui_sprite_group_entity_it_node * node;
    struct ui_sprite_group_entity_it_ctx ctx;

    bzero(&ctx, sizeof(ctx));

    ctx.m_alloc = alloc;
    ctx.m_world = group->m_world;

    if (ui_sprite_group_entity_it_resize(&ctx) != 0) return NULL;

    ui_sprite_group_entity_it_push_group(&ctx, group);

    while(ctx.m_stack_count > 0) {
    PROCESS_OTHER_NODE:
        node = &ctx.m_stack[ctx.m_stack_count - 1];

        for(;
            node->m_cur_binding != TAILQ_END(&node->m_gruop->m_elements);
            node->m_cur_binding = TAILQ_NEXT(node->m_cur_binding, m_next_for_group))
        {
            if (node->m_cur_binding->m_type == ui_sprite_group_binding_type_entity) {
                if (!node->m_cur_binding->m_element.m_entity->m_is_wait_destory) {
                    if (ui_sprite_group_entity_it_add_entity(&ctx, node->m_cur_binding->m_element.m_entity->m_id) != 0) {
                        if (ui_sprite_group_entity_it_push_group(&ctx, group) != 0) {
                            CPE_ERROR(repo->m_em, "group_entities: append entity id faild!");
                            goto BUILD_ERROR;
                        }
                    }
                }
            }
            else if (node->m_cur_binding->m_type == ui_sprite_group_binding_type_group) {
                if (ui_sprite_group_entity_it_push_group(&ctx, group) != 0) {
                    CPE_ERROR(repo->m_em, "group_entities: max level reached!");
                    goto BUILD_ERROR;
                }

                node->m_cur_binding = TAILQ_NEXT(node->m_cur_binding, m_next_for_group);
                goto PROCESS_OTHER_NODE; 
            }
        }

        --ctx.m_stack_count;
    }

    return ctx.m_it;

BUILD_ERROR:
    mem_free(alloc, ctx.m_it);
    return NULL;
}

void ui_sprite_group_visit(
    ui_sprite_group_t group, ui_sprite_group_visit_fun_t visit_fun, void * visit_ctx)
{
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_entity_it_t entity_it;
    ui_sprite_entity_t entity;

    entity_it = ui_sprite_group_entities(repo->m_alloc, group);
    if (entity_it == NULL) return;

    while((entity = ui_sprite_entity_it_next(entity_it))) {
        visit_fun(group, entity, visit_ctx);
    }

    ui_sprite_entity_it_free(entity_it);
}

uint32_t ui_sprite_group_count(ui_sprite_group_t group) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_entity_it_t entity_it;
    struct ui_sprite_group_entity_it_data * data;
    uint32_t r;

    entity_it = ui_sprite_group_entities(repo->m_alloc, group);
    if (entity_it == NULL) return 0;
    
    data = (void*)(entity_it + 1);

    r = data->m_entity_count;

    ui_sprite_entity_it_free(entity_it);

    return r;
}

uint8_t ui_sprite_group_is_empty(ui_sprite_group_t group) {
    struct ui_sprite_group_entity_it_node * node;
    struct ui_sprite_group_entity_it_ctx ctx;

    bzero(&ctx, sizeof(ctx));

    ctx.m_alloc = NULL;
    ctx.m_world = group->m_world;

    ui_sprite_group_entity_it_push_group(&ctx, group);

    while(ctx.m_stack_count > 0) {
    PROCESS_OTHER_NODE:
        node = &ctx.m_stack[ctx.m_stack_count - 1];

        for(;
            node->m_cur_binding != TAILQ_END(&node->m_gruop->m_elements);
            node->m_cur_binding = TAILQ_NEXT(node->m_cur_binding, m_next_for_group))
        {
            if (node->m_cur_binding->m_type == ui_sprite_group_binding_type_entity) {
                return 0;
            }
            else if (node->m_cur_binding->m_type == ui_sprite_group_binding_type_group) {
                if (ui_sprite_group_entity_it_push_group(&ctx, group) != 0) {
                    return 0;
                }

                node->m_cur_binding = TAILQ_NEXT(node->m_cur_binding, m_next_for_group);
                goto PROCESS_OTHER_NODE; 
            }
        }

        --ctx.m_stack_count;
    }

    return 1;
}

ui_sprite_entity_t ui_sprite_group_first_entity(ui_sprite_group_t group) {
    ui_sprite_repository_t repo = group->m_world->m_repo;
    ui_sprite_entity_it_t entity_it;
    ui_sprite_entity_t entity;

    entity_it = ui_sprite_group_entities(repo->m_alloc, group);
    if (entity_it == NULL) return NULL;

    entity = ui_sprite_entity_it_next(entity_it);

    ui_sprite_entity_it_free(entity_it);

    return entity;
}
