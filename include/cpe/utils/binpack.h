#ifndef CPE_UTILS_BINPACK_H
#define CPE_UTILS_BINPACK_H
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct binpack_rect_size {
	uint32_t width;
	uint32_t height;
    void * ctx;
};
    
struct binpack_rect {
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
    void * ctx;
};

/*max rects algorighm*/

/* Specifies the different heuristic rules that can be used when deciding where to place a new rectangle. */
typedef enum binpack_maxrects_choice_heuristic {
    binpack_maxrects_best_short_side_fit, /* -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best. */
    binpack_maxrects_best_lone_side_fit,  /* -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best. */
    binpack_maxrects_best_lone_area_fit,  /* -BAF: Positions the rectangle into the smallest free rect into which it fits. */
    binpack_maxrects_bottom_left_rule,    /* -BL: Does the Tetris placement. */
    binpack_maxrects_contact_point_rule,  /* -CP: Choosest the placement where the rectangle touches other rects as much as possible. */
} binpack_maxrects_choice_heuristic_t;

binpack_maxrects_ctx_t binpack_maxrects_ctx_create(mem_allocrator_t alloc, error_monitor_t em);
void binpack_maxrects_ctx_free(binpack_maxrects_ctx_t ctx);

uint8_t binpack_maxrects_ctx_span(binpack_maxrects_ctx_t ctx);
void binpack_maxrects_ctx_set_span(binpack_maxrects_ctx_t ctx, uint8_t span);

/* (Re)initializes the packer to an empty bin of width x height units. Call whenever
   you need to restart with a new bin.
*/
int binpack_maxrects_ctx_init(binpack_maxrects_ctx_t ctx, uint32_t width, uint32_t height);

uint32_t binpack_maxrects_ctx_width(binpack_maxrects_ctx_t ctx);
uint32_t binpack_maxrects_ctx_height(binpack_maxrects_ctx_t ctx);
    
/* Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
   @param rects The list of rectangles to insert. This vector will be destroyed in the process.
   @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
   @param method The rectangle placement rule to use when packing.
*/
int binpack_maxrects_ctx_bulk_insert(
    binpack_maxrects_ctx_t ctx,
    struct binpack_rect * dst, uint32_t * dst_size,
    struct binpack_rect_size * rects, uint32_t * rects_size,
    binpack_maxrects_choice_heuristic_t method, uint8_t accept_rotate);

/* Inserts a single rectangle into the bin, possibly rotated. */
binpack_rect_t binpack_maxrects_ctx_insert(
    binpack_maxrects_ctx_t ctx,
    uint32_t width, uint32_t height,
    binpack_maxrects_choice_heuristic_t method, uint8_t accept_rotate);

/* Computes the ratio of used surface area to the total bin area. */
float binpack_maxrects_ctx_occupancy(binpack_maxrects_ctx_t ctx);

/*utils*/
uint8_t binpack_rect_is_contained_in(binpack_rect_t a, binpack_rect_t b);

#ifdef __cplusplus
}
#endif

#endif
