#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/algorithm.h"
#include "binpack_maxrects_i.h"

binpack_maxrects_ctx_t
binpack_maxrects_ctx_create(mem_allocrator_t alloc, error_monitor_t em) {
    binpack_maxrects_ctx_t ctx = mem_calloc(alloc, sizeof(struct binpack_maxrects_ctx));

    if (ctx == NULL) {
        CPE_ERROR(em, "binpack_maxrects_ctx_create: alloc fail!");
        return NULL;
    }

    ctx->m_alloc = alloc;
    ctx->m_em = em;

    return ctx;
}

void binpack_maxrects_ctx_free(binpack_maxrects_ctx_t ctx) {
    if (ctx->m_used_rects) {
        mem_free(ctx->m_alloc, ctx->m_used_rects);
        ctx->m_used_rects = NULL;
    }

    if (ctx->m_free_rects) {
        mem_free(ctx->m_alloc, ctx->m_free_rects);
        ctx->m_free_rects = NULL;
    }

    mem_free(ctx->m_alloc, ctx);
}

uint8_t binpack_maxrects_ctx_span(binpack_maxrects_ctx_t ctx) {
    return ctx->m_span;
}

void binpack_maxrects_ctx_set_span(binpack_maxrects_ctx_t ctx, uint8_t span) {
    ctx->m_span = span;
}

uint32_t binpack_maxrects_ctx_width(binpack_maxrects_ctx_t ctx) {
    return ctx->m_bin_width;
}

uint32_t binpack_maxrects_ctx_height(binpack_maxrects_ctx_t ctx) {
    return ctx->m_bin_height;
}

int binpack_maxrects_ctx_init(binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height) {
    struct binpack_rect n = { 0, 0, width, height, NULL };

	ctx->m_bin_width = width;
	ctx->m_bin_height = height;

    ctx->m_used_rects = 0;
    ctx->m_free_rects = 0;

    if (binpack_maxrects_ctx_push_free_rect(ctx, &n) == NULL) {
        CPE_ERROR(ctx->m_em, "binpack_maxrects_ctx_init: push free rect fail!");
        return -1;
    }

    return 0;
}

static binpack_rect_t binpack_maxrects_ctx_place_rect(binpack_maxrects_ctx_t ctx, binpack_rect_t input_rect) {
    uint32_t num_rects_to_process;
    uint32_t i;

    num_rects_to_process = ctx->m_free_rects_count;
	for(i = 0; i < num_rects_to_process; ++i) {
        if (binpack_maxrects_ctx_split_free_node(ctx, ctx->m_free_rects + i, input_rect)) {
            binpack_maxrects_ctx_erase_free_rect(ctx, i);
			--i;
			--num_rects_to_process;
		}
	}

    binpack_maxrects_ctx_prune_free_list(ctx);

    return binpack_maxrects_ctx_push_used_rect(ctx, input_rect);
}


static int binpack_maxrects_ctx_score_rect(
    binpack_maxrects_ctx_t ctx, binpack_rect_t new_node,
    uint32_t width, uint32_t height,
    binpack_maxrects_choice_heuristic_t method, uint8_t accept_rotate,
    int32_t * score1, int32_t * score2)
{
    int rv = -1;
    
	*score1 = INT_MAX;
	*score2 = INT_MAX;
    
	switch(method) {
    case binpack_maxrects_best_short_side_fit:
        rv = binpack_maxrects_ctx_find_pos_best_short_side_fit(ctx, width, height, accept_rotate, new_node, score1, score2);
        break;
    case binpack_maxrects_best_lone_side_fit:
        rv = binpack_maxrects_ctx_find_pos_best_long_side_fit(ctx, width, height, accept_rotate, new_node, score1, score2);
        break;;
    case binpack_maxrects_best_lone_area_fit:
        rv = binpack_maxrects_ctx_find_pos_best_area_side_fit(ctx, width, height, accept_rotate, new_node, score1, score2);
        break;;
    case binpack_maxrects_bottom_left_rule:
        rv = binpack_maxrects_ctx_find_pos_bottom_left_rule(ctx, width, height, accept_rotate, new_node, score1, score2);
        break;
    case binpack_maxrects_contact_point_rule:
        rv = binpack_maxrects_ctx_find_pos_contact_point_rule(ctx, width, height, accept_rotate, new_node, score1);
        if (rv == 0) {
            *score1 = - *score1;
        }
        break;
    default:
        CPE_ERROR(ctx->m_em, "binpack_maxrects_ctx_insert: unknown mothod %d!", method);
        break;
	}

    if (rv != 0) {
        *score1 = INT_MAX;
        *score2 = INT_MAX;
    }

    return rv;
}

