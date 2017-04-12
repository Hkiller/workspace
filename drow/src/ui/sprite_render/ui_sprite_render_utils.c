#include <assert.h>
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_render_utils_i.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_sch_i.h"
#include "ui_sprite_render_anim_i.h"

ui_sprite_render_anim_t ui_sprite_render_anim_find_on_entity_by_id(ui_sprite_entity_t entity, uint32_t anim_id) {
    ui_sprite_render_sch_t render_sch;

    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) return NULL;

    return ui_sprite_render_anim_find_by_id(render_sch, anim_id);
}

ui_sprite_render_anim_t ui_sprite_render_anim_find_on_entity_by_name(ui_sprite_entity_t entity, const char * anim_name) {
    ui_sprite_render_sch_t render_sch;

    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) return NULL;

    return ui_sprite_render_anim_find_by_name(render_sch, anim_name);
}

ui_runtime_render_obj_ref_t
ui_sprite_render_find_entity_render_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name)
{
    ui_sprite_render_env_t env;
    ui_sprite_render_anim_t render_anim = NULL;
    ui_sprite_render_sch_t render_sch;

    env = ui_sprite_render_env_find(ui_sprite_entity_world(entity));
    if (env == NULL) return NULL;
    
    if ((render_sch = ui_sprite_render_sch_find(entity))) {
        render_anim = ui_sprite_render_anim_find_by_name(render_sch, name);
    }

    if (render_anim == NULL) {
        TAILQ_FOREACH(render_anim, &env->m_global_anims, m_next_for_group) {
            if (strcmp(render_anim->m_anim_name, name) == 0) break;
        }
    }
    
    return render_anim ? render_anim->m_render_obj_ref : NULL;
}

ui_runtime_render_obj_t
ui_sprite_render_find_render_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name)
{
    ui_runtime_render_obj_ref_t obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, name);
    if (obj_ref) return ui_runtime_render_obj_ref_obj(obj_ref);
    
    return ui_runtime_render_obj_find(module->m_runtime, name);
}


ui_runtime_render_obj_ref_t
ui_sprite_render_find_entity_render_obj_with_type(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name, uint8_t obj_type)
{
    ui_runtime_render_obj_ref_t obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, name);

    if (ui_runtime_render_obj_type_id(ui_runtime_render_obj_ref_obj(obj_ref)) != obj_type) return NULL;
    
    return obj_ref;
}

