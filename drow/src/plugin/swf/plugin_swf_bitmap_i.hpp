#ifndef PLUGIN_SWF_BITMAP_I_H
#define PLUGIN_SWF_BITMAP_I_H
#include <assert.h>
#include "plugin_swf_module_i.hpp"

struct plugin_swf_bitmap : public gameswf::bitmap_info {
    plugin_swf_render_handler_t m_render;
    TAILQ_ENTRY(plugin_swf_bitmap) m_next;
    ui_cache_res_t m_texture;
    
	plugin_swf_bitmap(plugin_swf_render_handler_t render, ui_cache_res_t texture);
	~plugin_swf_bitmap();

	virtual void layout();
	virtual int get_bpp() const;
	virtual unsigned char* get_data() const;
	virtual void activate();
		
	virtual int get_width() const;
	virtual int get_height() const;

};

#endif
