#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "binpack_maxrects_i.h"

binpack_rect_t binpack_maxrects_ctx_push_free_rect(binpack_maxrects_ctx_t ctx, binpack_rect_t rect) {
    binpack_rect_t r;

    if (ctx->m_free_rects_count + 1 > ctx->m_free_rects_capacity) {
        uint32_t new_capacity = ctx->m_free_rects_count < 64 ? 64 : ctx->m_free_rects_count * 2;
        struct binpack_rect * new_buf = mem_alloc(ctx->m_alloc, sizeof(struct binpack_rect) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(ctx->m_em, "binpack_maxrects_ctx_push_free_rect: alloc new buf fail, count=%d", new_capacity);
            return NULL;
        }

        if (ctx->m_free_rects) {
            memcpy(new_buf, ctx->m_free_rects, sizeof(struct binpack_rect) * ctx->m_free_rects_count);
            mem_free(ctx->m_alloc, ctx->m_free_rects);
        }

        ctx->m_free_rects_capacity = new_capacity;
        ctx->m_free_rects = new_buf;
    }

    r = &ctx->m_free_rects[ctx->m_free_rects_count++];

    *r = *rect;

    return r;
}

void binpack_maxrects_ctx_erase_free_rect(binpack_maxrects_ctx_t ctx, uint32_t idx) {
    assert(idx >= 0 && idx < ctx->m_free_rects_count);
    assert(ctx->m_free_rects);
    
    memmove(
        ctx->m_free_rects + idx, ctx->m_free_rects + idx + 1,
        sizeof(struct binpack_rect) * (ctx->m_free_rects_count - idx - 1));

    ctx->m_free_rects_count--;
}

binpack_rect_t binpack_maxrects_ctx_push_used_rect(binpack_maxrects_ctx_t ctx, binpack_rect_t rect) {
    binpack_rect_t r;

    if (ctx->m_used_rects_count + 1 > ctx->m_used_rects_capacity) {
        uint32_t new_capacity = ctx->m_used_rects_count < 64 ? 64 : ctx->m_used_rects_count * 2;
        struct binpack_rect * new_buf = mem_alloc(ctx->m_alloc, sizeof(struct binpack_rect) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(ctx->m_em, "binpack_maxrects_ctx_push_used_rect: alloc new buf fail, count=%d", new_capacity);
            return NULL;
        }

        if (ctx->m_used_rects) {
            memcpy(new_buf, ctx->m_used_rects, sizeof(struct binpack_rect) * ctx->m_used_rects_count);
            mem_free(ctx->m_alloc, ctx->m_used_rects);
        }

        ctx->m_used_rects_capacity = new_capacity;
        ctx->m_used_rects = new_buf;
    }

    r = &ctx->m_used_rects[ctx->m_used_rects_count++];

    *r = *rect;

    return r;
}

void binpack_maxrects_ctx_erase_used_rect(binpack_maxrects_ctx_t ctx, uint32_t idx) {
    assert(idx >= 0 && idx < ctx->m_used_rects_count);
    assert(ctx->m_used_rects);
    
    memmove(
        ctx->m_used_rects + idx, ctx->m_used_rects + idx + 1,
        sizeof(struct binpack_rect) * ctx->m_used_rects_count - idx - 1);
    ctx->m_used_rects_count--;
}

uint8_t binpack_rect_is_contained_in(binpack_rect_t a, binpack_rect_t b) {
	return a->x >= b->x && a->y >= b->y 
		&& a->x+a->width <= b->x+b->width 
		&& a->y+a->height <= b->y+b->height;
}

uint8_t binpack_maxrects_ctx_split_free_node(binpack_maxrects_ctx_t ctx, binpack_rect_t i_free_node, binpack_rect_t used_node) {
    struct binpack_rect free_node = *i_free_node;
    
	/* Test with SAT if the rectangles even intersect. */
	if (used_node->x >= free_node.x + free_node.width
        || used_node->x + used_node->width <= free_node.x
        || used_node->y >= free_node.y + free_node.height
        || used_node->y + used_node->height <= free_node.y)
    {
		return 0;
    }

	if (used_node->x < free_node.x + free_node.width && used_node->x + used_node->width > free_node.x) {
		/* New node at the top side of the used node.*/
		if (used_node->y > free_node.y && used_node->y < free_node.y + free_node.height) {
			struct binpack_rect new_node = free_node;
			new_node.height = used_node->y - new_node.y;
            if (binpack_maxrects_ctx_push_free_rect(ctx, &new_node) == NULL) return 0;
		}

		/* New node at the bottom side of the used node.*/
		if (used_node->y + used_node->height < free_node.y + free_node.height) {
			struct binpack_rect new_node = free_node;
			new_node.y = used_node->y + used_node->height;
			new_node.height = free_node.y + free_node.height - (used_node->y + used_node->height);
            if (binpack_maxrects_ctx_push_free_rect(ctx, &new_node) == NULL) return 0;
		}
	}

	if (used_node->y < free_node.y + free_node.height && used_node->y + used_node->height > free_node.y) {
		/* New node at the left side of the used node.*/
		if (used_node->x > free_node.x && used_node->x < free_node.x + free_node.width) {
			struct binpack_rect new_node = free_node;
			new_node.width = used_node->x - new_node.x;
            if (binpack_maxrects_ctx_push_free_rect(ctx, &new_node) == NULL) return 0;
		}

		/* New node at the right side of the used node.*/
		if (used_node->x + used_node->width < free_node.x + free_node.width) {
			struct binpack_rect new_node = free_node;
			new_node.x = used_node->x + used_node->width;
			new_node.width = free_node.x + free_node.width - (used_node->x + used_node->width);
            if (binpack_maxrects_ctx_push_free_rect(ctx, &new_node) == NULL) return 0;
		}
	}

	return 1;
}

void binpack_maxrects_ctx_prune_free_list(binpack_maxrects_ctx_t ctx) {
    uint32_t i, j;

	for(i = 0; i < ctx->m_free_rects_count; ++i) {
		for(j = i + 1; j < ctx->m_free_rects_count; ++j) {
			if (binpack_rect_is_contained_in(ctx->m_free_rects + i, ctx->m_free_rects + j)) {
                binpack_maxrects_ctx_erase_free_rect(ctx, i);
				--i;
				break;
			}

			if (binpack_rect_is_contained_in(ctx->m_free_rects + j, ctx->m_free_rects + i)) {
                binpack_maxrects_ctx_erase_free_rect(ctx, j);
				--j;
			}
		}
    }
}

