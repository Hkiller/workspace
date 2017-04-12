#include "gdpp/app/log.hpp"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "plugin_swf_bitmap_i.hpp"
#include "plugin_swf_render_i.hpp"

plugin_swf_bitmap::plugin_swf_bitmap(plugin_swf_render_handler_t render, ui_cache_res_t texture)
    : m_render(render)
    , m_texture(texture)
{
    TAILQ_INSERT_TAIL(&render->m_bitmaps, this, m_next);
}

plugin_swf_bitmap::~plugin_swf_bitmap() {
    if (m_texture) {
        ui_cache_res_free(m_texture);
        m_texture = NULL;
    }

    TAILQ_REMOVE(&m_render->m_bitmaps, this, m_next);
}

int plugin_swf_bitmap::get_width() const {
    return m_texture ? ui_cache_texture_width(m_texture) :0;
}

int plugin_swf_bitmap::get_height() const {
    return m_texture ? ui_cache_texture_height(m_texture) : 0;
}

int plugin_swf_bitmap::get_bpp() const {
    ui_cache_pixel_buf_t buf = m_texture ? ui_cache_texture_data_buf(m_texture) : NULL;
    return buf ? ui_cache_pixel_buf_stride(buf) : 0;
}

unsigned char* plugin_swf_bitmap::get_data() const {
    ui_cache_pixel_buf_t buf = m_texture ? ui_cache_texture_data_buf(m_texture) : NULL;
    return buf ? (unsigned char* )ui_cache_pixel_buf_pixel_buf(buf) : NULL;
}

void plugin_swf_bitmap::activate() {
    // assert(m_texture_id > 0);
    // glEnable(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, m_texture_id);
}
    
void plugin_swf_bitmap::layout() {
    if (m_texture == NULL) return;

    if (ui_cache_res_load_state(m_texture) != ui_cache_res_loaded) {
        ui_cache_res_load_sync(m_texture, NULL);
    }
    // if (m_texture == NULL) {
	// 	assert(m_suspended_image);

	// 	// Create the texture.
	// 	glEnable(GL_TEXTURE_2D);
	// 	glGenTextures(1, (GLuint*) &m_texture_id);
	// 	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// GL_NEAREST ?
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// 	m_width = m_suspended_image->m_width;
	// 	m_height = m_suspended_image->m_height;

	// 	int bpp = 4;
	// 	int format = GL_RGBA;

	// 	switch (m_suspended_image->m_type)
	// 	{
	// 		case image::image_base::RGB:
	// 		{
	// 			bpp = 3;
	// 			format = GL_RGB;
	// 		}

	// 		case image::image_base::RGBA:
	// 		{
	// 			int	w = p2(m_suspended_image->m_width);
	// 			int	h = p2(m_suspended_image->m_height);
	// 			if (w != m_suspended_image->m_width || h != m_suspended_image->m_height)
	// 			{
	// 				// Faster/simpler software bilinear rescale.
	// 				software_resample(bpp, m_suspended_image->m_width, m_suspended_image->m_height,
	// 					m_suspended_image->m_pitch, m_suspended_image->m_data, w, h);
	// 			}
	// 			else
	// 			{
	// 				// Use original image directly.
	// 				create_texture(format, w, h, m_suspended_image->m_data, 0);
	// 			}
	// 			break;
	// 		}

	// 		case image::image_base::ALPHA:
	// 		{
	// 			int	w = m_suspended_image->m_width;
	// 			int	h = m_suspended_image->m_height;
	// 			create_texture(GL_ALPHA, w, h, m_suspended_image->m_data, 0);
	// 			break;
	// 		}

	// 		default:
	// 			assert(0);
	// 	}

	// 	delete m_suspended_image;
	// 	m_suspended_image = NULL;
	// }
	// else {
	// 	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	// 	glEnable(GL_TEXTURE_2D);
	// }
}
