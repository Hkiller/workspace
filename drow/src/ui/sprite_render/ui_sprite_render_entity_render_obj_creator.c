#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_render_entity_render_obj_creator_i.h"
#include "ui_sprite_render_utils_i.h"

static int ui_sprite_render_create_entity_render_obj(
    ui_runtime_render_obj_ref_t * r, void * ctx, ui_sprite_world_t world, uint32_t entity_id, const char * res)
{
    ui_sprite_render_module_t module = ctx;
    ui_sprite_entity_t entity;
    ui_runtime_render_obj_ref_t from_obj_ref;
    ui_runtime_render_obj_ref_t obj_ref;
    
    entity = ui_sprite_entity_find_by_id(world, entity_id);
    if (entity == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_sprite_render_obj_creator_entity_render_obj: entity %d not exist!", entity_id);
        return -1;
    }

    from_obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, res);
    if (from_obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_sprite_render_obj_creator_entity_render_obj: anim %s not exit!", res);
        return -1;
    }

    obj_ref = ui_runtime_render_obj_ref_create_by_obj(ui_runtime_render_obj_ref_obj(from_obj_ref));
    if (obj_ref == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_sprite_render_obj_creator_entity_render_obj: create obj ref fail!");
        return -1;
    }
    
    *r = obj_ref;
    
    return 0;
}

int ui_sprite_render_entity_render_obj_creator_regist(ui_sprite_render_module_t module) {
    if (ui_sprite_render_module_register_obj_creator(
            module, "entity", ui_sprite_render_create_entity_render_obj, module)
        != 0)
    {
        CPE_ERROR(module->m_em, "ui_sprite_ui_sprite_render_obj_creator_entity_render_obj_regist: register fail!");
        return -1;
    }
    
    return 0;
}

void ui_sprite_render_entity_render_obj_creator_unregist(ui_sprite_render_module_t module) {
    ui_sprite_render_module_unregister_obj_creator(module, "entity");
}