static int binpack_maxrects_rect_size_cmp_with_rotate(void const * i_l, void const * i_r) {
    struct binpack_rect_size const * l = i_l;
    struct binpack_rect_size const * r = i_r;
    uint32_t l_min, l_max, r_min, r_max;

    if (l->width > l->height) {
        l_max = l->width;
        l_min = l->height;
    }
    else {
        l_max = l->height;
        l_min = l->width;
    }

    if (r->width > r->height) {
        r_max = r->width;
        r_min = r->height;
    }
    else {
        r_max = r->height;
        r_min = r->width;
    }

    if (l_max != r_max) {
        return (int)l_max - (int)r_max;
    }
    else {
        return (int)l_min - (int)r_min;
    }
}

static int binpack_maxrects_rect_size_cmp_width_height(void const * i_l, void const * i_r) {
    struct binpack_rect_size const * l = i_l;
    struct binpack_rect_size const * r = i_r;

    if (l->width != r->width) {
        return (int)l->width - (int)r->width;
    }
    else {
        return (int)l->height - (int)r->height;
    }
}

int binpack_maxrects_ctx_bulk_insert(
    binpack_maxrects_ctx_t ctx,
    struct binpack_rect * dst, uint32_t * dst_size,
    struct binpack_rect_size * rects, uint32_t * rects_size,
    binpack_maxrects_choice_heuristic_t method, uint8_t accept_rotate)
{
    uint32_t i;

    if (accept_rotate) {
        qsort(rects, *rects_size, sizeof(rects[0]), binpack_maxrects_rect_size_cmp_with_rotate);
    }
    else {
        qsort(rects, *rects_size, sizeof(rects[0]), binpack_maxrects_rect_size_cmp_width_height);
    }
    
    *dst_size = 0;

    while(*rects_size > 0) {
		int32_t best_score1 = INT_MAX;
		int32_t best_score2 = INT_MAX;
		int32_t best_rect_index = -1;
		struct binpack_rect best_node;
        binpack_rect_t placed_rect;
        struct binpack_rect * to_dst;
        
		for(i = 0; i < *rects_size; ++i) {
			int32_t score1;
			int32_t score2;
			struct binpack_rect new_node;

            if (binpack_maxrects_ctx_score_rect(
                    ctx, &new_node, rects[i].width + ctx->m_span, rects[i].height + ctx->m_span,
                    method, accept_rotate, &score1, &score2) != 0) continue;

			if (score1 < best_score1 || (score1 == best_score1 && score2 < best_score2)) {
				best_score1 = score1;
				best_score2 = score2;
				best_node = new_node;
				best_rect_index = i;
			}
		}

		if (best_rect_index == -1) {
            /* CPE_ERROR( */
            /*     ctx->m_em, "binpack_maxrects_ctx_bulk_insert: no left space to place, occupancy=%f!", */
            /*     binpack_maxrects_ctx_occupancy(ctx)); */
            return -1;
        }

        best_node.ctx =  rects[best_rect_index].ctx;
        placed_rect = binpack_maxrects_ctx_place_rect(ctx, &best_node);
        if (placed_rect == NULL) return -1;

		to_dst = &dst[(*dst_size)++];

        *to_dst = *placed_rect;
        to_dst->width -= ctx->m_span;
        to_dst->height -= ctx->m_span;

        //printf("   placed %d-%d, free-count=%d, left-count=%d\n", placed_rect->width, placed_rect->height, ctx->m_free_rects_count, (*rects_size) - 1);
        memmove(
            rects + best_rect_index, rects + best_rect_index + 1,
            sizeof(rects[0]) * (*rects_size - best_rect_index - 1));
        (*rects_size)--;
	}
    
    return 0;
}

