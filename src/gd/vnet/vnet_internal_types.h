#ifndef GD_VNET_INTERNAL_TYPES_H
#define GD_VNET_INTERNAL_TYPES_H
#include "cpe/utils/hash_string.h"
#include "gd/vnet/vnet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vnet_control_pkg {
    gd_app_context_t m_app;
    dp_req_t m_dp_req;
    uint32_t m_cmd;
    uint32_t m_connector_id;
    LPDRMETA m_data_meta;
};

#ifdef __cplusplus
}
#endif

#endif
