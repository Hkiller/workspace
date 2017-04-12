#include <assert.h>
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_render/ui_sprite_render_module.h"
#include "ui/sprite_render/ui_sprite_render_utils.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui_sprite_particle_utils_i.h"

plugin_particle_obj_t
ui_sprite_particle_find_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name, error_monitor_t em)
{
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;

    render_obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, name);
    if (render_obj_ref) {
        render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);;
        if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_PARTICLE) {
            CPE_ERROR(
                em, "entity %d(%s): ui_sprite_particle_find_obj: entity render obj %s is not particle obj!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
            return NULL;
        }

        return ui_runtime_render_obj_data(render_obj);
    }

    render_obj = ui_runtime_render_obj_find(ui_sprite_render_module_runtime(module), name);
    if (render_obj == NULL) return NULL;

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_PARTICLE) {
        CPE_ERROR(
            em, "entity %d(%s): ui_sprite_particle_find_obj: render obj %s is not particle obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
        return NULL;
    }

    return ui_runtime_render_obj_data(render_obj);
}


plugin_particle_obj_t
ui_sprite_particle_find_obj_on_entity(
    ui_sprite_entity_t entity, const char * name, error_monitor_t em)
{
    ui_sprite_render_sch_t render_sch;
    ui_sprite_render_anim_t render_anim;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_particle_obj_t particle_obj;

    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) return NULL;

    render_anim = ui_sprite_render_anim_find_by_name(render_sch, name);
    if (render_anim == NULL) return NULL;

    render_obj_ref = ui_sprite_render_anim_obj(render_anim);
    assert(render_obj_ref);
    
    render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
    assert(render_obj);

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_PARTICLE) {
        CPE_ERROR(
            em, "entity %d(%s): ui_sprite_particle_find_obj_on_entity: render obj %s is not particle obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
        return NULL;
    }

    particle_obj = ui_runtime_render_obj_data(render_obj);
    assert(particle_obj);
    return particle_obj;
}
