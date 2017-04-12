#ifndef PLUGIN_BASICANIM_IMGBLOCK_I_H
#define PLUGIN_BASICANIM_IMGBLOCK_I_H
#include "render/model/ui_object_ref.h"
#include "plugin/basicanim/plugin_basicanim_img_block.h"
#include "plugin_basicanim_module_i.h"

struct plugin_basicanim_img_block {
    ui_data_img_block_t m_img_block;
	ui_runtime_render_texture_filter_t m_filter;
};

int plugin_basicanim_img_block_register(plugin_basicanim_module_t module);
void plugin_basicanim_img_block_unregister(plugin_basicanim_module_t module);

#endif
