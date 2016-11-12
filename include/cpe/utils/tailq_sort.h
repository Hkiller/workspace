#ifndef CPE_UTILS_TAILQ_SORT_H
#define CPE_UTILS_TAILQ_SORT_H

#ifdef __cplusplus
extern "C" {
#endif

#define TAILQ_SORT(__head, __type, __headname, __field, __cmp, __cmp_ctx) \
    do {                                                                \
        struct __headname *head = __head;                               \
        struct __type *x, *y;                                           \
        size_t pwr2, merges, xsize, ysize;                              \
                                                                        \
        if (!TAILQ_FIRST(head))                                         \
            break;                                                      \
                                                                        \
        pwr2 = 1;                                                       \
        while (1) {                                                     \
            x = TAILQ_FIRST(head);                                      \
            merges = 0;                                                 \
                                                                        \
            while (x) {                                                 \
                /* [1] */                                               \
                merges++;                                               \
                y = x;                                                  \
                for (xsize = 0; y && xsize < pwr2; ++xsize) {           \
                    y = TAILQ_NEXT(y, __field);                         \
                }                                                       \
                /* [2] */                                               \
                ysize = pwr2;                                           \
                while (xsize > 0 || (ysize > 0 && y)) {                 \
                    /* [3] */                                           \
                    if (ysize == 0 || !y) {                             \
                        /* [4] */                                       \
                        break;                                          \
                    }                                                   \
                    if (xsize == 0) {                                   \
                        /* [5] */                                       \
                        do {                                            \
                            y = TAILQ_NEXT(y, __field);                 \
                            ysize--;                                    \
                        } while (ysize > 0 && y);                       \
                        break;                                          \
                    }                                                   \
                    /* [6] */                                           \
                    if (__cmp(x, y, __cmp_ctx) <= 0) {                  \
                        /* [7] */                                       \
                        xsize--;                                        \
                        x = TAILQ_NEXT(x, __field);                     \
                    }                                                   \
                    else {                                              \
                        /* [8] */                                       \
                        struct __type *tmp = TAILQ_NEXT(y, __field);    \
                        TAILQ_REMOVE(head, y, __field);                 \
                        TAILQ_INSERT_BEFORE(x, y, __field);             \
                        ysize--;                                        \
                        y = tmp;                                        \
                    }                                                   \
                }                                                       \
                x = y;                                                  \
            }                                                           \
            if (merges <= 1)                                            \
                break;                                                  \
            pwr2 *= 2;                                                  \
        }                                                               \
    } while(0)                                                          \

/*
 * Notes:
 *
 * [1] At this point, x points to L[k*2*pwr2] for some k. The
 *     preceding k sublists of length 2*pwr2 are individually
 *     sorted. The sublist L[k], L[k+1], ..., L[k+pwr2-1] is sorted
 *     (up to how many actually exist), as is the sublist L[k+pwr2],
 *     ..., L[k+2*pwr2-1].
 *
 * [2] We don't know that y has pwr2 elements after it, but we must
 *     not step y more than pwr2 along.
 * 
 * [3] The elements from L[k*2*pwr2] up to but excluding x are
 *     sorted. They are all smaller than both x and y. The sublist
 *     starting at x of length xsize is sorted. The sublist starting
 *     at y of length ysize (or till the end of the list) is
 *     sorted.
 *
 * [4] y has reached the end of its sublist, so the rest of the merge
 *     consists only of elements from the x sublist, which is already
 *     sorted. So we shortcircuit.
 * 
 * [5] x has reached the end of its sublist (so now we should actually
 *     have x == y). We need to step y to the end of its sublist, then
 *     break. Note that here we must have ysize > 0 and y != NULL, so
 *     "do {} while" is slightly better than "while {}".
 *
 * [6] Both the x and y sublists are non-empty. We need to figure out
 *     what the next element is.
 *
 * [7] Easy. x precedes y, so is already in the right position.
 *
 * [8] Hard. We need to remove y from the list and insert it before x,
 *     but we also need to make y point to its current successor.
 */

#ifdef __cplusplus
}
#endif

#endif
