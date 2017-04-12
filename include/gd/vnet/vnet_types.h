#ifndef GD_VNET_TYPES_H
#define GD_VNET_TYPES_H
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vnet_control_pkg * vnet_control_pkg_t;

enum vnet_control_cmd {
    vnet_control_evt_connected,
    vnet_control_evt_disconnected,

    vnet_control_op_disconnect,
    vnet_control_op_ignore_input,
};

#ifdef __cplusplus
}
#endif

#endif
