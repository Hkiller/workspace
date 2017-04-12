#ifndef DROW_PLUGIN_BARRAGE_DATA_BARRAGE_H
#define DROW_PLUGIN_BARRAGE_DATA_BARRAGE_H
#include "plugin_barrage_types.h"
#include "protocol/plugin/barrage/barrage_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/*barrage ops*/
plugin_barrage_data_barrage_t  plugin_barrage_data_barrage_create(plugin_barrage_module_t module, ui_data_src_t src);
void plugin_barrage_data_barrage_free(plugin_barrage_data_barrage_t barrage);

BARRAGE_BARRAGE_INFO * plugin_barrage_data_barrage_data(plugin_barrage_data_barrage_t barrage);
    
struct plugin_barrage_data_emitter_it {
    plugin_barrage_data_emitter_t (*next)(struct plugin_barrage_data_emitter_it * it);
    char m_data[64];
};

void plugin_barrage_data_barrage_emitters(
    plugin_barrage_data_emitter_it_t trigger_it,
    plugin_barrage_data_barrage_t barrage);

#define plugin_barrage_data_emitter_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

