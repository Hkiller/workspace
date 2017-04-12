#ifndef PLUGIN_SWF_RENDER_I_H
#define PLUGIN_SWF_RENDER_I_H
#include <assert.h>
#include "render/utils/ui_matrix_4x4.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "plugin_swf_module_i.hpp"

struct plugin_swf_render_handler : public gameswf::render_handler {
    plugin_swf_module_t m_module;
    plugin_swf_bitmap_list_t m_bitmaps;
	
	float	m_display_width;
	float	m_display_height;

    ui_matrix_4x4 m_current_matrix;
	gameswf::matrix m_current_matrix_swf;
	gameswf::cxform	m_current_cxform;

    uint32_t m_active_rgba;
	int m_mask_level;	// nested mask level

	plugin_swf_render_handler(plugin_swf_module_t module);
	~plugin_swf_render_handler();

	void open() {}
	void set_antialiased(bool enable) {}

	static void make_next_miplevel(int* width, int* height, Uint8* data);

    struct context {
        ui_runtime_render_t m_context;
        ui_rect_t m_clip_rect;
        struct ui_runtime_render_second_color m_second_color;
        ui_transform_t m_transform;
    };
	context	m_current_context;
    
	struct fill_style {
		enum mode {
			INVALID,
			COLOR,
			BITMAP_WRAP,
			BITMAP_CLAMP,
			LINEAR_GRADIENT,
			RADIAL_GRADIENT,
		};
		mode	m_mode;
		gameswf::rgba	m_color;
		plugin_swf_bitmap_t m_bitmap_info;
		gameswf::matrix	m_bitmap_matrix;
		gameswf::cxform	m_bitmap_color_transform;
		bool m_has_nonzero_bitmap_additive_color;
		float m_width;	// for line style
		
		fill_style();
		bool needs_second_pass() const;
		void apply_second_pass() const;
		void cleanup_second_pass() const;
		void disable() { m_mode = INVALID; }
		void set_color(gameswf::rgba color) { m_mode = COLOR; m_color = color; }
		void set_bitmap(gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, const gameswf::cxform& color_transform);
        bool _valid() const { return m_mode != INVALID; }
	};

	// Style state.
	enum style_index {
		LEFT_STYLE = 0,
		RIGHT_STYLE,
		LINE_STYLE,
		STYLE_COUNT
	};
	fill_style	m_current_styles[STYLE_COUNT];

	gameswf::bitmap_info*	create_bitmap_info_rgb(image::rgb* im);
	gameswf::bitmap_info*	create_bitmap_info_rgba(image::rgba* im);
	gameswf::bitmap_info*	create_bitmap_info_empty();
	gameswf::bitmap_info*	create_bitmap_info_alpha(int w, int h, Uint8* data);
	gameswf::video_handler*	create_video_handler();

	void begin_display(
		gameswf::rgba background_color,
		int viewport_x0, int viewport_y0,
		int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1);

	void end_display();
	void set_matrix(const gameswf::matrix& m);
	void set_cxform(const gameswf::cxform& cx) { m_current_cxform = cx; }

	void apply_color(const gameswf::rgba& c);

	void fill_style_disable(int fill_side);
	void line_style_disable();
	void fill_style_color(int fill_side, const gameswf::rgba& color);
	void line_style_color(gameswf::rgba color);
	void fill_style_bitmap(int fill_side, gameswf::bitmap_info* bi, const gameswf::matrix& m, bitmap_wrap_mode wm, bitmap_blend_mode bm);
	void line_style_width(float width);

	void draw_mesh_primitive(int primitive_type, const void* coords, int vertex_count);
	void draw_mesh_strip(const void* coords, int vertex_count);
	void draw_triangle_list(const void* coords, int vertex_count);
	void draw_line_strip(const void* coords, int vertex_count);

	void draw_bitmap(
		const gameswf::matrix& m,
		gameswf::bitmap_info* bi,
		const gameswf::rect& coords,
		const gameswf::rect& uv_coords,
		gameswf::rgba color);

	bool test_stencil_buffer(const gameswf::rect& bound, Uint8 pattern);
	void begin_submit_mask();
	void end_submit_mask();
	void disable_mask();
	bool is_visible(const gameswf::rect& bound);

private:
    //ui_runtime_render_op_t create_render_op(int mode, ui_runtime_render_program_type_t program_type);
};

#endif
