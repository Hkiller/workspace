#ifndef PLUGIN_SWF_OBJ_I_H
#define PLUGIN_SWF_OBJ_I_H
#include "gameswf/gameswf_root.h"
#include "plugin/swf/plugin_swf_obj.h"
#include "plugin_swf_module_i.hpp"

struct plugin_swf_obj {
    plugin_swf_module_t m_module;
    gc_ptr<root> m_root;
    uint8_t m_is_runing;
};

int plugin_swf_obj_do_init(void * ctx, ui_runtime_render_obj_t obj);
int plugin_swf_obj_do_set(void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url);    
int plugin_swf_obj_do_setup(void * ctx, ui_runtime_render_obj_t obj, char * args);    
void plugin_swf_obj_do_free(void * ctx, ui_runtime_render_obj_t obj);

int plugin_swf_obj_do_render(
    void * ctx, ui_runtime_render_obj_t obj,
    ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform);

void plugin_swf_obj_do_update(void * ctx, ui_runtime_render_obj_t obj, float delta);

uint8_t plugin_swf_obj_do_is_playing(void * ctx, ui_runtime_render_obj_t obj);

render_handler * plugin_swf_render_handler_create(void);
    
#endif
