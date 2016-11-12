#ifndef CPE_UTILS_BINPACK_MAXRECTS_I_H
#define CPE_UTILS_BINPACK_MAXRECTS_I_H
#include "cpe/utils/binpack.h"

#ifdef __cplusplus
extern "C" {
#endif

struct binpack_maxrects_ctx {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    uint8_t m_span;

    int32_t m_bin_width;
	int32_t m_bin_height;

    uint32_t m_used_rects_count;
    uint32_t m_used_rects_capacity;
    struct binpack_rect * m_used_rects;

    uint32_t m_free_rects_count;
    uint32_t m_free_rects_capacity;
    struct binpack_rect * m_free_rects;
};

binpack_rect_t binpack_maxrects_ctx_push_free_rect(binpack_maxrects_ctx_t ctx, binpack_rect_t rect);
void binpack_maxrects_ctx_erase_free_rect(binpack_maxrects_ctx_t ctx, uint32_t idx);
binpack_rect_t binpack_maxrects_ctx_push_used_rect(binpack_maxrects_ctx_t ctx, binpack_rect_t rect);
uint8_t binpack_maxrects_ctx_split_free_node(binpack_maxrects_ctx_t ctx, binpack_rect_t free_node, binpack_rect_t used_node);
void binpack_maxrects_ctx_prune_free_list(binpack_maxrects_ctx_t ctx);

int binpack_maxrects_ctx_find_pos_best_short_side_fit(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_short_side_fit, int32_t * best_long_side_fit);

int binpack_maxrects_ctx_find_pos_best_long_side_fit(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_short_side_fit, int32_t * best_long_side_fit);
    
int binpack_maxrects_ctx_find_pos_bottom_left_rule(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_y, int32_t * best_x);

int binpack_maxrects_ctx_find_pos_contact_point_rule(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_score);

int binpack_maxrects_ctx_find_pos_best_area_side_fit(
    binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height, uint8_t accept_rotate,
	binpack_rect_t result, int32_t * best_area_fit, int32_t * best_shot_side_fit);
    
#ifdef __cplusplus
}
#endif

#endif
