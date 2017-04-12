#ifndef PLUGIN_SPINE_ATLAS_I_H
#define PLUGIN_SPINE_ATLAS_I_H
#include "spine/SkeletonData.h"
#include "plugin/spine/plugin_spine_atlas.h"
#include "plugin_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_atlas {
    plugin_spine_module_t m_module;
    ui_data_src_t m_src;
    spAtlas * m_atlas;
};

#ifdef __cplusplus
}
#endif

#endif