binpack_rect_t binpack_maxrects_ctx_insert(
    binpack_maxrects_ctx_t ctx,
    uint32_t width, uint32_t height, binpack_maxrects_choice_heuristic_t method, uint8_t accept_rotate)
{
    struct binpack_rect new_node;
	int32_t score1; /* Unused in this function. We don't need to know the score after finding the position. */
	int32_t score2;
    binpack_rect_t to_dst;
    uint8_t resize_width = 0;
    uint8_t resize_height = 0;

    if (width + ctx->m_span <= ctx->m_bin_width) {
        resize_width = 1;
        width += ctx->m_span;
    }

    if (height + ctx->m_span <= ctx->m_bin_height) {
        resize_height = 1;
        height += ctx->m_span;
    }
    
    if (binpack_maxrects_ctx_score_rect(ctx, &new_node, width, height, method, accept_rotate, &score1, &score2) != 0) return NULL;

    to_dst = binpack_maxrects_ctx_place_rect(ctx, &new_node);

    if (to_dst) {
        assert(to_dst->width == width);
        assert(to_dst->height == height);
        
        if (resize_width) to_dst->width -= ctx->m_span;
        if (resize_height) to_dst->height -= ctx->m_span;
    }

    return to_dst;
}

float binpack_maxrects_ctx_occupancy(binpack_maxrects_ctx_t ctx) {
    uint32_t used_area = 0;
    uint32_t i;
	for(i = 0; i < ctx->m_used_rects_count; ++i) {
		used_area += ctx->m_used_rects[i].width * ctx->m_used_rects[i].height;
    }

	return (float)used_area / (ctx->m_bin_width * ctx->m_bin_height);
}

int binpack_maxrects_ctx_find_pos_best_short_side_fit(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_short_side_fit, int32_t * best_long_side_fit)
{
    uint32_t i;

	bzero(result, sizeof(*result));

	*best_short_side_fit = INT_MAX;

	for(i = 0; i < ctx->m_free_rects_count; ++i) {
		/* Try to place the rectangle in upright (non-flipped) orientation. */
		if (ctx->m_free_rects[i].width >= width && ctx->m_free_rects[i].height >= height) {
			int32_t leftoverHoriz = abs((int)ctx->m_free_rects[i].width - (int)width);
			int32_t leftoverVert = abs((int)ctx->m_free_rects[i].height - (int)height);
			int32_t shortSideFit = cpe_min(leftoverHoriz, leftoverVert);
			int32_t longSideFit = cpe_max(leftoverHoriz, leftoverVert);

			if (shortSideFit < *best_short_side_fit || (shortSideFit == *best_short_side_fit && longSideFit < *best_long_side_fit))
			{
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = width;
				result->height = height;
				*best_short_side_fit = shortSideFit;
				*best_long_side_fit = longSideFit;
			}
		}

        if (accept_rotate) {
            if (ctx->m_free_rects[i].width >= height && ctx->m_free_rects[i].height >= width) {
                int flippedLeftoverHoriz = abs((int)ctx->m_free_rects[i].width - (int)height);
                int flippedLeftoverVert = abs((int)ctx->m_free_rects[i].height - (int)width);
                int flippedShortSideFit = cpe_min(flippedLeftoverHoriz, flippedLeftoverVert);
                int flippedLongSideFit = cpe_max(flippedLeftoverHoriz, flippedLeftoverVert);

                if (flippedShortSideFit < *best_short_side_fit
                    || (flippedShortSideFit == *best_short_side_fit && flippedLongSideFit < *best_long_side_fit))
                {
                    result->x = ctx->m_free_rects[i].x;
                    result->y = ctx->m_free_rects[i].y;
                    result->width = height;
                    result->height = width;
                    *best_short_side_fit = flippedShortSideFit;
                    *best_long_side_fit = flippedLongSideFit;
                }
            }
        }
	}

	return result->height == 0 ? -1 : 0;
}

