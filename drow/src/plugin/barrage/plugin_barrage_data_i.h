#ifndef PLUGIN_BARRAGE_DATA_I_H
#define PLUGIN_BARRAGE_DATA_I_H
#include "cpe/dr/dr_types.h"
#include "plugin_barrage_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_data {
    uint32_t m_ref_count;
    struct dr_data m_data;
};

plugin_barrage_data_t plugin_barrage_data_create(plugin_barrage_env_t env, dr_data_t src);
void plugin_barrage_data_free(plugin_barrage_env_t env, plugin_barrage_data_t data);

#ifdef __cplusplus
}
#endif

#endif
