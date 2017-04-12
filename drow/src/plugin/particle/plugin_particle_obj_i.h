#ifndef UI_PLUGIN_PARTICLE_OBJ_I_H
#define UI_PLUGIN_PARTICLE_OBJ_I_H
#include "render/model/ui_object_ref.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "plugin_particle_module_i.h"
#include "plugin_particle_data_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj {
    plugin_particle_module_t m_module;
    plugin_particle_data_t m_data;
    plugin_particle_obj_emitter_list_t m_emitters;
    uint32_t m_particle_count;
    uint32_t m_active_emitter_count;
    uint8_t m_enable;
};

int plugin_particle_obj_init(void * ctx, ui_runtime_render_obj_t obj);
int plugin_particle_obj_set(void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url);
void plugin_particle_obj_free(void * ctx, ui_runtime_render_obj_t obj);
int plugin_particle_obj_render(void * ctx, ui_runtime_render_obj_t obj, ui_runtime_render_t context, ui_rect_t rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform);
void plugin_particle_obj_update(void * ctx, ui_runtime_render_obj_t obj, float delta);
uint8_t plugin_particle_obj_is_playing(void * ctx, ui_runtime_render_obj_t obj);

typedef void (*plugin_particle_mod_data_init_fun_t)(
    UI_PARTICLE_MOD const *  mod, plugin_particle_obj_mod_data_t mod_data);
typedef void (*plugin_particle_mod_init_fun_t)(
    UI_PARTICLE_MOD const *  mod, plugin_particle_obj_mod_data_t mod_data,
    plugin_particle_obj_particle_t particle);
typedef void (*plugin_particle_mod_update_fun_t)(
    UI_PARTICLE_MOD const *  mod, plugin_particle_obj_mod_data_t mod_r_data,
    plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta);

struct plugin_particle_mod_def {
    plugin_particle_mod_data_init_fun_t data_init;
    plugin_particle_mod_init_fun_t particle_init;
    plugin_particle_mod_update_fun_t particle_update;
};    
extern struct plugin_particle_mod_def g_mod_defs[UI_PARTICLE_MOD_TYPE_MAX];
    
#ifdef __cplusplus
}
#endif

#endif