int binpack_maxrects_ctx_find_pos_best_long_side_fit(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_short_side_fit, int32_t * best_long_side_fit)
{
    uint32_t i;

	bzero(result, sizeof(*result));

	*best_long_side_fit = INT_MAX;

	for(i = 0; i < ctx->m_free_rects_count; ++i) {
		/* Try to place the rectangle in upright (non-flipped) orientation. */
		if (ctx->m_free_rects[i].width >= width && ctx->m_free_rects[i].height >= height) {
			int32_t leftoverHoriz = abs((int)ctx->m_free_rects[i].width - (int)width);
			int32_t leftoverVert = abs((int)ctx->m_free_rects[i].height - (int)height);
			int32_t shortSideFit = cpe_min(leftoverHoriz, leftoverVert);
			int32_t longSideFit = cpe_max(leftoverHoriz, leftoverVert);

			if (longSideFit < *best_long_side_fit || (longSideFit == *best_long_side_fit && shortSideFit < *best_short_side_fit)) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = width;
				result->height = height;
				*best_short_side_fit = shortSideFit;
				*best_long_side_fit = longSideFit;
			}
		}

		if (ctx->m_free_rects[i].width >= height && ctx->m_free_rects[i].height >= width) {
			int32_t leftoverHoriz = abs((int)ctx->m_free_rects[i].width - (int)height);
			int32_t leftoverVert = abs((int)ctx->m_free_rects[i].height - (int)width);
			int32_t shortSideFit = cpe_min(leftoverHoriz, leftoverVert);
			int32_t longSideFit = cpe_max(leftoverHoriz, leftoverVert);

			if (longSideFit < *best_long_side_fit || (longSideFit == *best_long_side_fit && shortSideFit < *best_short_side_fit)) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = height;
				result->height = width;
				*best_short_side_fit = shortSideFit;
				*best_long_side_fit = longSideFit;
			}
		}
	}

	return result->height == 0 ? -1 : 0;
}

int binpack_maxrects_ctx_find_pos_bottom_left_rule(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_y, int32_t * best_x)
{
    uint32_t i;

	bzero(result, sizeof(*result));

	*best_y = INT_MAX;

	for(i = 0; i < ctx->m_free_rects_count; ++i) {
		/* Try to place the rectangle in upright (non-flipped) orientation. */
		if (ctx->m_free_rects[i].width >= width && ctx->m_free_rects[i].height >= height) {
			int32_t topSideY = ctx->m_free_rects[i].y + height;
			if (topSideY < *best_y || (topSideY == *best_y && ctx->m_free_rects[i].x < *best_x)) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = width;
				result->height = height;
				*best_y = topSideY;
				*best_x = ctx->m_free_rects[i].x;
			}
		}

		if (ctx->m_free_rects[i].width >= height && ctx->m_free_rects[i].height >= width) {
			int32_t topSideY = ctx->m_free_rects[i].y + width;
			if (topSideY < *best_y || (topSideY == *best_y && ctx->m_free_rects[i].x < *best_x)) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = height;
				result->height = width;
				*best_y = topSideY;
				*best_x = ctx->m_free_rects[i].x;
			}
		}
	}

	return result->height == 0 ? -1 : 0;
}

/* Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise. */
static int32_t binpack_maxrects_ctx_common_inter_value_length(int32_t i1start, int32_t i1end, int32_t i2start, int32_t i2end) {
	if (i1end < i2start || i2end < i1start) {
		return 0;
    }

	return cpe_min(i1end, i2end) - cpe_max(i1start, i2start);
}

