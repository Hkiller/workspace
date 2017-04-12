#include <assert.h>
#include "gameswf/gameswf_movie_def.h"
#include "gameswf/gameswf_impl.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/random.h"
#include "render/model/ui_data_src.h"
#include "plugin/swf/plugin_swf_data.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_swf_obj_i.hpp"
#include "plugin_swf_data_i.hpp"
#include "plugin_swf_render_i.hpp"

static void plugin_swf_obj_clear(plugin_swf_obj_t obj);

int plugin_swf_obj_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_swf_module_t module = (plugin_swf_module_t)ctx;
    plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);

    bzero(obj, sizeof(*obj));

    obj->m_module = module;
    new (&obj->m_root) gc_ptr<root>();
    obj->m_is_runing = 0;
    
    return 0;
}

int plugin_swf_obj_do_set(void * ctx, ui_runtime_render_obj_t render_obj, UI_OBJECT_URL const * obj_url) {
    plugin_swf_module_t module = (plugin_swf_module_t)ctx;
    UI_OBJECT_URL_DATA_SWF const * skeleton_data = &obj_url->data.swf;
    plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);
    ui_data_src_t swf_data_src;
    plugin_swf_data_t swf_data;
    
    swf_data_src = ui_runtime_module_find_src(module->m_runtime, &skeleton_data->src, ui_data_src_type_swf);
    if (swf_data_src == NULL) return -1;
    
    swf_data = (plugin_swf_data_t)ui_data_src_product(swf_data_src);
    if (swf_data == NULL || swf_data->m_movie_def == NULL) {
        CPE_ERROR(module->m_em, "create plugin_swf_obj: swf data not loaded!");
        return -1;
    }

    obj->m_root = swf_data->m_movie_def->create_instance();
    if (obj->m_root == NULL) {
        CPE_ERROR(module->m_em, "create plugin_swf_obj: create movie fail!");
        return -1;
    }

    // if (skeleton_data->anim_def[0]) {
    //     if (plugin_swf_obj_play_anims(obj, skeleton_data->anim_def, NULL) != 0) {
    //         CPE_ERROR(module->m_em, "create plugin_swf_obj: start anim %s fail!", skeleton_data->anim_def);
    //         plugin_swf_obj_clear(obj);
    //         return -1;
    //     }
    // }
    
    ui_runtime_render_obj_set_src(render_obj, swf_data_src);

    if (obj->m_root->m_movie) {
        obj->m_root->m_movie->goto_frame(0);
        obj->m_root->m_movie->set_play_state(character::PLAY);
        obj->m_is_runing = 1;
    }
    
    return 0;
}

int plugin_swf_obj_do_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    // plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);
    // ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
    // plugin_swf_module_t module = (plugin_swf_module_t)ctx;
    // char * str_value;
    int rv = 0;

    // if ((str_value = cpe_str_read_and_remove_arg(args, "debug-slots", ',', '='))) {
    //     obj->m_debug_slots = atoi(str_value);
    // }
    
    return rv;
}

void plugin_swf_obj_clear(plugin_swf_obj_t obj) {
    obj->m_root = NULL;
    obj->m_is_runing = 0;
}

void plugin_swf_obj_do_free(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);
    //plugin_swf_module_t module = ctx;
    obj->m_root.~gc_ptr<root>();
}

plugin_swf_module_t plugin_swf_obj_module(plugin_swf_obj_t obj) {
    return obj->m_module;
}

void plugin_swf_obj_do_update(void * ctx, ui_runtime_render_obj_t render_obj, float delta) {
    plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);

    if (!obj->m_is_runing) return;

    // iElapsed += dt;

    // bool tobestop = false;
    // int iFrame = imp->m_movie->get_current_frame();
    // if (m_listener && iFrame == iListenFrame) {
    //     (m_listener->*m_pfnSelector)(this);
    // }
    // if(! repeat){
    //     if (iFrameCount > 1 && iFrame == iFrameCount - 1) {
    //         tobestop = true;
    //     }

    //     if (iElapsed >= iDuration) {
    //         tobestop = true;
    //     }

    // }
    obj->m_root->advance(delta);
    
    // if(tobestop){
    //     this->stopAction();
    //     if(m_endListener){
    //         (this->m_endListener->*m_pfnEndSelector)(this);
    //     }
    // }
    
}

uint8_t plugin_swf_obj_do_is_playing(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);
    return obj->m_is_runing;
}

int plugin_swf_obj_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj, ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t input_transform)
{
    plugin_swf_module_t module = (plugin_swf_module_t)ctx;
    plugin_swf_obj_t obj = (plugin_swf_obj_t)ui_runtime_render_obj_data(render_obj);
    plugin_swf_render_handler::context * render_context;

    if (!obj->m_is_runing) return 0;
    
    assert(obj->m_root);
    assert(obj->m_root->m_movie);

    render_context = &module->m_render->m_current_context;
    render_context->m_context = context;
    render_context->m_clip_rect = clip_rect;
    render_context->m_second_color = *second_color;
    render_context->m_transform = input_transform;

    gameswf::render::begin_display(
        obj->m_root->m_background_color,
        obj->m_root->m_viewport_x0, obj->m_root->m_viewport_y0,
        obj->m_root->m_viewport_width, obj->m_root->m_viewport_height,
        obj->m_root->m_def->m_frame_size.m_x_min, obj->m_root->m_def->m_frame_size.m_x_max,
        obj->m_root->m_def->m_frame_size.m_y_min, obj->m_root->m_def->m_frame_size.m_y_max);

    obj->m_root->m_movie->display();

    gameswf::render::end_display();

    bzero(render_context, sizeof(*render_context));
    
    return 0;
}
