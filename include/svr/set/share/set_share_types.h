#ifndef SVR_SET_SHARE_TYPES_H
#define SVR_SET_SHARE_TYPES_H
#include "cpe/pal/pal_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum set_pkg_category {
    set_pkg_unknown = 0
    , set_pkg_request = 1
    , set_pkg_response = 2
    , set_pkg_notify = 3
} set_pkg_category_t;

typedef enum set_pkg_pack_state {
    set_pkg_not_packed = 0
    , set_pkg_packed = 1
} set_pkg_pack_state_t;

typedef struct set_chanel * set_chanel_t;

typedef enum set_chanel_error {
    set_chanel_evt_not_enouth_data = -1
    , set_chanel_error_bad_data = -2
    , set_chanel_error_chanel_full = -3
    , set_chanel_error_chanel_empty = -4
    , set_chanel_error_no_memory = -5
    , set_chanel_error_decode = -6
} set_chanel_error_t;

#ifdef __cplusplus
}
#endif

#endif
