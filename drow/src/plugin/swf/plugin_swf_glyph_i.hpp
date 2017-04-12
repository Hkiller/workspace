#ifndef PLUGIN_SWF_GLYPH_I_H
#define PLUGIN_SWF_GLYPH_I_H
#include <assert.h>
#include "plugin_swf_module_i.hpp"

struct plugin_swf_glyph_provider : public glyph_provider {
    plugin_swf_module_t m_module;
};

#endif