static int32_t binpack_maxrects_ctx_contact_point_score_node(binpack_maxrects_ctx_t ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    uint32_t i;
	int32_t score = 0;

	if (x == 0 || x + width == ctx->m_bin_width) {
		score += height;
    }
    
	if (y == 0 || y + height == ctx->m_bin_height) {
		score += width;
    }

	for(i = 0; i < ctx->m_used_rects_count; ++i) {
		if (ctx->m_used_rects[i].x == x + width || ctx->m_used_rects[i].x + ctx->m_used_rects[i].width == x) {
			score += binpack_maxrects_ctx_common_inter_value_length(ctx->m_used_rects[i].y, ctx->m_used_rects[i].y + ctx->m_used_rects[i].height, y, y + height);
        }
        
		if (ctx->m_used_rects[i].y == y + height || ctx->m_used_rects[i].y + ctx->m_used_rects[i].height == y) {
			score += binpack_maxrects_ctx_common_inter_value_length(ctx->m_used_rects[i].x, ctx->m_used_rects[i].x + ctx->m_used_rects[i].width, x, x + width);
        }
	}

	return score;
}

int binpack_maxrects_ctx_find_pos_contact_point_rule(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_score)
{
    uint32_t i;

	bzero(result, sizeof(*result));

	*best_score = -1;

	for(i = 0; i < ctx->m_free_rects_count; ++i) {
		/* Try to place the rectangle in upright (non-flipped) orientation. */
		if (ctx->m_free_rects[i].width >= width && ctx->m_free_rects[i].height >= height) {
			int32_t score = binpack_maxrects_ctx_contact_point_score_node(ctx, ctx->m_free_rects[i].x, ctx->m_free_rects[i].y, width, height);
			if (score > *best_score) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = width;
				result->height = height;
				*best_score = score;
			}
		}

		if (ctx->m_free_rects[i].width >= height && ctx->m_free_rects[i].height >= width) {
			int32_t score = binpack_maxrects_ctx_contact_point_score_node(ctx, ctx->m_free_rects[i].x, ctx->m_free_rects[i].y, height, width);
			if (score > *best_score) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = height;
				result->height = width;
				*best_score = score;
			}
		}
	}

	return result->height == 0 ? -1 : 0;
}

int binpack_maxrects_ctx_find_pos_best_area_side_fit(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_area_fit, int32_t * best_short_side_fit)
{
    uint32_t i;

	bzero(result, sizeof(*result));

	*best_area_fit = INT_MAX;

	for(i = 0; i < ctx->m_free_rects_count; ++i) {
		int32_t areaFit = ctx->m_free_rects[i].width * ctx->m_free_rects[i].height - width * height;

		/* Try to place the rectangle in upright (non-flipped) orientation. */
		if (ctx->m_free_rects[i].width >= width && ctx->m_free_rects[i].height >= height) {
			int32_t leftoverHoriz = abs((int)ctx->m_free_rects[i].width - (int)width);
			int32_t leftoverVert = abs((int)ctx->m_free_rects[i].height - (int)height);
			int32_t shortSideFit = cpe_min(leftoverHoriz, leftoverVert);

			if (areaFit < *best_area_fit || (areaFit == *best_area_fit && shortSideFit < *best_short_side_fit)) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = width;
				result->height = height;
				*best_short_side_fit = shortSideFit;
				*best_area_fit = areaFit;
			}
		}

		if (ctx->m_free_rects[i].width >= height && ctx->m_free_rects[i].height >= width) {
			int32_t leftoverHoriz = abs((int)ctx->m_free_rects[i].width - (int)height);
			int32_t leftoverVert = abs((int)ctx->m_free_rects[i].height - (int)width);
			int32_t shortSideFit = cpe_min(leftoverHoriz, leftoverVert);

			if (areaFit < *best_area_fit || (areaFit == *best_area_fit && shortSideFit < *best_short_side_fit)) {
				result->x = ctx->m_free_rects[i].x;
				result->y = ctx->m_free_rects[i].y;
				result->width = height;
				result->height = width;
				*best_short_side_fit = shortSideFit;
				*best_area_fit = areaFit;
			}
		}
	}
    
	return result->height == 0 ? -1 : 0;
}
