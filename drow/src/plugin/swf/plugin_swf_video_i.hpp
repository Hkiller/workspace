#ifndef PLUGIN_SWF_VIDEO_I_H
#define PLUGIN_SWF_VIDEO_I_H
#include <assert.h>
#include "plugin_swf_module_i.hpp"

struct plugin_swf_video_handler : public gameswf::video_handler {
    plugin_swf_module_t m_module;
	ui_cache_res_t m_texture;
	float m_scoord;
	float m_tcoord;
	gameswf::rgba m_background_color;

	plugin_swf_video_handler(plugin_swf_module_t module);
	~plugin_swf_video_handler();

	void display(
        Uint8* data, int width, int height, 
        const gameswf::matrix* m, const gameswf::rect* bounds, const gameswf::rgba& color);
};

#endif
