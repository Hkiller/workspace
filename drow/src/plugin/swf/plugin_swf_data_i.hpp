#ifndef PLUGIN_SWF_DATA_I_H
#define PLUGIN_SWF_DATA_I_H
#include "plugin/swf/plugin_swf_data.h"
#include "plugin_swf_module_i.hpp"

struct plugin_swf_data {
    plugin_swf_module_t m_module;
    ui_data_src_t m_src;
    gameswf::gc_ptr<gameswf::movie_def_impl> m_movie_def;
};

/*int plugin_swf_data_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);*/
int plugin_swf_data_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);

int plugin_swf_data_update_usings(ui_data_src_t src);

#endif
