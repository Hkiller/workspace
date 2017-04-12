#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_box_i.h"

int ui_sprite_touch_touchable_load(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_touch_touchable_t touchable = ui_sprite_component_data(component);
    ui_sprite_touch_mgr_t module = ctx;
    struct cfg_it child_cfg_it;
    cfg_t child_cfg;

    ui_sprite_touch_touchable_set_z(touchable, cfg_get_float(cfg, "z-index", ui_sprite_touch_touchable_z(touchable)));

    cfg_it_init(&child_cfg_it, cfg_find_cfg(cfg, "boxes"));
    while((child_cfg = cfg_it_next(&child_cfg_it))) {
        const char * str_shape = cfg_get_string(child_cfg, "shape", NULL);
        ui_sprite_touch_box_t box;
        UI_SPRITE_TOUCH_SHAPE shape;

        if (str_shape == NULL) {
            CPE_ERROR(
                module->m_em, "%s: entity %d(%s): create Touchable: box: shape type not configure",
                ui_sprite_touch_mgr_name(module),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        if (strcmp(str_shape, "box") == 0) {
            shape.type = UI_SPRITE_TOUCH_SHAPE_BOX;
            shape.data.box.lt.x = cfg_get_float(child_cfg, "lt.x", 0.0f);
            shape.data.box.lt.y = cfg_get_float(child_cfg, "lt.y", 0.0f);
            shape.data.box.rb.x = cfg_get_float(child_cfg, "rb.x", 0.0f);
            shape.data.box.rb.y = cfg_get_float(child_cfg, "rb.y", 0.0f);

            if (shape.data.box.rb.x <= shape.data.box.lt.x || shape.data.box.rb.y <= shape.data.box.lt.y) {
                CPE_ERROR(
                    module->m_em, "%s: entity %d(%s): create Touchable: box lt=(%f,%f), rb=(%f,%f) error!",
                    ui_sprite_touch_mgr_name(module),
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    shape.data.box.lt.x, shape.data.box.lt.y, shape.data.box.rb.x, shape.data.box.rb.y);
                return -1;
            }
        }
        else if (strcmp(str_shape, "circle") == 0) {
            const char * base_pos = cfg_get_string(child_cfg, "base-pos", NULL);

            if (base_pos == NULL) {
                CPE_ERROR(
                    module->m_em, "%s: entity %d(%s): create Touchable: circle: base pos not conrigured!",
                    ui_sprite_touch_mgr_name(module),
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }

            shape.type = UI_SPRITE_TOUCH_SHAPE_CIRCLE;

            shape.data.circle.base_pos = ui_sprite_2d_transform_pos_policy_from_str(base_pos);
            if (shape.data.circle.base_pos == 0) {
                CPE_ERROR(
                    module->m_em, "%s: entity %d(%s): create Touchable: circle: base pos %s error!",
                    ui_sprite_touch_mgr_name(module),
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), base_pos);
                return -1;
            }

            shape.data.circle.radius = cfg_get_float(child_cfg, "radius", 0.0f);
            if (shape.data.circle.radius <= 0.0f) {
                CPE_ERROR(
                    module->m_em, "%s: entity %d(%s): create Touchable: circle: radius %f error!",
                    ui_sprite_touch_mgr_name(module),
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), shape.data.circle.radius);
                return -1;
            }
        }
        else if (strcmp(str_shape, "entity-rect") == 0) {
            shape.type = UI_SPRITE_TOUCH_SHAPE_ENTITY_RECT;
            shape.data.entity_rect.adj.x = cfg_get_float(child_cfg, "adj.x", 0.0f);
            shape.data.entity_rect.adj.y = cfg_get_float(child_cfg, "adj.y", 0.0f);
        }
        else {
            CPE_ERROR(
                module->m_em, "%s: entity %d(%s): create Touchable: box: shape type %s unknown",
                ui_sprite_touch_mgr_name(module),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_shape);
            return -1;
        }

        box = ui_sprite_touch_box_create(touchable, &shape);
        if (box == NULL) {
            CPE_ERROR(
                module->m_em, "%s: entity %d(%s): create Touchable: create box fail!",
                ui_sprite_touch_mgr_name(module),
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    return 0;
}


