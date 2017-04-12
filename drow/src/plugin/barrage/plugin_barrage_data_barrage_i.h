#ifndef PLUGIN_BARRAGE_DATA_BARRAGE_I_H
#define PLUGIN_BARRAGE_DATA_BARRAGE_I_H
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/barrage/plugin_barrage_data_barrage.h"
#include "plugin_barrage_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_data_barrage {
    plugin_barrage_module_t m_module;
    ui_data_src_t m_src;
    plugin_barrage_data_emitter_list_t m_emitters;
    BARRAGE_BARRAGE_INFO m_data;
};


const char * plugin_barrage_data_barrage_dump(plugin_barrage_data_barrage_t barrage);

int plugin_barrage_data_barrage_update_using(ui_data_src_t src);
    
#ifdef __cplusplus
}
#endif

#endif
