#ifndef CPE_XCALC_TYPES_H
#define CPE_XCALC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xtoken * xtoken_t;
typedef struct xcomputer * xcomputer_t;
typedef struct xcomputer_args * xcomputer_args_t;
typedef struct xtoken_it * xtoken_it_t;

typedef enum xtoken_data_type {
    xtoken_data_none = 0,
    xtoken_data_int = 1,
    xtoken_data_double = 2,
    xtoken_data_str = 3,
} xtoken_data_type_t;

#ifdef __cplusplus
}
#endif

#endif
