#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "plugin_render_s3d_module_i.hpp"

// Vertex buffers	4096	256 MB
// Index buffers	4096	128 MB
// Programs	4096	16 MB
// Textures	4096	128 MB¹
// Cube textures	4096	256 MB

int plugin_render_s3d_module_init_capacity(plugin_render_s3d_module_t module) {
    module->m_capacity_vertex_size = 65535;
    module->m_capacity_index_size = 524287;
    return 0;
}

void plugin_render_s3d_module_fini_capacity(plugin_render_s3d_module_t module) {
}
